#include "../../headers/common_proto.h"

#define MAX_ID 1e7
#define MAX_PAYLOAD_SIZE 1024
#define SLEEP_TIME 4

int send_message_to_kernel(struct client_repr *client, unsigned char type);