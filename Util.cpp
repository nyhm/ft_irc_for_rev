/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Util.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hnagashi <hnagashi@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/02 00:30:52 by hnagashi          #+#    #+#             */
/*   Updated: 2025/09/02 07:11:28 by hnagashi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Util.hpp"

std::vector<pollfd> pfds;
std::string serverPassword;

std::set<std::string> requiresRegistrationInit;
std::map<std::string, Client::CommandHandler> commandHandlersInit;
std::set<std::string> &requiresRegistration = requiresRegistrationInit;
std::map<std::string, Client::CommandHandler> &commandHandlers = commandHandlersInit;

void sendNumeric(Client &client, int code, const std::string &target, const std::string &message)
{
    char codeStr[4];
    std::snprintf(codeStr, sizeof(codeStr), "%03d", code);
    std::string response = ":server " + std::string(codeStr) + " " + target + " " + message + "\r\n";
    client.wbuf += response;
}

void sendWelcome(Client &client)
{
    std::string nick = client.nickname.empty() ? "*" : client.nickname;

    sendNumeric(client, 001, nick, "Welcome to the Internet Relay Network " + nick);
    sendNumeric(client, 002, nick, "Your host is server, running version 1.0");
    sendNumeric(client, 003, nick, "This server was created today");
    sendNumeric(client, 004, nick, "server 1.0 o o");
    sendNumeric(client, 375, nick, ":- server Message of the day -");
    sendNumeric(client, 372, nick, ":███████╗ ████████╗        ██████╗ ██████╗  ██████╗ ");
    sendNumeric(client, 372, nick, ":██╔════╝ ╚══██╔══╝          ██╔═╝ ██╔══██╗██╔════╝ ");
    sendNumeric(client, 372, nick, ":███████╗    ██║             ██║   ██████╔╝██║      ");
    sendNumeric(client, 372, nick, ":██╔════╝    ██║             ██║   ██╔══██╗██║      ");
    sendNumeric(client, 372, nick, ":██║         ██║ ████████╗ ██████╗ ██║  ██║╚██████╗ ");
    sendNumeric(client, 372, nick, ":╚═╝         ╚═╝ ╚═══════╝ ╚═════╝ ╚═╝  ╚═╝ ╚═════╝ ");
    sendNumeric(client, 372, nick, ":- Welcome to ft_irc server!");
    sendNumeric(client, 376, nick, ":End of /MOTD command.");
}

void sendNeedMoreParams(Client &client, const std::string &command)
{
    sendNumeric(client, 461, client.nickname.empty() ? "*" : client.nickname, command + " :Not enough parameters.");
}

void sendNotRegistered(Client &client, const std::string &)
{
    sendNumeric(client, 451, "*", "You have not registered");
}

void sendNotonchannel(Client &client, const std::string &channelName)
{
    sendNumeric(client, 442, client.nickname, channelName + " :You're not on that channel");
}
std::string prefix(const Client &client)
{
    std::string nick = client.nickname.empty() ? "unknown" : client.nickname;
    std::string user = client.username.empty() ? "unknown" : client.username;
    return ":" + nick + "!" + user + "@localhost";
}

void setPollout(int fd)
{
    for (size_t i = 0; i < pfds.size(); ++i)
    {
        if (pfds[i].fd == fd)
        {
            pfds[i].events |= POLLOUT;
            std::printf("SETPOLLOUT: fd=%d, events=0x%x\n", fd, pfds[i].events);
            break;
        }
    }
}

void handleUnknown(Client &client, const Message &msg)
{
    sendNumeric(client, 421, client.nickname, msg.cmd + " :Unknown command");
    return;
}
