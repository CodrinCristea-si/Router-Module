
#ifndef __RULES_INTERCEPTOR_H___
#define __RULES_INTERCEPTOR_H___

#include <linux/module.h>  
#include <linux/kernel.h> 
#include <linux/list.h>
#include <linux/random.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/udp.h>
#include <linux/inet.h>

extern int banned_ports_minor[];
extern int size_banned_ports_minor;
extern int banned_ports_major[];
extern int size_banned_ports_major;

__be32 get_special_client(struct network_details* lan);

bool check_if_not_belonged_to_router(struct sk_buff *skb, struct network_details* lan);

bool check_for_special_clients(struct sk_buff *skb, struct network_details* lan);

bool check_for_spcecial_client_test(struct sk_buff *skb, struct network_details* lan);

bool check_if_outside_network_source(struct sk_buff *skb, struct network_details* lan);

bool check_if_outside_network_destination(struct sk_buff *skb, struct network_details* lan);

bool check_if_outside_network(struct sk_buff *skb, struct network_details* lan);

bool check_if_a_test_message(struct sk_buff *skb, struct network_details* lan);

bool check_if_client_can_send_message(struct sk_buff *skb, struct network_details* lan, struct client_def *client);

bool check_if_client_can_receive_message(struct sk_buff *skb, struct network_details* lan, struct client_def *client);

unsigned char* create_pack_based_on_infectivity( struct sk_buff *skb, struct client_def *client, unsigned int* col_size);

#endif