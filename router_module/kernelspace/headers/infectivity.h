#ifndef ___INFECTIVITY_INTERCEPTOR_H___
#define ___INFECTIVITY_INTERCEPTOR_H___

#include <linux/module.h>  
#include <linux/kernel.h> 
#include <linux/list.h>
#include <linux/types.h>
#include <uapi/linux/if_ether.h>

#define MIN_GRADE 0
enum{
	UNKNOWN=MIN_GRADE,     //0
#define UNKNOWN UNKNOWN
	UNINFECTED,            //1
#define UNINFECTED UNINFECTED
	SUSPICIOUS,            //2
#define SUSPICIOUS SUSPICIOUS
	INFECTED_MINOR,        //3
#define INFECTED_MINOR INFECTED_MINOR
	INFECTED_MAJOR,        //4
#define INFECTED_MAJOR INFECTED_MAJOR
	INFECTED_SEVER         //5
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
extern struct mutex* inf_mutex;
extern struct mutex* lockdown_mutex;

int __initialize_infectivity_lists(void);

int __clear_list(struct clients_list* collector);

int __clear_infectivity_lists(void);

int __add_client_to_list(struct clients_list* list, const __be32 client_ip_addr, const unsigned char* client_mac_addr, const unsigned char type, struct mutex* inf_mutex);

bool __add_client_generic(const __be32 client_ip_addr, const unsigned char* client_mac_addr, struct mutex* inf_mutex);

bool __remove_client_from_list(struct clients_list* list_from,const __be32 client_ip_addr, const unsigned char* client_mac_addr, struct mutex* inf_mutex);

bool __remove_client_generic(const __be32 client_ip_addr, const unsigned char* client_mac_addr, struct mutex* inf_mutex);

struct client_def* __get_client_from_list(struct clients_list* list_from, const __be32 client_ip_addr, const unsigned char* client_mac_addr, struct client_def* client_collector, struct mutex* inf_mutex);

struct client_def* __get_client_from_list_by_mac(struct clients_list* list_from, const unsigned char* client_mac_addr, struct client_def* client_collector, struct mutex* inf_mutex);

struct client_def* __get_client_generic(const __be32 client_ip_addr, const unsigned char* client_mac_addr, struct client_def* client_collector, struct mutex* inf_mutex);

int __get_all_clients(struct clients_list* list_collector, struct mutex* inf_mutex);

int __get_size_of_clients_list(struct clients_list* cl_list, struct mutex* inf_mutex);

struct clients_list* __get_list_according_to_status(unsigned char status);

bool __transfer_client(unsigned char status_from, unsigned char status_to, const __be32 ip_addr, const unsigned char* mac_addr, struct mutex* inf_mutex);

bool __transfer_client_generic( unsigned char status_to, const __be32 ip_addr, const unsigned char* mac_addr, struct mutex* inf_mutex);

inline bool __check_status_validity(int status);

inline struct client_def* __create_empty_client(void);

inline struct clients_list* __create_empty_list(void);

void __print_list(struct clients_list* list, struct mutex* inf_mutex);

#define CHECH_STATUS(STATUS) __check_status_validity(STATUS)

///not thread safe

#define PRINT_LIST_UNINFECTED() __print_list(uninfected_list, NULL)
#define PRINT_LIST_SUSPICIOUS() __print_list(suspicious_list, NULL)
#define PRINT_LIST_INFECTED_MINOR() __print_list(infected_minor_list, NULL)
#define PRINT_LIST_INFECTED_MAJOR() __print_list(infected_major_list, NULL)
#define PRINT_LIST_INFECTED_SEVER() __print_list(infected_sever_list, NULL)

#define ADD_CLIENT_GENERIC(IP_ADDR, MAC_ADDR) __add_client_generic(IP_ADDR, MAC_ADDR, NULL)

#define ADD_CLIENT_UNINFECTED(IP_ADDR, MAC_ADDR) __add_client_to_list(uninfected_list, IP_ADDR, MAC_ADDR, UNINFECTED, NULL)
#define ADD_CLIENT_SUSPICIOUS(IP_ADDR, MAC_ADDR) __add_client_to_list(suspicious_list, IP_ADDR, MAC_ADDR, SUSPICIOUS, NULL)
#define ADD_CLIENT_INFECTED_MINOR(IP_ADDR, MAC_ADDR) __add_client_to_list(infected_minor_list, IP_ADDR, MAC_ADDR, INFECTED_MINOR, NULL)
#define ADD_CLIENT_INFECTED_MAJOR(IP_ADDR, MAC_ADDR) __add_client_to_list(infected_major_list, IP_ADDR, MAC_ADDR, INFECTED_MAJOR, NULL)
#define ADD_CLIENT_INFECTED_SEVER(IP_ADDR, MAC_ADDR) __add_client_to_list(infected_sever_list, IP_ADDR, MAC_ADDR, INFECTED_SEVER, NULL)

#define REMOVE_CLIENT_GENERIC(IP_ADDR, MAC_ADDR) __remove_client_generic(IP_ADDR, MAC_ADDR, NULL)

#define REMOVE_CLIENT_UNINFECTED(IP_ADDR, MAC_ADDR) __remve_client_from_list(uninfected_list, IP_ADDR, MAC_ADDR, NULL)
#define REMOVE_CLIENT_SUSPICIOUS(IP_ADDR, MAC_ADDR) __remve_client_from_list(suspicious_list, IP_ADDR, MAC_ADDR, NULL)
#define REMOVE_CLIENT_INFECTED_MINOR(IP_ADDR, MAC_ADDR) __remve_client_from_list(infected_minor_list, IP_ADDR, MAC_ADDR, NULL)
#define REMOVE_CLIENT_INFECTED_MAJOR(IP_ADDR, MAC_ADDR) __remve_client_from_list(infected_major_list, IP_ADDR, MAC_ADDR, NULL)
#define REMOVE_CLIENT_INFECTED_SEVER(IP_ADDR, MAC_ADDR) __remve_client_from_list(infected_sever_list, IP_ADDR, MAC_ADDR, NULL)

#define GET_CLIENT_GENERIC(IP_ADDR, MAC_ADDR, CLIENT) __get_client_generic(IP_ADDR, MAC_ADDR, CLIENT, NULL)

#define GET_CLIENT_UNINFECTED(IP_ADDR, MAC_ADDR, CLIENT) __get_client_from_list(uninfected_list, IP_ADDR, MAC_ADDR, CLIENT, NULL)
#define GET_CLIENT_SUSPICIOUS(IP_ADDR, MAC_ADDR, CLIENT) __get_client_from_list(suspicious_list, IP_ADDR, MAC_ADDR, CLIENT, NULL)
#define GET_CLIENT_INFECTED_MINOR(IP_ADDR, MAC_ADDR, CLIENT) __get_client_from_list(infected_minor_list, IP_ADDR, MAC_ADDR, CLIENT, NULL)
#define GET_CLIENT_INFECTED_MAJOR(IP_ADDR, MAC_ADDR, CLIENT) __get_client_from_list(infected_major_list, IP_ADDR, MAC_ADDR, CLIENT, NULL)
#define GET_CLIENT_INFECTED_SEVER(IP_ADDR, MAC_ADDR, CLIENT) __get_client_from_list(infected_sever_list, IP_ADDR, MAC_ADDR, CLIENT, NULL)

#define GET_CLIENT_UNINFECTED_BY_MAC(MAC_ADDR, CLIENT) __get_client_from_list_by_mac(uninfected_list, MAC_ADDR, CLIENT, NULL)
#define GET_CLIENT_SUSPICIOUS_BY_MAC(MAC_ADDR, CLIENT) __get_client_from_list_by_mac(suspicious_list, MAC_ADDR, CLIENT, NULL)
#define GET_CLIENT_INFECTED_MINOR_BY_MAC(MAC_ADDR, CLIENT) __get_client_from_list_by_mac(infected_minor_list, MAC_ADDR, CLIENT, NULL)
#define GET_CLIENT_INFECTED_MAJOR_BY_MAC(MAC_ADDR, CLIENT) __get_client_from_list_by_mac(infected_major_list, MAC_ADDR, CLIENT, NULL)
#define GET_CLIENT_INFECTED_SEVER_BY_MAC(MAC_ADDR, CLIENT) __get_client_from_list_by_mac(infected_sever_list, MAC_ADDR, CLIENT, NULL)

#define GET_ALL_CLIENTS(collector) __get_all_clients(collector, NULL)

#define TRANSFER_CLIENT_BETWEEN(FROM,TO,IP_ADDR, MAC_ADDR) __transfer_client(FROM,TO,IP_ADDR, MAC_ADDR, NULL);
#define TRANSFER_CLIENT_GENERIC(TO,IP_ADDR, MAC_ADDR) __transfer_client_generic(TO,IP_ADDR, MAC_ADDR, NULL);

///thread safe 

#define PRINT_LIST_UNINFECTED_SAFE() __print_list(uninfected_list, inf_mutex)
#define PRINT_LIST_SUSPICIOUS_SAFE() __print_list(suspicious_list, inf_mutex)
#define PRINT_LIST_INFECTED_MINOR_SAFE() __print_list(infected_minor_list, inf_mutex)
#define PRINT_LIST_INFECTED_MAJOR_SAFE() __print_list(infected_major_list, inf_mutex)
#define PRINT_LIST_INFECTED_SEVER_SAFE() __print_list(infected_sever_list, inf_mutex)

#define ADD_CLIENT_GENERIC_SAFE(IP_ADDR, MAC_ADDR) __add_client_generic(IP_ADDR, MAC_ADDR, inf_mutex)

#define ADD_CLIENT_UNINFECTED_SAFE(IP_ADDR, MAC_ADDR) __add_client_to_list(uninfected_list, IP_ADDR, MAC_ADDR, UNINFECTED, inf_mutex)
#define ADD_CLIENT_SUSPICIOUS_SAFE(IP_ADDR, MAC_ADDR) __add_client_to_list(suspicious_list, IP_ADDR, MAC_ADDR, SUSPICIOUS, inf_mutex)
#define ADD_CLIENT_INFECTED_MINOR_SAFE(IP_ADDR, MAC_ADDR) __add_client_to_list(infected_minor_list, IP_ADDR, MAC_ADDR, INFECTED_MINOR, inf_mutex)
#define ADD_CLIENT_INFECTED_MAJOR_SAFE(IP_ADDR, MAC_ADDR) __add_client_to_list(infected_major_list, IP_ADDR, MAC_ADDR, INFECTED_MAJOR, inf_mutex)
#define ADD_CLIENT_INFECTED_SEVER_SAFE(IP_ADDR, MAC_ADDR) __add_client_to_list(infected_sever_list, IP_ADDR, MAC_ADDR, INFECTED_SEVER, inf_mutex)

#define REMOVE_CLIENT_GENERIC_SAFE(IP_ADDR, MAC_ADDR) __remove_client_generic(IP_ADDR, MAC_ADDR, inf_mutex)

#define REMOVE_CLIENT_UNINFECTED_SAFE(IP_ADDR, MAC_ADDR) __remve_client_from_list(uninfected_list, IP_ADDR, MAC_ADDR, inf_mutex)
#define REMOVE_CLIENT_SUSPICIOUS_SAFE(IP_ADDR, MAC_ADDR) __remve_client_from_list(suspicious_list, IP_ADDR, MAC_ADDR, inf_mutex)
#define REMOVE_CLIENT_INFECTED_MINOR_SAFE(IP_ADDR, MAC_ADDR) __remve_client_from_list(infected_minor_list, IP_ADDR, MAC_ADDR, inf_mutex)
#define REMOVE_CLIENT_INFECTED_MAJOR_SAFE(IP_ADDR, MAC_ADDR) __remve_client_from_list(infected_major_list, IP_ADDR, MAC_ADDR, inf_mutex)
#define REMOVE_CLIENT_INFECTED_SEVER_SAFE(IP_ADDR, MAC_ADDR) __remve_client_from_list(infected_sever_list, IP_ADDR, MAC_ADDR, inf_mutex)

#define GET_CLIENT_GENERIC_SAFE(IP_ADDR, MAC_ADDR, CLIENT) __get_client_generic(IP_ADDR, MAC_ADDR, CLIENT, inf_mutex)

#define GET_CLIENT_UNINFECTED_SAFE(IP_ADDR, MAC_ADDR, CLIENT) __get_client_from_list(uninfected_list, IP_ADDR, MAC_ADDR, CLIENT, inf_mutex)
#define GET_CLIENT_SUSPICIOUS_SAFE(IP_ADDR, MAC_ADDR, CLIENT) __get_client_from_list(suspicious_list, IP_ADDR, MAC_ADDR, CLIENT, inf_mutex)
#define GET_CLIENT_INFECTED_MINOR_SAFE(IP_ADDR, MAC_ADDR, CLIENT) __get_client_from_list(infected_minor_list, IP_ADDR, MAC_ADDR, CLIENT, inf_mutex)
#define GET_CLIENT_INFECTED_MAJOR_SAFE(IP_ADDR, MAC_ADDR, CLIENT) __get_client_from_list(infected_major_list, IP_ADDR, MAC_ADDR, CLIENT, inf_mutex)
#define GET_CLIENT_INFECTED_SEVER_SAFE(IP_ADDR, MAC_ADDR, CLIENT) __get_client_from_list(infected_sever_list, IP_ADDR, MAC_ADDR, CLIENT, inf_mutex)

#define GET_CLIENT_UNINFECTED_BY_MAC_SAFE(MAC_ADDR, CLIENT) __get_client_from_list_by_mac(uninfected_list, MAC_ADDR, CLIENT, inf_mutex)
#define GET_CLIENT_SUSPICIOUS_BY_MAC_SAFE(MAC_ADDR, CLIENT) __get_client_from_list_by_mac(suspicious_list, MAC_ADDR, CLIENT, inf_mutex)
#define GET_CLIENT_INFECTED_MINOR_BY_MAC_SAFE(MAC_ADDR, CLIENT) __get_client_from_list_by_mac(infected_minor_list, MAC_ADDR, CLIENT, inf_mutex)
#define GET_CLIENT_INFECTED_MAJOR_BY_MAC_SAFE(MAC_ADDR, CLIENT) __get_client_from_list_by_mac(infected_major_list, MAC_ADDR, CLIENT, inf_mutex)
#define GET_CLIENT_INFECTED_SEVER_BY_MAC_SAFE(MAC_ADDR, CLIENT) __get_client_from_list_by_mac(infected_sever_list, MAC_ADDR, CLIENT, inf_mutex)

#define GET_ALL_CLIENTS_SAFE(collector) __get_all_clients(collector, inf_mutex)

#define TRANSFER_CLIENT_BETWEEN_SAFE(FROM,TO,IP_ADDR, MAC_ADDR) __transfer_client(FROM,TO,IP_ADDR, MAC_ADDR, inf_mutex);
#define TRANSFER_CLIENT_GENERIC_SAFE(TO,IP_ADDR, MAC_ADDR) __transfer_client_generic(TO,IP_ADDR, MAC_ADDR, inf_mutex);

///LOCKDOWN

#define IS_LOCKDOWN_MODE_ENABLED() __is_lockdown_mode()
#define DISABLE_LOCKDOWN_MODE() __disable_lockdown_mode()
#define ENABLE_LOCKDOWN_MODE() __enable_lockdown_mode()

static inline bool __is_lockdown_mode(void){
	bool is_lockdown = false;
	if(lockdown_mutex) mutex_lock(lockdown_mutex);
	is_lockdown = LOCKDOWN_MODE == LOCKDOWN_UP ? true : false;
	if(lockdown_mutex) mutex_unlock(lockdown_mutex);
	return is_lockdown;
}

static inline int __enable_lockdown_mode(void){
	if(lockdown_mutex) mutex_lock(lockdown_mutex);
	LOCKDOWN_MODE = LOCKDOWN_UP;
	if(lockdown_mutex) mutex_unlock(lockdown_mutex);
	return 0;
}

static inline int __disable_lockdown_mode(void){
	if(lockdown_mutex) mutex_lock(lockdown_mutex);
	LOCKDOWN_MODE = LOCKDOWN_DOWN;
	if(lockdown_mutex) mutex_unlock(lockdown_mutex);
	return 0;
}



#endif