#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <sys/types.h>

#include "../headers/communication.h"

void print_response(struct kernel_response* response){
	int nr_el;
	size_t i;
	struct client_repr* client;
	if(response){
		switch (response->type)
		{
		case CLIENTS_DATA:
			nr_el= response->opt;
			for(i=0;i<nr_el;i++){
				client= (struct client_repr* )&response->data[i];
				printf("ip = %pI4, mac = %x:%x:%x:%x:%x:%x, infec = %x\n",&client->ip_addr,
				client->mac_addr[0],client->mac_addr[1],client->mac_addr[2],client->mac_addr[3],
				client->mac_addr[4],client->mac_addr[5],client->infectivity);
			}
			break;
		case ERROR:
			printf("err = %s\n",response->data);
			break;
		case CONFIRM:
			printf("ok = %s\n",response->data);
			break;
		default:
			break;
		}
		
	}
}

int main(int argc,char** argv){
	int payload_id;
	struct kernel_response* response;
	response = send_and_receive_kernel(NULL,GET_CLIENTS);
	if(response)
		perror("Cannot send message");
	
	print_response(response);
	clear_response(response);
	return 0;
}