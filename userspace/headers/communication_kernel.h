#include "../../common/common_proto.h"
#include <linux/netlink.h>

#define MAX_ID 1e7
#define SLEEP_TIME 4

struct kernel_response{
	unsigned char type;
	int opt;
	unsigned char *data;
};

struct infec_msg* create_add_client_msg(struct client_repr *client, unsigned char type);

struct infec_msg* create_get_clients_msg(unsigned char type);

struct infec_msg* create_infec_msg_by_type(unsigned char* data, unsigned char type);

void clear_infec_msg(struct infec_msg * msg_infec);

int send_message_to_kernel(unsigned char* data, unsigned char type);

int extract_nr_clients_from_payload(unsigned char* data);

void extract_clients_data(struct infec_msg* msg, struct client_repr ** collector);

struct kernel_response *extract_kernel_response(struct nlmsghdr* nlh,int data_len, int payload_id);

struct kernel_response* receive_from_kernel(int payload_id);

void clear_response_kernel(struct kernel_response* response);

struct kernel_response* send_and_receive_kernel(unsigned char* data, unsigned char type);
