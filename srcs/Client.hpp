/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hnagashi <hnagashi@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/12 19:20:40 by hnagashi          #+#    #+#             */
/*   Updated: 2025/09/02 06:56:04 by hnagashi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>
#include <map>
#include <unistd.h>
#include <cstdio>
#include <cctype> 
#include "Parser.hpp"

enum ChannelRole
{
    ROLE_NORMAL = 0,
    ROLE_OPERATOR = 1,
    ROLE_VOICE = 2
};

class Client
{
public:
    int fd;
    std::string rbuf;
    std::string wbuf;
    bool registered;
    std::string nickname;
    std::string username;
    std::string realname;
    bool passOk;
    bool capDone;
    bool logout;
    Client(int f = -1) : fd(f), registered(false), passOk(false), capDone(true), logout(false) {}
    static std::map<int, Client> clients;
    typedef void (*CommandHandler)(Client &, const Message &);
    void checkRegistrationComplete();
    static Client* findClientByNick(const std::string& nick);
};
bool isValidNickname(const std::string &nickname);
void handleNick(Client &client, const Message &msg);
void handleUser(Client &client, const Message &msg);
void handlePass(Client &client, const Message &msg);
void handleQuit(Client &client, const Message &msg);
void cleanupClient(int clientFd);
#endif