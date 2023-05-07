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

int send_message_to_kernel(struct client_repr *client, unsigned char type){
	
	int fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_PROTO_INFECTED);
	if (fd < 0) {
		perror("Cannot open socket\n");
	}
	printf("Socket created\n");
	struct sockaddr_nl addr; 
	memset(&addr, 0, sizeof(addr));
	addr.nl_family = AF_NETLINK;
	addr.nl_pid = 0;  // For Linux kernel
	addr.nl_groups = 0;
	
	unsigned char *data = (unsigned char *)calloc(20,sizeof(char));
	int len = create_client_repr_payload(client,data,FLAG_WITH_IP|FLAG_WITH_MAC);
	printf("Data created\n");
	// printf("data p %p si %x ip %x.%x.%x.%x sm %x mac %x:%x:%x:%x:%x:%x \n", data, data[0],data[1],data[2],
	// 	data[3],data[4],data[5],data[6],data[7],data[8],data[9],data[10],data[11]);

	struct header_payload *hdr_inf = (struct header_payload *)calloc(1, sizeof(struct header_payload));
	create_header(rand()%(int)(MAX_ID), type,hdr_inf);
	// printf("hdr p %p  s %x%x%x%x t %x i %x\n",hdr_inf,hdr_inf->signiture[0], hdr_inf->signiture[1], hdr_inf->signiture[2],
	// 	hdr_inf->signiture[3], hdr_inf->payload_type, hdr_inf->payload_id);

	printf("Header created\n");

	struct infec_msg* msg_infec = (struct infec_msg *)malloc(INF_MSG_LEN_H(hdr_inf));
	create_message(hdr_inf,data,len,msg_infec);
	// printf("inf p %p h %p d %p \n",msg_infec,INF_MSG_HEADER(msg_infec), INF_MSG_DATA(msg_infec));
	// printf("len %ld\n", INF_MSG_LEN(msg_infec));
	printf("Msg created\n");

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
	printf("Massage created\n");

	
	sendmsg(fd, &msg, 0);
	free(msg_infec);
	free(hdr_inf);
	free(data);
	printf("Sent message to kernel\n");
}
