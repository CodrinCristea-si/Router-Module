
#include "networking.h"
#pragma pack(1)
#define IP_3RD_DEVICE "192.168.1.2"
#define PORT_3RD_DEVICE 5005
#define PORT_UDP_3RD_DEVICE 5006

enum request_type{
	CLIENT_CONNECT = 1,
	CLIENT_DISCONNECT = 2,
	CLIENT_TRANSFER = 3,
	CLIENT_PACKAGE = 4,
	LOCKDOWN_ENABLED = 5,
	LOCKDOWN_DISABLED = 6,
	AUTOMATIC_ENABLED = 7,
	AUTOMATIC_DISABLED = 8
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

int create_udp_socket(struct sockaddr_in* servaddr_col);

int send_to_network_udp(int sockfd, struct sockaddr_in* servaddr, unsigned char* data,unsigned int size);

void destroy_udp_socket(int sockfd);