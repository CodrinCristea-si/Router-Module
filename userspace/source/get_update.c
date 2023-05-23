#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
 #include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "../headers/communication_local.h"
#include "../headers/utils.h"

// void print_response(struct kernel_response* response){
// 	int nr_el;
// 	size_t i;
// 	struct client_repr* client;
// 	if(response){
// 		switch (response->type)
// 		{
// 		case CLIENTS_DATA:
// 			nr_el= response->opt;
// 			for(i=0;i<nr_el;i++){
// 				client= (struct client_repr* )&response->data[i];
// 				printf("ip = %pI4, mac = %x:%x:%x:%x:%x:%x, infec = %x\n",&client->ip_addr,
// 				client->mac_addr[0],client->mac_addr[1],client->mac_addr[2],client->mac_addr[3],
// 				client->mac_addr[4],client->mac_addr[5],client->infectivity);
// 			}
// 			break;
// 		case ERROR:
// 			printf("err = %s\n",response->data);
// 			break;
// 		case CONFIRM:
// 			printf("ok = %s\n",response->data);
// 			break;
// 		default:
// 			break;
// 		}
		
// 	}
// }

int write_data(int file,char* buf, int size_data){
	int len_data =0 ;
	int size_payload = size_data;
	int curr_len =0 ;
	while(curr_len <= size_payload){
		len_data = write(file, &buf[curr_len],sizeof(size_payload-curr_len));
		if(len_data <0 ) return -1; 
		curr_len += len_data;
	}
	return 0;
}


void print_response_get_all(struct response *response){
	size_t i,length;
	printf("Response pointer %p\n",response);
	printf("Nr ent %d\n", response->nr_ent);
	length = response->nr_ent;
	for(i =0;i<length;i++){
		struct client_infectivity *client = get_from_list(response->data,i);
		if(client)
			printf("Client with ip %d.%d.%d.%d  mac %x:%x:%x:%x:%x:%x infectivity %d\n",client->ipv4[0],
				client->ipv4[1],client->ipv4[2],client->ipv4[3],client->mac[0],
				client->mac[1],client->mac[2],client->mac[3],client->mac[4],client->mac[5],
				client->infectivity);
		else{
			printf("NULL\n");
		}
	}
}

void create_json_get_all(struct response *response, char* output){
	//int file;
	FILE *file;
	size_t i, length;
	//file = open(output,'w');
	file = fopen(output,"w");
	if(file){
		// char begining[] = "[{\"data\":[";
		// write_data(file,begining,strlen(begining));
		fprintf(file,"[{\"data\":[");
		length = response->nr_ent;
		for(i =0;i<length;i++){
			struct client_infectivity *client = get_from_list(response->data,i);
			if(client){
				// char client_str[100]={0};
				// sprintf(client_str,"{\"ip\":\"%d.%d.%d.%d\",\"mac\":\"%x:%x:%x:%x:%x:%x\",\"infect\":\"%d\"}",client->ipv4[0],
				// client->ipv4[1],client->ipv4[2],client->ipv4[3],client->mac[0],
				// client->mac[1],client->mac[2],client->mac[3],client->mac[4],client->mac[5],
				// client->infectivity);
				// write_data(file,client_str,strlen(client_str));

				fprintf(file,"{\"ip\":\"%d.%d.%d.%d\",\"mac\":\"%x:%x:%x:%x:%x:%x\",\"infect\":\"%d\"}",client->ipv4[0],
				client->ipv4[1],client->ipv4[2],client->ipv4[3],client->mac[0],
				client->mac[1],client->mac[2],client->mac[3],client->mac[4],client->mac[5],
				client->infectivity);
			}
			if(i != length -1 && client){
				// char sep[]=",\n";
				// write_data(file,sep,strlen(sep));
				fprintf(file,",\n");
			}
		}
		// char ending[]="]}]\n";
		// write_data(file,ending,strlen(ending));
		// close(file);

		fprintf(file,"]}]\n");
		fclose(file);
	}
}

void print_response_get_updates(struct response *response){
	size_t i;
	size_t length;
	printf("Response pointer %p\n",response);
	printf("Nr ent %d\n", response->nr_ent);
	length = response->nr_ent;
	for(i =0;i<length;i++){
		struct job *job = get_from_list(response->data,i);
		if(job){
			printf("Action %d with client with ip %d.%d.%d.%d  mac %x:%x:%x:%x:%x:%x infectivity %d\n",job->job_type,job->client.ipv4[0],
				job->client.ipv4[1],job->client.ipv4[2],job->client.ipv4[3],job->client.mac[0],
				job->client.mac[1],job->client.mac[2],job->client.mac[3],job->client.mac[4],job->client.mac[5],
				job->client.infectivity);
		}else{
			printf("NULL\n");
		}
	}
}

void create_json_get_updates(struct response *response, char* output){
	//int file;
	FILE* file;
	size_t i;
	size_t length;
	//file = open(output,'w');
	file = fopen(output,"w");
	if(file){
		//char begining[] = "[{\"updates\":[";
		//write_data(file,begining,strlen(begining));
		fprintf(file,"[{\"updates\":[");
		length = response->nr_ent;
		for(i =0;i<length;i++){
			struct job *job = get_from_list(response->data,i);
			if(job){
				// char job_str[100]={0};
				// sprintf(job_str,"{\"type\":\"%d\",\"ip\":\"%d.%d.%d.%d\",\"mac\":\"%x:%x:%x:%x:%x:%x\",\"infect\":\"%d\"}",job->job_type,job->client.ipv4[0],
				// job->client.ipv4[1],job->client.ipv4[2],job->client.ipv4[3],job->client.mac[0],
				// job->client.mac[1],job->client.mac[2],job->client.mac[3],job->client.mac[4],job->client.mac[5],
				// job->client.infectivity);
				// write_data(file,job_str,strlen(job_str));
				fprintf(file,"{\"type\":\"%d\",\"ip\":\"%d.%d.%d.%d\",\"mac\":\"%x:%x:%x:%x:%x:%x\",\"infect\":\"%d\"}",job->job_type,job->client.ipv4[0],
				job->client.ipv4[1],job->client.ipv4[2],job->client.ipv4[3],job->client.mac[0],
				job->client.mac[1],job->client.mac[2],job->client.mac[3],job->client.mac[4],job->client.mac[5],
				job->client.infectivity);
			}
			if(i != length -1 && job){
				// char sep[]=",\n";
				// write_data(file,sep,strlen(sep));
				fprintf(file,",\n");
			}
		}
		// char ending[]="]}]\n";
		// write_data(file,ending,strlen(ending));
		// close(file);
		fprintf(file,"]}]\n");
		fclose(file);
	}
}

void create_json_error(char* error, char* output){
	//int file;
	FILE *file;
	size_t i, length;
	// file = open(output,'w');
	file = fopen(output,"w");
	if(file){
		// char error_str[200];
		// snprintf(error_str,200,"{error:%s}",error);
		// write_data(file,error_str,strlen(error_str));
		// close(file);
		fprintf(file,"{error:%s}",error);
		fclose(file);
	}
}

int main(int argc,char** argv){
	int payload_id;
	struct response response;
	char msg[100];
	char filename [100];
	//struct kernel_response* response;
	//response = send_and_receive_kernel(NULL,GET_CLIENTS);
	if(argc > 1){
		//printf("has args\n");
		if(strncmp(argv[1], "--get-all", 10) == 0){
			//printf("received --get-all\n");
			if(argc > 2){
				strncpy(filename,argv[2],100);
				//printf("copy\n");
				response = send_and_receive_from_monitor(NULL,GET_ALL);
				//printf("daca nu se afiseaza asta atunci primesc Bus Error de la return :( \n");
				//printf("%p\n",response);
				if(response.nr_ent < 0){
					//printf("e null\n");
					strncpy(msg,"Cannot send message",100);
					perror(msg);
					create_json_error(msg, filename);
				}
				else{
					//printf("hai sa print\n");
					//print_response_get_all(&response);
					//printf("hai sa save file\n");
					create_json_get_all(&response,filename);
					//printf("hai sa stergem raspuns\n");
					clear_response(&response,true);
				}
			}
			else{
				perror("Missing output file name\n");
			}
		}
		else if(strncmp(argv[1],"--get-updates", 20) == 0){
			if(argc > 2){
				strncpy(filename,argv[2],100);
				response = send_and_receive_from_monitor(NULL,GET_UPDATES); 
				if(response.nr_ent < 0){
					strncpy(msg,"Cannot send message",100);
					perror(msg);
					create_json_error(msg, filename);
				}else{
					//print_response_get_updates(&response);
					create_json_get_updates(&response,filename);
					clear_response(&response,true);
				}
			}
			else{
				perror("Missing output file name\n");
			}
		}
		else if(strncmp(argv[1],"--transfer", 20) == 0){
			if(argc > 4){
				char *ip,*mac,infec;
				struct client_infectivity client;
				ip = (char*)malloc(20*sizeof(char));
				mac = (char*)malloc(20*sizeof(char));
				strncpy(ip,argv[2],20);
				strncpy(mac,argv[3],20);
				infec = (char)(atoi(argv[4]));
				if(compare_regex(ip,REGEX_IPV4) != 0) {
					perror("Invalid IP\n");
					return -1;
				}
				if(compare_regex(mac,REGEX_MAC) != 0) {
					perror("Invalid MAC\n");
					return -1;
				}
				if(infec > 5){
					perror("Invalid Infectivity type\n");
					return -1;
				}
				str2ipv4(ip,(unsigned char*)&client.ipv4);
				str2mac(mac,(unsigned char*)&client.mac);
				client.infectivity=infec;
			//1683265132 bc:d0:74:84:95:78 192.168.1.123 * 01:bc:d0:74:84:95:78
				send_to_monitor((unsigned char*)&client,TRANSFER);
				free(ip);
				free(mac);
			}else{
				perror("Missing ip, mac or infectivity type\n");
			}
		}
	}
	else{
		perror("Invalid command, try --get-all, --get-updates or --transfer\n");
	}
	return 0;
}