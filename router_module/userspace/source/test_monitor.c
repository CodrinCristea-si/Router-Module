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
#include "../headers/utils.h"
#include "../headers/communication_local.h"

int get_client_data_from_argv(char **argv, int first_poz, int nr_args, struct client_infectivity* collector){
	char *ip,*mac,infec;
	struct client_infectivity client;
	
	//ip
	if(nr_args == 0) goto end;
	ip = (char*)malloc(20*sizeof(char));
	strncpy(ip,argv[first_poz],20);
	if(compare_regex(ip,REGEX_IPV4) != 0) {
		perror("Invalid IP\n");
		goto error;
	}
	str2ipv4(ip,(unsigned char*)&collector->ipv4);
	nr_args--;

	//mac
	if(nr_args == 0) goto end;
	mac = (char*)malloc(20*sizeof(char));
	strncpy(mac,argv[first_poz+1],20);
	if(compare_regex(mac,REGEX_MAC) != 0) {
		perror("Invalid MAC\n");
		goto error;
	}
	str2mac(mac,(unsigned char*)&collector->mac);
	nr_args--;

	//infectivity
	if(nr_args == 0) goto end;
	infec = (char)(atoi(argv[first_poz+2]));
	if(infec > 5){
		perror("Invalid Infectivity type\n");
		goto error;
	}
	collector->infectivity=infec;
	nr_args--;

end:
	return 0;

error:
	if(ip) free(ip);
	if(mac) free(mac);
	return -1;
}

void print_response_get_all(struct response *response){
	size_t i,length;
	printf("Response pointer %p\n",response);
	printf("Nr ent %d\n", response->nr_ent);
	length = response->nr_ent;
	for(i =0;i<length;i++){
		void *data = get_from_list(response->data,i);
		if(is_ui_job(get_job_type_from_data(data))){
			struct ui_job *job = data;
			if(job){
				if(is_lockdown_job(job->job_type))
					printf("Lockdown %d \n",job->job_type == LOCKDOWN_UP ? 0 : 1);
				else
					printf("Automatic %d \n",job->job_type == AUTO_UP ? 1 : 0);
			}else{
				printf("NULL\n");
			}
		}
		else{
			struct client_job *job = data;
			if(job)
				printf("Client with ip %d.%d.%d.%d  mac %x:%x:%x:%x:%x:%x infectivity %d\n",job->client.ipv4[0],
					job->client.ipv4[1],job->client.ipv4[2],job->client.ipv4[3],job->client.mac[0],
					job->client.mac[1],job->client.mac[2],job->client.mac[3],job->client.mac[4],job->client.mac[5],
					job->client.infectivity);
			else{
				printf("NULL\n");
			}
		}
	}
}

void print_response_get_updates(struct response *response){
	size_t i;
	size_t length;
	printf("Response pointer %p\n",response);
	printf("Nr ent %d\n", response->nr_ent);
	length = response->nr_ent;
	for(i =0;i<length;i++){
		void *data = get_from_list(response->data,i);
		if(is_ui_job(get_job_type_from_data(data))){
			struct ui_job *job = data;
			if(job){
				if(is_lockdown_job(job->job_type))
					printf("Lockdown %d \n",job->job_type == LOCKDOWN_UP ? 0 : 1);
				else
					printf("Automatic %d \n",job->job_type == AUTO_UP ? 1 : 0);
			}else{
				printf("NULL\n");
			}
		}
		else{
			struct client_job *job = data;
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
}

int main(int argc, char** argv){
	struct client_infectivity* collector;
	struct response response;
	int res;
	//connect
	if(strncmp(argv[1], "-c", 4) == 0){
		collector = (struct client_infectivity*)malloc(sizeof(struct client_infectivity));
		res = get_client_data_from_argv(argv,2,2,collector);
		if(res == 0){
			send_to_monitor((unsigned char*)collector,ADD);
		}
		free(collector);
	}
	//disconnect
	if(strncmp(argv[1], "-d", 4) == 0){
		collector = (struct client_infectivity*)malloc(sizeof(struct client_infectivity));
		res = get_client_data_from_argv(argv,2,2,collector);
		if(res == 0){
			send_to_monitor((unsigned char*)collector,REMOVE);
		}
		free(collector);
	}
	//transfer
	if(strncmp(argv[1], "-t", 4) == 0){
		collector = (struct client_infectivity*)malloc(sizeof(struct client_infectivity));
		res = get_client_data_from_argv(argv,2,3,collector);
		if(res == 0){
			send_to_monitor((unsigned char*)collector,TRANSFER);
		}
		free(collector);
	}
	//get all
	if(strncmp(argv[1], "-ga", 5) == 0){
		response = send_and_receive_from_monitor(NULL,GET_ALL);
		print_response_get_all(&response);
		clear_response(&response,true);
	}
	//get updates
	if(strncmp(argv[1], "-gu", 5) == 0){
		response = send_and_receive_from_monitor(NULL,GET_UPDATES);
		print_response_get_updates(&response);
		clear_response(&response,true);
	}
	//set automatic
	if(strncmp(argv[1], "-sa", 5) == 0){
		int set =  atoi(argv[2]);
		send_to_monitor(NULL, set == 1 ? AUTO_UP : AUTO_DOWN);
	}
	//set lockdown
	if(strncmp(argv[1], "-sl", 5) == 0){
		int set =  atoi(argv[2]);
		send_to_monitor(NULL, set == 1 ? LOCKDOWN_DOWN : LOCKDOWN_UP);
	}

	return 0;
}