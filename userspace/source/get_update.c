#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <sys/types.h>

#include "../headers/communication_local.h"

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

void print_response_get_all(struct response *response){
	size_t i,length;
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

void print_response_get_updates(struct response *response){
	size_t i,length;
	printf("Nr ent %d\n", response->nr_ent);
	length = response->nr_ent;
	for(i =0;i<length;i++){
		struct job *job = get_from_list(response->data,i);
		printf("Action %d with client with ip %d.%d.%d.%d  mac %x:%x:%x:%x:%x:%x infectivity %d\n",job->job_type,job->client.ipv4[0],
			job->client.ipv4[1],job->client.ipv4[2],job->client.ipv4[3],job->client.mac[0],
			job->client.mac[1],job->client.mac[2],job->client.mac[3],job->client.mac[4],job->client.mac[5],
			job->client.infectivity);
	}
}

int main(int argc,char** argv){
	int payload_id;
	struct response *response = NULL;
	//struct kernel_response* response;
	//response = send_and_receive_kernel(NULL,GET_CLIENTS);
	if(argc > 1){
		if(atoi(argv[1]) == 1){
			response = send_and_receive_from_monitor(NULL,GET_ALL); 
			//printf("%p\n",response);
			if(!response)
				perror("Cannot send message");
			else{
				print_response_get_all(response);
				clear_response(response);
			}
		}
		else if(atoi(argv[1]) == 2){
			response = send_and_receive_from_monitor(NULL,GET_UPDATES); 
			if(!response)
				perror("Cannot send message");
			else{
				print_response_get_updates(response);
				clear_response(response);
			}
		}
		else{
			//1683265132 bc:d0:74:84:95:78 192.168.1.123 * 01:bc:d0:74:84:95:78
			struct client_infectivity client ={
				.ipv4 ={192,168,1,123},
				.mac = {0xbc,0xd0,0x74,0x84,0x95,0x78},
				.infectivity = 4
			};
			send_to_monitor((unsigned char*)&client,TRANSFER);
		}
	}
	else{
		perror("Invalid command, try 1, 2 or 3\n");
	}
	return 0;
}