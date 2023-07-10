#include <linux/module.h>  
#include <linux/kernel.h>  
#include <linux/fs.h>
#include <linux/list.h>
#include <linux/types.h>

unsigned int interceptor_hook_handle(void *priv, struct sk_buff *skb, const struct nf_hook_state *state);

//unsigned int local_hook_handle(void *priv, struct sk_buff *skb, const struct nf_hook_state *state);

int initialise_interceptor(void);

int clear_interceptor(void);