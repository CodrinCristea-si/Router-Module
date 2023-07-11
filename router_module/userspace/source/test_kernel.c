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

int send_state(struct client_repr *cl_rpr, int type){
	send_message_to_kernel((unsigned char*)cl_rpr,type);
}

void send_ready(){
	struct network_details main_network ={
		.ip_router = 0x0100a8c0,
		.netmask =0x00ffffff,
		.subnet =0x0000a8c0,
	};
	send_message_to_kernel((unsigned char*)&main_network,CONFIGURE);
}

int send_package_to_client(struct client_repr *who, unsigned char* data, unsigned int size){
	int sockfd;
	struct sockaddr_in servaddr;

	// Creating socket file descriptor
	if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
		perror("socket creation failed");
		exit(EXIT_FAILURE);
	}

	memset(&servaddr, 0, sizeof(servaddr));

	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(PORT_UDP_3RD_DEVICE);
	servaddr.sin_addr.s_addr = who->ip_addr;

	int n;
	socklen_t len;

	int sent = 0, so_far =0;
	while(sent <size){
		so_far = sendto(sockfd, (unsigned char *)data, size,
		MSG_CONFIRM, (const struct sockaddr *) &servaddr, 
			sizeof(servaddr));
		sent+= so_far;
	}
	return 0;
}

int main(int argc, char** argv){

	send_ready(); //without interceptor

	struct client_repr client1 = {
		.ip_addr = 0x0501a8c0,
		.mac_addr = {0x75, 0x56, 0xfe, 0xab, 0xc3, 0xd9},
		.infectivity = SUSPICIOUS,
	};

	struct client_repr client2 = {
		.ip_addr = 0x0701a8c0,
		.mac_addr = {0x10, 0xf4, 0x76, 0x00, 0x12, 0x45},
		.infectivity = SUSPICIOUS,
	};

	struct client_repr client3 = {
		.ip_addr = 0x0901a8c0,
		.mac_addr = {0x45, 0xf4, 0x8d, 0x93, 0xdc, 0x10},
		.infectivity = SUSPICIOUS,
	};
	struct client_repr client4 = {
		.ip_addr = 0x0b01a8c0,
		.mac_addr = {0x45, 0xf4, 0x8d, 0x93, 0xdc, 0x10},
		.infectivity = SUSPICIOUS,
	};

	struct client_repr client_me = {
		.ip_addr = 0x7200a8c0,
		.mac_addr = {0x00,0x0c,0x29,0xcd,0x4e,0x98},
		.infectivity = SUSPICIOUS,
	};

	char message[] = "hello";

	send_state(&client1,ADD_CLIENT);
	send_state(&client2,ADD_CLIENT);
	send_state(&client1,REMOVE_CLIENT);
	send_state(&client3,ADD_CLIENT);
	send_state(&client4,ADD_CLIENT);
	send_state(&client_me,ADD_CLIENT);

	//send packages
	client2.infectivity=INFECTED_MAJOR;
	client3.infectivity=INFECTED_MINOR;
	client4.infectivity=INFECTED_SEVER;
	client_me.infectivity=INFECTED_MAJOR;

	send_state(&client3,TRANSFER_CLIENT);
	send_package_to_client(&client3,message,strlen(message));
	send_state(&client2,TRANSFER_CLIENT);
	send_package_to_client(&client2,message,strlen(message));
	send_state(&client4,TRANSFER_CLIENT);
	send_state(&client_me,TRANSFER_CLIENT);
	send_package_to_client(&client4,message,strlen(message));


	return 0;
}