#include "util.h"

#include <arpa/inet.h>   //inet_ntop
#include <netinet/in.h>  //sockaddr_in
#include <sys/socket.h>
#include <unistd.h>

#include <iostream>

#include "client.h"
using namespace std;

extern client_t clients[];
extern int online;

int listen_port(int serv_port) {
    sockaddr_in servaddr{};
    int listenfd = socket(AF_INET, SOCK_STREAM, 0);

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(serv_port);

    bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr));
    listen(listenfd, LISTENQ);
    cout << "* listening on port " << serv_port << endl;

    return listenfd;
}

int handle_connection(int listenfd) {
    sockaddr_in cliaddr{};
    socklen_t clilen = sizeof(cliaddr);
    int connfd = accept(listenfd, (struct sockaddr*)&cliaddr, &clilen);

    char buffer[MAXLINE];
    inet_ntop(AF_INET, &cliaddr.sin_addr.s_addr, buffer, INET_ADDRSTRLEN);
    cout << "* client connected from " << buffer << ':' << cliaddr.sin_port
         << '\n';

    int i;
    for (i = 0; i < FD_SETSIZE; i++)
        if (clients[i].fd < 0) {
            clients[i].fd = connfd;
            clients[i].host = buffer;
            break;
        }

    if (i == FD_SETSIZE) {
        perror("too many clients");
        exit(1);
    }

    return connfd;
}

void handle_disconnection(int i, fd_set& allset) {
    if (clients[i].is_registered()) online--;
    close(clients[i].fd);
    FD_CLR(clients[i].fd, &allset);
    clients[i].reset();
}