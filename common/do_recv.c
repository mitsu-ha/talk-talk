/*************************************************************************
	> File Name: do_recv.c
	> Author: 
	> Mail: 
	> Created Time: Sat 11 Jul 2020 10:21:54 AM CST
 ************************************************************************/

#include "head.h"

extern int sockfd;
extern WINDOW *message_sub;
void *do_recv () {
    while (1) {
		
    	struct ChatMsg msg;
        bzero(&msg, sizeof(msg));
        recv(sockfd, (void *)&msg, sizeof(msg), 0);
        
	    show_message(message_sub, &msg, 0);
    }
}
