#include <linux/module.h>  
#include <linux/kernel.h> 
#include <linux/list.h>
#include <linux/random.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/udp.h>
#include <linux/inet.h>
#include <linux/netpoll.h>
#include <linux/socket.h>
#include <linux/netlink.h>
#include <linux/types.h>
#include <linux/net.h>
#include <linux/fs.h>
#include <linux/in.h>
#include <asm/uaccess.h>
#include <linux/slab.h>
//#include <linux/inet_sock.h>


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
		for(i =0;i< INF_MSG_DATA_LEN(msg_infec); i++){
			printk(KERN_CONT "%02x ",((unsigned char*)INF_MSG_DATA(msg_infec))[i]);
			if(i%10 == 0)
				printk(KERN_INFO "\n");
			if(i > 100)break;
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

struct infec_msg* create_packet_msg(unsigned char* pack_data, int size, unsigned char type){
	unsigned char *data;
	struct header_payload *hdr_inf;
	struct infec_msg* msg_infec = NULL;
	int id, len_payload, nr_cl;

	if(pack_data){
		//creating data payload
		printk(KERN_INFO "data size %d\n",size);
		data = (unsigned char *)kcalloc(1,size,GFP_KERNEL);
		memcpy(data,pack_data,size);
		printk(KERN_INFO "Data created\n");
		
		hdr_inf = (struct header_payload *)kcalloc(1, sizeof(struct header_payload),GFP_KERNEL);
		get_random_bytes(&id, sizeof(id));
		create_header(id, type, size, hdr_inf);
		printk(KERN_INFO "Header created\n");

		msg_infec = (struct infec_msg *)kcalloc(1,INF_MSG_LEN_H(hdr_inf),GFP_KERNEL);
		create_message(hdr_inf,data,msg_infec);

		kfree(data);
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
	case PACKAGE:
		package = create_packet_msg(data,payload_id,type);
		return package;
	default:
		break;
	}
	return NULL;
}

void clear_package_data(struct package_data* pack, bool with_data){
	if(pack){
		// if(with_data && pack->data){
		// 	kfree(pack->data);
		// }
		kfree(pack);
	}
}

void clear_package_data_v2(unsigned char* pack, bool with_data){
	if(pack){
		// if(with_data && pack->data){
		// 	kfree(pack->data);
		// }
		kfree(pack);
	}
}

// int get_size_of_package_data(struct package_data* pack){
// 	if(pack){
// 		return sizeof(struct package_data)+ pack->data_len + 1;
// 	}
// 	return 0;
// }

void print_package_data(struct package_data* pack){
	unsigned int i;
	printk(KERN_INFO "Pointer pack %p\n",pack);
	if(pack){
		printk(KERN_INFO "%pI4:%d -> %pI4:%d\n",&pack->sourceIP,pack->sourcePort,&pack->destIP,pack->destPort);
		printk(KERN_INFO "NP:%x, TP:%x\n",pack->network_proto, pack->transport_proto);
		printk(KERN_INFO "Data size: %d\n",pack->data_len);
		// for(i=0;i<pack->data_len > 100? 100: pack->data_len;i++){
		// 	printk(KERN_CONT "%02x ",((unsigned char*)(pack+sizeof(struct package_data)+1))[i]);
		// 	if(i>0 && i%10 == 0)
		// 		printk(KERN_INFO "\n");
		// }
		
	}else{
		printk(KERN_INFO "NULL\n");
	}
}

void print_package_data_v2(unsigned char* pack, int size, bool with_data){
	unsigned int i;
	struct package_data* pack_pack;
	printk(KERN_INFO "Pointer pack %p\n",pack);
	if(pack){
		if(!with_data){
			pack_pack = (struct package_data*)pack;
			printk(KERN_INFO "%pI4:%d -> %pI4:%d\n",&pack_pack->sourceIP,pack_pack->sourcePort,&pack_pack->destIP,pack_pack->destPort);
			printk(KERN_INFO "NP:%x, TP:%x\n",pack_pack->network_proto, pack_pack->transport_proto);
			printk(KERN_INFO "Data size: %d\n",pack_pack->data_len);
			// for(i=0;i<pack->data_len > 100? 100: pack->data_len;i++){
			// 	printk(KERN_CONT "%02x ",((unsigned char*)(pack+sizeof(struct package_data)+1))[i]);
			// 	if(i>0 && i%10 == 0)
			// 		printk(KERN_INFO "\n");
			// }
		}
		else{
			printk(KERN_INFO "Data size: %d\n",size);
			for(i=0;i<size;i++){
				printk(KERN_CONT "%02x ",pack[i]);
				if(i>0 && i%20 == 0)
					printk(KERN_INFO "\n");
				if(i > 100)break;
			}
		}
		
	}else{
		printk(KERN_INFO "NULL\n");
	}
}

struct package_data* create_package_data(struct sk_buff* skb, bool with_data){
	struct package_data* pack;
	struct iphdr* ip_h;
	struct tcphdr* tcp_h;
	struct udphdr* udp_h;
	__be32 source_ip, dest_ip;
	unsigned int payload_len;
	unsigned char *payload, *tmp_data;
	struct skb_frag_struct *frag;
	unsigned int frag_size;
	unsigned char current_poz=0;
	struct sk_buff* iter_skb,*cpy_skb;
	skb_frag_t *skb_frag;
	int frag_index, nr_frags, i, curr_dim = 0;
	unsigned char *curr_poz=NULL;
	size_t dim_max;

	if (!skb) {return NULL;}
	if (skb->len < 0 || skb->protocol < 0 || !skb->data){
		printk(KERN_ERR "Validation failed\n");
		return NULL;
	}
	ip_h = ip_hdr(skb);
	if(!ip_h) return NULL;
	source_ip = ip_h->saddr;
	dest_ip = ip_h->daddr;

	if(with_data){
		dim_max = skb->len;
		printk(KERN_INFO "SIZE of %ld  and %d\n",sizeof(struct package_data) + skb->len + 2, skb->len);
		pack = (struct package_data*)kcalloc(sizeof(struct package_data) + skb->len + 2,sizeof(char),GFP_KERNEL);
		//printk(KERN_INFO "1.1\n");
	}
	else{
		printk(KERN_INFO "SIZE of %ld\n",sizeof(struct package_data) + 1);
		pack = (struct package_data*)kcalloc(sizeof(struct package_data) + 1,sizeof(char),GFP_KERNEL);
		//printk(KERN_INFO "1.2\n");
	}
	
	if(pack){
		pack->sourceIP = source_ip;
		//printk(KERN_INFO "2\n");
		pack->destIP = dest_ip;
		//printk(KERN_INFO "3\n");
		//printk(KERN_INFO "4\n");
		pack->network_proto = skb->protocol;
		//printk(KERN_INFO "5\n");
		pack->transport_proto = ip_h->protocol;
		//printk(KERN_INFO "6\n");
		if ( pack->transport_proto == IPPROTO_TCP){
			tcp_h = tcp_hdr(skb);
			pack->sourcePort = tcp_h->source;
			//printk(KERN_INFO "7.1\n");
			pack->destPort = tcp_h->dest;
			//printk(KERN_INFO "7.2\n");
		}
		else if (pack->transport_proto == IPPROTO_UDP){
			udp_h = udp_hdr(skb);
			pack->sourcePort = udp_h->source;
			//printk(KERN_INFO "8.1\n");
			pack->destPort = udp_h->dest;
			//printk(KERN_INFO "8.2\n");
		}
		if(with_data){
			//pack->data = (unsigned char*)kcalloc(pack->data_len,sizeof(unsigned char),GFP_KERNEL);
			printk(KERN_INFO "ici1\n");
			printk(KERN_INFO "is fragmented %d and nr frags %d\n",skb_is_nonlinear(skb), skb_shinfo(skb)->nr_frags);
			// if(pack->data){
			if (skb_is_nonlinear(skb)) {
				current_poz = 0;
				nr_frags = skb_shinfo(skb)->nr_frags;
				for(frag_index=0;frag_index<nr_frags;frag_index++){
					skb_frag = &skb_shinfo(skb)->frags[frag_index];
					payload =  skb_frag_address_safe(skb_frag);
					payload_len = skb_frag_size(skb_frag);
					if(payload){
						memcpy(pack + sizeof(struct package_data) + 1 + current_poz, payload, payload_len);
						current_poz += payload_len;
					}
				}
				// skb_walk_frags(skb, iter_skb) {
				// 	//tmp_data = skb_frag_address_safe(frag);
				// 	cpy_skb = skb_copy(iter_skb,GFP_ATOMIC);
				// 	payload = cpy_skb->data;
				// 	printk(KERN_INFO "ici2.1\n");
				// 	payload_len = cpy_skb->data_len;
				// 	printk(KERN_INFO "ici3.1\n");
				// 	if(payload){
				// 		// frag_size = skb_frag_size(frag);
				// 		memcpy(pack + sizeof(struct package_data) + 1 + current_poz, payload, payload_len);
				// 		current_poz += payload_len;
				// 	}
				// 	kfree(cpy_skb);
				// }
				pack->data_len = current_poz;
			}
			else{
				//cpy_skb = skb_copy(skb,GFP_ATOMIC);
				//payload = cpy_skb->data;
				printk(KERN_INFO "ici2.2\n");
				//payload_len = cpy_skb->data_len;
				
				//memcpy(pack + sizeof(struct package_data) + 1, payload, payload_len);
				//pack->data_len = payload_len;
				curr_dim = 0;
				curr_poz = skb->head;
				printk(KERN_INFO "data %p end %p tail %p and size %d\n", skb->data,skb->end, skb->tail, skb->len);
				if (skb->data)
					//while(curr_poz != skb->end && curr_poz != skb->tail && curr_dim < dim_max){
					while (curr_dim < dim_max + skb_headroom(skb)){
						//*((unsigned char*)(pack + sizeof(struct package_data) + 1 + curr_dim)) = *curr_poz;
						curr_dim++;
						printk(KERN_CONT "%02x ", *curr_poz);
						curr_poz++;
					}
				printk(KERN_INFO "ici3.2\n");
				pack->data_len = curr_dim;
				//kfree(cpy_skb);
				//pack->data = ((unsigned char*)(&pack->data)) + 1;
				//copy_uchar_values(payload,(unsigned char*)pack + sizeof(struct package_data) + 1,payload_len);
			}
			printk(KERN_INFO "9\n");
			// }
		}
		// else{
		// 	pack->data=NULL;
		// }
	}
	else return NULL;
	return pack;
}

unsigned char* create_package_data_v2(struct sk_buff* skb, int *col_size, bool with_data){
	unsigned char* pack;
	struct package_data* pack_pack;
	struct iphdr* ip_h;
	struct tcphdr* tcp_h;
	struct udphdr* udp_h;
	unsigned int payload_len;
	unsigned char *payload;
	struct skb_frag_struct *frag;
	unsigned int frag_size;
	skb_frag_t *skb_frag;
	int frag_index, nr_frags, i, curr_dim = 0;
	unsigned char current_poz=0;

	if(with_data){
		
		if (skb_is_nonlinear(skb)) {
			pack = (unsigned char*)kcalloc(skb->len ,sizeof(char),GFP_KERNEL);
			current_poz = 0;
			memcpy(pack + current_poz, skb->data, skb_headlen(skb));
			current_poz += skb_headlen(skb);
			nr_frags = skb_shinfo(skb)->nr_frags;
			for(frag_index=0;frag_index<nr_frags;frag_index++){
				skb_frag = &skb_shinfo(skb)->frags[frag_index];
				payload =  skb_frag_address_safe(skb_frag);
				payload_len = skb_frag_size(skb_frag);
				if(payload){
					memcpy(pack + current_poz, payload, payload_len);
					current_poz += payload_len;
				}
			}
			*col_size = current_poz;

		}
		else{
			pack = (unsigned char*)kcalloc(skb->len ,sizeof(char),GFP_KERNEL);
			memcpy(pack, skb->data, skb->len);
			*col_size = skb->len;
		}
		
	}
	else{
		pack = (unsigned char*)kcalloc(1,sizeof(struct package_data),GFP_KERNEL);
		if (!skb) {return NULL;}
		if (skb->len < 0 || skb->protocol < 0 || !skb->data){
			printk(KERN_ERR "Validation failed\n");
			return NULL;
		}
		ip_h = ip_hdr(skb);
		if(!ip_h) return NULL;

		pack_pack = (struct package_data*)pack;

		pack_pack->sourceIP = ip_h->saddr;
		//printk(KERN_INFO "2\n");
		pack_pack->destIP = ip_h->daddr;
		//printk(KERN_INFO "3\n");
		//printk(KERN_INFO "4\n");
		pack_pack->network_proto = skb->protocol;
		//printk(KERN_INFO "5\n");
		pack_pack->transport_proto = ip_h->protocol;
		//printk(KERN_INFO "6\n");
		if ( pack_pack->transport_proto == IPPROTO_TCP){
			tcp_h = tcp_hdr(skb);
			pack_pack->sourcePort = tcp_h->source;
			//printk(KERN_INFO "7.1\n");
			pack_pack->destPort = tcp_h->dest;
			//printk(KERN_INFO "7.2\n");
		}
		else if (pack_pack->transport_proto == IPPROTO_UDP){
			udp_h = udp_hdr(skb);
			pack_pack->sourcePort = udp_h->source;
			//printk(KERN_INFO "8.1\n");
			pack_pack->destPort = udp_h->dest;
			//printk(KERN_INFO "8.2\n");
		}
		*col_size = sizeof(struct package_data);
	}
	return pack;
}


void send_to_user_server(unsigned char* data,int data_size){
	struct socket* sock;
	struct msghdr msg;
	struct kvec vec;
	struct sockaddr_in sin;
	int ret;
	mm_segment_t oldmm;
	__be32 ip_dest = {1,0,0,127};

	// Create a TCP socket
	ret = sock_create(AF_INET, SOCK_STREAM, 0, &sock);
	if (ret < 0) {
		printk(KERN_ERR "Failed to create socket\n");
		return;
	}
	printk(KERN_INFO "Socket created\n");

	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_port = htons(SERVER_PORT);
	sin.sin_addr.s_addr = htonl(ip_dest);

	// Connect to the user-space server
	ret = sock->ops->connect(sock, (struct sockaddr *)&sin, sizeof(sin), O_RDWR);
	if (ret < 0) {
		printk(KERN_ERR "Failed to connect to server\n");
		sock_release(sock);
		return;
	}
	printk(KERN_INFO "Connected to server \n");
 	

        msg.msg_name    = 0;
        msg.msg_namelen = 0;
        msg.msg_control = NULL;
        msg.msg_controllen = 0;
        msg.msg_flags   = 0;

        //oldmm = get_fs(); set_fs(KERNEL_DS);

	vec.iov_len = sizeof(int);
        vec.iov_base = (char *)&data_size;

        //len = sock_sendmsg(sock, &msg, left);
        ret = kernel_sendmsg(sock, &msg, &vec, data_size, data_size);

	// kiov.iov_base = (void *)data;
	// kiov.iov_len = data_size;

	// memset(&msg, 0, sizeof(struct msghdr));
	// msg.msg_name = &sin;
	// msg.msg_namelen = sizeof(sin);
	// msg.msg_control = NULL;
	// msg.msg_controllen = 0;
	//msg.msg_iocb = &kiov;
	//msg.msg_iovlen = 1;

	// ret = sock_sendmsg(sock, &msg);
	if (ret < 0) {
		printk(KERN_ERR "Failed to send message to server\n");
		return;
	}
	printk(KERN_INFO "Message sent to server\n");

	sock_release(sock);
}

// void send_to_user_package(struct sock* netlink_socket,unsigned char* data,int data_size){
// 	struct nlmsghdr *nlh;
// 	struct infec_msg* msg;
// 	struct sk_buff *skb_out;
// 	int res;

// 	// msg = create_infec_msg((unsigned char*)clients_list,type,payload_id);
// 	// if(!msg){
// 	// 	printk(KERN_ERR "Failed to create new message\n");
// 	// 	goto cleanup;
// 	// }
// 	// printk(KERN_INFO "msg created\n");
// 	// print_infec_msg(msg);
	
// 	//create reply
// 	skb_out = nlmsg_new(data_size, 0);
// 	if (!skb_out) {
// 		printk(KERN_ERR "Failed to allocate new skb\n");
// 		goto cleanup;
// 	}
// 	printk(KERN_INFO "skbuff created\n");

// 	nlh = nlmsg_put(skb_out, 0, 0, NLMSG_DONE, data_size, 0);
// 	NETLINK_CB(skb_out).dst_group = 0; /* not in mcast group */
// 	copy_uchar_values((unsigned char*)data,(unsigned char *) NLMSG_DATA(nlh), data_size);
// 	printk(KERN_INFO "data copied\n");
// 	res = nlmsg_unicast(netlink_socket, skb_out, requester);
// 	if (res < 0)
// 		printk(KERN_ERR "Error while sending bak to user\n");
// 	printk(KERN_INFO "msg sent to user\n");
// cleanup:
// 	//if(msg) kfree(msg);

// }

//unicast
void send_to_user_broadcast(struct sock* netlink_socket,unsigned char* data, int size, unsigned char type,int payload_id , struct mutex *nl_mutex){
	struct nlmsghdr *nlh;
	struct infec_msg* msg;
	struct sk_buff *skb_out;
	int res;

	//msg = create_infec_msg(data,type,size);
	msg = create_packet_msg(data,size,type);
	if(!msg){
		printk(KERN_ERR "Failed to create new message\n");
		goto cleanup;
	}
	//printk(KERN_INFO "msg created\n");
	print_infec_msg(msg);
	
	//create reply
	skb_out = nlmsg_new(INF_MSG_LEN(msg), 0);
	if (!skb_out) {
		printk(KERN_ERR "Failed to allocate new skb\n");
		goto cleanup;
	}

	nlh = nlmsg_put(skb_out, 0, 0, NLMSG_DONE, INF_MSG_LEN(msg), 0);
	NETLINK_CB(skb_out).dst_group = 1; /* not in mcast group */
	memcpy((unsigned char *) NLMSG_DATA(nlh), (unsigned char*)msg, INF_MSG_LEN(msg));

	mutex_lock(nl_mutex);
	res = netlink_broadcast(netlink_socket, skb_out,0,1, GFP_KERNEL);
	mutex_unlock(nl_mutex);

	if (res < 0)
		printk(KERN_ERR "Error while sending bak to user\n");
	printk(KERN_INFO "msg sent to user\n");
cleanup:
	if(msg) kfree(msg);
	//if(skb_out) nlmsg_free(skb_out);
}