/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hnagashi <hnagashi@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/21 07:18:23 by hnagashi          #+#    #+#             */
/*   Updated: 2025/09/02 07:44:44 by hnagashi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"
#include "Util.hpp"
#include "Channel.hpp"

std::map<int, Client> Client::clients;
Client *Client::findClientByNick(const std::string &nick)
{
    for (std::map<int, Client>::iterator it = Client::clients.begin();
         it != Client::clients.end(); ++it)
    {
        if (it->second.nickname == nick)
        {
            return &it->second;
        }
    }
    return NULL;
}

void Client::checkRegistrationComplete()
{
    if (registered)
        return;
    if (!passOk || nickname.empty() || username.empty())
        return;
    if (!capDone)
        return;
    registered = true;
    sendWelcome(*this);
}

void handlePass(Client &client, const Message &msg)
{
    if (client.passOk)
    {
        sendNumeric(client, 462, client.nickname.empty() ? "*" : client.nickname, ":Connection already registered");
        return;
    }
    if (msg.args.size() < 1)
    {
        sendNumeric(client, 461, client.nickname.empty() ? "*" : client.nickname, "PASS :Not enough parameters.");
        return;
    }

    if (msg.args[0] == serverPassword)
    {
        client.passOk = true;
        client.checkRegistrationComplete();
    }
    else
    {
        std::string nick = client.nickname.empty() ? "*" : client.nickname;
        client.wbuf += "ERROR :Closing link: (" + nick + "@localhost) [Bad password]\r\n";
        client.logout = true;
    }
}

bool isValidNickname(const std::string &nickname)
{
    if (nickname.empty() || nickname.length() > 9)
        return false;
    unsigned char c0 = static_cast<unsigned char>(nickname[0]);
    if (!isalpha(c0) && c0 != '[' && c0 != ']' &&
        c0 != '\\' && c0 != '`' && c0 != '_' &&
        c0 != '^' && c0 != '{' && c0 != '}' &&
        c0 != '|')
        return false;
    for (size_t i = 1; i < nickname.length(); ++i)
    {
        unsigned char c = static_cast<unsigned char>(nickname[i]);
        if (!isalnum(c) && c != '[' && c != ']' &&
            c != '\\' && c != '`' && c != '_' &&
            c != '^' && c != '{' && c != '}' &&
            c != '|' && c != '-')
            return false;
    }
    return true;
}

void handleNick(Client &client, const Message &msg)
{
    if (msg.args.size() < 1)
    {
        sendNumeric(client, 461, client.nickname.empty() ? "*" : client.nickname, "nick :Not enough parameters.");
        return;
    }
    std::string newNick = msg.args[0];
    if (!isValidNickname(newNick))
    {
        sendNumeric(client, 432, client.nickname.empty() ? "*" : client.nickname, newNick + " :Erroneous nickname");
        return;
    }
    for (std::map<int, Client>::iterator it = Client::clients.begin(); it != Client::clients.end(); ++it)
    {
        if (it->second.nickname == newNick)
        {
            sendNumeric(client, 433, client.nickname.empty() ? "*" : client.nickname, newNick + " :Nickname is already in use");
            return;
        }
    }
    const std::string oldNick = client.nickname;
    client.nickname = newNick;
    if (!client.username.empty() && client.passOk == false)
    {
        client.wbuf += "ERROR :Closing link: (" + client.nickname + "@localhost" + ") [You are not allowed to connect to this server]\r\n";
        client.logout = true;
    }
    client.checkRegistrationComplete();
    for (std::map<std::string, Channel>::iterator chIt = channels.begin();
         chIt != channels.end(); ++chIt)
    {
        Channel &ch = chIt->second;
        if (ch.hasMember(oldNick))
        {
            ChannelRole role = ch.getRole(oldNick);
            ch.removeMember(oldNick);
            ch.addMember(newNick, role);
        }
    }
    if (client.passOk && client.registered && !oldNick.empty())
    {
        std::string nickMsg = ":" + oldNick + "!" + client.username +
                              "@localhost NICK :" + newNick + "\r\n";
        for (std::map<std::string, Channel>::iterator chIt = channels.begin();
             chIt != channels.end(); ++chIt)
        {
            Channel &ch = chIt->second;
            if (ch.hasMember(newNick))
            {
                for (std::map<int, Client>::iterator ci = Client::clients.begin();
                     ci != Client::clients.end(); ++ci)
                {
                    if (ch.hasMember(ci->second.nickname))
                    {
                        ci->second.wbuf += nickMsg;
                        setPollout(ci->first);
                    }
                }
            }
        }
    }
}

void handleUser(Client &client, const Message &msg)
{
    if (msg.args.size() < 4)
    {
        sendNumeric(client, 461, client.nickname.empty() ? "*" : client.nickname, "USER :Not enough parameters.");
        return;
    }
    if (client.registered)
    {
        sendNumeric(client, 462, client.nickname, ":You may not reregister");
        return;
    }
    client.username = msg.args[0];
    client.realname = msg.args[3];
    if (!client.nickname.empty() && client.passOk == false)
    {
        client.wbuf += "ERROR :Closing link: (" + client.nickname + "@localhost" + ") [You are not allowed to connect to this server]\r\n";
        client.logout = true;
    }
    client.checkRegistrationComplete();
}

void handleQuit(Client &client, const Message &msg)
{
    std::vector<std::string> channelsToLeave;
    std::string reason = (msg.args.size() > 0) ? msg.args[0] : "Leaving";
    std::string quitMsg = prefix(client) + " QUIT :" + reason + "\r\n";
    for (std::map<std::string, Channel>::iterator it = channels.begin(); it != channels.end(); ++it)
    {
        if (it->second.hasMember(client.nickname))
            channelsToLeave.push_back(it->first);
    }
    for (std::vector<std::string>::iterator it = channelsToLeave.begin(); it != channelsToLeave.end(); ++it)
    {
        std::string channelName = *it;
        Channel *channel = findChannel(channelName);

        if (channel)
        {
            for (std::map<int, Client>::iterator clientIt = Client::clients.begin(); clientIt != Client::clients.end(); ++clientIt)
            {
                if (channel->hasMember(clientIt->second.nickname))
                {
                    clientIt->second.wbuf += quitMsg;
#ifdef DEBUG
                    std::printf("QUIT: sending notification to fd=%d, nickname=%s\n",
                                clientIt->first, clientIt->second.nickname.c_str());
#endif
                    setPollout(clientIt->first);
                }
            }
            channel->removeMember(client.nickname);
            channel->invited.erase(client.nickname);
#ifdef DEBUG
            std::printf("QUIT: %s left %s, remaining members=%zu\n",
                        client.nickname.c_str(), channelName.c_str(), channel->getMemberCount());
#endif
            if (channel->isEmpty())
            {
                channels.erase(channelName);
#ifdef DEBUG
                std::printf("QUIT: %s removed (empty)\n", channelName.c_str());
#endif
            }
        }
    }
    close(client.fd);
    Client::clients.erase(client.fd);
#ifdef DEBUG
    std::printf("QUIT: %s disconnected from %zu channels\n",

                client.nickname.c_str(), channelsToLeave.size());
#endif
}

void cleanupClient(int clientFd)
{
    std::map<int, Client>::iterator it = Client::clients.find(clientFd);
    if (it == Client::clients.end())
        return;
    Client &client = it->second;
    std::string quitMsg = prefix(client) + " QUIT :Connection lost\r\n";
    std::vector<std::string> channelsToLeave;
    for (std::map<std::string, Channel>::iterator channelIt = channels.begin(); channelIt != channels.end(); ++channelIt)
    {
        if (channelIt->second.hasMember(client.nickname))
            channelsToLeave.push_back(channelIt->first);
    }
    for (std::vector<std::string>::iterator channelIt = channelsToLeave.begin(); channelIt != channelsToLeave.end(); ++channelIt)
    {
        std::string channelName = *channelIt;
        Channel *channel = findChannel(channelName);
        if (channel)
        {
            for (std::map<int, Client>::iterator clientIt = Client::clients.begin(); clientIt != Client::clients.end(); ++clientIt)
            {
                if (clientIt->first != clientFd && channel->hasMember(clientIt->second.nickname))
                {
                    clientIt->second.wbuf += quitMsg;
#ifdef DEBUG
                    std::printf("DISCONNECT: sending notification to fd=%d, nickname=%s\n",
                                clientIt->first, clientIt->second.nickname.c_str());
#endif
                    setPollout(clientIt->first);
                }
            }
            channel->removeMember(client.nickname);
            channel->invited.erase(client.nickname);
#ifdef DEBUG
            std::printf("DISCONNECT: %s left %s, remaining members=%zu\n",
                        client.nickname.c_str(), channelName.c_str(), channel->getMemberCount());
#endif
            if (channel->isEmpty())
            {
                channels.erase(channelName);
#ifdef DEBUG
                std::printf("DISCONNECT: %s removed (empty)\n", channelName.c_str());
#endif
            }
        }
    }
#ifdef DEBUG
    std::printf("DISCONNECT: %s cleaned up from %zu channels\n",
                client.nickname.c_str(), channelsToLeave.size());
#endif
    Client::clients.erase(clientFd);
}
