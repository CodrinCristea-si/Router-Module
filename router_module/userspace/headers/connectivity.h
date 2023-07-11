#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <regex.h>
#include <pthread.h>
#include <linux/netlink.h>
#include <sys/socket.h>
#include <time.h>

#include "communication_local.h"
#include "utils.h"


#define WORD_SEPARATOR " "
#define MAX_WORD_SIZE 100

#define MASK_NAME_ASSIGNED 001u
#define MASK_IPV4_ASSIGNED 010u
#define MASK_MAC_ASSIGNED 100u
#define MASK_CLIENT_COMPLETE 111u
#define MASK_CLIENT_EMPTY 000u

#define MD5_HASH_SIZE 16
#define MAC_LENGTH 6
#define IPV4_LENGTH 4


enum {
	FORMAT_LEASE=1,
	FORMAT_MAC,
	FORMAT_IPV4,
	FORMAT_NAME,
	FORMAT_UNKNOWN
};

struct client_def{
	char ipv4[MAX_WORD_SIZE];
	char mac[MAX_WORD_SIZE];
	char name[MAX_WORD_SIZE];
	u_int32_t complete_mask;
};

struct client_node{
    unsigned char ipv4[IPV4_LENGTH];
    unsigned char mac[MAC_LENGTH];
    unsigned char hash[MD5_HASH_SIZE];
    unsigned int state;
    void* next;
};

void convert_node_2_repr(struct client_node* client,struct client_repr* collector);

int hash_client(struct client_def *client, char* hash_collector);

int get_type_of_dhcp_word(char* word);

void process_word_on_client(char*word, struct client_def *client);

void clear_client(struct client_def *client);

bool insert_or_update_client(struct client_node* list, struct client_node* node);

void delete_old_magic(struct client_node* list);

bool process_client(struct client_def *client);

void process_dhcp_file(char* filename);

time_t check_if_file_was_modified(char* filename, time_t last_modified);

bool is_file_available(char* filename);

void connectivity_checker(char* file);