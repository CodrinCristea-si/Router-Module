
#include "networking.h"
#pragma pack(1)
#define IP_3RD_DEVICE "192.168.1.2"
#define PORT_3RD_DEVICE 5005
enum request_type{
	CLIENT_CONNECT = 1,
	CLIENT_DISCONNECT = 2,
	CLIENT_TRANSFER = 3,
	CLIENT_PACKAGE = 4
}; 

struct network_package{
	unsigned char type;
	void* data;
}__attribute__((packed));

struct network_client_data{
	unsigned char type;
	unsigned char ipv4[4];
	unsigned char mac[6];
	unsigned char infectivity;
} __attribute__((packed));

struct network_package_data{
	unsigned char ip_source[4];
	unsigned char ip_dest[4];
	int port_source;
	int port_destination;
};

struct network_client_data create_package_by_type(unsigned char * data, unsigned char type);

void clear_package(struct network_package *pack);

int send_to_network(unsigned char * data, unsigned char type);