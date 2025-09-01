/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hnagashi <hnagashi@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/21 10:34:13 by hnagashi          #+#    #+#             */
/*   Updated: 2025/09/02 06:34:10 by hnagashi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

void handleCap(Client &client, const Message &msg)
{
    if (msg.args.size() >= 1 && msg.args[0] == "LS")
    {
        std::printf(":irc.example.com CAP %s LS :\r\n",
                    client.nickname.c_str());
        client.wbuf += ":irc.example.com CAP " + (client.nickname.empty() ? std::string("*") : client.nickname) + " LS :\r\n";
    }
    else if (msg.args.size() >= 1 && msg.args[0] == "END")
    {

        client.capDone = true;
        std::printf("capDone\r\n");
        client.checkRegistrationComplete();
    }
    else if (msg.args.size() >= 1 && msg.args[0] == "REQ")
    {
        if (msg.args.size() >= 2)
        {
            std::printf(":irc.example.com CAP %s NAK :%s\r\n",
                        client.nickname.c_str(),
                        msg.args[1].c_str());
            client.wbuf += ":irc.example.com CAP " + (client.nickname.empty() ? std::string("*") : client.nickname) + " NAK :" + msg.args[1] + "\r\n";
        }
    }
    else
    {
        std::printf(":irc.example.com CAP %s ASK :\r\n",
                    client.nickname.c_str());
        client.wbuf += ":irc.example.com CAP " + (client.nickname.empty() ? std::string("*") : client.nickname) + " ACK :\r\n";
    }
    std::printf("CMD=%s\n", msg.cmd.c_str());
}

void handlePing(Client &client, const Message &msg)
{
    if (msg.args.size() < 1)
    {
        sendNumeric(client, 409, client.nickname.empty() ? "*" : client.nickname, ":No origin specified");
        return;
    }
    std::string token = msg.args[0];
    std::string pongMsg = ":server PONG server :" + token + "\r\n";
    client.wbuf += pongMsg;
    std::printf("PING: %s pinged with token '%s', sent PONG response\n",
                client.nickname.empty() ? "unknown" : client.nickname.c_str(), token.c_str());
}
