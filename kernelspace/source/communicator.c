#include <linux/module.h>  
#include <linux/kernel.h> 
#include <linux/list.h>
#include <linux/random.h>
#include <linux/netlink.h>

#include "../headers/communicator.h"
#include "../headers/utils.h"

void print_infec_msg(struct infec_msg * msg_infec){
	struct header_payload *hdr_inf;
	unsigned char i;
	if(msg_infec){
		hdr_inf = (struct header_payload *)INF_MSG_HEADER(msg_infec);
		printk( KERN_INFO "hdr p %p  s %x%x%x%x len %d t %x i %x\n",hdr_inf,hdr_inf->signiture[0], hdr_inf->signiture[1], hdr_inf->signiture[2],
			hdr_inf->signiture[3], hdr_inf->payload_len, hdr_inf->payload_type, hdr_inf->payload_id);
		printk(KERN_INFO "inf p %p h %p d %p \n",msg_infec,INF_MSG_HEADER(msg_infec), INF_MSG_DATA(msg_infec));
		printk(KERN_INFO "len %ld\n", INF_MSG_LEN(msg_infec));
		for(i =0;i< INF_MSG_DATA_LEN(msg_infec);i++){
			printk(KERN_INFO "%x ",((unsigned char*)INF_MSG_DATA(msg_infec))[i]);
		}
	}
	else {
		printk(KERN_INFO "NULL\n");
	}
}

inline void convert_from_def_to_repr(struct client_def* client,struct client_repr* collector){
	collector->ip_addr= client->ip_addr;
	copy_uchar_values(client->mac_addr,collector->mac_addr,MAC_LEN);
	collector->infectivity = client->infectivity;
}

int create_multi_clients_payload(struct clients_list* clients_data,unsigned char *collector, int size){
	struct clients_list *listptr, *tmp;
	struct client_repr* decoy_client;
	int poz, len_client,i;
	len_client = 0;
	if(clients_data){
		//creating data payload
		//copy len Signiture
		poz =0;
		collector[poz]= SIGNITURE_NR_ENT;
		poz++;
		int2ch(size,&collector[poz]);
		poz+=4;
		//add clients
		list_for_each_entry_safe(listptr, tmp, &clients_data->list, list){
			decoy_client = (struct client_repr* )kcalloc(1,sizeof(struct client_repr),GFP_KERNEL);
			convert_from_def_to_repr(&listptr->client,decoy_client);
			//problem here
			len_client=create_client_repr_payload((struct client_repr*)&listptr->client,&collector[poz],FLAG_WITH_INFECTIVITY|FLAG_WITH_IP|FLAG_WITH_MAC);
			printk(KERN_INFO "current_index %d\n",poz);
			kfree(decoy_client);
		}
		
	}
	return poz;
}

struct infec_msg* create_clients_data_msg(struct clients_list* clients_data, unsigned char type){
	unsigned char *data;
	struct header_payload *hdr_inf;
	struct infec_msg* msg_infec = NULL;
	int id, len_payload, nr_cl;

	if(clients_data){
		//creating data payload
		nr_cl = __get_size_of_clients_list(clients_data);
		data = (unsigned char *)kcalloc(1,MULTI_CLIENTS_PAYLOAD_SIZE(nr_cl),GFP_KERNEL);
		len_payload = create_multi_clients_payload(clients_data,data, nr_cl);
		printk(KERN_INFO "Data created\n");
		
		hdr_inf = (struct header_payload *)kcalloc(1, sizeof(struct header_payload),GFP_KERNEL);
		get_random_bytes(&id, sizeof(id));
		create_header(id, type, len_payload, hdr_inf);
		printk(KERN_INFO "Header created\n");

		msg_infec = (struct infec_msg *)kcalloc(1,INF_MSG_LEN_H(hdr_inf),GFP_KERNEL);
		create_message(hdr_inf,data,msg_infec);

		kfree(data);
		kfree(hdr_inf);
		printk(KERN_INFO "Message created\n");

	}
	return msg_infec;
}

struct infec_msg* create_confirm_msg(unsigned char* data, unsigned char type){
	unsigned char *payload;
	struct header_payload *hdr_inf;
	struct infec_msg* msg_infec = NULL;
	int id, len_payload, nr_cl;

	if(data){
		//creating data payload
		payload = (unsigned char *)kcalloc(MAX_LEN_CONFIRM,sizeof(unsigned char),GFP_KERNEL);
		copy_uchar_values(data, payload,MAX_LEN_CONFIRM);

		hdr_inf = (struct header_payload *)kcalloc(1, sizeof(struct header_payload),GFP_KERNEL);
		get_random_bytes(&id, sizeof(id));
		create_header(id, type,MAX_LEN_CONFIRM, hdr_inf);
		
		msg_infec = (struct infec_msg *)kcalloc(1,INF_MSG_LEN_H(hdr_inf),GFP_KERNEL);
		create_message(hdr_inf,payload,msg_infec);

		kfree(payload);
		kfree(hdr_inf);
		printk(KERN_INFO "Message created\n");
	}
	return msg_infec;
}

struct infec_msg* create_error_msg(unsigned char* data, unsigned char type){
	unsigned char *payload;
	struct header_payload *hdr_inf;
	struct infec_msg* msg_infec = NULL;
	int id, len_payload, nr_cl;

	if(data){
		//creating data payload
		payload = (unsigned char *)kcalloc(MAX_LEN_ERROR,sizeof(unsigned char),GFP_KERNEL);
		copy_uchar_values(data, payload,MAX_LEN_ERROR);

		hdr_inf = (struct header_payload *)kcalloc(1, sizeof(struct header_payload),GFP_KERNEL);
		get_random_bytes(&id, sizeof(id));
		create_header(id, type, MAX_LEN_ERROR, hdr_inf);
		
		msg_infec = (struct infec_msg *)kcalloc(1,INF_MSG_LEN_H(hdr_inf),GFP_KERNEL);
		create_message(hdr_inf, payload, msg_infec);

		kfree(payload);
		kfree(hdr_inf);
		printk(KERN_INFO "Message created\n");
	}
	return msg_infec;
}

struct infec_msg* create_infec_msg(unsigned char* data,unsigned char type, int payload_id){
	struct infec_msg* package = NULL;
	switch (type)
	{
	case CLIENTS_DATA:
		package= create_clients_data_msg((struct clients_list*)data,type);
		if(payload_id > 0)package->header.payload_id=payload_id;
		return package;
	case CONFIRM:
		package= create_confirm_msg(data,type);
		if(payload_id > 0)package->header.payload_id=payload_id;
		return package;
	case ERROR:
		package= create_error_msg(data,type);
		if(payload_id > 0)package->header.payload_id=payload_id;
		return package;
	default:
		break;
	}
	return NULL;
}

//unicast
void send_to_user_unicast(struct sock* netlink_socket,unsigned char* clients_list,unsigned char type,int requester, int payload_id){
	struct nlmsghdr *nlh;
	struct infec_msg* msg;
	struct sk_buff *skb_out;
	int res;

	msg = create_infec_msg((unsigned char*)clients_list,type,payload_id);
	if(!msg){
		printk(KERN_ERR "Failed to create new message\n");
		goto cleanup;
	}
	// printk(KERN_INFO "msg created\n");
	// print_infec_msg(msg);
	
	//create reply
	skb_out = nlmsg_new(INF_MSG_LEN(msg), 0);
	if (!skb_out) {
		printk(KERN_ERR "Failed to allocate new skb\n");
		goto cleanup;
	}

	nlh = nlmsg_put(skb_out, 0, 0, NLMSG_DONE, INF_MSG_LEN(msg), 0);
	NETLINK_CB(skb_out).dst_group = 0; /* not in mcast group */
	copy_uchar_values((unsigned char*)msg,(unsigned char *) NLMSG_DATA(nlh), INF_MSG_LEN(msg));

	res = nlmsg_unicast(netlink_socket, skb_out, requester);
	
	if (res < 0)
		printk(KERN_ERR "Error while sending bak to user\n");
	// printk(KERN_INFO "msg sent to user\n");
cleanup:
	if(msg) kfree(msg);
}