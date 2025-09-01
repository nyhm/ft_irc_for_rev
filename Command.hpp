/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Command.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hnagashi <hnagashi@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/12 19:20:40 by hnagashi          #+#    #+#             */
/*   Updated: 2025/09/02 05:52:48 by hnagashi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef COMMAND_HPP
#define COMMAND_HPP

#include <cstdlib>
#include <sstream>
#include "Client.hpp"
#include "Util.hpp"
#include "Channel.hpp"

void handleWhois(Client &client, const Message &msg);
void handleWho(Client &client, const Message &msg);
void handleJoin(Client &client, const Message &msg);
void handleMode(Client &client, const Message &msg);
void handleKick(Client &client, const Message &msg);
void handleInvite(Client &client, const Message &msg);
void handleTopic(Client &client, const Message &msg);
void handlePrivmsg(Client &client, const Message &msg);
void handlePart(Client &client, const Message &msg);

#endif