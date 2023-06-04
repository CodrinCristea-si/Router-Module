#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdbool.h>

#include "../headers/communication_local.h"




struct client_job* create_job(unsigned char* data, unsigned char type){
	struct client_job* jobs = NULL;
	jobs =(struct client_job*) malloc(sizeof(struct client_job));
	jobs->job_type=type;
	if(data){
		struct client_infectivity *client;
		client = (struct client_infectivity *) data;
		
		copy_uchar_values(client->ipv4,jobs->client.ipv4,IPV4_SIZE);
		copy_uchar_values(client->mac,jobs->client.mac,MAC_SIZE);
		jobs->client.infectivity = client->infectivity;
	}
	return jobs;
}

int send_to_monitor(unsigned char*data, unsigned char type){
	struct client_job* job = create_job(data,type);
	if(!job){
		perror("Failed to create job\n");
		return -1;
	}
	//print_job(job);
	int sockfd;
	struct sockaddr_in servaddr;
	int len;
	// socket create and verification
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1) {
		perror("socket creation failed...\n");
		return -1;
	}
	//printf("Socket created\n");
	memset(&servaddr,0, sizeof(servaddr));

	// assign IP, PORT
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	servaddr.sin_port = htons(PORT_LISTEN);

	// connect the client socket to server socket
	if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr))!= 0) {
		perror("connection with the server failed...\n");
		return -1;
	}
	//printf("Connected\n");
	len = send_data(sockfd,(unsigned char*)job,sizeof(struct client_job));
	if(len <= 0){
		perror("error while sending the message");
		return -1;
	}
	//printf("Message sent\n");
	free(job);
}


struct response create_invalid_response(){
	struct response resp ={
		.data = NULL,
		.nr_ent = -1
	};
	return resp;
}


struct response receive_from_monitor(int *sockfd){
	unsigned char type;
	int len;
	int nr_ent;
	int poz =0;
	size_t i;
	struct response response = create_invalid_response();
	len = receive_data(*sockfd,(unsigned char*)&type);
	//printf("type received %x\n",type);
	if(len > 0){
		// response = (struct response *)malloc(sizeof(struct response));
		response.data = create_list();
		len = receive_data(*sockfd,(unsigned char *)&nr_ent);
		response.nr_ent=nr_ent;
		//printf("nr ent received %x\n",nr_ent);
		//printf("type received %x and %x\n",type,ALL_DATA);
		if (type == ALL_DATA){
			//printf("All_DATA\n");
			for(i=0;i<nr_ent;i++){
				//printf("for %d\n",i);
				struct client_infectivity *buf= (struct client_infectivity *)malloc(sizeof(struct client_infectivity));
				//if (!buf) printf("Ceva a mers prost\n");
				len = receive_data(*sockfd,(unsigned char*)buf);
				//printf("len %d\n",len);
				if(len<=0) break;
				else{
					push_to_list(response.data,buf);
					//printf("received ");
					//print_client_infectivity(buf);
				}
			}
		}
		else if(type == UPDATES){
			for(i=0;i<nr_ent;i++){
				struct job *buf= (struct job *)malloc(sizeof(struct client_job));
				len = receive_data(*sockfd,(unsigned char*)buf);
				if(len<=0) break;
				else{
					push_to_list(response.data,buf);
				}
			}
		}
		else{
			printf("Invalid type %x\n",type);
		}
	}
	//printf("Exit commun\n");
	//free(type);
	return response;
	
}

struct response send_and_receive_from_monitor(unsigned char*data, unsigned char type){
	struct client_job* job = create_job(data,type);
	if(!job){
		perror("Failed to create job\n");
		return create_invalid_response();
	}
	//printf("job created\n");
	int sockfd;
	struct sockaddr_in servaddr;
	int len;
	// socket create and verification
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1) {
		perror("socket creation failed...\n");
		return create_invalid_response();
	}
	//printf("socket created\n");
	memset(&servaddr,0, sizeof(servaddr));

	// assign IP, PORT
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	servaddr.sin_port = htons(PORT_LISTEN);

	// connect the client socket to server socket
	if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr))!= 0) {
		perror("connection with the server failed...\n");
		return create_invalid_response();
	}
	//printf("connected\n");
	len = send_data(sockfd,(unsigned char*)job,sizeof(struct client_job));
	if(len <= 0){
		perror("error while sending the message");
		return create_invalid_response();
	}
	//printf("message sent\n");
	free(job);
	return receive_from_monitor(&sockfd);
}

void clear_response(struct response* response, bool is_static){
	if(response){
		clear_list(response->data);
		free(response->data);
		if(!is_static)free(response);	
	}
}