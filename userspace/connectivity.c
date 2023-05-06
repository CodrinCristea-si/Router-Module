#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <regex.h>
#include <pthread.h>
#include <linux/netlink.h>
#include <sys/socket.h>
#include <time.h>


#include "./libtomcrypt/hashes/md5.h"

//#include <openssl/sha1.h>

#define WORD_SEPARATOR " "
#define MAX_WORD_SIZE 100
#define MAX_ID 1e7

#define REGEX_LEASE "^([0-9])*$"
#define REGEX_MAC "^([0-9A-Fa-f]{2}:){5}([0-9A-Fa-f]{2})$"
#define REGEX_IPV4 "^([0-9]{1,3}\\.){3}([0-9]{1,3})$"
#define REGEX_NAME "^([0-9a-zA-Z_\\-]+)$|^(\\*)$"

#define MASK_NAME_ASSIGNED 001u
#define MASK_IPV4_ASSIGNED 010u
#define MASK_MAC_ASSIGNED 100u
#define MASK_CLIENT_COMPLETE 111u
#define MASK_CLIENT_EMPTY 000u

#define MD5_HASH_SIZE 16
#define MAC_LENGTH 6
#define IPV4_LENGTH 4

#define MAX_PAYLOAD_SIZE 1024

enum {
	FORMAT_LEASE=1,
	FORMAT_MAC,
	FORMAT_IPV4,
	FORMAT_NAME,
	FORMAT_UNKNOWN
};

struct client_def{
	char ipv4[MAX_WORD_SIZE];
	char mac[MAX_WORD_SIZE];
	char name[MAX_WORD_SIZE];
	u_int32_t complete_mask;
};

struct client_node{
    unsigned char ipv4[IPV4_LENGTH];
    unsigned char mac[MAC_LENGTH];
    unsigned char hash[MD5_HASH_SIZE];
    unsigned int state;
    void* next;
};

unsigned int magic_number = 0;
struct client_node start_client={
    .ipv4 = {0},
    .mac = {0},
    .hash = {0},
    .state = 0,
    .next = NULL
};

pthread_mutex_t lock;
u_int8_t shutdown_order = 0;

void copy_uchar_values(unsigned char* from, unsigned char* to, size_t size){
    for(size_t i=0;i< size;i++){
        to[i]=from[i];
    }
}

int cmp_uchar_values(unsigned char* hash1, unsigned char* hash2, size_t size){
    for(size_t i=0;i< size;i++){
        int dif = hash1[i] - hash2[i];
        if(dif != 0) return dif;
    }
    return 0;
}


int hash_client(struct client_def *client, char* hash_collector){
	int err;
    unsigned long hash_len = sizeof(MD5_HASH_SIZE);
    hash_state md;

    // Initialize the hash state
    err = md5_init(&md);
    if (err != CRYPT_OK) {
        perror("Failed initializing MD5 hash\n");
        return -3;
    }

    // Hash the content
    err = md5_process(&md, (const unsigned char *)client, sizeof(struct client_def));
    if (err != CRYPT_OK) {
        perror("Error processing MD5 hash");
        return -2;
    }

    // Finalize the hash and store the result in hash_collector[]
    err = md5_done(&md, hash_collector);
    if (err != CRYPT_OK) {
        perror("Error finalizing MD5 hash");
        return -1;
    }

    return 0;

}

int compare_regex(char *str, char *regex_str){
	regex_t regex_expr;
	int status;

	status = regcomp(&regex_expr, regex_str,REG_EXTENDED);
	if(status) {
		//printf("-1\n");
		regfree(&regex_expr);
		return -1;
	}
	status = regexec(&regex_expr,str,0,NULL,0);
	if(status == 0) {
		//printf("0\n");
		regfree(&regex_expr);
		return 0;
	}
	//printf("1\n");
	regfree(&regex_expr);
	return 1;
}

int get_type_of_dhcp_word(char* word){
	if(compare_regex(word,REGEX_LEASE) == 0)
		return FORMAT_LEASE;	
	if(compare_regex(word,REGEX_IPV4) == 0)
		return FORMAT_IPV4;
	if(compare_regex(word,REGEX_MAC) == 0)
		return FORMAT_MAC;
	if(compare_regex(word,REGEX_NAME) == 0)
		return FORMAT_NAME;
	return FORMAT_UNKNOWN;
}

void process_word_on_client(char*word, struct client_def *client){
	int type = get_type_of_dhcp_word(word);
	//printf("processing word %s of type %d\n", word,type);
	switch (type)
	{
		case FORMAT_MAC:
			strncpy(client->mac,word, MAX_WORD_SIZE);
			client->complete_mask |= MASK_MAC_ASSIGNED; 
			break;
		case FORMAT_IPV4:
			strncpy(client->ipv4,word, MAX_WORD_SIZE);
			client->complete_mask |= MASK_IPV4_ASSIGNED; 
			break;
		case FORMAT_NAME:
			strncpy(client->name,word, MAX_WORD_SIZE);
			client->complete_mask |= MASK_NAME_ASSIGNED; 
			break;
		default:
			break;
	}
}

void clear_client(struct client_def *client){
	if(client){
		strncmp(client->ipv4,"\0",1);
		strncmp(client->mac,"\0",1);
		strncmp(client->name,"\0",1);
		client->complete_mask=MASK_CLIENT_EMPTY;
	}
}

void str2ipv4(unsigned char * str, unsigned char* collector){
    char buf[4];
    char nr;
    int len =0, poz =0;
    for(size_t i =0;i<strlen(str);i++){
        if(str[i] == '.'){
            buf[len++] = '\0';
            nr = (char)atoi(buf);
            collector[poz++]=nr;
            len =0;
        }
        else buf[len++] = str[i];
    }
    buf[len++] = '\0';
    nr = (char)atoi(buf);
    collector[poz++]=nr;
    len =0;
}

char hex_value_str(char ch){
    if(isdigit(ch)) return ch -'0';
    else{
        switch (ch)
        {
        case 'a':
        case 'A':
            return 10;
        case 'b':
        case 'B':
            return 11;
        case 'c':
        case 'C':
            return 12;
        case 'd':
        case 'D':
            return 13;
        case 'e':
        case 'E':
            return 14;
        case 'f':
        case 'F':
            return 15;
        default:
            return 0;
        }
    }
}

void str2mac(unsigned char * str, unsigned char* collector){
    char buf[4];
    char nr;
    int len =0, poz =0;
    for(size_t i =0;i<strlen(str);i++){
        if(str[i] == ':'){
            buf[len++] = '\0';
            nr = hex_value_str(buf[0]) * 16 + hex_value_str(buf[1]);
            collector[poz]=nr;
            poz++;
            len=0;
        }
        else buf[len++] = str[i];
    }
    buf[len++] = '\0';
    nr = hex_value_str(buf[0]) * 16 + hex_value_str(buf[1]);
    collector[poz]=nr;
    poz++;
}
//to add new things to kernel
bool insert_or_update_client(struct client_node* list, struct client_node* node){
    unsigned char empty_hash[MD5_HASH_SIZE] ={0};
    struct client_node *first = list->next, *prev = list;
    bool updated = false, added = false;
    //printf("%p & %p & %d & %d\n",list,node,cmp_uchar_values(node->hash, empty_hash, MD5_HASH_SIZE),node->state);
    if(!list || !node || cmp_uchar_values(node->hash, empty_hash, MD5_HASH_SIZE) == 0 || node->state != magic_number){
        perror("Insert or update validation failed \n");
        return -1;
    }
    while (first != NULL){
        if(cmp_uchar_values(first->hash, node->hash, MD5_HASH_SIZE) == 0){
            first->state = node->state;
            updated = true;
            break;
        }
        prev = first;
        first = (struct client_node *) first->next;
    }
    if(!updated && prev){
        printf("Added client ip=%d.%d.%d.%d, mac=%02x:%02x:%02x:%02x:%02x:%02x\n", node->ipv4[0],node->ipv4[1],
        node->ipv4[2],node->ipv4[3], node->mac[0],node->mac[1],node->mac[2],node->mac[3],node->mac[4],node->mac[5]);
		
        prev->next = (void*)node;
        added = true;
    }
    return added || updated;
}
//to add new things to kernel
void delete_old_magic(struct client_node* list){
    struct client_node *current = list->next, *prev = list;
    bool updated = false;
    if(!list){
        perror("Empty list!\n");
        return;
    }
    while (current != NULL){
        bool delete = false;
        if(current->state != magic_number){
            prev->next = (void*)current->next;
            delete = true;
        }
        if(delete){
            printf("Removed client ip=%d.%d.%d.%d, mac=%02x:%02x:%02x:%02x:%02x:%02x\n", current->ipv4[0],current->ipv4[1],
            current->ipv4[2],current->ipv4[3], current->mac[0],current->mac[1],current->mac[2],current->mac[3],current->mac[4],current->mac[5]);
            
			free(current);
            current=prev;
        }
        else prev = current;
        current = (struct client_node *) current->next;
    }
}

bool process_client(struct client_def *client){
    bool ok = false;
	if(client->complete_mask & MASK_CLIENT_COMPLETE == MASK_CLIENT_COMPLETE){
		//printf("Client complete ip=%s, mac=%s, name=%s\n", client->ipv4, client->mac, client->name);
        char hash_c[MD5_HASH_SIZE];
        unsigned char ipv4[IPV4_LENGTH], mac[MAC_LENGTH];
        struct client_node *new_node;
        int res = hash_client(client,hash_c);
        if(!res){
            new_node = (struct client_node *)calloc(1,sizeof(struct client_node));
            if(new_node){
                copy_uchar_values(hash_c,new_node->hash, MD5_HASH_SIZE);
                new_node->state = magic_number;
                str2ipv4(client->ipv4,ipv4);
                copy_uchar_values(ipv4,new_node->ipv4, IPV4_LENGTH);
                str2mac(client->mac,mac);
            //     printf("GET client ip=%02x.%02x.%02x.%02x, mac=%02x:%02x:%02x:%02x:%02x:%02x\n", ipv4[0],ipv4[1],
            // ipv4[2],ipv4[3], mac[0],mac[1],mac[2],current->mac[3],current->mac[4],current->mac[5]);

                copy_uchar_values(mac, new_node->mac, MAC_LENGTH);
                new_node->next = NULL;
                if (insert_or_update_client(&start_client, new_node)){
                    ok = true;
                    //printf("Added client ip=%s, mac=%s\n", client->ipv4, client->mac);
                }
                else perror("Failed to add or update client list");
            }
        }
	}
	else {
		// printf("Client incomplete ip=%s, mac=%s, name=%s\n",
		// 	client->complete_mask & MASK_IPV4_ASSIGNED == 0 ? client->ipv4 : "NULL",
		// 	client->complete_mask & MASK_MAC_ASSIGNED == 0 ? client->mac : "NULL",
		// 	client->complete_mask & MASK_NAME_ASSIGNED == 0 ? client->name : "NULL");
	}
	clear_client(client);
    return ok;
}



void process_dhcp_file(char* filename){
	FILE *file_descriptor;
	bool modification= false;
	char ch;

	int line_number = 0;
	int word_number = 0;

	char error[MAX_WORD_SIZE];
	
	magic_number++;
	file_descriptor = fopen(filename,"r");
	if(file_descriptor){
		//status = fscanf(file_descriptor,"%s",buf);
		char buf[MAX_WORD_SIZE];
		size_t dim_buf=0;
		word_number = 0;
		struct client_def client;
		while ((ch = fgetc(file_descriptor)) ){
			//printf("%x\n",ch);
			if (ch == ' ' || ch == '\n' ||  ch == EOF || ch == 0xff){
				buf[dim_buf] ='\0';
				dim_buf = 0;
				process_word_on_client(buf,&client);

				if(ch == '\n' ||  ch == EOF || ch == 0xff){
					line_number++;
					word_number=0;
					modification |= process_client(&client);
					if(ch == EOF || ch == 0xff) break;
				}
				else if(ch == ' ') word_number++;
				continue;
			}
			if(dim_buf >= MAX_WORD_SIZE - 2){
				snprintf(error, MAX_WORD_SIZE,"Word too large (line %d, word %d)!", line_number, word_number+1);
				perror(error);
			}
			buf[dim_buf++]=ch;
		}
	}
	else{
		perror("Unable to open dhcp file\n");
		exit(1);
	}
    if(modification)
        delete_old_magic(&start_client);
}

time_t check_if_file_was_modified(char* filename, time_t last_modified){
	struct stat filestat;
    int err = stat(filename, &filestat);
    if (err != 0) {
        perror("Cannot access file stats");
        return 0;
    }
	//printf("%ld and %ld\n", filestat.st_mtime, last_modified);
    return filestat.st_mtime - last_modified;
}

bool is_order_to_shut_down(){
	bool to_shutdown=false;
	pthread_mutex_lock(&lock);
	if(shutdown_order)
		to_shutdown=true;
	pthread_mutex_unlock(&lock);
	return to_shutdown;
}

void *thread_checker(void *argv){
	printf("Thread awake\n");
	time_t last_modified = 0;
	char* filename = (char*)(argv);
	while(1){
		time_t how_long_ago = check_if_file_was_modified(filename,last_modified);
		if( how_long_ago > 0){
			printf("File has been modified\n");
			last_modified += how_long_ago;
			printf("Running processing...\n");
			process_dhcp_file(filename);
		}
		else{
			printf("Nothing new\n");
			if(is_order_to_shut_down())
				break;
		}
		sleep(5);
	}
	printf("Thread dead\n");
}

int shutdown_thread(pthread_t thr_id){
	int ret;
	printf("Atempting to shutdown...\n");
	pthread_mutex_lock(&lock);
	shutdown_order = 1;
	pthread_mutex_unlock(&lock);
	printf("Shutdown order given\n");
	if(thr_id){
		ret = pthread_join(thr_id, NULL);
		printf("attempting to close thread\n");
		if (ret)
		{
			printf("Failed to stop thread!\n" );
			return -1;
		}   
	}
	printf("Thread shutdowned\n");
	return 0;
}

int deploy_thread(pthread_t *thr_id, char* filename){
	printf("Atempting to deploy...\n");
	pthread_mutex_lock(&lock);
	shutdown_order = 0;
	pthread_mutex_unlock(&lock);
	printf("Deploy order given\n");
	pthread_create(thr_id, NULL, thread_checker, (void*)filename);
	printf("Thread lauched\n");
	return thr_id == NULL;
}

int main(int argc, char** argv){
	srand(time(NULL));
	//"dhcp-probs.txt"
	char filename[20] = "dhcp-probs.txt";
	strncpy(filename,argv[1],20);

	pthread_t thr_id;
	
	//process_dhcp_file();
	deploy_thread(&thr_id, filename);
	sleep(60);
	shutdown_thread(thr_id);

	return 0;
}