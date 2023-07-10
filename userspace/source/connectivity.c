#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>

#include "../libtomcrypt/hashes/md5.h"
#include "../headers/connectivity.h"


unsigned int magic_number = 0;
struct client_node start_client={
    .ipv4 = {0},
    .mac = {0},
    .hash = {0},
    .state = 0,
    .next = NULL
};

void convert_node_2_infectivity(struct client_node* client,struct client_infectivity* collector){
	//ch2int(client->ipv4,&collector->ipv4);
	copy_uchar_values(client->ipv4,collector->ipv4,IPV4_LENGTH);
	copy_uchar_values(client->mac,collector->mac,MAC_LENGTH);
	collector->infectivity=UNKNOW_INFECTION;
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

bool check_if_client_is_reachable(unsigned char * ip){
	char command[100];
	snprintf(command,100, "ping -c 3 -W 2 %d.%d.%d.%d", (unsigned int)ip[0],(unsigned int)ip[1],(unsigned int)ip[2],(unsigned int)ip[3]);
	printf("command %s\n",command);
	int result = system(command);
	FILE* pingOutput = popen(command, "r");

	int packetsSent = 0, packetsReceived = 0;

	if (pingOutput != NULL) {
		char outputLine[128];

		while (fgets(outputLine, sizeof(outputLine), pingOutput) != NULL) {
			// Check if the line contains the "packets transmitted" information
			if (sscanf(outputLine, "%d packets transmitted, %d packets received", &packetsSent, &packetsReceived) == 2) {
				break;
			}
		}

		pclose(pingOutput);
	}
	printf("%d and %d\n", packetsSent, packetsReceived);

	if (packetsReceived > 0) {
		printf("At least one packet was sent.\n");
		return true;
	} else {
		printf("No packets were sent.\n");
		return false;
	}
	// Check the return value of the `system` function
	// A return value of 0 indicates success, i.e., the client is reachable
	if (result == 0) {
		return true; // Client is reachable
	} else {
		return false; // Client is not reachable
	}
}

bool insert_or_update_client(struct client_node* list, struct client_node* node){
    unsigned char empty_hash[MD5_HASH_SIZE] ={0};
    struct client_node *first = list->next, *prev = list;
    bool updated = false, added = false;
    //printf("%p & %p & %d & %d\n",list,node,cmp_uchar_values(node->hash, empty_hash, MD5_HASH_SIZE),node->state);
    if(!list || !node || cmp_uchar_values(node->hash, empty_hash, MD5_HASH_SIZE) == 0 || node->state != magic_number){
        perror("Insert or update validation failed \n");
        return false;
    }
    if (!check_if_client_is_reachable(node->ipv4))
    {
	printf("nu e gasit la adaugare\n");
	perror("Client is unreachable\n");
        return false;
    }else{
	printf("clientul e reachable\n");
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
	struct client_infectivity cl_infec;
	convert_node_2_infectivity(node,&cl_infec);
	send_to_monitor((unsigned char *)&cl_infec,ADD);
        prev->next = (void*)node;
        added = true;
    }
    return added || updated;
}

void delete_unreachable_clients(struct client_node* list){
	struct client_node *current = list->next, *prev = list;
	bool updated = false;
	if(!list){
		perror("Empty list!\n");
		return;
	}
	while (current != NULL){
		bool delete = false;
		bool is_reachable=check_if_client_is_reachable(current->ipv4);
		if(!is_reachable){
			//printf("Is reachable %d\n",is_reachable);
			prev->next = (void*)current->next;
			delete = true;
		}
		if(delete){
			printf("Removed client ip=%d.%d.%d.%d, mac=%02x:%02x:%02x:%02x:%02x:%02x\n", current->ipv4[0],current->ipv4[1],
			current->ipv4[2],current->ipv4[3], current->mac[0],current->mac[1],current->mac[2],current->mac[3],current->mac[4],current->mac[5]);
			struct client_infectivity cl_infec;
			convert_node_2_infectivity(current,&cl_infec);
			send_to_monitor((unsigned char *)&cl_infec,REMOVE);
			free(current);
			current=prev;
		}
		else prev = current;
		current = (struct client_node *) current->next;
	}
}

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
		struct client_infectivity cl_infec;
		convert_node_2_infectivity(current,&cl_infec);
		send_to_monitor((unsigned char *)&cl_infec,REMOVE);
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
			if (ch == ' ' || ch == '\n' ||  feof(file_descriptor)){
				buf[dim_buf] ='\0';
				dim_buf = 0;
				process_word_on_client(buf,&client);

				if(ch == '\n' ||  feof(file_descriptor)){
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
	//delete_unreachable_clients(&start_client);
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

bool is_file_available(char* filename){
	struct stat filestat;
	int err = stat(filename, &filestat);
    if (err != 0) {
        perror("Cannot access file stats");
        return false;
    }
	return true;
}

void connectivity_checker(char* file){
	printf("Thread awake\n");
	time_t last_modified = 0;
	char extension[5]=".cp";
	char* filename = (char*)malloc(sizeof(char)*strlen(file)+7);
	char* filename_copy = (char*)malloc(sizeof(char)*(strlen(file)+ strlen(extension))+7);
	char *command = (char*)malloc(sizeof(char)*300);
	strncpy(filename,file,strlen(file)+1);
	strncpy(filename_copy,file,strlen(file)+1);
	strncat(filename_copy,extension,strlen(extension));
	snprintf(command,300,"cp %s %s",filename,filename_copy);
	
	if(is_file_available(filename)){
		while(1){
			// time_t how_long_ago = check_if_file_was_modified(filename,last_modified);
			// if( how_long_ago > 0){
			// printf("File has been modified\n");
			// last_modified += how_long_ago;
			printf("Running processing...\n");
			system(command);
			process_dhcp_file(filename_copy);
			// }
			// else{
			// 	printf("Nothing new\n");
			// }
			sleep(SLEEP_TIME);
		}
	}
	free(filename);
	printf("Thread dead\n");
}

