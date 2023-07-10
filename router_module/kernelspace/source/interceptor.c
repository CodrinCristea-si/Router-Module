#include <linux/module.h>  
#include <linux/kernel.h>  
#include <linux/fs.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/udp.h>
#include <linux/types.h>
#include <linux/netlink.h>
#include <linux/netdevice.h>
#include <net/sock.h>
#include <linux/mutex.h>

#include <linux/net_namespace.h>
//#include <linux/dhcp.h>
#include "../headers/interceptor.h"
#include "../headers/communicator.h"
#include "../headers/utils.h"
#include "../headers/rules.h"

struct nf_hook_ops *interceptor_hook_ops = NULL;
struct sock *netlink_socket = NULL;
struct mutex *nl_mutex;

//struct nf_hook_ops *local_hook_ops = NULL;
struct network_details lan;
struct network_details *collector = NULL; 
bool is_configured = false;



unsigned int interceptor_hook_handle(void *priv, struct sk_buff *skb, const struct nf_hook_state *state){

	struct iphdr* ip_h;
	struct tcphdr* tcp_h;
	struct udphdr* udp_h;
	struct ethhdr *mac_header;
	__be32 source_ip, dest_ip;
	//struct package_data* pack;
	unsigned char* pack=NULL;
	struct client_def *client = NULL, *res_cl;
	int pack_size =0;

	bool can_communicate = false;

	//must be thread safe
	// if(is_lockdown_mode())
	// 	return NF_DROP;

	//only ipv4 based packages allowed
	if (!skb || skb->protocol != htons(ETH_P_IP)){
		//printk(KERN_WARNING "Intercept 1\n");
		return NF_ACCEPT;
	}
	//check if the package is sent from the router
	if (check_if_not_belonged_to_router(skb,&lan)){
		//printk(KERN_WARNING "Intercept 2\n");
		return NF_ACCEPT;
	}
	if(check_for_special_clients(skb,&lan)){
		//printk(KERN_WARNING "Intercept 3\n");
		return NF_ACCEPT;
	}

	if(check_for_spcecial_client_test(skb,&lan)){ //just for demo presentation
		//printk(KERN_WARNING "Intercept 3\n");
		return NF_ACCEPT;
	}

	if (check_if_a_test_message(skb,&lan)){
		return NF_ACCEPT;
	}


	if(IS_LOCKDOWN_MODE_ENABLED()){
		//printk(KERN_WARNING "Intercept 3.5\n");
		return NF_DROP;
	}
	//ip forwarding, dhcp or others
	if (check_if_outside_network(skb,&lan)){
		//printk(KERN_WARNING "Intercept 4\n");
		return NF_ACCEPT;
	}
	ip_h = ip_hdr(skb);
	//if cannot retrive ipv4 data then drop
	if(!ip_h){
		//printk(KERN_WARNING "Intercept 5\n");
		return NF_ACCEPT;
	}
	mac_header = eth_hdr(skb);
	//if cannot retrive mac data then drop
	if(!mac_header){
		//printk(KERN_WARNING "Intercept 6\n");
		return NF_ACCEPT;
	}


	client = (struct client_def *)kcalloc(1, sizeof(struct client_def), GFP_KERNEL);
	//get client details
	if (!check_if_outside_network_source(skb,&lan)){
		res_cl = GET_CLIENT_GENERIC_SAFE(ip_h->saddr, mac_header->h_source,client);

		//no client details then drop, possible MitM
		//or static in that case accept
		if (!res_cl){
			//printk(KERN_WARNING "Intercept 7\n");
			kfree(client);
			return NF_ACCEPT;
		}
		if(check_if_client_can_send_message(skb,&lan,client))
			can_communicate = true;
		else{
			//printk(KERN_WARNING "Intercept 7.5\n");
			can_communicate = false;
		}
		
	}
	// else if (!check_if_outside_network_destination(skb,&lan)){
	// 	res_cl = GET_CLIENT_GENERIC_SAFE(ip_h->daddr, mac_header->h_dest,client);
	// 	//no client details then drop, possible MitM
	// 	//or static in that case accept
	// 	if (!res_cl){
	// 		//printk(KERN_WARNING "Intercept 8\n");
	// 		kfree(client);
	// 		return NF_ACCEPT;
	// 	}
	// 	if(check_if_client_can_receive_message(skb,&lan,client))
	// 		can_communicate = true;
	// 	else{
	// 		//printk(KERN_WARNING "Intercept 8.5\n");
	// 		can_communicate = false;
	// 	}
	// }

	
	pack = create_pack_based_on_infectivity(skb,client,&pack_size);
	if(pack && pack_size){
		send_to_user_broadcast(netlink_socket,(unsigned char*)pack,pack_size,PACKAGE,0,nl_mutex);
	}

	kfree(client);
	if(can_communicate){
		return NF_ACCEPT;
	}else{
		return NF_DROP;
	}
	
	// pack = create_package_data_v2(skb,&pack_size,true);
	// if (pack_size){
	// 	//print_package_data_v2(pack,pack_size,true);
	// 	send_to_user_broadcast(netlink_socket,(unsigned char*)pack,pack_size,PACKAGE,0,nl_mutex);
	// }
	//send_to_user_broadcast_v2((unsigned char*)pack, pack_size, PACKAGE, NETLINK_PROTO_INFECTED , 2);
	//send_to_user_server((unsigned char*)pack,get_size_of_package_data(pack));
	// clear_package_data_v2(pack,true);
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

int init_hook(void){
	if(!interceptor_hook_ops){
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
		return -1;
	}
	return 0;
}

void clear_hook(void){
	if(interceptor_hook_ops){
		nf_unregister_net_hook(&init_net, interceptor_hook_ops);
		kfree(interceptor_hook_ops);
		interceptor_hook_ops=NULL;
		printk(KERN_INFO "Interceptor removed\n");
	}
}

static void netlink_handle(struct sk_buff *skb){
	struct nlmsghdr *nhl;
	struct infec_msg* msg;
	struct header_payload* hdr;
	struct client_def *client, *empty_client;
	struct clients_list *all_clients;
	struct configure* config;

	int nr_cl, size;
	int ret;
	printk(KERN_INFO "Message received\n");

	//extracting the netlink message
	nhl = (struct nlmsghdr *) skb->data;
	//printk(KERN_INFO "1\n");
	//extracting the infec_msg
	msg = (struct infec_msg*) NLMSG_DATA(nhl);
	//print_infec_msg(msg);
	// printk(KERN_INFO "msg p %p \n",msg);
	//printk(KERN_INFO "2\n");
	//extract the header payload
	
	hdr = (struct header_payload*)INF_MSG_HEADER(msg);
	//printk(KERN_INFO "Message type %x\n", hdr->payload_type);
	//process msg
	switch(hdr->payload_type){
		case ADD_CLIENT:
			client = (struct client_def *)kcalloc(1,sizeof(struct client_def),GFP_KERNEL);
			extract_client_repr_payload_ext(msg,(struct client_repr *)client,0,FLAG_WITH_IP|FLAG_WITH_MAC);
			//printk(KERN_INFO "Client with %pI4 extracted\n", &client->ip_addr);
			ret = ADD_CLIENT_GENERIC_SAFE(client->ip_addr, client->mac_addr);
			if(!ret) printk(KERN_ERR "Failed to add client %pI4 err %d\n",&client->ip_addr,ret);
			else printk(KERN_INFO "Added client %pI4\n",&client->ip_addr);
			kfree(client);
			break;
		case REMOVE_CLIENT:
			client = (struct client_def *)kcalloc(1,sizeof(struct client_def),GFP_KERNEL);
			extract_client_repr_payload_ext(msg,(struct client_repr *)client,0,FLAG_WITH_IP|FLAG_WITH_MAC);
			//printk(KERN_INFO "Client with %pI4 extracted\n", &client->ip_addr);
			ret = REMOVE_CLIENT_GENERIC_SAFE(client->ip_addr, client->mac_addr);
			if(!ret) printk(KERN_ERR "Failed to remove client %pI4 err %d\n",&client->ip_addr,ret);
			else printk(KERN_INFO "Removed client %pI4\n",&client->ip_addr);
			kfree(client);
			break;
		case TRANSFER_CLIENT:
			client = (struct client_def *)kcalloc(1,sizeof(struct client_def),GFP_KERNEL);
			extract_client_repr_payload_ext(msg,(struct client_repr *)client,0,FLAG_WITH_IP|FLAG_WITH_MAC|FLAG_WITH_INFECTIVITY);
			//printk(KERN_INFO "Client with %pI4 extracted\n", &client->ip_addr);
			ret = TRANSFER_CLIENT_GENERIC_SAFE(client->infectivity,client->ip_addr, client->mac_addr);
			if(!ret) printk(KERN_ERR "Failed to transfer client %pI4 to %x err %d\n",&client->ip_addr,client->infectivity,ret);
			else printk(KERN_INFO "Transfered client %pI4 to %x\n",&client->ip_addr, client->infectivity);
			kfree(client);
			break;

		case CONFIGURE:
			if(!is_configured){
				config = INF_MSG_DATA(msg);
				if(config){
					lan.subnet_ip = config->subnet;
					lan.mask = config->netmask;
					lan.ip_addr = config->ip_router;
					is_configured = true;
					DISABLE_LOCKDOWN_MODE();
					if(init_hook())clear_hook();
					printk(KERN_INFO "CONFIGURE successfull on %pI4(%pI4)\n",&config->subnet, &config->netmask);
				}
			}
			break;
		case LOCK_UP:
			if(IS_LOCKDOWN_MODE_ENABLED()){
				DISABLE_LOCKDOWN_MODE();
			}
			break;
		case LOCK_DOWN:
			if(!IS_LOCKDOWN_MODE_ENABLED()){
				ENABLE_LOCKDOWN_MODE();
			}
			break;
		// usefull and broken, maybe some rework and it may become something 
		// case GET_CLIENT:
		// 	client = (struct client_def *)kcalloc(1,sizeof(struct client_def),GFP_KERNEL);
		// 	extract_client_repr_payload(msg,(struct client_repr *)client,0,FLAG_WITH_IP|FLAG_WITH_MAC);
		// 	printk(KERN_INFO "Client with %pI4 extracted\n", &client->ip_addr);
		// 	ret = REMOVE_CLIENT_GENERIC(client->ip_addr, client->mac_addr);
		// 	if(!ret) printk(KERN_ERR "Failed to remove client %pI4 err %d\n",&client->ip_addr,ret);
		// 	else printk(KERN_INFO "Removed client %pI4\n",&client->ip_addr);
		// 	kfree(client);
		// 	break;
		// case GET_CLIENTS:
		// 	/// list creation
		// 	all_clients = __create_empty_list();
		// 	if(all_clients){
		// 		nr_cl = GET_ALL_CLIENTS(all_clients);
		// 		printk(KERN_INFO " all_clients after get all %p and %p\n",all_clients,&all_clients->list);
		// 		__print_list(all_clients);
		// 		printk(KERN_INFO " all_clients after print %p and %p\n",all_clients,&all_clients->list);
		// 		nr_cl = __get_size_of_clients_list(all_clients);
		// 		//size = MULTI_CLIENTS_MSG_SIZE(nr_cl);
		// 		//send_to_user_clients(netlink_socket,(unsigned char*)&size,CONFIRM,nhl->nlmsg_pid,hdr->payload_id);
		// 		send_to_user_unicast(netlink_socket,(unsigned char*)all_clients,CLIENTS_DATA,nhl->nlmsg_pid,hdr->payload_id);
		// 		printk(KERN_INFO " all_clients after send %p and %p\n",all_clients,&all_clients->list);
		// 		//__print_list(all_clients);
		// 		__clear_list(all_clients);
		// 		printk(KERN_INFO " all_clients after clear %p and %p\n",all_clients,&all_clients->list);
		// 		kfree(all_clients);
		// 		printk(KERN_INFO "Clients Data resolved with success");
		// 	}
		// 	break;
		default:
			printk(KERN_ERR "Not implemented\n");
			break;
	}
	// all_clients = __create_empty_list();
	// if(all_clients){
	// 	nr_cl = GET_ALL_CLIENTS_SAFE(all_clients);
	// 	__print_list(all_clients,NULL);
	// 	__clear_list(all_clients);
	// 	kfree(all_clients);
	// }
	// printk(KERN_INFO "Done\n");
}

struct netlink_kernel_cfg config = {
	.groups  = 1,
	.input = netlink_handle,
};

int initialise_interceptor(void){
	// if(!DISABLE_LOCKDOWN_MODE())
	// 	clear_interceptor();

	// collector = (struct network_details *) kcalloc(1,sizeof(struct network_details), GFP_KERNEL);
	// if(!collector || initialize_network_interfaces_list(collector)){
	// 	printk(KERN_INFO "Collector initialize failed\n");
	// 	clear_interceptor();
	// 	goto end;
	
	// }
	// get_network_interfaces_list(collector);
	// lan = get_lan_network_from_list(collector);
	
	nl_mutex = (struct mutex *)kcalloc(1,sizeof(struct mutex), GFP_KERNEL);
	if (!nl_mutex) {
		printk(KERN_INFO "Mutex initialize failed\n");
		clear_interceptor();
		goto end;
	}
	mutex_init(nl_mutex);
	config.cb_mutex = nl_mutex;

	netlink_socket = netlink_kernel_create(&init_net, NETLINK_PROTO_INFECTED, &config);
	if (!netlink_socket) {
		printk(KERN_INFO "Socket initialize failed\n");
		clear_interceptor();
		goto end;
	}
	printk(KERN_INFO "Socket initialized with success\n");
	
	return 0;
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

	if(nl_mutex){
		mutex_destroy(nl_mutex);
		kfree(nl_mutex);
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
