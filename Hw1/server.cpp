#include <arpa/inet.h>   //inet_ntop
#include <netinet/in.h>  //sockaddr_in
#include <sys/socket.h>
#include <unistd.h>

#include <iostream>
#include <map>
#include <sstream>

#include "client.h"
#include "global.h"
#include "util.h"

using namespace std;

client_t clients[FD_SETSIZE];
map<string, channel_info> channels;
int online = 0;

int main(int argc, char* argv[]) {
    socklen_t clilen;
    sockaddr_in cliaddr{};
    string split = "--------------------------------------------------\n";

    if (argc < 2) {
        cerr << "too few arguments" << endl;
        exit(1);
    }

    int listenfd = listen_port(atoi(argv[1]));

    int maxfd = listenfd;

    fd_set rset, allset;
    FD_ZERO(&allset);
    FD_SET(listenfd, &allset);

    for (;;) {
        rset = allset; /* structure assignment */
        int nready = select(maxfd + 1, &rset, NULL, NULL, NULL);

        if (FD_ISSET(listenfd, &rset)) { /* new client connection */
            int connfd = handle_connection(listenfd);
            FD_SET(connfd, &allset);            /* add new descriptor to set */
            if (connfd > maxfd) maxfd = connfd; /* for select */
            if (--nready <= 0) continue; /* no more readable descriptors */
        }

        for (int i = 0; i < FD_SETSIZE; i++) { /* check all clients for data */

            if (clients[i].fd < 0) continue;
            if (FD_ISSET(clients[i].fd, &rset)) {
                char buf[MAXLINE] = {};
                if (read(clients[i].fd, buf, MAXLINE) == 0) {
                    handle_disconnection(i, allset);
                } else {
                    stringstream ss_buf(buf);
                    string line;
                    while (getline(ss_buf, line)) {
                        cout << "[debug] " << line << endl;
                        stringstream ss(line);
                        string cmd;
                        ss >> cmd;
                        if (cmd == "NICK") {
                            bool is_ready = clients[i].is_ready();
                            string new_name;
                            ss >> new_name;
                            if (!clients[i].set_name(new_name)) continue;

                            if (is_ready) {
                                clients[i].nickname = new_name;
                                clients[i].send_welcome(++online);
                            } else if (clients[i].is_registered()) {
                                clients[i].send_nick(new_name);
                            }
                            clients[i].nickname = new_name;

                        } else if (cmd == "USER") {
                            string str;
                            bool is_ready = clients[i].is_ready();
                            ss >> clients[i].username;
                            if (!getline(ss, str, ':')) {
                                clients[i].err_needmoreparams_461("USER");
                                continue;
                            }
                            if (!getline(ss, clients[i].realname)) {
                                clients[i].err_needmoreparams_461("USER");
                                continue;
                            }
                            if (is_ready) clients[i].send_welcome(++online);
                        } else if (cmd == "JOIN") {
                            string channel_name;
                            ss >> channel_name;
                            clients[i].set_channel(channel_name);
                            channel_info& channel = clients[i].get_channel();
                            channel.join(i);
                            clients[i].send_join(channel_name);
                        } else if (cmd == "PART") {
                            string channel_name;
                            ss >> channel_name;
                            clients[i].send_part(channel_name, i);
                        } else if (cmd == "PING") {
                            string server;
                            ss >> server;
                            clients[i].send_pong(server);
                        } else if (cmd == "LIST") {
                            string channel_name;
                            ss >> channel_name;
                            clients[i].send_list(channel_name);
                        } else if (cmd == "PRIVMSG") {
                            string channel_name, msg;
                            ss >> channel_name;
                            getline(ss, msg, ':');
                            getline(ss, msg);
                            clients[i].send_privmsg(channel_name, msg);
                        } else if (cmd == "USERS") {
                            clients[i].send_users();
                        } else if (cmd == "NAMES") {
                            string channel_name;
                            ss >> channel_name;
                            clients[i].send_names(channel_name);
                        } else if (cmd == "TOPIC") {
                            string channel_name, topic;
                            ss >> channel_name;
                            getline(ss, topic, ':');
                            if (!getline(ss, topic)) topic = "";
                            // if (isspace(topic)) topic.clear();
                            clients[i].send_topic(channel_name, topic);
                        } else if (cmd == "QUIT") {
                            handle_disconnection(i, allset);
                        } else if (!cmd.empty()) {
                            cout << buf << endl;
                            clients[i].send_unknown_cmd(cmd);
                        }
                        cout << "qaq: " << buf << endl;

                    }
                }
                if (--nready <= 0) break; /* no more readable descriptors */
            }
        }
    }

    return 0;
}