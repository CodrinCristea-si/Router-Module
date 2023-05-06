#include <linux/module.h>  
#include <linux/kernel.h>  
#include <linux/fs.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/ip.h>
#include <linux/udp.h>
#include <linux/types.h>
#include <linux/netlink.h>
#include <net/sock.h>
#include <linux/net_namespace.h>
//#include <linux/dhcp.h>
#include "../headers/interceptor.h"
#include "../headers/infectivity.h"
#include "../headers/common_proto.h"
#include "../headers/utils.h"

struct nf_hook_ops *interceptor_hook_ops = NULL;
struct sock *netlink_socket = NULL;

//struct nf_hook_ops *local_hook_ops = NULL;
struct network_details lan;
struct network_details *collector = NULL; 

unsigned int interceptor_hook_handle(void *priv, struct sk_buff *skb, const struct nf_hook_state *state){

	struct iphdr* ip_h;
	__be32 source_ip, dest_ip;

	// if(is_lockdown_mode())
	// 	return NF_DROP;

	//only ipv4 based packages allowed
	if (!skb || skb->protocol != htons(ETH_P_IP))
		return NF_ACCEPT;

	ip_h = ip_hdr(skb);
	source_ip = ip_h->saddr;
	dest_ip = ip_h->daddr;

	//printk(KERN_INFO "Intercepted package from %pI4 to %pI4 of type %02x\n", &source_ip, &dest_ip, ip_h->protocol);
	//if the client is not part of any kind, then no packages are allowed
	//check if the package comes from the router
	// if (source_ip == lan.ip_addr){
    //     if(is_a_new_client_connection(ip_h)){
	//
    //     }
	// }
	//
	//check if a new client wants to connect
	//
	//check if the package comes from the lan network 
	// if(check_ip_belong_to_network(&lan,source_ip)){
	//	
	// }
	return NF_ACCEPT;
}
// unsigned int local_hook_handle(void *priv, struct sk_buff *skb, const struct nf_hook_state *state){
//
// 	struct iphdr* ip_h;
// 	__be32 source_ip, dest_ip;
//
// 	// if(is_lockdown_mode())
// 	// 	return NF_DROP;
//
// 	//only ipv4 based packages allowed
// 	if (!skb || skb->protocol != htons(ETH_P_IP))
// 		return NF_ACCEPT;
//
// 	ip_h = ip_hdr(skb);
// 	source_ip = ip_h->saddr;
// 	dest_ip = ip_h->daddr;
//
// 	printk(KERN_INFO "Intercepted package from %pI4 to %pI4 of type %02x\n", &source_ip, &dest_ip, ip_h->protocol);
//	if the client is not part of any kind, then no packages are allowed
//	check if the package comes from the router
//	if (source_ip == lan.ip_addr){
//     if(is_a_new_client_connection(ip_h)){
//
//     }
// }
//
//	check if a new client wants to connect
//
//	check if the package comes from the lan network 
//	if(check_ip_belong_to_network(&lan,source_ip)){
//		
// }
// 	return NF_ACCEPT;
// }
//
// static bool is_a_new_client_connection(struct iphdr* ip_h){
//     struct udphdr *udp_h;
//     struct dhcp_packet *dhcp_pack;
//     unsigned char *dhcp_options;
//
//     if (ip_h->protocol != IPPROTO_UDP) {
//         return false;
//     }
//
//     udp_h = (struct udphdr *)((unsigned char *)ip_h + (ip_h->ihl * 4));
//     dhcp_pack = (struct dhcp_packet *)((unsigned char *)udp_h + sizeof(struct udphdr));
//     dhcp_options = (unsigned char *)dhcp_pack + sizeof(struct dhcp_packet);
//
//     if (dhcp_pack->op != BOOTREPLY || dhcp_pack->htype != 1 || dhcp_pack->hlen != 6 || dhcp_pack->cookie != htonl(0x63825363)) {
//         return false;
//     }
//     return true;
//}

void extract_client_def_payload(struct infec_msg* msg, struct client_def* client_collector){
	unsigned char* data = INF_MSG_DATA(msg);
	unsigned char poz = 0;
	if(CHECK_SIGNITURE(data[poz],SIGNITURE_IP)){
		poz++;
		ch2int_le(&data[poz],&client_collector->ip_addr);
		printk(KERN_INFO "Ip extracted %pI4\n",&client_collector->ip_addr);
		poz +=4;
	}
	if(CHECK_SIGNITURE(data[poz],SIGNITURE_MAC)){
		copy_mac_address(&data[poz+1],client_collector->mac_addr);
		printk(KERN_INFO "Mac extracted %02X:%02X:%02X:%02X:%02X:%02X\n",client_collector->mac_addr[0],
		client_collector->mac_addr[1],client_collector->mac_addr[2],client_collector->mac_addr[3],
		client_collector->mac_addr[4],client_collector->mac_addr[5]);
		poz +=7;
	}
	client_collector->infectivity = UNKNOWN;
}

static void netlink_handle(struct sk_buff *skb){
	struct nlmsghdr *nhl;
	struct infec_msg* msg;
	struct header_payload* hdr;
	struct client_def *client;

	int ret;
	
	printk(KERN_INFO "Message received\n");

	//extracting the netlink message
	nhl = (struct nlmsghdr *) skb->data;
	printk(KERN_INFO "1\n");
	//extracting the infec_msg
	msg = (struct infec_msg*) NLMSG_DATA(nhl);
	printk(KERN_INFO "msg p %p \n",msg);
	printk(KERN_INFO "2\n");
	//extract the header payload
	
	hdr = (struct header_payload*)INF_MSG_HEADER(msg);
	printk(KERN_INFO "Message type %x\n", hdr->payload_type);
	//process msg
	switch(hdr->payload_type){
		case ADD_CLIENT:
			client = (struct client_def *)kcalloc(1,sizeof(struct client_def),GFP_KERNEL);
			extract_client_def_payload(msg,client);
			printk(KERN_INFO "Client with %pI4 extracted\n", &client->ip_addr);
			ret = ADD_CLIENT_SUSPICIOUS(client->ip_addr, client->mac_addr);
			if(ret != 0) printk(KERN_ERR "Failed to add client %pI4 err %d\n",&client->ip_addr,ret);
			else printk(KERN_INFO "Added client %pI4\n",&client->ip_addr);
			break;
		case REMOVE_CLIENT:
			client = (struct client_def *)kcalloc(1,sizeof(struct client_def),GFP_KERNEL);
			extract_client_def_payload(msg,client);
			printk(KERN_INFO "Client with %pI4 extracted\n", &client->ip_addr);
			ret = REMOVE_CLIENT_GENERIC(client->ip_addr, client->mac_addr);
			if(!ret) printk(KERN_ERR "Failed to remove client %pI4 err %d\n",&client->ip_addr,ret);
			else printk(KERN_INFO "Removed client %pI4\n",&client->ip_addr);
			break;
		default:
			printk(KERN_ERR "Not implemented\n");
			break;
	}
	printk(KERN_INFO "Done\n");
}

struct netlink_kernel_cfg config = {
	.groups  = 1,
	.input = netlink_handle,
};

int initialise_interceptor(void){
	// if(!DISABLE_LOCKDOWN_MODE())
	// 	clear_interceptor();

	collector = (struct network_details *) kcalloc(1,sizeof(struct network_details), GFP_KERNEL);
	if(!collector || initialize_network_interfaces_list(collector)){
		printk(KERN_INFO "Collector initialize failed\n");
		clear_interceptor();
		goto end;
	
	}
	get_network_interfaces_list(collector);
	lan = get_lan_network_from_list(collector);
	
	netlink_socket = netlink_kernel_create(&init_net, NETLINK_PROTO_INFECTED, &config);
	if (!netlink_socket) {
		printk(KERN_INFO "Socket initialize failed\n");
		clear_interceptor();
		goto end;
	}
	printk(KERN_INFO "Socket initialized with success\n");
	interceptor_hook_ops = (struct nf_hook_ops*)kcalloc(1,sizeof(struct nf_hook_ops), GFP_KERNEL);
	if(interceptor_hook_ops){
		interceptor_hook_ops->hook = (nf_hookfn*)interceptor_hook_handle;
		interceptor_hook_ops->hooknum = NF_INET_PRE_ROUTING;
		interceptor_hook_ops->pf = NFPROTO_IPV4;
		interceptor_hook_ops->priority = NF_IP_PRI_FIRST;
		
		nf_register_net_hook(&init_net, interceptor_hook_ops);
		printk(KERN_INFO "Hook initialized with success\n");
		return 0;
	}


	// local_hook_ops = (struct nf_hook_ops*)kcalloc(1,sizeof(struct nf_hook_ops), GFP_KERNEL);
	// if(local_hook_ops){
	// 	local_hook_ops->hook = (nf_hookfn*)local_hook_handle;
	// 	local_hook_ops->hooknum = NF_INET_LOCAL_IN;
	// 	local_hook_ops->pf = NFPROTO_IPV4;
	// 	local_hook_ops->priority = NF_IP_PRI_FIRST;
		
	// 	nf_register_net_hook(&init_net, local_hook_ops);
	// 	printk(KERN_INFO "Hook initialized with success\n");
	// 	return 0;
	// }
end:
	return -1;
}

int clear_interceptor(void){
	if (netlink_socket) {
		netlink_kernel_release(netlink_socket);
		netlink_socket=NULL;
		printk(KERN_INFO "Socket removed\n");
	}
	if(interceptor_hook_ops){
		nf_unregister_net_hook(&init_net, interceptor_hook_ops);
		kfree(interceptor_hook_ops);
		interceptor_hook_ops=NULL;
		printk(KERN_INFO "Interceptor removed\n");
	}

	// if(local_hook_ops){
	// 	nf_unregister_net_hook(&init_net, local_hook_ops);
	// 	kfree(local_hook_ops);
	// }

	if(collector){
		clear_network_interfaces_list(collector);
		kfree(collector);
		collector=NULL;
		printk(KERN_INFO "Collector removed\n");
	}
	DISABLE_LOCKDOWN_MODE();
	printk(KERN_INFO "Hook cleared with success\n");
	return 0;
}
