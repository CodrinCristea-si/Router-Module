#include <linux/module.h>
#include <linux/netdevice.h>
#include <linux/inetdevice.h>
#include <uapi/linux/if_ether.h>
#include <linux/list.h>
#include <linux/types.h>

struct network_details{
	__be32 subnet_ip;
	__be32 ip_addr;
	__be32 mask;
	struct list_head list;
};

#define EMPTY_IPV4_ADDRESS 0

#ifndef BIG_ENDIAN
#define BIG_ENDIAN
#endif

#ifdef BIG_ENDIAN 
#define SUBNET_A 0X0000000A
#define MASK_A   0X000000FF
#define SUBNET_B 0X000001AC
#define MASK_B   0X00000FFF
#define SUBNET_C 0X0000A8C0
#define MASK_C   0X0000FFFF
#else
#define SUBNET_A 0X0A000000
#define MASK_A   0XFF000000
#define SUBNET_B 0XAC010000
#define MASK_B   0XFF0F0000
#define SUBNET_C 0XC0A80000
#define MASK_C   0XFFFF0000
#endif

int get_network_interfaces_list(struct network_details* collector);

struct network_details get_lan_network_from_list(struct network_details* collector);

bool check_ip_belong_to_network(struct network_details* network, __be32 ip_add);

inline int copy_mac_address(const unsigned char* from, unsigned char* to);

inline int cmp_mac_address(const unsigned char* mac1, const unsigned char* mac2);

int initialize_network_interfaces_list(struct network_details* collector);

int clear_network_interfaces_list(struct network_details* collector);