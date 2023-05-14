#include <linux/if_ether.h> // for ETH_ALEN

///source code from /linux/drdb.h
#ifdef __KERNEL__
#include <linux/types.h>
#include <asm/byteorder.h>
#else
#include <sys/types.h>
#include <sys/wait.h>
#include <limits.h>

#if __BYTE_ORDER == __LITTLE_ENDIAN
#define __LITTLE_ENDIAN_BITFIELD
#elif __BYTE_ORDER == __BIG_ENDIAN
#define __BIG_ENDIAN_BITFIELD
#else
# error "Undefined endianness"
#endif

#endif

#ifndef BIG_ENDIAN
#define BIG_ENDIAN
#endif

#define NETLINK_PROTO_INFECTED 25

#define SIGNITURE_HEADER 0x43435248   //CCRH
#define SIGNITURE_HEADER_LENGTH 4
#define ACK_LENGTH 2

#define ACK_OK 	0x4F4B     //OK
#define ACK_ER 	0x4552     //ER

#define SIGNITURE_IP 0x48
#define SIGNITURE_MAC 0x49
#define SIGNITURE_NR_ENT 0x50
#define SIGNITURE_INFECT_TYPE 0x51

#define MAX_LEN_ERROR 100
#define MAX_LEN_CONFIRM 100
#define MAX_PAYLOAD_SIZE 2048

#define UNKNOW_INFECTION 0

#define MAC_LEN ETH_ALEN
// REQUESTS
enum {
	/// CRUD ops for client infectivity
	ADD_CLIENT = 1,		//1
#define ADD_CLIENT ADD_CLIENT
	REMOVE_CLIENT,		//2
#define REMOVE_CLIENT REMOVE_CLIENT
	TRANSFER_CLIENT,	//3
#define TRANSFER_CLIENT TRANSFER_CLIENT
	GET_CLIENT,			//4
#define GET_CLIENT GET_CLIENT
	GET_CLIENTS,		//5
#define GET_CLIENTS GET_CLIENTS

};

//RESPONSES
enum{
	CONFIRM	= 10,		//10
#define CONFIRM CONFIRM
	ERROR,			//11
#define ERROR ERROR
	CLIENTS_DATA,		//12
#define CLIENTS_DATA CLIENTS_DATA
//inca ceva
};

/*

Visual Representation of the Protocol

       struct infect_msg
   0  1  2  3  4  5  6  7   bite
  -------------------------- ____
  |                        |     }
  |   header_payload 	   |     } 8 bytes
  |                        | ____}
  |------------------------|____
  |                        |     }
  |       payload          |     } payload_len
  |                        | ____}
  --------------------------


   struct header_payload
    0  1  2  3  4  5  6  7    bite
  --------------------------
0 |                        |
2 |      signiture         |
3 |                        |
4 |                        |
  |------------------------|
5 |      payload_id        |
  |------------------------|
6 |      payload_type      |
  |------------------------|
7 |      payload_len       |
8 |                        |
  --------------------------
byte

REQUESTS

layout payload ADD_CLIENT:
	   payload
     0  1  2  3  4  5  6  7   bite
   --------------------------
 0 |	  SIGNITURE_IP      |
   |------------------------|
 1 |                        |
 2 |         IPV4           |
 3 |                        |
 4 |                        |
   |------------------------|
 5 |	  SIGNITURE_MAC     |
   |------------------------|
 6 |                        |
 7 |                        |
 8 |         MAC            |
 9 |                        |
10 |                        |
11 |                        |
   |------------------------|
12 |                        |
13 |        Padding         |
   --------------------------
byte

layout payload REMOVE_CLIENT:
	same as ADD_CLIENT

layout payload GET_CLIENT:
	same as ADD_CLIENT

layout payload GET_CLIENTS:
	empty (payload_len = 0)

RESPONSES

layout payload CLIENTS_DATA:
		   payload
     0  1  2  3  4  5  6  7   bite
   -------------------------- ____
 0 |    SIGNITURE_NR_ENT    |     }
   |------------------------|     }
 1 |                        |     } 
 2 |       NR_CLIENTS       |     }-->NUMBER OF ENTITIES                
 3 |                        |     }
 4 |                        |     }
   |------------------------|_____}____________________________________ 
 5 |	  SIGNITURE_IP      |     }                                    }
   |------------------------|     }                                    }
 6 |                        |     }                                    }
 7 |         IPV4           |     }                                    }
 8 |                        |     }                                    }
 9 |                        |     }                                    }
   |------------------------|     }                                    }
10 |	  SIGNITURE_MAC     |     }-->STANDARD GET CLIENT PAYLOAD      }
   |------------------------|     }                                    }
11 |                        |     }                                    }
12 |                        |     }                                    }
13 |          MAC           |     }                                    }
14 |                        |     }                                    }
15 |                        |     }                                    }-->  NUMBER OF ENTITIES 
16 |                        |     }                                    }         X
   |------------------------|     }                                    }    sizeof(STANDARD GET CLIENT PAYLOAD)
17 | SIGNITURE_INFECT_TYPE  |     }                                    }
   |------------------------|     }                                    }
18 |    INFECTION_TYPE      |     }                                    }
   |------------------------| ____}                                    }
-  . . . . . . . . . . . . .                                           }    
-  . . . . . . . . . . . . .                                           }
-  . . . . . . . . . . . . .                                           }
-  . . . . . . . . . . . . .                                           }
   |------------------------|                                          }
a  |                        |                                          }
b  |                        |                                          }
c  |      STANDARD GET      |                                          }
-  |    CLIENT PAYLOAD      |                                          }
-  |                        |                                          }
-  |                        |                                          }
s  |                        | _________________________________________}
   |------------------------|
t  |                        |
u  |        Padding         |
   --------------------------
byte

layout payload CONFIRM:
	  payload
    0  1  2  3  4  5  6  7   bites
   --------------------------  ___
   |	                    |     }
   |      ADDITIONAL        |     }  MAX_LEN_CONFIRM 
   |        INFO            |     }
   |                        | ____}  
   |------------------------|
   |                        |
   |        Padding         |
   --------------------------

layout payload ERROR:
	    payload
     0  1  2  3  4  5  6  7   bites
   -------------------------- ____
   |	                    |     }
   |                        |     }      
   |         ERROR          |     }  MAX_LEN_ERROR
   |      DESCRIPTION       |     }
   |                        |     }
   |                        | ____}
   |------------------------|
   |                        |
   |        Padding         |
   --------------------------
*/

struct client_repr{
	int ip_addr;
	unsigned char mac_addr [ETH_ALEN];
	unsigned char infectivity;
};

struct client_repr_ext{
	unsigned char signiture_ip;
	int ip_addr;
	unsigned char signiture_mac;
	unsigned char mac_addr [ETH_ALEN];
	unsigned char signiture_infectivity;
	unsigned char infectivity;
};



struct header_payload{
	unsigned char signiture[SIGNITURE_HEADER_LENGTH];
	unsigned char payload_id;
	unsigned char payload_type;
	unsigned int payload_len; 
};

struct infec_msg{
	struct header_payload header;
};

static void ch2int(char* ch, int* col){
#ifdef __BIG_ENDIAN_BITFIELD
	*col = ((ch[0] & 0xFF) << 24) | ((ch[1] & 0xFF) << 16) | ((ch[2] & 0xFF) << 8) | (ch[3] & 0xFF);
#endif
#ifdef __LITTLE_ENDIAN_BITFIELD
	*col = ((ch[3] & 0xFF) << 24) | ((ch[2] & 0xFF) << 16) | ((ch[1] & 0xFF) << 8) | (ch[0] & 0xFF);
#endif
}

static void int2ch(int in, char* col){
#ifdef __LITTLE_ENDIAN_BITFIELD
	col[0] = (char)(in & 0xFF);  
	col[1] = (char)((in >> 8) & 0xFF);  
	col[2] = (char)((in >> 16) & 0xFF); 
	col[3] = (char)((in >> 24) & 0xFF);
#endif
#ifdef __BIG_ENDIAN_BITFIELD
	col[3] = (char)(in & 0xFF);  
	col[2] = (char)((in >> 8) & 0xFF);  
	col[1] = (char)((in >> 16) & 0xFF); 
	col[0] = (char)((in >> 24) & 0xFF);
#endif
}


#define CHECK_SIGNITURE(cand,sign) ((cand & sign) == sign)

#define CHECK_SIGNITURE_HEADER(infec_msg) ((infec_msg->header.signiture & SIGNITURE_HEADER) == SIGNITURE_HEADER)
//2 for padding cus C compiler may calculate the wrong sizeof(struct header_payload)
#define INF_MSG_LEN(infec_msg) (infec_msg->header.payload_len + sizeof(struct header_payload) + 2)
#define INF_MSG_LEN_H(hdr) (hdr->payload_len + sizeof(struct header_payload) + 2)
#define INF_MSG_HEADER(infec_msg) ((void*)(&infec_msg->header))
#define INF_MSG_START(infec_msg) INF_MSG_HEADER(infec_msg)
#define INF_MSG_HEADER_LEN(infec_msg) (sizeof(struct header_payload))
#define INF_MSG_DATA(infec_msg) ((void*)((char*)(&infec_msg->header) + infec_msg->header.payload_len))
#define INF_MSG_DATA_LEN(infec_msg) (infec_msg->header.payload_len)
#define INF_MSG_END(infec_msg) ((void*)(((char *)(&infec_msg->header)) + sizeof(struct header_payload) + infec_msg->header.payload_len))

#define MULTI_CLIENTS_PAYLOAD_SIZE(nr_clients) (nr_clients*sizeof(struct client_repr_ext) + 5)
#define MULTI_CLIENTS_MSG_SIZE(nr_clients) MULTI_CLIENTS_PAYLOAD_SIZE(nr_clients) + sizeof(struct header_payload)
#define CONFIRM_MSG_SIZE(nr_clients) MAX_LEN_CONFIRM + sizeof(struct header_payload)
#define ERROR_MSG_SIZE(nr_clients) MAX_LEN_ERROR + sizeof(struct header_payload)


static void copy_uchar_values(unsigned char* from, unsigned char* to, size_t size){
    unsigned char i=0;
	for(i=0;i< size;i++){
        to[i]=from[i];
    }
}

static int cmp_uchar_values(unsigned char* hash1, unsigned char* hash2, size_t size){
    unsigned char i=0;
	for(i=0;i< size;i++){
        int dif = hash1[i] - hash2[i];
        if(dif != 0) return dif;
    }
    return 0;
}


#define FLAG_WITH_IP 0001
#define FLAG_WITH_MAC 0010
#define FLAG_WITH_INFECTIVITY 0100

#define CHECK_FLAG(cand,flag) CHECK_SIGNITURE(cand,flag)

static int extract_client_repr_payload(struct infec_msg* msg, struct client_repr* client_collector, unsigned char poz, unsigned char flags){
	unsigned char* data = INF_MSG_DATA(msg);
	int initial_poz = poz;
	if(CHECK_SIGNITURE(data[poz],SIGNITURE_IP) && CHECK_FLAG(flags,FLAG_WITH_IP)){
		poz++;
		ch2int(&data[poz],&client_collector->ip_addr);
		poz +=4;
	}
	else{
		client_collector->ip_addr = 0;
	}
	if(CHECK_SIGNITURE(data[poz],SIGNITURE_MAC) && CHECK_FLAG(flags,FLAG_WITH_MAC)){
		copy_uchar_values(&data[poz+1],client_collector->mac_addr,ETH_ALEN);
		// printf(KERN_INFO "Mac extracted %02X:%02X:%02X:%02X:%02X:%02X\n",client_collector->mac_addr[0],
		// client_collector->mac_addr[1],client_collector->mac_addr[2],client_collector->mac_addr[3],
		// client_collector->mac_addr[4],client_collector->mac_addr[5]);
		poz +=ETH_ALEN+1;
	}
	else{
		unsigned char empty[ETH_ALEN]={0};
		copy_uchar_values(empty,client_collector->mac_addr,ETH_ALEN);
	}
	if(CHECK_SIGNITURE(data[poz],SIGNITURE_INFECT_TYPE) && CHECK_FLAG(flags,FLAG_WITH_INFECTIVITY)){
		client_collector->infectivity = data[poz+1];
		poz+=2;
	}
	else{
		client_collector->infectivity=UNKNOW_INFECTION;
	}
	poz+=2; //this is for padding do not delete!!!
	return poz-initial_poz;
}

static int create_client_repr_payload(struct client_repr* client,unsigned char* collector, unsigned char flags){
	int poz =0;
	collector[poz]= SIGNITURE_IP;
	poz++;
	int2ch(client->ip_addr,&collector[poz]);
	poz+=4;
	collector[poz]= SIGNITURE_MAC;
	poz++;
	copy_uchar_values(client->mac_addr,&collector[poz],ETH_ALEN);
	poz+=ETH_ALEN;
	if(CHECK_FLAG(flags,FLAG_WITH_INFECTIVITY)){
		collector[poz]= SIGNITURE_INFECT_TYPE;
		poz++;
		collector[poz]= client->infectivity;
		poz++;
	}
	poz+= 2;// this is for padding, do not delete!
	return poz;
}

static int create_header(char id,char type,struct header_payload *collector){
	collector->payload_id=id;
	collector->payload_type=type;
	int cp = SIGNITURE_HEADER;
	int2ch(cp,collector->signiture);
	collector->payload_len = 0;
	return 0;
}

static int create_message(struct header_payload *header, unsigned char* data, int data_len, struct infec_msg* collector){
	header->payload_len = data_len;
	if(header)
		copy_uchar_values((unsigned char*)header, (unsigned char*)INF_MSG_HEADER(collector),INF_MSG_HEADER_LEN(collector));
	if(data)
		copy_uchar_values((unsigned char*)data, (unsigned char*)INF_MSG_DATA(collector), data_len);
	return 0;
}