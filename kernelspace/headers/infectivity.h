#include <linux/module.h>  
#include <linux/kernel.h> 
#include <linux/list.h>
#include <linux/types.h>
#include <uapi/linux/if_ether.h>

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

struct client_def{
	__be32 ip_addr;
	unsigned char mac_addr [ETH_ALEN];
	unsigned char infectivity;
};

struct clients_list{
	struct client_def client;
	struct list_head list;
};

enum{
	LOCKDOWN_UP,
	LOCKDOWN_DOWN
};

extern unsigned char LOCKDOWN_MODE;

extern struct clients_list* uninfected_list;
extern struct clients_list* suspicious_list;
extern struct clients_list* infected_minor_list;
extern struct clients_list* infected_major_list;
extern struct clients_list* infected_sever_list;

int __initialize_infectivity_lists(void);

int __clear_list(struct clients_list* collector);

int __clear_infectivity_lists(void);

int __add_client_to_list(struct clients_list* list, const __be32 client_ip_addr, const unsigned char* client_mac_addr, const unsigned char type);

bool __add_client_generic(const __be32 client_ip_addr, const unsigned char* client_mac_addr);

bool __remove_client_from_list(struct clients_list* list_from,const __be32 client_ip_addr, const unsigned char* client_mac_addr);

bool __remove_client_generic(const __be32 client_ip_addr, const unsigned char* client_mac_addr);

struct client_def* __get_client_from_list(struct clients_list* list_from, const __be32 client_ip_addr, const unsigned char* client_mac_addr);

struct client_def* __get_client_from_list_by_mac(struct clients_list* list_from, const unsigned char* client_mac_addr);

struct client_def* __get_client_generic(const __be32 client_ip_addr, const unsigned char* client_mac_addr);

int __get_all_clients(struct clients_list* list_collector);

int __get_size_of_clients_list(struct clients_list* cl_list);

struct clients_list* __get_list_according_to_status(unsigned char status);

bool __transfer_client(unsigned char status_from, unsigned char status_to, const __be32 ip_addr, const unsigned char* mac_addr);

bool __transfer_client_generic( unsigned char status_to, const __be32 ip_addr, const unsigned char* mac_addr);

inline bool __check_status_validity(int status);

inline struct client_def* __create_empty_client(void);

inline struct clients_list* __create_empty_list(void);

void __print_list(struct clients_list* list);

#define CHECH_STATUS(STATUS) __check_status_validity(STATUS)

#define PRINT_LIST_UNINFECTED() __print_list(uninfected_list)
#define PRINT_LIST_SUSPICIOUS() __print_list(suspicious_list)
#define PRINT_LIST_INFECTED_MINOR() __print_list(infected_minor_list)
#define PRINT_LIST_INFECTED_MAJOR() __print_list(infected_major_list)
#define PRINT_LIST_INFECTED_SEVER() __print_list(infected_sever_list)

#define ADD_CLIENT_GENERIC(IP_ADDR, MAC_ADDR) __add_client_generic(IP_ADDR, MAC_ADDR)

#define ADD_CLIENT_UNINFECTED(IP_ADDR, MAC_ADDR) __add_client_to_list(uninfected_list, IP_ADDR, MAC_ADDR, UNINFECTED)
#define ADD_CLIENT_SUSPICIOUS(IP_ADDR, MAC_ADDR) __add_client_to_list(suspicious_list, IP_ADDR, MAC_ADDR, SUSPICIOUS)
#define ADD_CLIENT_INFECTED_MINOR(IP_ADDR, MAC_ADDR) __add_client_to_list(infected_minor_list, IP_ADDR, MAC_ADDR, INFECTED_MINOR)
#define ADD_CLIENT_INFECTED_MAJOR(IP_ADDR, MAC_ADDR) __add_client_to_list(infected_major_list, IP_ADDR, MAC_ADDR, INFECTED_MAJOR)
#define ADD_CLIENT_INFECTED_SEVER(IP_ADDR, MAC_ADDR) __add_client_to_list(infected_sever_list, IP_ADDR, MAC_ADDR, INFECTED_SEVER)

#define REMOVE_CLIENT_GENERIC(IP_ADDR, MAC_ADDR) __remove_client_generic(IP_ADDR, MAC_ADDR)

#define REMOVE_CLIENT_UNINFECTED(IP_ADDR, MAC_ADDR) __remve_client_from_list(uninfected_list, IP_ADDR, MAC_ADDR)
#define REMOVE_CLIENT_SUSPICIOUS(IP_ADDR, MAC_ADDR) __remve_client_from_list(suspicious_list, IP_ADDR, MAC_ADDR)
#define REMOVE_CLIENT_INFECTED_MINOR(IP_ADDR, MAC_ADDR) __remve_client_from_list(infected_minor_list, IP_ADDR, MAC_ADDR)
#define REMOVE_CLIENT_INFECTED_MAJOR(IP_ADDR, MAC_ADDR) __remve_client_from_list(infected_major_list, IP_ADDR, MAC_ADDR)
#define REMOVE_CLIENT_INFECTED_SEVER(IP_ADDR, MAC_ADDR) __remve_client_from_list(infected_sever_list, IP_ADDR, MAC_ADDR)

#define GET_CLIENT_GENERIC(IP_ADDR, MAC_ADDR) __get_client_generic(IP_ADDR, MAC_ADDR)

#define GET_CLIENT_UNINFECTED(IP_ADDR, MAC_ADDR) __get_client_from_list(uninfected_list, IP_ADDR, MAC_ADDR)
#define GET_CLIENT_SUSPICIOUS(IP_ADDR, MAC_ADDR) __get_client_from_list(suspicious_list, IP_ADDR, MAC_ADDR)
#define GET_CLIENT_INFECTED_MINOR(IP_ADDR, MAC_ADDR) __get_client_from_list(infected_minor_list, IP_ADDR, MAC_ADDR)
#define GET_CLIENT_INFECTED_MAJOR(IP_ADDR, MAC_ADDR) __get_client_from_list(infected_major_list, IP_ADDR, MAC_ADDR)
#define GET_CLIENT_INFECTED_SEVER(IP_ADDR, MAC_ADDR) __get_client_from_list(infected_sever_list, IP_ADDR, MAC_ADDR)

#define GET_CLIENT_UNINFECTED_BY_MAC(MAC_ADDR) __get_client_from_list_by_mac(uninfected_list, MAC_ADDR)
#define GET_CLIENT_SUSPICIOUS_BY_MAC(MAC_ADDR) __get_client_from_list_by_mac(suspicious_list, MAC_ADDR)
#define GET_CLIENT_INFECTED_MINOR_BY_MAC(MAC_ADDR) __get_client_from_list_by_mac(infected_minor_list, MAC_ADDR)
#define GET_CLIENT_INFECTED_MAJOR_BY_MAC(MAC_ADDR) __get_client_from_list_by_mac(infected_major_list, MAC_ADDR)
#define GET_CLIENT_INFECTED_SEVER_BY_MAC(MAC_ADDR) __get_client_from_list_by_mac(infected_sever_list, MAC_ADDR)

#define GET_ALL_CLIENTS(collector) __get_all_clients(collector)

#define TRANSFER_CLIENT_BETWEEN(FROM,TO,IP_ADDR, MAC_ADDR) __transfer_client(FROM,TO,IP_ADDR, MAC_ADDR);
#define TRANSFER_CLIENT_GENERIC(TO,IP_ADDR, MAC_ADDR) __transfer_client_generic(TO,IP_ADDR, MAC_ADDR);

#define IS_LOCKDOWN_MODE_ENABLED (LOCKDOWN_MODE == LOCKDOWN_UP)
#define DISABLE_LOCKDOWN_MODE() __disable_lockdown_mode()
#define ENABLE_LOCKDOWN_MODE() __enable_lockdown_mode()

static inline int __enable_lockdown_mode(void){
	LOCKDOWN_MODE = LOCKDOWN_UP;
	return 0;
}

static inline int __disable_lockdown_mode(void){
	LOCKDOWN_MODE = LOCKDOWN_DOWN;
	return 0;
}


