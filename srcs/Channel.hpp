/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hnagashi <hnagashi@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/21 09:41:49 by hnagashi          #+#    #+#             */
/*   Updated: 2025/09/02 01:10:27 by hnagashi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CHANNEL_HPP
#define CHANNEL_HPP
#include <string>
#include <set>
#include <map>
#include <ctime>
#include "Client.hpp"

struct Channel
{
    std::string name;
    std::string topic;
    std::map<std::string, ChannelRole> members;
    std::set<std::string> modes;
    std::set<std::string> invited;
    std::string key;
    int limit;
    time_t creationTime;
    time_t topicTime;

    Channel(const std::string &n = "") : name(n), limit(0), creationTime(0), topicTime(0) {}

    void addMember(const std::string &nick, ChannelRole role = ROLE_NORMAL)
    {
        members[nick] = role;
    }

    void removeMember(const std::string &nick)
    {
        members.erase(nick);
    }

    size_t getMemberCount() const
    {
        return members.size();
    }

    bool isEmpty() const
    {
        return members.empty();
    }

    bool hasMember(const std::string &nick) const
    {
        return members.find(nick) != members.end();
    }

    ChannelRole getRole(const std::string &nick) const
    {
        std::map<std::string, ChannelRole>::const_iterator it = members.find(nick);
        return (it != members.end()) ? it->second : ROLE_NORMAL;
    }

    void setRole(const std::string &nick, ChannelRole role)
    {
        if (hasMember(nick))
            members[nick] = role;
    }
};

extern std::map<std::string, Channel> channels;
Channel *findChannel(const std::string &name);
Channel *getOrCreateChannel(const std::string &name);
void removeEmptyChannels();
bool isValidChannelName(const std::string &channelName);

#endif