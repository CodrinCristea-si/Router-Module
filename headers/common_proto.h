/*
	Something
*/
#define NETLINK_PROTO_INFECTED 25

#define SIGNITURE_HEADER 0x43435248   //CCRH
#define SIGNITURE_HEADER_LENGTH 4
#define ACK_LENGTH 2

#define ACK_OK 	0x4F4B     //OK
#define ACK_ER 	0x4552     //ER

#define SIGNITURE_IP 0x48
#define SIGNITURE_MAC 0x49
#define SIGNITURE_INFECT_TYPE 0x50

#define MAX_LEN_ERROR 100

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
//inca ceva
};

/*

Visual Representation of the Protocol

     struct infect_msg
  0  1  2  3  4  5  6  7   bite
 -------------------------- ____
 |	                  |	}
 |     header_payload     |	} 8 bytes
 |	                  | ____}
 -------------------------- ____
 |	                  |     }
 |        payload         |     } payload_len
 |	                  | ____}
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
7 |	 payload_len       |
8 |	                   |
  --------------------------
byte

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
 7 |         IPV4           |
 8 |                        |
 9 |                        |
10 |                        |
11 |                        |
   |------------------------|
byte

layout payload REMOVE_CLIENT:
	same as ADD_CLIENT

layout payload CONFIRM:
	empty (payload_len = 0)

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
   --------------------------
*/
struct header_payload{
	unsigned char signiture[SIGNITURE_HEADER_LENGTH];
	unsigned char payload_id;
	unsigned char payload_type;
	unsigned int payload_len; 
};

struct infec_msg{
	struct header_payload header;
};

static void ch2int_be(char* ch, int* col){
	*col = ((ch[0] & 0xFF) << 24) | ((ch[1] & 0xFF) << 16) | ((ch[2] & 0xFF) << 8) | (ch[3] & 0xFF);
}

static void ch2int_le(char* ch, int* col){
	*col = ((ch[3] & 0xFF) << 24) | ((ch[2] & 0xFF) << 16) | ((ch[1] & 0xFF) << 8) | (ch[0] & 0xFF);
}

static void int2ch_be(int in, char* col){
	col[0] = (char)(in & 0xFF);  
	col[1] = (char)((in >> 8) & 0xFF);  
	col[2] = (char)((in >> 16) & 0xFF); 
	col[3] = (char)((in >> 24) & 0xFF);
}

static void int2ch_le(int in, char* col){
	col[3] = (char)(in & 0xFF);  
	col[2] = (char)((in >> 8) & 0xFF);  
	col[1] = (char)((in >> 16) & 0xFF); 
	col[0] = (char)((in >> 24) & 0xFF);
}

#define CHECK_SIGNITURE(cand,sign) ((cand & sign) == sign)

#define CHECK_SIGNITURE_HEADER(infec_msg) ((infec_msg->header.signiture & SIGNITURE_HEADER) == SIGNITURE_HEADER)

#define INF_MSG_LEN(infec_msg) (infec_msg->header.payload_len + sizeof(struct header_payload))
#define INF_MSG_LEN_H(hdr) (hdr->payload_len + sizeof(struct header_payload))
#define INF_MSG_HEADER(infec_msg) ((void*)(&infec_msg->header))
#define INF_MSG_START(infec_msg) INF_MSG_HEADER(infec_msg)
#define INF_MSG_HEADER_LEN(infec_msg) (sizeof(struct header_payload))
#define INF_MSG_DATA(infec_msg) ((void*)((char*)(&infec_msg->header) + infec_msg->header.payload_len))
#define INF_MSG_DATA_LEN(infec_msg) (infec_msg->header.payload_len)
#define INF_MSG_END(infec_msg) ((void*)(((char *)(&infec_msg->header)) + sizeof(struct header_payload) + infec_msg->header.payload_len))