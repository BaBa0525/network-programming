#include "client.h"

#include <sys/socket.h>

#include <map>

#include "global.h"

using namespace std;
extern client_t clients[];
extern map<string, channel_info> channels;

#define send_msg(fmt, ...)                                      \
    {                                                           \
        char ret_msg[MAXLINE] = {};                             \
        int n = snprintf(ret_msg, MAXLINE, fmt, ##__VA_ARGS__); \
        send(this->fd, ret_msg, n, 0);                          \
    }

bool client_t::set_name(const string& new_name) {
    if (new_name.empty()) {
        err_nonicknamegiven_431();
        return false;
    }
    for (int i = 0; i < FD_SETSIZE; i++) {
        if (clients[i].nickname == new_name) {
            err_nickcollision_436(new_name);
            return false;
        }
    }
    return true;
}

#pragma region api

void client_t::reset() { *this = client_t(); }

void client_t::set_channel(const string& channel_name) {
    this->channel_name = channel_name;
}

channel_info& client_t::get_channel() const {
    channel_info& channel = channels[this->channel_name];
    if (channel.name.empty()) channel.name = this->channel_name;
    return channel;
}

void client_t::send_welcome(int online) const {
    rpl_welcome_001();
    rpl_luserclient_251(online);
    rpl_motdstart_375();
    rpl_motd_372(" Hello, World!");
    rpl_motd_372("              @                    _ ");
    rpl_motd_372("  ____  ___   _   _ _   ____.     | |");
    rpl_motd_372(" /  _ `'_  \\ | | | '_/ /  __|  ___| |");
    rpl_motd_372(" | | | | | | | | | |   | |    /  _  |");
    rpl_motd_372(" | | | | | | | | | |   | |__  | |_| |");
    rpl_motd_372(" |_| |_| |_| |_| |_|   \\____| \\___,_|");
    rpl_motd_372(" minimized internet relay chat daemon");
    rpl_motd_372("");
    rpl_endofmotd_376();
}

void client_t::send_join(const string& channel_name) const {
    if (channel_name.empty()) {
        err_needmoreparams_461("JOIN");
        return;
    }
    channel_info& channel = get_channel();
    for (int i = 0; i < FD_SETSIZE; i++) {
        if (channel.is_here[i]) clients[i].rpl_join(this->nickname);
    }
    if (channel.topic.empty())
        rpl_notopic_331();
    else
        rpl_topic_332(channel.name, channel.topic);

    send_names(channel_name);
}

void client_t::send_part(const string& channel_name, int i) {
    if (channel_name.empty()) {
        err_needmoreparams_461("PART");
        return;
    }

    auto it = channels.find(channel_name);
    if (it == channels.end()) {
        err_nosuchchannel_403(channel_name);
        return;
    }

    if (this->channel_name != channel_name) {
        err_notonchannel_442(channel_name);
        return;
    }
    const channel_info& channel = channels[channel_name];
    for (int i = 0; i < FD_SETSIZE; i++) {
        if (channel.is_here[i]) clients[i].rpl_part(this->nickname);
    }
    this->channel_name.clear();
    channels[channel_name].leave(i);
}

void client_t::send_pong(const string& server) const {
    if (server.empty()) {
        err_noorigin_409();
        return;
    }
    rpl_ping(server);
}

void client_t::send_nick(const string& new_name) const {
    if (new_name.empty()) {
        err_nonicknamegiven_431();
        return;
    }
    for (int i = 0; i < FD_SETSIZE; i++) {
        if (clients[i].nickname == new_name) {
            err_nickcollision_436(new_name);
            return;
        }
    }
    if (this->channel_name.empty()) {
        rpl_nick(this->nickname, new_name);
        return;
    }
    const channel_info& channel = get_channel();
    for (int i = 0; i < FD_SETSIZE; i++) {
        if (channel.is_here[i]) clients[i].rpl_nick(this->nickname, new_name);
    }
}

void client_t::send_list(const string& channel_name) const {
    rpl_liststart_321();
    for (auto& [_, channel] : channels) {
        if (channel_name.empty() || channel_name == channel.name)
            rpl_list_322(channel);
    }
    rpl_listend_323();
}

void client_t::send_privmsg(const string& channel_name,
                            const string& msg) const {
    if (channel_name.empty()) {
        err_norecipient_411("PRIVMSG");
        return;
    }
    if (msg.empty()) {
        err_notexttosend_412();
        return;
    }
    auto it = channels.find(channel_name);
    if (it == channels.end()) {
        err_nosuchnick_401(channel_name);
        return;
    }
    channel_info& channel = it->second;
    for (int i = 0; i < FD_SETSIZE; i++) {
        if (channel.is_here[i] && this->fd != clients[i].fd)
            clients[i].rpl_privmsg(this->nickname, msg);
    }
}

void client_t::send_users() const {
    rpl_usersstart_392();
    for (int i = 0; i < FD_SETSIZE; i++) {
        if (clients[i].fd >= 0)
            rpl_users_393(clients[i].nickname, "-", clients[i].host);
    }
    rpl_endofusers_394();
}

void client_t::send_names(const string& channel_name) const {
    if (channel_name.empty()) {
        for (auto& [name, _] : channels) {
            send_names(name);
        }
        return;
    }

    string user_names;
    auto it = channels.find(channel_name);
    if (it == channels.end()) {
        rpl_namreply_353(channel_name, "");
        rpl_endofnames_366();
        return;
    }
    channel_info& channel = it->second;
    for (int i = 0; i < FD_SETSIZE; i++) {
        if (channel.is_here[i]) user_names += (clients[i].nickname + " ");
    }
    user_names.pop_back();
    rpl_namreply_353(channel_name, user_names);
    rpl_endofnames_366();
}

void client_t::send_topic(const string& channel_name,
                          const string& topic) const {
    if (channel_name.empty()) {
        err_needmoreparams_461("TOPIC");
        return;
    }
    if (this->channel_name != channel_name) {
        err_notonchannel_442(channel_name);
        return;
    }
    channel_info& channel = channels[channel_name];
    if (topic.empty()) {
        if (channel.topic.empty())
            rpl_notopic_331();
        else
            rpl_topic_332(channel_name, channel.topic);
        return;
    }
    channel.topic = topic;

    for (int i = 0; i < FD_SETSIZE; i++) {
        if (channel.is_here[i]) clients[i].rpl_topic_332(channel_name, topic);
    }
}

void client_t::send_unknown_cmd(const string& cmd) const {
    err_unknowncommand_421(cmd);
}

#pragma endregion

#pragma region reply

void client_t::rpl_join(const string& join_client) const {
    send_msg(":%s JOIN %s\n", join_client.data(), this->channel_name.data());
}

void client_t::rpl_part(const string& part_client) const {
    send_msg(":%s PART %s\n", part_client.data(), this->channel_name.data());
}

void client_t::rpl_ping(const string& server) const {
    send_msg("PONG %s\n", server.data());
}

void client_t::rpl_nick(const string& old_name, const string& new_name) const {
    send_msg(":%s NICK %s\n", old_name.data(), new_name.data());
}

void client_t::rpl_privmsg(const string& sender, const string& msg) const {
    send_msg(":%s PRIVMSG %s :%s\n", sender.data(), this->channel_name.data(),
             msg.data());
}

void client_t::rpl_welcome_001() const {
    send_msg(":mircd 001 %s :Welcome to the minimized IRC daemon!\n",
             this->nickname.data());
}

void client_t::rpl_luserclient_251(int online) const {
    send_msg(":mircd 251 %s :There are %d users and 0 invisible on 1 servers\n",
             this->nickname.data(), online);
}

void client_t::rpl_liststart_321() const {
    send_msg(":mircd 321 %s Channel :Users Name\n", this->nickname.data());
}

void client_t::rpl_list_322(const channel_info& channel) const {
    send_msg(":mircd 322 %s %s %d :%s\n", this->nickname.data(),
             channel.name.data(), channel.n_people, channel.topic.data());
}

void client_t::rpl_listend_323() const {
    send_msg(":mircd 323 %s :End of List\n", this->nickname.data());
}

void client_t::rpl_notopic_331() const {
    send_msg(":mircd 331 %s %s :No topic is set\n", this->nickname.data(),
             this->channel_name.data());
}

void client_t::rpl_topic_332(const string& channel_name,
                             const string& topic) const {
    send_msg(":mircd 332 %s %s :%s\n", this->nickname.data(),
             channel_name.data(), topic.data());
}

void client_t::rpl_namreply_353(const string& channel_name,
                                const string& name) const {
    send_msg(":mircd 353 %s %s :%s\n", this->nickname.data(),
             channel_name.data(), name.data());
}

void client_t::rpl_endofnames_366() const {
    send_msg(":mircd 366 %s %s :End of Names List\n", this->nickname.data(),
             this->channel_name.data());
}

void client_t::rpl_motd_372(const string& msg) const {
    send_msg(":mircd 372 %s :- %s\n", this->nickname.data(), msg.data());
}

void client_t::rpl_motdstart_375() const {
    send_msg(":mircd 375 %s :- mircd Message of the day - \n",
             this->nickname.data());
}

void client_t::rpl_endofmotd_376() const {
    send_msg(":mircd 376 %s :End of /MOTD command\n", this->nickname.data());
}

void client_t::rpl_usersstart_392() const {
    send_msg(":mircd 392 %s :UserID   Terminal  Host\n", this->nickname.data());
};

void client_t::rpl_users_393(const string& user_id, const string& terminal,
                             const string& host) const {
    send_msg(":mircd 393 %s :%-8s %-9s %-8s\n", this->nickname.data(),
             user_id.data(), terminal.data(), host.data());
}

void client_t::rpl_endofusers_394() const {
    send_msg(":mircd 394 %s :End of users\n", this->nickname.data());
}

#pragma endregion

#pragma region error

void client_t::err_nosuchnick_401(const string& nickname) const {
    send_msg(":mircd 401 %s %s :No such nick/channel\n", this->nickname.data(),
             nickname.data());
}

void client_t::err_nosuchchannel_403(const string& channel_name) const {
    send_msg(":mircd 403 %s %s :No such channel\n", this->nickname.data(),
             channel_name.data());
}

void client_t::err_noorigin_409() const {
    send_msg(":mircd 409 %s :No origin specified\n", this->nickname.data());
}

void client_t::err_norecipient_411(const string& cmd) const {
    send_msg(":mircd 411 %s :No recipient given (%s)\n", this->nickname.data(),
             cmd.data());
}

void client_t::err_notexttosend_412() const {
    send_msg(":mircd 412 %s :No text to send\n", this->nickname.data());
}

void client_t::err_unknowncommand_421(const string& cmd) const {
    send_msg(":mircd 421 %s :Unknown command\n", cmd.data());
}

void client_t::err_nonicknamegiven_431() const {
    send_msg(":mircd 431 :No nickname given\n");
}

void client_t::err_nickcollision_436(const string& collision_name) const {
    send_msg(":mircd 436 %s :Nickname collision KILL\n", collision_name.data());
}

void client_t::err_notonchannel_442(const string& channel_name) const {
    send_msg(":mircd 442 %s %s :You are not on that channel\n",
             this->nickname.data(), channel_name.data());
}

void client_t::err_needmoreparams_461(const string& cmd) const {
    send_msg(":mircd 461 %s %s :Not enought parameters\n",
             this->nickname.data(), cmd.data());
}

#pragma endregion
