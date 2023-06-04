#include <linux/module.h>  
#include <linux/kernel.h> 
#include <linux/list.h>
#include <linux/skbuff.h>
#include <net/sock.h>

#include "infectivity.h"
#include "../common/common_proto.h"

#define MAX_USER_PAYLOAD_SIZE MAX_PAYLOAD_SIZE
#define SERVER_PORT 7895

void print_infec_msg(struct infec_msg * msg_infec);

void print_package_data(struct package_data* pack);

void print_package_data_v2(unsigned char* pack, int size, bool with_data);

void clear_package_data(struct package_data* pack, bool with_data);

void clear_package_data_v2(unsigned char* pack, bool with_data);

//int get_size_of_package_data(struct package_data* pack);

struct package_data* create_package_data(struct sk_buff* skb, bool with_data); //not working, pls do not use

unsigned char* create_package_data_v2(struct sk_buff* skb, int *col_size, bool with_data);

void send_to_user_server(unsigned char* data,int data_size);

//void send_to_user_package(struct sock* netlink_socket,unsigned char* data,int data_size);

void send_to_user_broadcast(struct sock* netlink_socket,unsigned char* data, int size, unsigned char type, int payload_id, struct mutex *nl_mutex);