#include "../headers/utils.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <regex.h>
#include <ctype.h>


int compare_regex(char *str, char *regex_str){
	regex_t regex_expr;
	int status;

	status = regcomp(&regex_expr, regex_str,REG_EXTENDED);
	if(status) {
		//printf("-1\n");
		regfree(&regex_expr);
		return -1;
	}
	status = regexec(&regex_expr,str,0,NULL,0);
	if(status == 0) {
		//printf("0\n");
		regfree(&regex_expr);
		return 0;
	}
	//printf("1\n");
	regfree(&regex_expr);
	return 1;
}


void str2ipv4(unsigned char * str, unsigned char* collector){
    char buf[4];
    char nr;
    int len =0, poz =0;
    for(size_t i =0;i<strlen(str);i++){
        if(str[i] == '.'){
            buf[len++] = '\0';
            nr = (char)atoi(buf);
            collector[poz++]=nr;
            len =0;
        }
        else buf[len++] = str[i];
    }
    buf[len++] = '\0';
    nr = (char)atoi(buf);
    collector[poz++]=nr;
    len =0;
}

char hex_value_str(char ch){
    if(isdigit(ch)) return ch -'0';
    else{
        switch (ch)
        {
        case 'a':
        case 'A':
            return 10;
        case 'b':
        case 'B':
            return 11;
        case 'c':
        case 'C':
            return 12;
        case 'd':
        case 'D':
            return 13;
        case 'e':
        case 'E':
            return 14;
        case 'f':
        case 'F':
            return 15;
        default:
            return 0;
        }
    }
}

void str2mac(unsigned char * str, unsigned char* collector){
    char buf[4];
    char nr;
    int len =0, poz =0;
    for(size_t i =0;i<strlen(str);i++){
        if(str[i] == ':'){
            buf[len++] = '\0';
            nr = hex_value_str(buf[0]) * 16 + hex_value_str(buf[1]);
            collector[poz]=nr;
            poz++;
            len=0;
        }
        else buf[len++] = str[i];
    }
    buf[len++] = '\0';
    nr = hex_value_str(buf[0]) * 16 + hex_value_str(buf[1]);
    collector[poz]=nr;
    poz++;
}