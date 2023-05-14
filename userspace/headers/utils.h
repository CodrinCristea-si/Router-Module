#define REGEX_LEASE "^([0-9])*$"
#define REGEX_MAC "^([0-9A-Fa-f]{2}:){5}([0-9A-Fa-f]{2})$"
#define REGEX_IPV4 "^([0-9]{1,3}\\.){3}([0-9]{1,3})$"
#define REGEX_NAME "^([0-9a-zA-Z_\\-]+)$|^(\\*)$"

void str2ipv4(unsigned char * str, unsigned char* collector);

char hex_value_str(char ch);

void str2mac(unsigned char * str, unsigned char* collector);

int compare_regex(char *str, char *regex_str);