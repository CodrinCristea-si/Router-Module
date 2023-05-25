#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdbool.h>

#include "../headers/communicator_network.h"

void print_package_client(struct network_client_data* pack){
	printf("Pack pointer %p\n",pack);
	if (pack){
		printf("Pack type %x",pack->type);
		printf("Pack ip %d.%d.%d.%d  mac %x:%x:%x:%x:%x:%x infectivity %d\n",pack->ipv4[0],
			pack->ipv4[1],pack->ipv4[2],pack->ipv4[3],pack->mac[0],pack->mac[1],pack->mac[2],
			pack->mac[3],pack->mac[4],pack->mac[5],pack->infectivity);
	}
	else{
		printf("NULL");
	}
}

struct network_client_data create_package_by_type(unsigned char * data, unsigned char type){
	struct network_client_data pack ={
		.type = 0,
		.ipv4 = {0},
		.mac = {0},
		.infectivity = {0}
	};
	size_t i;
	struct network_client_data *cl_data = (struct network_client_data*)data;
	switch (type){
		case CLIENT_CONNECT:
		case CLIENT_DISCONNECT:
		case CLIENT_TRANSFER:
			pack.type = type;
			for(i=0;i<4;i++) pack.ipv4[i] = cl_data->ipv4[i];
			for(i=0;i<6;i++) pack.mac[i] = cl_data->mac[i];
			pack.infectivity = cl_data->infectivity;
		default:
			break;
	}
	return pack;
}

void clear_package(struct network_package *pack){
	if(pack){
		if (pack->data)
			free(pack->data);
	}
}

int send_to_network(unsigned char * data, unsigned char type){
	int sockfd;
	struct sockaddr_in servaddr;
	int len;
	struct network_client_data pack;
	// socket create and verification
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1) {
		perror("socket creation failed...\n");
		return -1;
	}
	printf("Socket created\n");
	memset(&servaddr,0, sizeof(servaddr));

	// assign IP, PORT
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = inet_addr(IP_3RD_DEVICE);
	servaddr.sin_port = htons(PORT_3RD_DEVICE);

	// connect the client socket to server socket
	if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr))!= 0) {
		perror("connection with the server failed...\n");
		return -1;
	}
	printf("connected\n");
	pack = create_package_by_type(data,type);
	printf("ack size %d\n",sizeof(struct network_client_data));
	print_package_client(&pack);
	if (pack.type == 0) {
		perror("cannot create network package\n");
		return -1;
	}
	//printf("Connected\n");
	len = send_data(sockfd,(unsigned char*)&pack,sizeof(struct network_client_data));
	if(len <= 0){
		perror("error while sending the message");
		return -1;
	}
	printf("Message sent\n");
	//clear_package(&pack);
	return 0;
}