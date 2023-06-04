#include "communication_kernel.h"
#include "list.h"
#include "communicator_network.h"


#define MAX_FILE_PATH_SIZE 200

#define MAX_BUFFER_SIZE 1024

#define MAX_POOL_SIZE 10
#define NUMBER_OF_WORKERS 2
#define PORT_LISTEN 7895

#ifndef IPV4_SIZE
#define IPV4_SIZE 4
#endif

#ifndef MAC_SIZE
#define MAC_SIZE MAC_LEN
#endif

#define MIN_GRADE 0
enum{
	UNKNOWN=MIN_GRADE,
#define UNKNOWN UNKNOWN
	UNINFECTED,
#define UNINFECTED UNINFECTED
	SUSPICIOUS,
#define SUSPICIOUS SUSPICIOUS
	INFECTED_MINOR,
#define INFECTED_MINOR INFECTED_MINOR
	INFECTED_MAJOR,
#define INFECTED_MAJOR INFECTED_MAJOR
	INFECTED_SEVER
#define INFECTED_SEVER INFECTED_SEVER
};

enum{
	ADD = 1,
	REMOVE,
	TRANSFER,
	GET_ALL,
	GET_UPDATES,
	PACKAGE_RECEIVED
};

enum{
	ALL_DATA = 10,
	UPDATES
};

struct client_infectivity{
	unsigned char ipv4[IPV4_SIZE];
	unsigned char mac[MAC_SIZE];
	unsigned char infectivity;
};

struct client_job{
	unsigned char job_type;
	struct client_infectivity client;
};

struct kernel_job{
	unsigned char job_type;
	void *pack;
};

struct task{
	int len;
	int sender;
	void* job;
};

int start_monitoring(char *filename, struct network_details* main_net);

void print_job(struct client_job *job);

void print_client_infectivity(struct client_infectivity* client);