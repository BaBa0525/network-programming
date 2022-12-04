#ifndef GLOBAL_H
#define GLOBAL_H

#include <string>
#include <vector>

constexpr int MAXLINE = 1000;
constexpr int LISTENQ = FD_SETSIZE;

struct channel_info {
    channel_info() : is_here(FD_SETSIZE, false), n_people(0) {}
    std::vector<bool> is_here;
    int n_people;
    std::string name, topic;
    void join(int i) {
        is_here[i] = true;
        n_people++;
    }
    void leave(int i) {
        is_here[i] = false;
        n_people--;
    }
};

#endif