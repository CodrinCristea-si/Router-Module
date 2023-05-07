#include <linux/module.h>  
#include <linux/kernel.h>  
#include <linux/init.h>
#include <linux/types.h>
// #include "logger.h"
// #include "interceptor.h"
#include "../headers/infectivity.h"
#include "../headers/interceptor.h"
#include "../headers/utils.h"
//#include "../headers/connectivity_thread.h"

static int __init initialize(void)
{
//     unsigned char mac1[6] = {0x56,0xcc,0x0e,0xbd,0xb9,0x48};
//     unsigned char mac2[6] = {0x36,0x70,0xec,0xe5,0x6a,0xc4};
//     unsigned char mac3[6] = {0xbe,0xd0,0x74,0x84,0x95,0x78};

//     __be32 ip1 = 0x0a40a8c0;
//     __be32 ip2 = 0x0b40a8c0;
//     __be32 ip3 = 0x0c40a8c0;
//     int rez = 0;

     if(__initialize_infectivity_lists()) goto cleanup;
//     rez = ADD_CLIENT_UNINFECTED(ip1,mac1);
//     printk(KERN_ALERT "Res %d\n", rez);
//     rez = ADD_CLIENT_SUSPICIOUS(ip2,mac2);
//     printk(KERN_ALERT "Res %d\n", rez);
//     rez = ADD_CLIENT_INFECTED_MAJOR(ip3,mac3);
//     printk(KERN_ALERT "Res %d\n", rez);

//     PRINT_LIST_UNINFECTED();
//     PRINT_LIST_SUSPICIOUS();
//     PRINT_LIST_INFECTED_MAJOR();
    
//     transfer_client(UNINFECTED,SUSPICIOUS,ip1,mac1);
//     PRINT_LIST_UNINFECTED();
//     PRINT_LIST_SUSPICIOUS();
//     PRINT_LIST_INFECTED_MAJOR();

//     __clear_infectivity_lists();
    //init_thread();

    if(initialise_interceptor()) clear_interceptor();
    return 0;
cleanup:
    __clear_infectivity_lists();
    return 0;
}

static void __exit uninstall(void)
{
    clear_interceptor();
    __clear_infectivity_lists();
    printk(KERN_INFO "Module extracted!\n");
}

module_init(initialize);
module_exit(uninstall);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Infectivity Test");
MODULE_DESCRIPTION("Infectivity Test");