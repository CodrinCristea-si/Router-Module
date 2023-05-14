#include <linux/module.h>  
#include <linux/kernel.h> 
#include <linux/list.h>
#include <net/sock.h>

#include "infectivity.h"
#include "../common/common_proto.h"

#define MAX_USER_PAYLOAD_SIZE MAX_PAYLOAD_SIZE

void print_infec_msg(struct infec_msg * msg_infec);

void send_to_user_unicast(struct sock* netlink_socket,unsigned char* clients_list,unsigned char type,int requester, int payload_id);