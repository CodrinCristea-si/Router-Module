
#ifndef __INFECTIVITY_MONITOR_H__
#define __INFECTIVITY_MONITOR_H__

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

#define MIN_NR_CLIENT_LOCKDOWN 4
#define MIN_THRESHOLD_LOCKDOWN 75

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
	PACKAGE_RECEIVED,
	AUTO_UP,
	AUTO_DOWN,
	LOCKDOWN_UP,
	LOCKDOWN_DOWN,
	
};

enum{
	ALL_DATA = 20,
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

struct ui_job{
	unsigned char job_type;
	void *ui_data;
};

struct task{
	int len;
	int sender;
	void* job;
};

int start_monitoring(char *filename, struct network_details* main_net);

void print_job(struct client_job *job);

void print_client_infectivity(struct client_infectivity* client);

bool is_lockdown_job(unsigned char job_type);

unsigned char get_job_type_from_data(unsigned char *data);

unsigned char is_ui_job(unsigned char job_type);

#endif