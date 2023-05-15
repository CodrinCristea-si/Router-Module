#include <linux/module.h>  
#include <linux/kernel.h> 
#include <linux/list.h>
#include <linux/types.h>
#include <uapi/linux/if_ether.h>
#include "../headers/utils.h"
#include "../headers/infectivity.h"

unsigned char LOCKDOWN_MODE;

struct clients_list* uninfected_list = NULL;
struct clients_list* suspicious_list = NULL;
struct clients_list* infected_minor_list = NULL;
struct clients_list* infected_major_list = NULL;
struct clients_list* infected_sever_list = NULL;

int __add_client_to_list(struct clients_list* list, const __be32 client_ip_addr, const unsigned char* client_mac_addr, const unsigned char type){
	struct clients_list* new_client;
	//printk(KERN_INFO "Add %p and %p and %pI4\n",list,client_mac_addr,&client_ip_addr);
	if(!list || !client_mac_addr || client_ip_addr == 0)
		return -2;
	new_client = (struct clients_list *) kcalloc(1,sizeof(struct clients_list), GFP_KERNEL);
	if(!new_client) goto cleanup;
	new_client->client.ip_addr = client_ip_addr;
	// new_client->mac_addr = (unsigned char *) kcalloc(ETH_ALEN,sizeof(unsigned char), GFP_KERNEL);
	// if(!new_client->mac_addr) goto cleanup;
	copy_mac_address(client_mac_addr,new_client->client.mac_addr);
	new_client->client.infectivity = type;
	list_add(&new_client->list,&list->list);
	printk(KERN_INFO "Client with ip %pI4 and mac %02X:%02X:%02X:%02X:%02X:%02X added to type %d\n", &client_ip_addr,
		client_mac_addr[0],client_mac_addr[1],client_mac_addr[2],client_mac_addr[3],client_mac_addr[4],client_mac_addr[5], type);
	return 0;

cleanup:
	if(new_client && new_client->client.mac_addr){
		kfree(new_client->client.mac_addr);
	}
	if(new_client){
		kfree(new_client);
	}
    	printk(KERN_ERR "Failed to add client with ip %pI4 and mac %02X:%02X:%02X:%02X:%02X:%02X \n",&client_ip_addr,
            client_mac_addr[0],client_mac_addr[1],client_mac_addr[2],client_mac_addr[3],client_mac_addr[4],client_mac_addr[5]);
	return -1;
}

bool __add_client_generic(const __be32 client_ip_addr, const unsigned char* client_mac_addr){
	bool added = false;
	int res;
	struct client_def* old_client = NULL;
	old_client = __get_client_generic(client_ip_addr,client_mac_addr);
	printk(KERN_INFO "old_client %p\n", old_client);
	if(old_client == NULL){
		res = ADD_CLIENT_SUSPICIOUS(client_ip_addr,client_mac_addr);
		printk(KERN_INFO "res %d\n", res);
		if(res >= 0) added = true; 
	}
	return added;
}

bool __transfer_client(unsigned char status_from, unsigned char status_to, const __be32 ip_addr, const unsigned char* mac_addr){
	struct clients_list *list_from, *list_to;
	bool ok;
	bool transfered = false;
	int status;

	list_from = __get_list_according_to_status(status_from);
	list_to = __get_list_according_to_status(status_to);
	if(list_from && list_to){
		ok = __remove_client_from_list(list_from,ip_addr,mac_addr);
		if(ok){
			status = __add_client_to_list(list_to,ip_addr,mac_addr,status_to);
			if(!status) transfered = true;
		}
	}
	return transfered;
}

bool __transfer_client_generic( unsigned char status_to, const __be32 ip_addr, const unsigned char* mac_addr){
	struct clients_list *list_from, *list_to;
	struct client_def *client;
	bool transfered = false;
	client = GET_CLIENT_GENERIC(ip_addr,mac_addr);
	if(client){
		list_from = __get_list_according_to_status(client->infectivity);
		list_to = __get_list_according_to_status(status_to);
		transfered = __transfer_client(client->infectivity, status_to,ip_addr,mac_addr);
	}
	return transfered;
}

struct clients_list* __get_list_according_to_status(unsigned char status){
	switch(status){
		case UNINFECTED:{
			return uninfected_list;
		}
		case SUSPICIOUS:{
			return suspicious_list;
		}
		case INFECTED_MINOR:{
			return infected_minor_list;
		}
		case INFECTED_MAJOR:{
			return infected_major_list;
		}
		case INFECTED_SEVER:{
			return infected_sever_list;
		}
		default:{
			return NULL;
		}
	}
}

bool __remove_client_from_list(struct clients_list* list_from, const __be32 client_ip_addr, const unsigned char* client_mac_addr){
    struct clients_list* listptr, * tmp;
    bool deleted = false;
    if(list_from){
        // version 1
        list_for_each_entry_safe(listptr,tmp,&list_from->list,list){
            if(listptr ->client.ip_addr == client_ip_addr &&  cmp_mac_address(listptr->client.mac_addr, client_mac_addr) == 0){
                list_del(&listptr->list);
                kfree(listptr);
                deleted = true;
                printk(KERN_INFO "Client with ip %pI4 and mac %02X:%02X:%02X:%02X:%02X:%02X removed from type %d\n", &client_ip_addr,
                    client_mac_addr[0],client_mac_addr[1],client_mac_addr[2],client_mac_addr[3],client_mac_addr[4],client_mac_addr[5], list_from->client.infectivity);
                break;
            }
        }

        /*
        //version 2 __get_client_from_list must not be const
        tmp = __get_client_from_list(list_from, client_ip_addr, client_mac_addr);
        if(tmp){
           list_del(&tmp->list);
            kfree(listptr); 
            deleted = true;
        }
        */
    }
    return deleted;
}

bool __remove_client_generic(const __be32 client_ip_addr, const unsigned char* client_mac_addr){
    struct client_def *client;
    struct clients_list* list;
    bool deleted = false;
	client = GET_CLIENT_GENERIC(client_ip_addr,client_mac_addr);
	if(client){
		list = __get_list_according_to_status(client->infectivity);
		deleted = __remove_client_from_list(list,client_ip_addr,client_mac_addr);
	}
    return deleted;
}

struct client_def* __get_client_from_list(struct clients_list* list_from, const __be32 client_ip_addr, const unsigned char* client_mac_addr){
    struct clients_list *tmp=NULL;
    struct list_head *listptr;
    if(list_from){
        list_for_each(listptr, &list_from->list) {
            tmp = list_entry(listptr, struct clients_list, list);
            if(tmp->client.ip_addr == client_ip_addr &&  cmp_mac_address(tmp->client.mac_addr, client_mac_addr) == 0){
                break;
            }
	    tmp = NULL;
        }
    }
    return tmp ==NULL ? NULL : &tmp->client;
}

struct client_def* __get_client_generic(const __be32 client_ip_addr, const unsigned char* client_mac_addr){
	struct client_def* client = NULL;
	client = GET_CLIENT_UNINFECTED(client_ip_addr,client_mac_addr);
	if(client != NULL)
		return client;
	client = GET_CLIENT_SUSPICIOUS(client_ip_addr,client_mac_addr);
	if(client != NULL)
		return client;
	client = GET_CLIENT_INFECTED_MINOR(client_ip_addr,client_mac_addr);
	if(client != NULL)
		return client;
	client = GET_CLIENT_INFECTED_MAJOR(client_ip_addr,client_mac_addr);
	if(client != NULL)
		return client;
	client = GET_CLIENT_INFECTED_SEVER(client_ip_addr,client_mac_addr);
	if(client != NULL)
		return client;
	return client;
}

struct client_def* __get_client_from_list_by_mac(struct clients_list* list_from, const unsigned char* client_mac_addr){
    struct clients_list *tmp=NULL;
    struct list_head *listptr;
    if(list_from){
        list_for_each(listptr, &list_from->list) {
            tmp = list_entry(listptr, struct clients_list, list);
            if(cmp_mac_address(tmp->client.mac_addr, client_mac_addr) == 0){
                break;
            }
        }
    }
    return &tmp->client;
}

int __get_size_of_clients_list(struct clients_list* cl_list){
	int nr;
	struct clients_list *listptr, *tmp;
	nr =0;
	if(cl_list){
		list_for_each_entry_safe(listptr, tmp, &cl_list->list, list){
			nr++;
		}
	}
	return nr;
}

int __get_all_clients(struct clients_list* list_collector){
	struct clients_list *listptr, *tmp;
	int nr_cl=0;
	if(list_collector){
		list_for_each_entry_safe(listptr, tmp, &uninfected_list->list, list) {
			__add_client_to_list(list_collector,listptr->client.ip_addr, listptr->client.mac_addr,listptr->client.infectivity);
			nr_cl++;
		}
		list_for_each_entry_safe(listptr, tmp, &suspicious_list->list, list){
			__add_client_to_list(list_collector,listptr->client.ip_addr, listptr->client.mac_addr,listptr->client.infectivity);
			nr_cl++;
		}
		list_for_each_entry_safe(listptr, tmp, &infected_minor_list->list, list){
			__add_client_to_list(list_collector,listptr->client.ip_addr, listptr->client.mac_addr,listptr->client.infectivity);
			nr_cl++;
		}
		list_for_each_entry_safe(listptr, tmp, &infected_major_list->list, list){
			__add_client_to_list(list_collector,listptr->client.ip_addr, listptr->client.mac_addr,listptr->client.infectivity);
			nr_cl++;
		}
		list_for_each_entry_safe(listptr, tmp, &infected_sever_list->list, list){
			__add_client_to_list(list_collector,listptr->client.ip_addr, listptr->client.mac_addr,listptr->client.infectivity);
			nr_cl++;
		}
	}
	return nr_cl;
}

void __print_list(struct clients_list* list_from){
    struct clients_list *tmp, *listptr;
    if(list_from)
        list_for_each_entry_safe(listptr, tmp, &list_from->list, list) {
            printk(KERN_INFO "Client with ip %pI4 and mac %02X:%02X:%02X:%02X:%02X:%02X and status %d \n", &listptr->client.ip_addr,
		listptr->client.mac_addr[0],listptr->client.mac_addr[1],listptr->client.mac_addr[2],listptr->client.mac_addr[3],
		listptr->client.mac_addr[4],listptr->client.mac_addr[5], listptr->client.infectivity);
        }
}

inline bool __check_status_validity(const int status){
    return (status == UNINFECTED) || (status == SUSPICIOUS) || (status == INFECTED_MINOR) \
        || (status == INFECTED_MAJOR) || (status == INFECTED_SEVER);   
}

inline struct client_def* __create_empty_client(void){
	struct client_def* empty_client = NULL;
	empty_client = (struct client_def *) kcalloc(1,sizeof(struct client_def), GFP_KERNEL);
	if(empty_client){
		empty_client->ip_addr = 0;
		//empty_client->mac_addr = {-1,-1,-1,-1,-1,-1};
		empty_client->infectivity = MIN_GRADE;
	}
	return empty_client;
}

inline struct clients_list* __create_empty_list(void){
	struct clients_list* lista = NULL;
	struct client_def* empty_client;
	empty_client= __create_empty_client();
	if (empty_client){
		lista = (struct clients_list*) kcalloc(1,sizeof(struct clients_list), GFP_KERNEL);
		if(lista){
			lista->client = *empty_client;
			INIT_LIST_HEAD(&lista->list);
		}
		if(empty_client){
			kfree(empty_client);
		}
	}
	return lista;
}

int __initialize_infectivity_lists(void){
	struct client_def* empty_client; 
	//initialize a header client
	empty_client = (struct client_def *) kcalloc(1,sizeof(struct client_def), GFP_KERNEL);
	if(!empty_client) goto cleanup;
	empty_client->ip_addr = 0;
	//empty_client->mac_addr = {-1,-1,-1,-1,-1,-1};
	empty_client->infectivity = MIN_GRADE;

	uninfected_list = (struct clients_list*) kcalloc(1,sizeof(struct clients_list), GFP_KERNEL);
	if(!uninfected_list) goto cleanup;
	uninfected_list->client = *empty_client;
	uninfected_list->client.infectivity = UNINFECTED;
	INIT_LIST_HEAD(&uninfected_list->list);

	suspicious_list = (struct clients_list*) kcalloc(1,sizeof(struct clients_list), GFP_KERNEL);
	if(!suspicious_list) goto cleanup;
	suspicious_list->client = *empty_client;
	suspicious_list->client.infectivity = SUSPICIOUS;
	INIT_LIST_HEAD(&suspicious_list->list);

	infected_minor_list = (struct clients_list*) kcalloc(1,sizeof(struct clients_list), GFP_KERNEL);
	if(!infected_minor_list) goto cleanup;
	infected_minor_list->client = *empty_client;
	infected_minor_list->client.infectivity = INFECTED_MINOR;
	INIT_LIST_HEAD(&infected_minor_list->list);

	infected_major_list = (struct clients_list*) kcalloc(1,sizeof(struct clients_list), GFP_KERNEL);
	if(!infected_major_list) goto cleanup;
	infected_major_list->client = *empty_client;
	infected_major_list->client.infectivity = INFECTED_MAJOR;
	INIT_LIST_HEAD(&infected_major_list->list);

	infected_sever_list = (struct clients_list*) kcalloc(1,sizeof(struct clients_list), GFP_KERNEL);
	if(!infected_sever_list) goto cleanup;
	infected_sever_list->client = *empty_client;
	infected_sever_list->client.infectivity = INFECTED_SEVER;
	INIT_LIST_HEAD(&infected_sever_list->list);

	if(empty_client)
		kfree(empty_client);
	printk(KERN_INFO "Lists initialized with success!\n");
	return 0;

cleanup:
	__clear_infectivity_lists();
	if(empty_client){
	kfree(empty_client);
		printk(KERN_INFO "cleared empty_client\n");
	}
	printk(KERN_ERR "Lists failed to initialize!\n");
	return -1;
}

int __clear_list(struct clients_list* collector){
    struct clients_list *listptr, *tmp;
    if(collector){
	//__print_list(collector);
        list_for_each_entry_safe(listptr,tmp,&collector->list,list){
		if(listptr){
			list_del(&listptr->list);
			kfree(listptr);
		}
        }
        return 0;
    }
    return -1;
}

int __clear_infectivity_lists(void){
    if(uninfected_list){
	__clear_list(uninfected_list);
	kfree(uninfected_list);
	uninfected_list = NULL;
	printk(KERN_INFO "cleared uninfected_list\n");
    } 
    if(suspicious_list){
	__clear_list(suspicious_list);
	kfree(suspicious_list);
	suspicious_list=NULL;
	printk(KERN_INFO "cleared suspicious_list\n");
    }
    if(infected_minor_list){
        __clear_list(infected_minor_list);
        kfree(infected_minor_list);
	infected_minor_list=NULL;
	printk(KERN_INFO "cleared infected_minor_list\n");
    }
    if(infected_major_list){
        __clear_list(infected_major_list);
        kfree(infected_major_list);
	infected_major_list=NULL;
	printk(KERN_INFO "cleared infected_major_list\n");
    }
    if(infected_sever_list){
        __clear_list(infected_sever_list);
        kfree(infected_sever_list);
	infected_sever_list=NULL;
	printk(KERN_INFO "cleared infected_sever_list\n");
    }
    printk(KERN_INFO "Lists cleared with success!\n");
    return 0;
}