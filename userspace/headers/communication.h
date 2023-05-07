#include "../../common/common_proto.h"

#define MAX_ID 1e7
#define MAX_PAYLOAD_SIZE 1024
#define SLEEP_TIME 4

struct infec_msg* create_add_client_msg(struct client_repr *client, unsigned char type);

struct infec_msg* create_infec_msg_by_type(unsigned char* data, unsigned char type);

void clear_infec_msg(struct infec_msg * msg_infec);

int send_message_to_kernel(unsigned char* data, unsigned char type);