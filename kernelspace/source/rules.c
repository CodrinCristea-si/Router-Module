
#ifndef __RULES_INTERCEPTOR_C___
#define __RULES_INTERCEPTOR_C___

#include <linux/module.h>  
#include <linux/kernel.h> 
#include <linux/list.h>
#include <linux/random.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/udp.h>
#include <linux/inet.h>

#include "../headers/utils.h"
#include "../headers/infectivity.h"
#include "../headers/communicator.h"
#include "../headers/rules.h"

int banned_ports_minor[] ={111, 135, 119, 433, 514, 530, 563, 991};
int size_banned_ports_minor = 8; 

int banned_ports_major[] = {5,7, 9, 11, 17,18,19, 20,21, 22, 23, 25, 42, 43, 69, 93, 107, 111,113, 115, 119, 135, 161, 162, 180, 199, 389, 427, 433, 514,
			     517, 530, 563, 593, 601, 666, 830, 831, 832,833, 873, 989, 990, 992, 1194, 1293};
int size_banned_ports_major = 46; 

bool check_if_not_belonged_to_router(struct sk_buff *skb, struct network_details* lan){
	struct iphdr* ip_h;
	__be32 tmp = LOOPBACK_IP;
	if (!skb) return true;
	ip_h = ip_hdr(skb);
	if(!ip_h) return true;
	//printk(KERN_WARNING "CMP %pI4 %pI4 %pI4 \n",&ip_h->saddr, &tmp, &lan.ip_addr);
	if (ip_h->saddr == tmp || ip_h->saddr == lan->ip_addr)
		return true;
	return false;
}

bool check_if_outside_network_source(struct sk_buff *skb, struct network_details* lan){
	struct iphdr* ip_h;
	if (!skb || !lan) return true;
	ip_h = ip_hdr(skb);
	if(!ip_h) return true;
	//printk(KERN_WARNING "CMP %pI4 %pI4 %pI4 \n",&ip_h->saddr, &tmp, &lan.ip_addr);
	if (check_ip_belong_to_network(lan,ip_h->saddr))
		return false;
	return true;
}

bool check_for_special_clients(struct sk_buff *skb, struct network_details* lan){
	bool is_special = false;
	struct iphdr* ip_h;
	if (!skb || !lan) return true;
	ip_h = ip_hdr(skb);
	if(!ip_h) return true;
#ifdef __LITTLE_ENDIAN_BITFIELD
	if (ip_h->saddr == (lan->ip_addr + (1<<18)) || ip_h->daddr == (lan->ip_addr + (1<<18))) //3rd_device
		is_special = true;
#endif
#ifdef __BIG_ENDIAN_BITFIELD
	if (ip_h->saddr == (lan->ip_addr + 1) || ip_h->daddr == (lan->ip_addr + 1)) //3rd_device
		is_special = true;
#endif
	return is_special;
};

bool check_if_outside_network_destination(struct sk_buff *skb, struct network_details* lan){
	struct iphdr* ip_h;
	if (!skb || !lan) return true;
	ip_h = ip_hdr(skb);
	if(!ip_h) return true;
	//printk(KERN_WARNING "CMP %pI4 %pI4 %pI4 \n",&ip_h->saddr, &tmp, &lan.ip_addr);
	if (check_ip_belong_to_network(lan,ip_h->daddr))
		return false;
	return true;
}

bool check_if_outside_network(struct sk_buff *skb, struct network_details* lan){
	struct iphdr* ip_h;
	if (!skb || !lan) return true;
	ip_h = ip_hdr(skb);
	if(!ip_h) return true;
	printk(KERN_WARNING "CMP %pI4 %pI4 %pI4 \n",&ip_h->saddr, &ip_h->daddr, &lan->ip_addr);
	if (check_ip_belong_to_network(lan,ip_h->saddr) || check_ip_belong_to_network(lan,ip_h->daddr)) 
		return false;
	return true;
}

int get_inf_status_for_destination_client(struct sk_buff *skb){
	struct iphdr* ip_h;
	struct ethhdr *mac_header;
	struct client_def *client = NULL, *res_cl;
	int infec = -1;
	if (!skb) return -1;
	ip_h = ip_hdr(skb);
	if(!ip_h) return -1;
	mac_header = eth_hdr(skb);
	if(!mac_header) return -1;

	client = (struct client_def *)kcalloc(1, sizeof(struct client_def), GFP_KERNEL);
	res_cl = GET_CLIENT_GENERIC_SAFE(ip_h->daddr,mac_header->h_dest, client);
	if(client){
		infec = client->infectivity;
	}
	else infec = -1;
	kfree(client);
	return infec;
}

int get_inf_status_for_source_client(struct sk_buff *skb){
	struct iphdr* ip_h;
	struct ethhdr *mac_header;
	struct client_def *client = NULL, *res_cl;
	int infec = -1;
	if (!skb) return -1;
	ip_h = ip_hdr(skb);
	if(!ip_h) return -1;
	mac_header = eth_hdr(skb);
	if(!mac_header) return -1;

	client = (struct client_def *)kcalloc(1, sizeof(struct client_def), GFP_KERNEL);
	res_cl = GET_CLIENT_GENERIC_SAFE(ip_h->saddr,mac_header->h_source,client);
	if(client){
		infec = client->infectivity;
	}
	else infec = -1;
	kfree(client);
	return infec;
}

bool check_if_broadcast_message(struct sk_buff *skb, struct network_details* lan){
	struct iphdr* ip_h;
	ip_h = ip_hdr(skb);
	if(check_if_broadcast(lan,ip_h->daddr))
		return true;
	return false;
}

bool is_ok_status_for_inf_minor(int inf_status){
	return inf_status == UNINFECTED || inf_status == INFECTED_MINOR ||
		inf_status == INFECTED_MAJOR;
}

bool is_ok_status_for_inf_major(int inf_status){
	return inf_status == INFECTED_MINOR || inf_status == INFECTED_MAJOR;
}

int get_destination_port(struct sk_buff *skb){
	int dest_port = -1;
	struct iphdr* ip_h;
	ip_h = ip_hdr(skb);
	struct tcphdr* tcp_h;
	struct udphdr* udp_h;
	//printk(KERN_INFO "6\n");
	if (ip_h->protocol == IPPROTO_TCP){
		tcp_h = tcp_hdr(skb);
		dest_port = tcp_h->dest;
	}
	else if (ip_h->protocol == IPPROTO_UDP){
		udp_h = udp_hdr(skb);
		dest_port = udp_h->dest;
	}
	return dest_port;
}

bool check_if_banned_port(int port, int *banned_ports, int size_list){
	unsigned int i= 0;
	for(i = 0 ;i< size_list;i++){
		if(banned_ports[i] == port)
			return true;
	}
	return false;
}

bool check_for_icmp_protocol(struct sk_buff *skb){
	bool is_icmp = false;
	struct iphdr* ip_h;
	ip_h = ip_hdr(skb);
	struct tcphdr* tcp_h;
	if (ip_h->protocol == IPPROTO_ICMP){
		is_icmp = true;
	}
	return is_icmp;
}

//for sending
bool check_if_client_can_send_message(struct sk_buff *skb, struct network_details* lan, struct client_def *client){
	int inf_dest_status, dest_port;
	switch (client->infectivity)
	{
	case UNINFECTED:
		return true;
	case SUSPICIOUS:
		if (check_if_outside_network_destination(skb,lan)) //from Internet
			return true;
		else 
			return false;
	case INFECTED_MINOR:
		//todo
		//cannot broadcast
		if (check_if_outside_network_destination(skb,lan)) //from Internet
			return true;
		else {
			inf_dest_status = get_inf_status_for_destination_client(skb);
			if(inf_dest_status == -1) //possible MitM
				return false;
			else {
				if(check_if_broadcast_message(skb,lan)) //cannot broadcast
					return false;
				if(is_ok_status_for_inf_minor(inf_dest_status)) //can communicate with others
				{
					dest_port = get_destination_port(skb);
					if(dest_port == -1) //maybe ICMP
						return true;
					else if (check_if_banned_port(dest_port, banned_ports_minor, size_banned_ports_minor)) //banned ports
						return false;
					else 
						return true;
				}
				else 
					return false;
			}
		}
		break;
	case INFECTED_MAJOR:
		if (check_if_outside_network_destination(skb,lan)) //from Internet
			return true;
		else {
			inf_dest_status = get_inf_status_for_destination_client(skb);
			if(inf_dest_status == -1) //possible MitM
				return false;
			else {
				if(check_if_broadcast_message(skb,lan)) //cannot broadcast
					return false;
				if (check_for_icmp_protocol(skb)){
					return false; //no ICMP
				}
				if(is_ok_status_for_inf_major(inf_dest_status)){ //can communivcate with others
					dest_port = get_destination_port(skb);
					if(dest_port == -1) //maybe other proto
						return false; 
					else if (check_if_banned_port(dest_port, banned_ports_major, size_banned_ports_major)) //banned ports
						return false;
					else 
						return true;
				}	
				else 
					return false;
			}
		}
		break;
	case INFECTED_SEVER:
		return false;
	default:
		return false;
	}
	return false; //any case
}

//for receiving
bool check_if_client_can_receive_message(struct sk_buff *skb, struct network_details* lan, struct client_def *client){
	int inf_source_status, source_port;
	switch (client->infectivity)
	{
	case UNINFECTED:
		printk(KERN_WARNING "hapciu 1\n");
		return true;
	case SUSPICIOUS:
		if (check_if_outside_network_source(skb,lan)){ //from Internet
			printk(KERN_WARNING "hapciu 2\n");
			return true;
		}
		else{
			printk(KERN_WARNING "hapciu 3\n");
			return false;
		}
	case INFECTED_MINOR:
		if (check_if_outside_network_source(skb,lan)){ //from Internet
			printk(KERN_WARNING "hapciu 4\n");
			return true;
		}
		else{
			inf_source_status = get_inf_status_for_source_client(skb);
			if(inf_source_status == -1){ //possible MitM
				printk(KERN_WARNING "hapciu 5\n");
				return false;
			}
			else {
				if(is_ok_status_for_inf_minor(inf_source_status)){
					printk(KERN_WARNING "hapciu 6\n");
					return true;
				}
				else{
					printk(KERN_WARNING "hapciu 7\n");
					return false;
				}
			}
		}
		break;
	case INFECTED_MAJOR:
		if (check_if_outside_network_source(skb,lan)){ //from Internet
			printk(KERN_WARNING "hapciu 8\n");
			return true;
		}
		else{
			if (check_for_icmp_protocol(skb)){
				printk(KERN_WARNING "hapciu 9\n");
				return false; //no ICMP
			}
			inf_source_status = get_inf_status_for_source_client(skb);
			if(inf_source_status == -1){ //possible MitM
				printk(KERN_WARNING "hapciu 10\n");
				return false;
			}
			else {
				if(is_ok_status_for_inf_major(inf_source_status)){
					printk(KERN_WARNING "hapciu 11\n");
					return true;
				}
				else{
					printk(KERN_WARNING "hapciu 12\n");
					return false;
				}
			}
		}
		break;
	case INFECTED_SEVER:
		printk(KERN_WARNING "hapciu 13\n");
		return false;
	default:
		printk(KERN_WARNING "hapciu 14\n");
		return false;
	}
	printk(KERN_WARNING "hapciu 15\n");
	return false; //for any
}


unsigned char* create_pack_based_on_infectivity( struct sk_buff *skb, struct client_def *client, unsigned int* col_size){
	unsigned char* pack=NULL;
	switch (client->infectivity)
	{
	case UNINFECTED:
		pack = create_package_data_v2(skb,col_size,false);
		break;
	case SUSPICIOUS:
		pack = create_package_data_v2(skb,col_size,true);
		break;
	case INFECTED_MINOR:
		pack = create_package_data_v2(skb,col_size,true);
		break;
	case INFECTED_MAJOR:
		pack = create_package_data_v2(skb,col_size,true);
		break;
	case INFECTED_SEVER:
		pack = NULL;
		break;
	default:
		break;
	}
	return pack;
}


#endif