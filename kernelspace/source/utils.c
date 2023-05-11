#include <linux/module.h>
#include <linux/netdevice.h>
#include <linux/inetdevice.h>
#include <linux/list.h>
#include <linux/types.h>
#include <uapi/linux/if_ether.h>
#include "../headers/utils.h"

int get_network_interfaces_list(struct network_details* collector)
{
    struct net_device *dev;
    struct in_device *in_dev;
    struct in_ifaddr *ifa;
    struct network_details* data;

    for_each_netdev(&init_net, dev) {
        if (dev->flags & IFF_UP) {
            in_dev = in_dev_get(dev);
            if (in_dev) {
                for (ifa = in_dev->ifa_list; ifa; ifa = ifa->ifa_next) {
                    if (ifa->ifa_address && ifa->ifa_mask) {
                        data = (struct network_details*)kcalloc(1,sizeof(struct network_details), GFP_KERNEL);
                        if(data){
                            data -> ip_addr = ifa->ifa_address;
                            data -> mask = ifa->ifa_mask;
                            data -> subnet_ip = ifa->ifa_address & ifa->ifa_mask;
                            list_add(&data->list,&collector->list);
                        }
                    }
                }
                in_dev_put(in_dev);
            }
        }
    }
    return 0;
}

struct network_details get_lan_network_from_list(struct network_details* collector){
    struct network_details*tmp=NULL;
    struct list_head *listptr;
    bool found = false;
    struct network_details lan_network = {
            .subnet_ip = 0,
            .mask = 0,
            .ip_addr = 0,
            .list = NULL,
    };

    if(collector){
        list_for_each(listptr, &collector->list) {
                tmp = list_entry(listptr, struct network_details, list);
                if((tmp -> subnet_ip & MASK_C) == SUBNET_C){
                    found = true;
                    break;
                }
                else if((tmp -> subnet_ip & MASK_B) == SUBNET_B){
                    found = true;
                    break;
                }
                else if((tmp -> subnet_ip & MASK_A) == SUBNET_A){
                    found = true;
                    break;
                }
        }
        if(tmp && found){
            lan_network.subnet_ip =  tmp->subnet_ip;
            lan_network.ip_addr = lan_network.subnet_ip | 0X00000001;
            lan_network.mask =  tmp->mask;
        }
    }
    return lan_network;
}

bool check_ip_belong_to_network(struct network_details* network, __be32 ip_add){
    return (network->mask & ip_add) == network ->subnet_ip;
}

inline int copy_mac_address(const unsigned char* from, unsigned char* to){
    unsigned char i;
    if(from && to){
        for(i =0;i<ETH_ALEN;i++){
            to[i]=from[i];
        }
        return 0;
    }
    return -1;
}

inline int cmp_mac_address(const unsigned char* mac1, const unsigned char* mac2){
    int sum = 0;
    unsigned char i;
    if(mac1 && mac2){
        for(i=0;i<ETH_ALEN;i++){
            sum += mac1[i] - mac2[i];
        }
    }
    return sum;
}

int initialize_network_interfaces_list(struct network_details* collector){
    if(collector){
        collector->subnet_ip = EMPTY_IPV4_ADDRESS;
        collector->mask = EMPTY_IPV4_ADDRESS;
        collector->ip_addr = EMPTY_IPV4_ADDRESS;
        INIT_LIST_HEAD(&collector->list);
        return 0;
    }
    return -1;
}

int clear_network_interfaces_list(struct network_details* collector){
    struct network_details* listptr, * tmp;
    if(collector){
        list_for_each_entry_safe(listptr,tmp,&collector->list,list){
                list_del(&listptr->list);
                kfree(listptr);
        }
        return 0;
    }
    return -1;
}
