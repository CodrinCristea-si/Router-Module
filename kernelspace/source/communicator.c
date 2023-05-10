#include <linux/module.h>  
#include <linux/kernel.h> 
#include <linux/list.h>
#include <linux/random.h>
#include <linux/netlink.h>

#include "../headers/communicator.h"


void convert_from_def_to_repr(struct client_def* client,struct client_repr* collector){
	collector->ip_addr= client->ip_addr;
	copy_uchar_values(client->mac_addr,client->mac_addr,MAC_LEN);
	collector->infectivity = client->infectivity;
}

int get_size_of_clients_list(struct clients_list* cl_list){
	int nr;
	struct list_head *listptr;
	nr =0;
	if(cl_list){
		list_for_each(listptr, &cl_list->list){
			nr++;
		}
	}
	return nr;
}

int create_multi_clients_payload(struct clients_list* clients_data,unsigned char *collector, int size){
	struct clients_list *tmp=NULL;
	struct client_repr* decoy_client;
	struct list_head *listptr;
	int poz, len_client;
	printk(KERN_INFO "adding multi_clients\n");
	//printk(KERN_INFO, "struct client_repr size", sizeof(struct client_repr);)
	if(clients_data){
		//creating data payload
		decoy_client = (struct client_repr* )kcalloc(1,sizeof(struct client_repr),GFP_KERNEL); 
		//copy len Signiture
		printk(KERN_INFO "current_index %d\n",poz);
		poz =0;
		collector[poz]= SIGNITURE_NR_ENT;
		poz++;
		int2ch(size,&collector[poz]);
		poz+=4;
		printk(KERN_INFO "current_index %d\n",poz);
		//add clients
		list_for_each(listptr, &clients_data->list){
			tmp = list_entry(listptr, struct clients_list, list);
			convert_from_def_to_repr(&tmp->client,decoy_client);
			len_client=create_client_repr_payload(decoy_client,&collector[poz],FLAG_WITH_IP|FLAG_WITH_MAC|FLAG_WITH_INFECTIVITY);
			printk(KERN_INFO "len client payload %d\n",len_client);
			poz+=len_client;
			printk(KERN_INFO "current_index %d\n",poz);
		}
	}
	return poz;
}

struct infec_msg* create_clients_data_msg(struct clients_list* clients_data, unsigned char type){
	unsigned char *data;
	struct header_payload *hdr_inf;
	struct infec_msg* msg_infec = NULL;
	int id, len_payload;

	if(clients_data){
		//creating data payload
		data = (unsigned char *)kcalloc(MAX_USER_PAYLOAD_SIZE,sizeof(char),GFP_KERNEL);
		printk(KERN_INFO "NR_CL %d\n",get_size_of_clients_list(clients_data));
		len_payload = create_multi_clients_payload(clients_data,data, get_size_of_clients_list(clients_data));
		printk(KERN_INFO "Data created\n");
		printk(KERN_INFO "len payload %d\n",len_payload);
		
		hdr_inf = (struct header_payload *)kcalloc(1, sizeof(struct header_payload),GFP_KERNEL);
		get_random_bytes(&id, sizeof(id));
		create_header(id, type, hdr_inf);
		printk(KERN_INFO "Header created\n");
		
		msg_infec = (struct infec_msg *)kcalloc(1,INF_MSG_LEN_H(hdr_inf),GFP_KERNEL);
		create_message(hdr_inf,data,len_payload,msg_infec);

		kfree(data);
		kfree(hdr_inf);
		printk(KERN_INFO "Message created\n");

	}
	return msg_infec;
}

struct infec_msg* create_infec_msg(unsigned char* data,unsigned char type, int payload_id){
	struct infec_msg* package;
	switch (type)
	{
	case CLIENTS_DATA:
		package= create_clients_data_msg((struct clients_list*)data,type);
		if(payload_id)package->header.payload_id=payload_id;
		return package;
	
	default:
		break;
	}
	return NULL;
}


void send_to_user_clients(struct sock* netlink_socket,struct clients_list* clients_list,int requester, int payload_id){
	struct nlmsghdr *nlh;
	struct infec_msg* msg;
	struct sk_buff *skb_out;
	int res;

	msg = create_infec_msg((unsigned char*)clients_list,CLIENTS_DATA,payload_id);

	skb_out = nlmsg_new(INF_MSG_LEN(msg), 0);
	if (!skb_out) {
		printk(KERN_ERR "Failed to allocate new skb\n");
		return;
	}

	nlh = nlmsg_put(skb_out, 0, 0, NLMSG_DONE, INF_MSG_LEN(msg), 0);
	printk(KERN_INFO "new msg created\n");
	NETLINK_CB(skb_out).dst_group = 0; /* not in mcast group */
	copy_uchar_values((unsigned char*)msg,(unsigned char *) NLMSG_DATA(nlh), INF_MSG_LEN(msg));
	printk(KERN_INFO "msg copied\n");

	res = nlmsg_unicast(netlink_socket, skb_out, requester);
	printk(KERN_INFO "msg sent to user\n");
	if (res < 0)
		printk(KERN_ERR "Error while sending bak to user\n");

	kfree(msg);
}