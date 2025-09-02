/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hnagashi <hnagashi@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/01 23:27:56 by hnagashi          #+#    #+#             */
/*   Updated: 2025/09/01 23:32:22 by hnagashi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Channel.hpp"

std::map<std::string, Channel> channels;
std::vector<std::string> channelsToLeave;

Channel *findChannel(const std::string &name)
{
    std::map<std::string, Channel>::iterator it = channels.find(name);
    return (it != channels.end()) ? &(it->second) : NULL;
}

Channel *getOrCreateChannel(const std::string &name)
{
    Channel *channel = findChannel(name);
    if (!channel)
    {
        channels[name] = Channel(name);
        channel = &channels[name];
        channel->creationTime = time(NULL);
        channel->modes.insert("n");
        channel->modes.insert("t");
    }
    return channel;
}

void removeEmptyChannels()
{
    std::map<std::string, Channel>::iterator it = channels.begin();
    while (it != channels.end())
    {
        if (it->second.isEmpty())
            channels.erase(it++);
        else
            ++it;
    }
}
bool isValidChannelName(const std::string &channelName)
{
    if (channelName.empty() || channelName.length() > 50)
        return false;
    if (channelName[0] != '#')
        return false;
    for (size_t i = 0; i < channelName.length(); ++i)
    {
        unsigned char c = channelName[i];
        if (c <= 31 || c == ' ' || c == ',')
            return false;
    }

    return true;
}
