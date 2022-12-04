#ifndef CLIENT_H
#define CLIENT_H

#include <string>

#include "global.h"

class client_t {
   public:
    client_t() : fd(-1) {}
    bool is_registered() const {
        return !(nickname.empty() || username.empty());
    }
    bool is_ready() const { return nickname.empty() != username.empty(); }
    void reset();
    void set_channel(const std::string& channel_name);

    void err_needmoreparams_461(const std::string& cmd) const;
    
    channel_info& get_channel() const;

    bool set_name(const std::string& new_name);

    void send_welcome(int online) const;
    void send_join(const std::string& channel_name) const;
    void send_part(const std::string& channel_name, int i);
    void send_pong(const std::string& server) const;
    void send_nick(const std::string& new_name) const;
    void send_list(const std::string& channel_name) const;
    void send_privmsg(const std::string& channel_name,
                      const std::string& msg) const;
    void send_users() const;
    void send_names(const std::string& channel_name) const;
    void send_topic(const std::string& channel_name,
                    const std::string& topic) const;
    void send_unknown_cmd(const std::string& cmd) const;

    std::string nickname, username, realname, channel_name, host;
    int fd;

   private:
    void rpl_join(const std::string& join_client) const;
    void rpl_part(const std::string& part_client) const;
    void rpl_ping(const std::string& server) const;
    void rpl_nick(const std::string& old_name,
                  const std::string& new_name) const;
    void rpl_privmsg(const std::string& sender, const std::string& msg) const;

    void rpl_welcome_001() const;
    void rpl_luserclient_251(int online) const;
    void rpl_liststart_321() const;
    void rpl_list_322(const channel_info& channel) const;
    void rpl_listend_323() const;
    void rpl_notopic_331() const;
    void rpl_topic_332(const std::string& channel_name,
                       const std::string& topic) const;
    void rpl_namreply_353(const std::string& channel_name,
                          const std::string& name) const;
    void rpl_endofnames_366() const;
    void rpl_motd_372(const std::string& msg) const;
    void rpl_motdstart_375() const;
    void rpl_endofmotd_376() const;
    void rpl_usersstart_392() const;
    void rpl_users_393(const std::string& user_id, const std::string& terminal,
                       const std::string& host) const;
    void rpl_endofusers_394() const;

    void err_nosuchnick_401(const std::string& nickname) const;
    void err_nosuchchannel_403(const std::string& channel_name) const;
    void err_noorigin_409() const;
    void err_norecipient_411(const std::string& cmd) const;
    void err_notexttosend_412() const;
    void err_unknowncommand_421(const std::string& cmd) const;
    void err_nonicknamegiven_431() const;
    void err_nickcollision_436(const std::string& collision_name) const;
    void err_notonchannel_442(const std::string& channel_name) const;
};

#endif