/*************************************************************************
	> File Name: client.c
	> Author: suyelu 
	> Mail: suyelu@126.com
	> Created Time: Wed 08 Jul 2020 04:32:12 PM CST
 ************************************************************************/

#include "head.h"

int server_port = 0;
char server_ip[20] = {0};
char *conf = "./football.conf";
int sockfd = -1;
int msgnum;
char name[20] = {0};
WINDOW *message_win, *message_sub,  *info_win, *info_sub, *input_win, *input_sub;


void logout(int signum) {
    struct ChatMsg msg;
    bzero(&msg, sizeof(msg));
    msg.type = CHAT_FIN;
    send(sockfd, (void *)&msg, sizeof(msg), 0);
    close(sockfd);
    printf(L_PINK"拜拜~"NONE"\n");
    exit(0);
}

int main(int argc, char **argv) {
    int opt;
    struct LogRequest request;
    struct LogResponse response;
    bzero(&request, sizeof(request));
    bzero(&response, sizeof(response));
    while ((opt = getopt(argc, argv, "h:p:t:m:n:")) != -1) {
        switch (opt) {
            case 't':
                request.team = atoi(optarg);
                break;
            case 'h':
                strcpy(server_ip, optarg);
                break;
            case 'p':
                server_port = atoi(optarg);
                break;
            case 'm':
                strcpy(request.msg, optarg);
                break;
            case 'n':
                strcpy(request.name, optarg);
                break;
            default:
                fprintf(stderr, "Usage : %s [-hptmn]!\n", argv[0]);
                exit(1);
        }
    }
    

    if (!server_port) server_port = atoi(get_conf_value(conf, "SERVERPORT"));
    if (!request.team) request.team = atoi(get_conf_value(conf, "TEAM"));
    if (!strlen(server_ip)) strcpy(server_ip, get_conf_value(conf, "SERVERIP"));
    if (!strlen(request.name)) strcpy(request.name, get_conf_value(conf, "NAME"));
    if (!strlen(request.msg)) strcpy(request.msg, get_conf_value(conf, "LOGMSG"));


    DBG("<"GREEN"Conf Show"NONE"> : server_ip = %s, port = %d, team = %s, name = %s\n%s",\
        server_ip, server_port, request.team ? "BLUE": "RED", request.name, request.msg);

    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(server_port);
    server.sin_addr.s_addr = inet_addr(server_ip);

    socklen_t len = sizeof(server);
    setlocale(LC_ALL,"");
    init_ui();
    if ((sockfd = socket_udp()) < 0) {
        perror("socket_udp()");
        exit(1);
    }
    
    pthread_t recv_t;          
    int err = pthread_create(&recv_t, NULL, do_recv, NULL);
    
    if (err < 0) {
        perror("create pthread fail!\n");
        exit(1);
    }

    sendto(sockfd, (void *)&request, sizeof(request), 0, (struct sockaddr *)&server, len);

    fd_set rfds;
    FD_ZERO(&rfds);
    FD_SET(sockfd, &rfds);
    struct timeval tv;
    tv.tv_sec = 5;
    tv.tv_usec = 0;
    DBG("<"PINK"ADD rfds"NONE"> : set %d in rfds.\n", sockfd);
    if (select(sockfd + 1 , &rfds, NULL, NULL, &tv) <= 0) {
        perror("response error!");
        exit(1);
    }
    if (FD_ISSET(sockfd, &rfds)) {
        recvfrom(sockfd, (void *)&response, sizeof(response), 0,(struct sockaddr *)&server, &len);
        if ((!(strlen(response.msg) < 512 && strlen(response.msg) > 0)) || response.type == 1) {
            fprintf(stderr, "server refused! : %s\n", response.msg);
            exit(1);
        }
        
    }
    if (connect(sockfd, (const struct sockaddr*)&server, len) < 0) {
            perror("connect fail");
            exit(1);
        }

    DBG(GREEN"connect successfully"NONE"<%s> : %s\n", server_ip, request.msg);
    
    signal(SIGINT, logout);
    strcpy(name, request.name);

    noecho();
    cbreak();
    //keypad(stdscr, TRUE);
    while (1) {
        send_chat();
    }
}
