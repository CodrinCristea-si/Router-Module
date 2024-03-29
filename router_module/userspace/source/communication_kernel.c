#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <sys/types.h>
#include <linux/netlink.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "../headers/communication_kernel.h"
//#include "../../headers/common_proto.h"

#define MAX_SIZE_PAYLOAD 4096

void print_infec_msg(struct infec_msg * msg_infec){
	struct header_payload *hdr_inf;
	size_t i;
	if(msg_infec){
		hdr_inf = (struct header_payload *)INF_MSG_HEADER(msg_infec);
		printf("hdr p %p s %x%x%x%x len %d t %x i %x\n",hdr_inf,hdr_inf->signiture[0], hdr_inf->signiture[1], hdr_inf->signiture[2],
			hdr_inf->signiture[3],hdr_inf->payload_len, hdr_inf->payload_type, hdr_inf->payload_id);
		printf("inf p %p h %p d %p \n",msg_infec,INF_MSG_HEADER(msg_infec), INF_MSG_DATA(msg_infec));
		printf("len %ld\n", INF_MSG_LEN(msg_infec));
		for(i =0;i< INF_MSG_DATA_LEN(msg_infec);i++){
			printf("%x at %ld\n",((unsigned char*)INF_MSG_DATA(msg_infec))[i], i);
		}
		printf("\n");
	}
	else{
		printf("NULL\n");
	}
}

struct infec_msg* create_add_client_msg(struct client_repr *client, unsigned char type){
	struct header_payload *hdr_inf;
	struct infec_msg* msg_infec = NULL;
	if(client){
		unsigned char *data = (unsigned char *)calloc(sizeof(struct client_repr_ext),sizeof(char));
		int len = create_client_repr_payload_ext(client,data,FLAG_WITH_IP|FLAG_WITH_MAC|FLAG_WITH_INFECTIVITY);
		//printf("Data created %d\n",len);
		/// printf("data p %p si %x ip %x.%x.%x.%x sm %x mac %x:%x:%x:%x:%x:%x \n", data, data[0],data[1],data[2],
		// 	data[3],data[4],data[5],data[6],data[7],data[8],data[9],data[10],data[11]);

		hdr_inf = (struct header_payload *)calloc(1, sizeof(struct header_payload));
		create_header(rand()%(int)(MAX_ID), type,len,hdr_inf);
		//printf("Header created\n");
		// printf("hdr p %p  s %x%x%x%x t %x i %x\n",hdr_inf,hdr_inf->signiture[0], hdr_inf->signiture[1], hdr_inf->signiture[2],
		// 	hdr_inf->signiture[3], hdr_inf->payload_type, hdr_inf->payload_id);
		msg_infec = (struct infec_msg *)calloc(INF_MSG_LEN_H(hdr_inf), sizeof(char));
		create_message(hdr_inf,data,msg_infec);

		free(data);
		free(hdr_inf);

		//printf("Msg created\n");
	}
	return msg_infec;
}

struct infec_msg* create_empty_msg(unsigned char type){
	struct header_payload *hdr_inf = (struct header_payload *)calloc(1, sizeof(struct header_payload));
	create_header(rand()%(int)(MAX_ID), type,0,hdr_inf);
	//printf("Header created\n");
	// printf("hdr p %p  s %x%x%x%x t %x i %x\n",hdr_inf,hdr_inf->signiture[0], hdr_inf->signiture[1], hdr_inf->signiture[2],
	// 	hdr_inf->signiture[3], hdr_inf->payload_type, hdr_inf->payload_id);
	struct infec_msg* msg_infec = (struct infec_msg *)malloc(INF_MSG_LEN_H(hdr_inf) + 2);
	create_message(hdr_inf,NULL,msg_infec);
	free(hdr_inf);

	//printf("Msg created\n");
	return msg_infec;
}

struct infec_msg* create_config_msg(struct network_details *network, unsigned char type){
	struct header_payload *hdr_inf;
	struct infec_msg* msg_infec = NULL;
	if(network){
		unsigned char *data = (unsigned char *)calloc(sizeof(struct network_details),sizeof(char));
		memcpy(data, network, sizeof(struct network_details));
		//printf("Data created %ld\n", sizeof(struct network_details));
		/// printf("data p %p si %x ip %x.%x.%x.%x sm %x mac %x:%x:%x:%x:%x:%x \n", data, data[0],data[1],data[2],
		// 	data[3],data[4],data[5],data[6],data[7],data[8],data[9],data[10],data[11]);

		hdr_inf = (struct header_payload *)calloc(1, sizeof(struct header_payload));
		create_header(rand()%(int)(MAX_ID), type, sizeof(struct network_details),hdr_inf);
		//printf("Header created\n");
		// printf("hdr p %p  s %x%x%x%x t %x i %x\n",hdr_inf,hdr_inf->signiture[0], hdr_inf->signiture[1], hdr_inf->signiture[2],
		// 	hdr_inf->signiture[3], hdr_inf->payload_type, hdr_inf->payload_id);
		msg_infec = (struct infec_msg *)calloc(INF_MSG_LEN_H(hdr_inf), sizeof(char));
		create_message(hdr_inf,data,msg_infec);

		free(data);
		free(hdr_inf);

		//printf("Msg created\n");
	}
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
	case TRANSFER_CLIENT:
		return create_add_client_msg((struct client_repr *)data,type);
		break;
	case GET_CLIENTS:
		return create_empty_msg(type);
	case CONFIGURE:
		return create_config_msg((struct network_details *)data, type);
		break;
	case LOCK_UP:
		return create_empty_msg(type);
	case LOCK_DOWN:
		return create_empty_msg(type);
	default:
		break;
	}
}

void clear_infec_msg(struct infec_msg * msg_infec){
	free(msg_infec);
}


int send_message_to_kernel(unsigned char* data, unsigned char type){
	int payload_id, res;
	int fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_PROTO_INFECTED);
	if (fd < 0) {
		perror("Cannot open socket\n");
		return -1;
	}
	//printf("Socket kernel created\n");
	struct sockaddr_nl addr; 
	memset(&addr, 0, sizeof(addr));
	addr.nl_family = AF_NETLINK;
	addr.nl_pid = 0;  // For Linux kernel
	addr.nl_groups = 0;
	// printf("inf p %p h %p d %p \n",msg_infec,INF_MSG_HEADER(msg_infec), INF_MSG_DATA(msg_infec));
	// printf("len %ld\n", INF_MSG_LEN(msg_infec));
	struct infec_msg* msg_infec = create_infec_msg_by_type(data,type);
	if(msg_infec){
		//print_infec_msg(msg_infec);
		struct nlmsghdr *nlh = (struct nlmsghdr *) malloc(NLMSG_SPACE(INF_MSG_LEN(msg_infec)));
		//printf("done malloc\n");
		//memset(nlh, 0, NLMSG_SPACE(MAX_PAYLOAD_SIZE));
		nlh->nlmsg_len = NLMSG_SPACE(INF_MSG_LEN(msg_infec));
		nlh->nlmsg_pid = getpid();
		nlh->nlmsg_flags = 0;
		copy_uchar_values((unsigned char*)msg_infec,(unsigned char *) NLMSG_DATA(nlh), INF_MSG_LEN(msg_infec));
		payload_id = msg_infec->header.payload_id;
		// printf("nhl dat s %x%x%x%x\n",((unsigned char*)NLMSG_DATA(nlh))[0],((unsigned char *)NLMSG_DATA(nlh))[1],
		// 	((unsigned char *)NLMSG_DATA(nlh))[2],((unsigned char *)NLMSG_DATA(nlh))[3]);
		//printf("nhl created\n");
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
		//printf("Message created\n");
		
		res = sendmsg(fd, &msg, 0);
		if (res < 0){
			perror("Cannot send to kernel\n");
		}
		// else{
		// 	printf("Sent message to kernel\n");
		// }
		clear_infec_msg(msg_infec);
		
		
		return payload_id;
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

void extract_clients_data(struct infec_msg* msg, struct client_repr ** collector){
	//pass nr signature
	int poz = 5;
	size_t i=0;
	int len_cl;
	int nr_clients;
	nr_clients = extract_nr_clients_from_payload((unsigned char*)INF_MSG_DATA(msg));
	for (i=0;i<nr_clients;i++){
		collector[i] = (struct client_repr *)malloc(sizeof(struct client_repr));
		len_cl = extract_client_repr_payload(msg,(struct client_repr *)&collector[i],poz,FLAG_WITH_IP|FLAG_WITH_MAC|FLAG_WITH_INFECTIVITY);
		poz+=len_cl;
	}

}

struct kernel_response* extract_kernel_response(struct nlmsghdr* nlh,int data_len, int payload_id){
	//struct nlmsghdr *nh;
	struct kernel_response* response;
	struct infec_msg* msg_infec;
	unsigned char* collector;
	int nr_cl;

	//nh = (struct nlmsghdr*)data;
	while (NLMSG_OK(nlh, data_len)) {
		msg_infec = (struct infec_msg*)NLMSG_DATA(nlh);
		//print_infec_msg(msg_infec);
		//printf("Response type %x\n",msg_infec->header.payload_type);
		if(payload_id){
			switch (msg_infec->header.payload_type)
			{
			case CLIENTS_DATA:
				nr_cl = extract_nr_clients_from_payload((unsigned char*)INF_MSG_DATA(msg_infec));
				//printf("Nr cl %d\n",nr_cl);
				if (nr_cl<0) return NULL;
				else{
					response = (struct kernel_response*)calloc(1,sizeof(struct kernel_response));
					collector = (unsigned char*)malloc(nr_cl*sizeof(struct client_repr*));
					extract_clients_data(msg_infec,(struct client_repr**)collector);
					response->data = (unsigned char*)malloc(sizeof(struct client_repr)*nr_cl);
					copy_uchar_values(collector,response->data,sizeof(struct client_repr)*nr_cl);
					response->type=CLIENTS_DATA;
					response->opt=nr_cl;
					free(collector);
				}
				break;
			case ERROR:
				response = (struct kernel_response*)calloc(1,sizeof(struct kernel_response));
				//collector = (char*)malloc(strlen(INF_MSG_DATA(msg_infec))*sizeof(char));
				response->data = (unsigned char*)malloc(strlen(INF_MSG_DATA(msg_infec)) * sizeof(unsigned char));
				copy_uchar_values(INF_MSG_DATA(msg_infec),response->data,strlen(INF_MSG_DATA(msg_infec)));
				response->type=ERROR;
			case CONFIRM:
				response = (struct kernel_response*)calloc(1,sizeof(struct kernel_response));
				//collector = (char*)malloc(MAX_LEN_CONFIRM*sizeof(char));
				response->data = (unsigned char*)malloc(MAX_LEN_CONFIRM * sizeof(unsigned char));
				copy_uchar_values(INF_MSG_DATA(msg_infec),response->data,MAX_LEN_CONFIRM);
				response->type=CONFIRM;
			default:
				break;
			}
		}
		else{
			switch (msg_infec->header.payload_type){
			//ceva pachet
			case PACKAGE:
				//printf("package 1\n");
				response = (struct kernel_response*)calloc(1,sizeof(struct kernel_response));
				//printf("package 2\n");
				//collector = (char*)malloc(strlen(INF_MSG_DATA(msg_infec))*sizeof(char));
				response->data = (unsigned char*)malloc(msg_infec->header.payload_len * sizeof(unsigned char));
				//printf("package 3\n");
				memcpy(response->data, INF_MSG_DATA(msg_infec), msg_infec->header.payload_len);
				//printf("package 4\n");
				response->type= PACKAGE;
				//printf("package 5\n");
				response->opt = msg_infec->header.payload_len;
				//printf("package 5\n");
				break;
			}

		}
		nlh = NLMSG_NEXT(nlh, data_len);
	}
	//printf("Am iesit\n");
	return response;
}

void print_kernel_response(struct kernel_response *resp){
	int i;
	switch (resp->type)
	{
	case PACKAGE:
		printf("PACK LEN %d \n", resp->opt);
		for(i =0;i< resp->opt;i++){
			printf("%02x ",resp->data[i]);
		}
		printf("\n");
		break;
	
	default:
		printf("Not implemented\n");
		break;
	}
}

//needs rework
struct kernel_response* receive_from_kernel(int payload_id){
	struct nlmsghdr *nh;
	struct ndmsg *ndm;
	struct infec_msg* msg_infec;
	unsigned char* collector;
	struct kernel_response* response = NULL;
	int nr_cl;
	int fd;
	int len;

	fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_PROTO_INFECTED);
	if (fd < 0) {
		perror("Cannot open socket\n");
	}
	//printf("Socket created\n");

	struct sockaddr_nl addr; 
	memset(&addr, 0, sizeof(addr));
	addr.nl_family = AF_NETLINK;
	addr.nl_pid = 0;  // For Linux kernel
	addr.nl_groups = 1; //unicast

	char *buf = (char*)calloc(MAX_SIZE_PAYLOAD,sizeof(char));

    	int res = bind(fd, (struct sockaddr*)&addr, sizeof(addr));
	if(res < 0){
		perror("Error while binding!\n");
		return  NULL;
	}
	//printf("kernel bind created\n");
	len = recv(fd, buf, MAX_SIZE_PAYLOAD, 0);
	if(len<0){
		perror("Error while receiving!\n");
	
	}
	else{
		response = extract_kernel_response((struct nlmsghdr *)buf,len,payload_id);
		//print_kernel_response(response);
	}
	free(buf);
	close(fd);
	return response;
}



struct kernel_response* receive_from_kernel_multicast(){
	struct nlmsghdr *nh;
	struct ndmsg *ndm;
	struct infec_msg* msg_infec;
	unsigned char* collector;
	struct kernel_response* response = NULL;
	struct sockaddr_nl addr; 
	struct nlmsghdr *nl_msghdr;
	struct msghdr msghdr;
	struct iovec iov;
	int nr_cl;
	int fd;
	int len;

	fd = socket(PF_NETLINK, SOCK_RAW, NETLINK_PROTO_INFECTED);
	if (fd < 0) {
		perror("Cannot open socket\n");
	}
	// else 
	// 	printf("Socket created\n");

	
	memset(&addr, 0, sizeof(addr));
	addr.nl_family = AF_NETLINK;
	addr.nl_pid = 0;  // For Linux kernel
	addr.nl_groups = 1; //unicast

	char *buf = (char*)calloc(MAX_SIZE_PAYLOAD,sizeof(char));

    	int res = bind(fd, (struct sockaddr*)&addr, sizeof(addr));
	if(res < 0){
		perror("Error while binding!\n");
		return  NULL;
	}
	// else
	// 	printf("kernel bind created\n");

	nl_msghdr = (struct nlmsghdr*) malloc(NLMSG_SPACE(MAX_SIZE_PAYLOAD));
        memset(nl_msghdr, 0, NLMSG_SPACE(MAX_SIZE_PAYLOAD));

        iov.iov_base = (void*) nl_msghdr;
        iov.iov_len = NLMSG_SPACE(MAX_SIZE_PAYLOAD);

        msghdr.msg_name = (void*) &addr;
        msghdr.msg_namelen = sizeof(addr);
        msghdr.msg_iov = &iov;
        msghdr.msg_iovlen = 1;

        //printf("Waiting to receive message\n");
        len = recvmsg(fd, &msghdr, 0);
	//len = recv(fd, buf, MAX_SIZE_PAYLOAD, 0);
	if(len<0){
		perror("Error while receiving!\n");
	
	}
	else{
		response = extract_kernel_response(nl_msghdr,len,0);
		//print_kernel_response(response);
	}
	free(nl_msghdr);
	free(buf);
	close(fd);
	return response;
}


struct kernel_response* receive_from_kernel_broadcast(){
	struct nlmsghdr *nh;
	struct ndmsg *ndm;
	struct infec_msg* msg_infec;
	unsigned char* collector;
	struct kernel_response* response = NULL;
	int nr_cl;
	int fd;
	int len;

	fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_PROTO_INFECTED);
	if (fd < 0) {
		perror("Cannot open socket\n");
	}
	//printf("Socket created\n");

	struct sockaddr_nl addr; 
	memset(&addr, 0, sizeof(addr));
	addr.nl_family = AF_NETLINK;
	addr.nl_pid = 0;  // For Linux kernel
	addr.nl_groups = 1; //unicast

	char *buf = (char*)calloc(MAX_SIZE_PAYLOAD,sizeof(char));

    	bind(fd, (struct sockaddr*)&addr, sizeof(addr));
	len = recv(fd, buf, MAX_SIZE_PAYLOAD, 0);
	if(len<0){
		perror("Error while receiving!\n");
	}
	else{
		response = extract_kernel_response((struct nlmsghdr *)buf,len,0);
	}
	free(buf);
	return response;
}


void clear_response_kernel(struct kernel_response* response){
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
	int len_data, additional_len;
	struct kernel_response* response = NULL;
	int fd;
	struct sockaddr_nl dest_addr, src_addr; 
	struct nlmsghdr *nlh;
	struct iovec iov; 
	struct msghdr msg;

	struct infec_msg* msg_infec;
	int payload_id;

	fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_PROTO_INFECTED);
	if (fd < 0) {
		perror("Cannot open socket\n");
		return NULL;
	}
	//printf("Socket created\n");
	
	memset(&dest_addr, 0, sizeof(dest_addr));
	dest_addr.nl_family = AF_NETLINK;
	dest_addr.nl_pid = 0;  // For Linux kernel
	dest_addr.nl_groups = 0; //unicast

	memset(&src_addr, 0, sizeof(src_addr));
	src_addr.nl_family = AF_NETLINK;
	src_addr.nl_pid = getpid(); /* self pid */
	src_addr.nl_groups=0; //unicast

	bind(fd, (struct sockaddr*)&src_addr, sizeof(src_addr));
	// printf("inf p %p h %p d %p \n",msg_infec,INF_MSG_HEADER(msg_infec), INF_MSG_DATA(msg_infec));
	// printf("len %ld\n", INF_MSG_LEN(msg_infec));

	msg_infec = create_infec_msg_by_type(data,type);
	if(!msg_infec){
		perror("Cannot create message\n");
		goto cleanup;
	}
	payload_id = msg_infec->header.payload_id;
	nlh = (struct nlmsghdr *) malloc(NLMSG_SPACE(MAX_PAYLOAD_SIZE));
	//memset(nlh, 0, NLMSG_SPACE(MAX_PAYLOAD_SIZE));
	nlh->nlmsg_len = NLMSG_SPACE(MAX_PAYLOAD_SIZE);
	nlh->nlmsg_pid = getpid();
	nlh->nlmsg_flags = 0;

	copy_uchar_values((unsigned char*)msg_infec,(unsigned char *) NLMSG_DATA(nlh), INF_MSG_LEN(msg_infec));
	//printf("nhl dat s %x%x%x%x\n",((unsigned char*)NLMSG_DATA(nlh))[0],((unsigned char *)NLMSG_DATA(nlh))[1],
	//	((unsigned char *)NLMSG_DATA(nlh))[2],((unsigned char *)NLMSG_DATA(nlh))[3]);
	//printf("nhl created\n");
	
	memset(&iov, 0, sizeof(iov));
	iov.iov_base = (void *) nlh;
	iov.iov_len = nlh->nlmsg_len;

		
	memset(&msg, 0, sizeof(msg));
	msg.msg_name = (void *) &dest_addr;
	msg.msg_namelen = sizeof(dest_addr);
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;
	//printf("Message created\n");

	
	len_data = sendmsg(fd, &msg, 0);
	if(len_data < 0){
		perror("Failed to send message to kernel\n");
		goto cleanup;
	}
	printf("Sent message to kernel\n");

	memset(nlh, 0, NLMSG_SPACE(MAX_PAYLOAD_SIZE));
	//char *buf = (char*)calloc(MAX_SIZE_PAYLOAD,sizeof(char));
	len_data = recvmsg(fd, &msg, 0);
	//printf("Received len %d\n", len_data);
	if(len_data < 0){
		perror("Failed to receive message from kernel\n");
		goto cleanup;
	}
	response = extract_kernel_response(nlh,len_data,payload_id);
	// if(response->type == CONFIRM){
	// 	ch2int(response->data,additional_len);

	// }
	
	//free(buf);
	//return msg_infec->header.payload_id;
cleanup:
	clear_infec_msg(msg_infec);
	close(fd);
	return response;
}
