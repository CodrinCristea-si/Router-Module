#include <linux/if_ether.h> // for ETH_ALEN
#pragma pack(1) 
int receive_data(int sockfd,unsigned char* buf);

int send_data(int sockfd,unsigned char* buf, int size_data);

int send_data_udp(int sockfd,unsigned char* buf, int size_data);