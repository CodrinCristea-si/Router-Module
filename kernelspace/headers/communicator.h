#include <linux/module.h>  
#include <linux/kernel.h> 
#include <linux/list.h>
#include <net/sock.h>

#include "infectivity.h"
#include "../common/common_proto.h"

#define MAX_USER_PAYLOAD_SIZE 2048

void send_to_user_clients(struct sock* netlink_socket,struct clients_list* clients_list,int requester, int payload_id);