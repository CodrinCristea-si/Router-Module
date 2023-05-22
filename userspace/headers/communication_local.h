#include "infectivity_monitor.h"
#include "networking.h"


struct response{
	int nr_ent;
	List* data;
};

struct job* create_job(unsigned char* data, unsigned char type);

int send_to_monitor(unsigned char*data, unsigned char type);

struct response receive_from_monitor(int *sockfd);

struct response send_and_receive_from_monitor(unsigned char*data, unsigned char type);

void clear_response(struct response* response, bool is_static);