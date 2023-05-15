#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>

#include "../headers/infectivity_monitor.h"

//rights 777
char storage_file[MAX_FILE_PATH_SIZE] =".infect_file";
char aux_file[20] =".aux";

int size_pool;
List* task_pool;

pthread_mutex_t mutex;
pthread_cond_t cond;

pthread_mutex_t mutex_storage;

List *updates;

void print_client_infectivity(struct client_infectivity* client){
	printf("Client with ip %d.%d.%d.%d  mac %x:%x:%x:%x:%x:%x infectivity %d\n",client->ipv4[0],
			client->ipv4[1],client->ipv4[2],client->ipv4[3],client->mac[0],
			client->mac[1],client->mac[2],client->mac[3],client->mac[4],client->mac[5],
			client->infectivity);
}

void print_job(struct job *job){
	printf("Action type %d\n", job->job_type);
	print_client_infectivity(&job->client);
}

void print_task(struct task *task){
	printf("Task len %d and sender %d\n",task->len, task->sender);
	print_job(&task->job);
}

void write_client(FILE* file, struct client_infectivity* client){
	fprintf(file, "%d.%d.%d.%d~%x:%x:%x:%x:%x:%x~%d\n",client->ipv4[0],
	client->ipv4[1],client->ipv4[2],client->ipv4[3],client->mac[0],
	client->mac[1],client->mac[2],client->mac[3],client->mac[4],client->mac[5],
	client->infectivity);
}

int read_line (FILE* file,char*buf){
	int loc = 0;
	char ch ;//= fgetc(file);
	int max_size =100;
	while((ch = fgetc(file)) && !feof(file) && ch!='\n'  && loc <= max_size){
		buf[loc++]=ch;
		//printf("%x\n",ch);
	}
	return loc;
}

struct client_infectivity* read_client(FILE* file){
	//printf("read_client\n");
	int nr_bytes = 0;
	int max_line = 100;
	size_t nr_words;
	struct client_infectivity *client = NULL;
	client = (struct client_infectivity *)malloc(sizeof(struct client_infectivity));
	char *buf;
	buf = (char*)malloc(100*sizeof(char));
	if(client){
		//printf("allloc\n");
		//"%x.%x.%x.%x~%x:%x:%x:%x:%x:%x~%d
		nr_bytes = read_line(file,buf);
		//printf("get line nr bytes %d\n", nr_bytes);
		if(nr_bytes<=0){
			if(client) free(client);
			if(buf) free(buf);
			//printf("fail\n");
			return NULL;
		}
		else{
			char word[5];
			int len =0;
			size_t i;
			int phase = 0;
			int poz_ip = 0, poz_mac = 0;
			int nr_buf = 0;
			//char ch_buf;
			for(i =0;i<nr_bytes;i++){
				if(phase == 0){
					if(buf[i] == '.' || buf[i] == '~'){
						client->ipv4[poz_ip] =nr_buf;
						//printf("ip nr buf %d\n",nr_buf);
						nr_buf=0;
						poz_ip++;
						if(buf[i] == '~'){
							phase++;
						}
					}
					else if(buf[i] != '~'){
						nr_buf = nr_buf*10+(buf[i]-'0');
					}
				}
				else if(phase == 1){
					if(buf[i] == ':' || buf[i] == '~'){
						client->mac[poz_mac] =nr_buf;
						//printf("mac nr buf %d\n",nr_buf);
						nr_buf=0;
						poz_mac++;
						if(buf[i] == '~'){
							phase++;
						}
					}
					else if(buf[i] != '~'){
						int to_add = 0;
						if(buf[i] >= 'a' && buf[i] <='f')
							to_add = 10 + (buf[i] - 'a');
						else if(buf[i] >= 'A' && buf[i] <='F')
							to_add = 10 + (buf[i] - 'A');
						else if(buf[i] >= '0' && buf[i] <='9')
							to_add = (buf[i] - '0');
						//printf("to add %d at %c\n",to_add, buf[i]);
						nr_buf = nr_buf*16+to_add;
					}
				}
				else if(phase == 2){
					if(i == nr_bytes){
						client->infectivity = nr_buf;
						//printf("inf nr buf %x\n",nr_buf);
						nr_buf = 0;
					}
					else{
						nr_buf = nr_buf*10+(buf[i]-'0');
					}
				}
			}
			if(i == nr_bytes){
				client->infectivity = nr_buf;
				//printf("inf nr buf %x\n",nr_buf);
				nr_buf = 0;
			}
		}
	}
	if(buf) free(buf);
	//printf("done reading\n");
	return client;
}

int cmp_clients(struct client_infectivity* client1, struct client_infectivity* client2){
	return strncmp((char*)client1->ipv4, (char*)client2->ipv4, IPV4_SIZE) == 0 ? 
	cmp_uchar_values((char*)client1->mac, (char*)client2->mac, MAC_SIZE) : 
	cmp_uchar_values((char*)client1->ipv4, (char*)client2->ipv4, IPV4_SIZE);
}

void convert_infectivity_2_repr(struct client_infectivity* client,struct client_repr* collector){
	ch2int(client->ipv4,&collector->ip_addr);
	copy_uchar_values(client->mac,collector->mac_addr,MAC_LEN);
	collector->infectivity=client->infectivity;
}
//not thread safe!
bool check_for_same_user_in_file(char* filename, struct client_infectivity* to_find)
{
	FILE *file;
	struct client_infectivity *client = NULL;
	bool in_file = false;
	file = fopen(filename,"r");
	if(file){
		while(1){
			client = read_client(file);
			if(client == NULL){
				break;
			}
			else{
				if(cmp_clients(to_find,client)==0){
					in_file = true;
					free(client);
					break;
				}
				free(client);
			}
		}
		fclose(file);
	}
	return in_file;
}

void process_add_job(struct job *job){
	FILE *file;
	//printf("add\n");
	bool added = false;
	pthread_mutex_lock(&mutex_storage);
	if(!check_for_same_user_in_file(storage_file, &job->client)){
		file = fopen(storage_file,"a");
		if(file){
			//printf("open\n");
			job->client.infectivity=SUSPICIOUS;
			write_client(file,&job->client);
			//printf("wrote\n");
			fclose(file);
			added = true;
			//printf("closed\n");
			//updates
			struct job *job_copy = (struct job *)malloc(sizeof(struct job));
			//printf("malloc\n");
			copy_uchar_values((unsigned char*)job,(unsigned char*)job_copy,sizeof(struct job));
			//printf("copy\n"); 
			push_to_list(updates,(void*)job_copy);
			//printf("push\n");
			//kernel
			struct client_repr cl_rpr;
			convert_infectivity_2_repr(&job->client,&cl_rpr);
			send_message_to_kernel((unsigned char*)&cl_rpr,ADD_CLIENT);
		}
		
	}
	pthread_mutex_unlock(&mutex_storage);
}

void transfer_between_files(char* filename_from, char* filename_to){
	FILE *file1, *file2;
	struct client_infectivity *client;
	file1 = fopen(filename_from,"r");
	file2 = fopen(filename_to,"w");
	if(file1 && file2){
		while(1){
			client = read_client(file1);
			if(client == NULL){
				break;
			}
			write_client(file2,client);
		}
		fclose(file1);
		fclose(file2);
	}
}

void process_remove_job(struct job *job){
	FILE *file, *file_aux;
	struct client_infectivity *client;
	bool deleted = false;
	pthread_mutex_lock(&mutex_storage);
	file = fopen(storage_file,"r");
	file_aux = fopen(aux_file,"w");
	if(file && file_aux){
		while(1){
			client = read_client(file);
			if(client == NULL){
				break;
			}
			else{
				if(cmp_clients(&job->client,client)!=0){
					write_client(file_aux,client);
				}
				else{
					deleted = true;
				}
				free(client);
			}
		}
		fclose(file);
		fclose(file_aux);
		transfer_between_files(aux_file,storage_file);
		if(deleted){
			//updates
			struct job *job_copy = (struct job *)malloc(sizeof(struct job));
			copy_uchar_values((unsigned char*)job,(unsigned char*)job_copy,sizeof(struct job)); 
			push_to_list(updates,(void*)job_copy);
			//kernel
			struct client_repr cl_rpr;
			convert_infectivity_2_repr(&job->client,&cl_rpr);
			send_message_to_kernel((unsigned char*)&cl_rpr,REMOVE_CLIENT);
		}
	}
	pthread_mutex_unlock(&mutex_storage);
}

void process_transfer_job(struct job *job){
	FILE *file, *file_aux;
	struct client_infectivity *client;
	bool transfered = false;
	pthread_mutex_lock(&mutex_storage);
	file = fopen(storage_file,"r");
	file_aux = fopen(aux_file,"w");
	if(file && file_aux){
		while(1){
			client = read_client(file);
			if(client == NULL){
				break;
			}
			else{
				if(cmp_clients(&job->client,client) ==0){
					if(client->infectivity != job->client.infectivity){
						client->infectivity=job->client.infectivity;
						transfered = true;
					}
				}
				write_client(file_aux,client);
				free(client);
			}
		}
		fclose(file);
		fclose(file_aux);
		transfer_between_files(aux_file,storage_file);
		if(transfered){
			//updates
			struct job *job_copy = (struct job *)malloc(sizeof(struct job));
			copy_uchar_values((unsigned char*)job,(unsigned char*)job_copy,sizeof(struct job));
			push_to_list(updates,(void*)job_copy);
			//kernel
			struct client_repr cl_rpr;
			convert_infectivity_2_repr(&job->client,&cl_rpr);
			send_message_to_kernel((unsigned char*)&cl_rpr,TRANSFER_CLIENT);
		}
	}
	pthread_mutex_unlock(&mutex_storage);
}

void send_to_sender(int sockfd, unsigned char type, unsigned char* data){
	int len;
	size_t i,length;
	List *list;
	int list_size;
	//printf("send\n");
	switch(type){
		case ALL_DATA:
			list = (List*)data;
			list_size = list != NULL ? list->size : 0;
			//printf("to send %d clients\n",list_size);
			len = send_data(sockfd,&type,sizeof(unsigned char));
			if (len >0){
				len = send_data(sockfd,(unsigned char *)&list_size,sizeof(int));
				if(len > 0 && list_size > 0){
					length = list->size;
					for(i=0;i< length;i++){
						void* data = pop_from_list(list);
						send_data(sockfd,data,sizeof(struct client_infectivity));
					}
					clear_list(list);
				}
			}
			free(data);
			break;
		case UPDATES:
			list = (List*)data;
			list_size = list != NULL ? list->size : 0;
			//printf("to send %d updates\n",list_size);
			len = send_data(sockfd,(unsigned char *)&type,sizeof(unsigned char));
			if (len >0 ){
				len = send_data(sockfd,(unsigned char *)&list_size,sizeof(int));
				if(len > 0 && list_size > 0){
					length = list->size;
					for(i=0;i< length;i++){
						void* data = pop_from_list(list);
						send_data(sockfd,data,sizeof(struct job));
					}
					clear_list(list);
				}
			}
			free(data);
			break;
		default:
			break;
	}
}

List* process_get_updates_job(){
	//printf("process_get_updates_job\n");
	List* list = NULL;
	size_t i, length;
	pthread_mutex_lock(&mutex_storage);
	if(!is_empty(updates)){
		list = create_list();
		length = updates->size;
		for(i =0;i<length;i++){
			void *update_data = pop_from_list(updates);
			if(update_data)
				push_to_list(list,update_data);
		}
	}
	// else{
	// 	printf("is empty\n");
	// }
	pthread_mutex_unlock(&mutex_storage);
	return list;
}

List* process_get_all_job(){
	//printf("gel all\n");
	FILE *file;
	//int i=0;
	struct client_infectivity *client;
	List* list = NULL;
	pthread_mutex_lock(&mutex_storage);
	file = fopen(storage_file,"r");
	//printf("file opened\n");
	if(file){
		list = create_list();
		//printf("list created\n");
		while(1){
			client = read_client(file);
			if(client == NULL){
				break;
			}
			else{
				print_client_infectivity(client);
				push_to_list(list,(void*)client);
				//printf("pushed\n");
			}
			//i++;
		}
	}
	fclose(file);
	//printf("closed\n");
	pthread_mutex_unlock(&mutex_storage);
	return list;
}

void process_task(struct task to_execute){
	//printf("enter process\n");
	List* list;
	switch (to_execute.job.job_type)
	{
	case ADD:
		process_add_job(&to_execute.job);
		break;
	case REMOVE:
		process_remove_job(&to_execute.job);
		break;
	case TRANSFER:
		process_transfer_job(&to_execute.job);
		break;
	case GET_ALL:
		list = process_get_all_job();
		send_to_sender(to_execute.sender,ALL_DATA,(unsigned char*)list);
		break;
	case GET_UPDATES:
		list = process_get_updates_job();
		send_to_sender(to_execute.sender,UPDATES,(unsigned char*)list);
		break;
	default:
		break;
	}
	//printf("exit process\n");
}

bool is_job_a_getter(unsigned char job_type){
	return job_type == GET_ALL || job_type == GET_UPDATES;
}

struct job parse_job(unsigned char *data,int size){
	struct job job;
	size_t i;
	int poz =0;
	job.job_type = data[poz];
	poz++;
	if(!is_job_a_getter(job.job_type)){
		for(i=0;i<IPV4_SIZE;i++){
			job.client.ipv4[i]= data[poz];
			//printf("ip %d\n",data[poz]);
			poz++;
		}
		for(i=0;i<MAC_LEN;i++){
			job.client.mac[i]= data[poz];
			//printf("mac %x\n",data[poz]);
			poz++;
		}
		job.client.infectivity = data[poz];
	}
	return job;
}

struct task* parse_task(unsigned char *data,int size, int sender){
	//printf("Task creation\n");
	struct task *new_task = (struct task *)malloc(sizeof(struct task ));
	//printf("ok1\n");
	new_task->len=size;
	//printf("ok2\n");
	new_task->job = parse_job(data,size);
	//printf("ok3\n");
	new_task->sender= sender;
	//printf("ok4\n");
	return new_task;
}

void inject_task(char* data,int size, int sender){
	struct task* new_task;
	pthread_mutex_lock(&mutex);
	while(task_pool->size >= MAX_POOL_SIZE)
		pthread_cond_wait(&cond,&mutex);
	new_task =parse_task(data,size,sender);
	push_to_list(task_pool,new_task);
	//printf("ok5\n");
	//printf("Task added\n");
	//print_task(new_task);
	pthread_cond_signal(&cond);
	pthread_mutex_unlock(&mutex);
}

struct task extract_task(){
	struct task ext_task, *copy;
	size_t i;
	pthread_mutex_lock(&mutex);
	while(task_pool->size <= 0)
		pthread_cond_wait(&cond,&mutex);
	copy = pop_from_list(task_pool);
	ext_task = *copy;
	free(copy);
	//size_pool--;
	//printf("Task extracted\n");
	print_task(&ext_task);
	pthread_cond_signal(&cond);
	pthread_mutex_unlock(&mutex);
	return ext_task;
}



void *worker(){
	struct task task_to_execute;
	while(1){
		task_to_execute = extract_task();
		process_task(task_to_execute);
		close(task_to_execute.sender);
	}
}

//should change to tcp
void* main_server(){
	int sockfd, clientfd;
	struct sockaddr_in serv_addr;
	struct sockaddr_in client_addr;
	// Creating socket file descriptor
	if ( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0 ) {
		perror("socket creation failed");
		return NULL;
	}
	//printf("socket server created\n");

	memset(&serv_addr, 0, sizeof(serv_addr));
	
	// Filling server information
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(PORT_LISTEN);
	serv_addr.sin_addr.s_addr = INADDR_ANY;

	if ( bind(sockfd, (const struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0 )
	{
		perror("bind failed");
		return NULL;
	}
	printf("bind created\n");
	if ((listen(sockfd, 5)) != 0) {
		perror("Listen failed...\n");
		return NULL;
	}
	printf("listen created\n");
	memset(&client_addr, 0, sizeof(client_addr));
      
	int len_data;
	socklen_t len;
	
	// sendto(sockfd, (const char *)hello, strlen(hello),
	// 	0, (const struct sockaddr *) &servaddr,
	// 		sizeof(servaddr));
	// printf("Hello message sent.\n");
	char *buf = (char*)malloc(MAX_BUFFER_SIZE*sizeof(char));
	int size;
	while(1){
		clientfd = accept(sockfd,(struct sockaddr*)&client_addr,&len);
		printf("client with ip %s connected\n", inet_ntoa(client_addr.sin_addr));
		size = receive_data(clientfd,buf);
		if(size > 0)
			inject_task(buf,size,clientfd);
		else{
			perror("Cannot receive data\n");
		}
		//print_job((struct job*)buf);
		//len_data = recvfrom(sockfd, (char *)buf, MAX_BUFFER_SIZE,MSG_WAITALL, (struct sockaddr *) &client_addr, &len);
	}
	free(buf);
	// printf("Server : %s\n", buffer);

	close(sockfd);
}

int start_monitoring(char *filename){
	if(filename && strlen(filename) > 0)
		strncpy(storage_file,filename,sizeof(filename));
	pthread_t thr[NUMBER_OF_WORKERS +1];
	size_t i;
	printf("Creating threads ... \n");
	pthread_mutex_init(&mutex,NULL);
	pthread_mutex_init(&mutex_storage,NULL);
	pthread_cond_init(&cond,NULL);
	size_pool = 0;
	updates = create_list();
	task_pool = create_list();
	FILE* filedesc;
	filedesc = fopen(storage_file,"w");
	if(filedesc){
		//fprintf(filedesc,"\n");
		fclose(filedesc);
		printf("File opened\n");
		for(i=0;i<NUMBER_OF_WORKERS+1;i++){
			if(i == 0){
				pthread_create(&thr[i],NULL,main_server,NULL);
			}
			else{
				pthread_create(&thr[i],NULL,worker,NULL);
			}
		}
		printf("Threads created\n");
		for(i=0;i<NUMBER_OF_WORKERS+1;i++){
			pthread_join(thr[i],NULL);
		}
	}
	else{
		perror("Cannot open file\n");
	}

	clear_list(task_pool);
	free(task_pool);
	clear_list(updates);
	free(updates);
	pthread_mutex_destroy(&mutex);
	pthread_mutex_destroy(&mutex_storage);
	pthread_cond_destroy(&cond);
	printf("Threads killed\n");
	return 0;
}