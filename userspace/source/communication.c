#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <sys/types.h>
#include <linux/netlink.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "../headers/communication.h"
//#include "../../headers/common_proto.h"

#define MAX_SIZE_PAYLOAD 4096



struct infec_msg* create_add_client_msg(struct client_repr *client, unsigned char type){
	struct header_payload *hdr_inf;
	struct infec_msg* msg_infec = NULL;
	if(client){
		unsigned char *data = (unsigned char *)calloc(20,sizeof(char));
		int len = create_client_repr_payload(client,data,FLAG_WITH_IP|FLAG_WITH_MAC);
		printf("Data created\n");
		// printf("data p %p si %x ip %x.%x.%x.%x sm %x mac %x:%x:%x:%x:%x:%x \n", data, data[0],data[1],data[2],
		// 	data[3],data[4],data[5],data[6],data[7],data[8],data[9],data[10],data[11]);

		hdr_inf = (struct header_payload *)calloc(1, sizeof(struct header_payload));
		create_header(rand()%(int)(MAX_ID), type,hdr_inf);
		printf("Header created\n");
		// printf("hdr p %p  s %x%x%x%x t %x i %x\n",hdr_inf,hdr_inf->signiture[0], hdr_inf->signiture[1], hdr_inf->signiture[2],
		// 	hdr_inf->signiture[3], hdr_inf->payload_type, hdr_inf->payload_id);
		msg_infec = (struct infec_msg *)malloc(INF_MSG_LEN_H(hdr_inf));
		create_message(hdr_inf,data,len,msg_infec);

		free(data);
		free(hdr_inf);

		printf("Msg created\n");
	}
	return msg_infec;
}

struct infec_msg* create_get_clients_msg(unsigned char type){
	struct header_payload *hdr_inf = (struct header_payload *)calloc(1, sizeof(struct header_payload));
	create_header(rand()%(int)(MAX_ID), type,hdr_inf);
	printf("Header created\n");
	// printf("hdr p %p  s %x%x%x%x t %x i %x\n",hdr_inf,hdr_inf->signiture[0], hdr_inf->signiture[1], hdr_inf->signiture[2],
	// 	hdr_inf->signiture[3], hdr_inf->payload_type, hdr_inf->payload_id);
	struct infec_msg* msg_infec = (struct infec_msg *)malloc(INF_MSG_LEN_H(hdr_inf));
	create_message(hdr_inf,NULL,0,msg_infec);
	free(hdr_inf);

	printf("Msg created\n");
	return msg_infec;
}

struct infec_msg* create_infec_msg_by_type(unsigned char* data, unsigned char type){
	switch (type)
	{
	case ADD_CLIENT:
		return create_add_client_msg((struct client_repr *)data,type);
		break;
	case REMOVE_CLIENT:
		return create_add_client_msg((struct client_repr *)data,type);
		break;
	case GET_CLIENTS:
		return create_get_clients_msg(type);
		break;
	default:
		break;
	}
}

void clear_infec_msg(struct infec_msg * msg_infec){
	free(msg_infec);
}


int send_message_to_kernel(unsigned char* data, unsigned char type){
	
	int fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_PROTO_INFECTED);
	if (fd < 0) {
		perror("Cannot open socket\n");
		return -1;
	}
	printf("Socket created\n");
	struct sockaddr_nl addr; 
	memset(&addr, 0, sizeof(addr));
	addr.nl_family = AF_NETLINK;
	addr.nl_pid = 0;  // For Linux kernel
	addr.nl_groups = 0;
	// printf("inf p %p h %p d %p \n",msg_infec,INF_MSG_HEADER(msg_infec), INF_MSG_DATA(msg_infec));
	// printf("len %ld\n", INF_MSG_LEN(msg_infec));
	struct infec_msg* msg_infec = create_infec_msg_by_type(data,type);
	if(msg_infec){
		struct nlmsghdr *nlh = (struct nlmsghdr *) malloc(NLMSG_SPACE(MAX_PAYLOAD_SIZE));
		memset(nlh, 0, NLMSG_SPACE(MAX_PAYLOAD_SIZE));
		nlh->nlmsg_len = NLMSG_SPACE(MAX_PAYLOAD_SIZE);
		nlh->nlmsg_pid = getpid();
		nlh->nlmsg_flags = 0;
		copy_uchar_values((unsigned char*)msg_infec,(unsigned char *) NLMSG_DATA(nlh), INF_MSG_LEN(msg_infec));
		printf("nhl dat s %x%x%x%x\n",((unsigned char*)NLMSG_DATA(nlh))[0],((unsigned char *)NLMSG_DATA(nlh))[1],
			((unsigned char *)NLMSG_DATA(nlh))[2],((unsigned char *)NLMSG_DATA(nlh))[3]);
		printf("nhl created\n");
		struct iovec iov; 
		memset(&iov, 0, sizeof(iov));
		iov.iov_base = (void *) nlh;
		iov.iov_len = nlh->nlmsg_len;

		struct msghdr msg; 
		memset(&msg, 0, sizeof(msg));
		msg.msg_name = (void *) &addr;
		msg.msg_namelen = sizeof(addr);
		msg.msg_iov = &iov;
		msg.msg_iovlen = 1;
		printf("Message created\n");

		
		sendmsg(fd, &msg, 0);
		clear_infec_msg(msg_infec);
		
		printf("Sent message to kernel\n");
		return msg_infec->header.payload_id;
	}
	else{
		perror("Cannot create message\n");
		return -1;
	}
}

int extract_nr_clients_from_payload(unsigned char* data){
	int nr=-1;
	if(CHECK_SIGNITURE(data[0], SIGNITURE_NR_ENT))
		ch2int(&data[1],&nr);
	return nr;
}

void extract_clients_data(struct infec_msg* msg, struct client_repr ** collector, int nr_clients){
	//pass nr signature
	int poz = 5;
	size_t i=0;
	struct client_repr *decoy;
	for (i=0;i<nr_clients;i++){
		decoy = (struct client_repr *)malloc(sizeof(struct client_repr));
		extract_client_repr_payload(msg,decoy,poz,FLAG_WITH_IP|FLAG_WITH_MAC|FLAG_WITH_INFECTIVITY);
		collector[i]=decoy;
		poz+=sizeof(struct client_repr);
	}

}

struct kernel_response* extract_kernel_response(unsigned char* data,int data_len, int payload_id){
	struct nlmsghdr *nh;
	struct kernel_response* response;
	struct infec_msg* msg_infec;
	unsigned char* collector;
	int nr_cl;

	response = (struct kernel_response*)calloc(1,sizeof(struct kernel_response));
	nh = (struct nlmsghdr*)data;
	while (NLMSG_OK(nh, data_len)) {
		msg_infec = NLMSG_DATA(nh);
		if(payload_id){
			switch (msg_infec->header.payload_type)
			{
			case CLIENTS_DATA:
				nr_cl = extract_nr_clients_from_payload((unsigned char*)INF_MSG_DATA(msg_infec));
				if (nr_cl<0) return NULL;
				else{
					collector = (unsigned char*)malloc(nr_cl*sizeof(struct client_repr*));
					extract_clients_data(msg_infec,(struct client_repr**)collector,nr_cl);
					copy_uchar_values(collector,response->data,sizeof(struct client_repr)*nr_cl);
					response->type=CLIENTS_DATA;
					response->opt=nr_cl;
				}
				break;
			case ERROR:
				collector = (char*)malloc(strlen(INF_MSG_DATA(msg_infec))*sizeof(char));
				copy_uchar_values(INF_MSG_DATA(msg_infec),collector,strlen(INF_MSG_DATA(msg_infec)));
				copy_uchar_values(collector,response->data,strlen(INF_MSG_DATA(msg_infec)));
				response->type=ERROR;
			case CONFIRM:
				collector = (char*)malloc(1*sizeof(char));
				copy_uchar_values(collector,response->data,1);
				response->type=CONFIRM;
			default:
				break;
			}
		}
		else{
			//ceva update
		}
		nh = NLMSG_NEXT(nh, data_len);
	}
}


struct kernel_response* receive_from_kernel(int payload_id){
	struct nlmsghdr *nh;
	struct ndmsg *ndm;
	struct infec_msg* msg_infec;
	unsigned char* collector;
	struct kernel_response* response = NULL;
	int nr_cl;

	int fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_PROTO_INFECTED);
	int len;
	if (fd < 0) {
		perror("Cannot open socket\n");
	}
	printf("Socket created\n");

	struct sockaddr_nl addr; 
	memset(&addr, 0, sizeof(addr));
	addr.nl_family = AF_NETLINK;
	addr.nl_pid = 0;  // For Linux kernel
	addr.nl_groups = 0; //unicast

	char *buf = (char*)calloc(MAX_SIZE_PAYLOAD,sizeof(char));

    	bind(fd, (struct sockaddr*)&addr, sizeof(addr));
	len = recv(fd, buf, MAX_SIZE_PAYLOAD, 0);
	if(len<0){
		printf("Error while receiving!\n");
	
	}
	else{
		response = extract_kernel_response(buf,len,payload_id);
	}
	free(buf);
	return response;
}

void clear_response(struct kernel_response* response){
	int nr;
	size_t i;
	if(response){
		switch (response->type)
		{
		case CLIENTS_DATA:
			nr = response->opt;
			if(response->data){
				for(i=0;i<nr;i++){
					free(&response->data[i]);
				}
				free(response->data);
			}
			free(response);
			break;
		case ERROR:
		case CONFIRM:
			if(response->data)
				free(response->data);
			free(response);
			break;
		default:
			break;
		}
	}
}

struct kernel_response* send_and_receive_kernel(unsigned char* data, unsigned char type){
	int len_data;
	struct kernel_response* response = NULL;
	int fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_PROTO_INFECTED);
	if (fd < 0) {
		perror("Cannot open socket\n");
		return NULL;
	}
	printf("Socket created\n");
	struct sockaddr_nl addr; 
	memset(&addr, 0, sizeof(addr));
	addr.nl_family = AF_NETLINK;
	addr.nl_pid = 0;  // For Linux kernel
	addr.nl_groups = 0;
	// printf("inf p %p h %p d %p \n",msg_infec,INF_MSG_HEADER(msg_infec), INF_MSG_DATA(msg_infec));
	// printf("len %ld\n", INF_MSG_LEN(msg_infec));
	struct infec_msg* msg_infec = create_infec_msg_by_type(data,type);
	if(msg_infec){
		struct nlmsghdr *nlh = (struct nlmsghdr *) malloc(NLMSG_SPACE(MAX_PAYLOAD_SIZE));
		memset(nlh, 0, NLMSG_SPACE(MAX_PAYLOAD_SIZE));
		nlh->nlmsg_len = NLMSG_SPACE(MAX_PAYLOAD_SIZE);
		nlh->nlmsg_pid = getpid();
		nlh->nlmsg_flags = 0;
		copy_uchar_values((unsigned char*)msg_infec,(unsigned char *) NLMSG_DATA(nlh), INF_MSG_LEN(msg_infec));
		printf("nhl dat s %x%x%x%x\n",((unsigned char*)NLMSG_DATA(nlh))[0],((unsigned char *)NLMSG_DATA(nlh))[1],
			((unsigned char *)NLMSG_DATA(nlh))[2],((unsigned char *)NLMSG_DATA(nlh))[3]);
		printf("nhl created\n");
		struct iovec iov; 
		memset(&iov, 0, sizeof(iov));
		iov.iov_base = (void *) nlh;
		iov.iov_len = nlh->nlmsg_len;

		struct msghdr msg; 
		memset(&msg, 0, sizeof(msg));
		msg.msg_name = (void *) &addr;
		msg.msg_namelen = sizeof(addr);
		msg.msg_iov = &iov;
		msg.msg_iovlen = 1;
		printf("Message created\n");

		
		sendmsg(fd, &msg, 0);
		clear_infec_msg(msg_infec);
		printf("Sent message to kernel\n");

		char *buf = (char*)calloc(MAX_SIZE_PAYLOAD,sizeof(char));
		len_data = recv(fd, buf, MAX_SIZE_PAYLOAD, 0);
		if(len_data >0)
			response = extract_kernel_response(buf,len_data,msg_infec->header.payload_id);
		//return msg_infec->header.payload_id;
	}
	else{
		perror("Cannot create message\n");
	}
	return response;
}
