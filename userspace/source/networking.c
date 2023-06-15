#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include "../headers/networking.h"

int receive_data(int sockfd,unsigned char* buf){
	int len_data =0 ;
	int size_payload;
	len_data = read(sockfd,&size_payload,sizeof(int));
	if(len_data <= 0 || size_payload<=0) return -1;
	int curr_len =0 ;
	while(curr_len < size_payload){
		len_data = read(sockfd,&buf[curr_len],sizeof(size_payload-curr_len));
		if(len_data <0 ) return -1; 
		curr_len += len_data;
	}
	//printf("Data received\n");
	return size_payload;
}

int send_data(int sockfd,unsigned char* buf, int size_data){
	int len_data =0 ;
	int size_payload = size_data;
	len_data = write(sockfd,&size_payload,sizeof(int));
	if(len_data <= 0) return -1;
	int curr_len =0 ;
	while(curr_len < size_payload){
		len_data = write(sockfd,&buf[curr_len],sizeof(size_payload-curr_len));
		if(len_data <0 ) return -1; 
		curr_len += len_data;
		//printf("trimis in pizda %d\n",sockfd);
	}
	//printf("Data sent\n");
	return size_payload;
}

int send_data_udp(int sockfd,unsigned char* buf, int size_data){
	int len_data =0 ;
	int size_payload = size_data;
	int curr_len =0 ;
	while(curr_len < size_payload){
		len_data = write(sockfd,&buf[curr_len],sizeof(size_payload-curr_len));
		if(len_data <0 ) return -1; 
		curr_len += len_data;
		//printf("trimis in pizda %d\n",sockfd);
	}
	//printf("Data sent\n");
	return size_payload;
}