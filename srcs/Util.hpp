/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Util.hpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hnagashi <hnagashi@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/02 00:30:40 by hnagashi          #+#    #+#             */
/*   Updated: 2025/09/02 16:25:46 by hnagashi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef UTIL_HPP
#define UTIL_HPP
#include <string>
#include <map>
#include <set>
#include <poll.h>
#include "Client.hpp"
#include "Parser.hpp"

extern std::vector<pollfd> pfds;
void sendNumeric(Client &client, int code, const std::string &target, const std::string &message);
void sendWelcome(Client &client);
void sendNeedMoreParams(Client &client, const std::string &command);
void sendNotRegistered(Client &client, const std::string &);
void sendNotonchannel(Client &client, const std::string &channelName);
extern std::string serverPassword;
extern std::set<std::string> requiresRegistrationInit;
extern std::map<std::string, Client::CommandHandler> commandHandlersInit;
extern std::set<std::string> &requiresRegistration;
extern std::map<std::string, Client::CommandHandler> &commandHandlers;
std::string prefix(const Client &client);
void setPollout(int fd);
void handleUnknown(Client &client, const Message &msg);

#endif