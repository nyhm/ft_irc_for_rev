/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hnagashi <hnagashi@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/21 10:47:23 by hnagashi          #+#    #+#             */
/*   Updated: 2025/09/02 08:04:29 by hnagashi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <netinet/in.h>
#include <fcntl.h>
#include <cstdlib>
#include <cstring>
#include <cerrno>
#include "Parser.hpp"
#include "Client.hpp"
#include "Util.hpp"
#include "Channel.hpp"
#include "Command.hpp"
#include "Server.hpp"

static void dispatchCommand(Client &client, const Message &msg)
{
#ifdef DEBUG
    std::printf("DISPATCH: cmd=%s, registered=%d, nickname='%s', passOk=%d, username='%s'\n",
                msg.cmd.c_str(), client.registered, client.nickname.c_str(), client.passOk, client.username.c_str());
#endif
    if (requiresRegistration.find(msg.cmd) != requiresRegistration.end() && !client.registered)
    {
#ifdef DEBUG
        std::printf("SENDING 451: %s requires registration\n", msg.cmd.c_str());
#endif
        sendNotRegistered(client, msg.cmd);
        return;
    }
    std::map<std::string, Client::CommandHandler>::const_iterator it = commandHandlers.find(msg.cmd);
    if (it != commandHandlers.end())
    {
#ifdef DEBUG
        std::printf("HANDLING: %s\n", msg.cmd.c_str());
#endif
        it->second(client, msg);
#ifdef DEBUG
        std::printf("AFTER HANDLER: wbuf.size=%zu\n", client.wbuf.size());
#endif
    }
    else
    {
#ifdef DEBUG
        std::printf("UNKNOWN COMMAND: %s\n", msg.cmd.c_str());
#endif
        if (client.registered)
            commandHandlers.at("UNKNOWN")(client, msg);
    }
}

static void set_nonblock(int fd)
{
    if (fcntl(fd, F_SETFL, O_NONBLOCK) == -1)
    {
        perror("fcntl O_NONBLOCK");
        std::exit(1);
    }
}

static bool pop_line(std::string &buf, std::string &line)
{
    std::string::size_type p = buf.find("\r\n");
    if (p != std::string::npos)
    {
        line.assign(buf, 0, p);
        buf.erase(0, p + 2);
        return true;
    }
    p = buf.find('\n');
    if (p != std::string::npos)
    {
        line.assign(buf, 0, p);
        if (!line.empty() && line[line.size() - 1] == '\r')
            line.erase(line.size() - 1);
        buf.erase(0, p + 1);
        return true;
    }
    return false;
}

static bool line_too_long(const std::string &buf) { return buf.size() > 510; }

int main(int argc, char **argv)
{
    requiresRegistrationInit.insert("JOIN");
    requiresRegistrationInit.insert("PART");
    requiresRegistrationInit.insert("MODE");
    requiresRegistrationInit.insert("TOPIC");
    requiresRegistrationInit.insert("NAMES");
    requiresRegistrationInit.insert("LIST");
    requiresRegistrationInit.insert("INVITE");
    requiresRegistrationInit.insert("KICK");
    requiresRegistrationInit.insert("UNKNOWN");

    commandHandlersInit["PASS"] = handlePass;
    commandHandlersInit["JOIN"] = handleJoin;
    commandHandlersInit["PART"] = handlePart;
    commandHandlersInit["MODE"] = handleMode;
    commandHandlersInit["TOPIC"] = handleTopic;
    commandHandlersInit["INVITE"] = handleInvite;
    commandHandlersInit["KICK"] = handleKick;
    commandHandlersInit["PRIVMSG"] = handlePrivmsg;
    commandHandlersInit["NICK"] = handleNick;
    commandHandlersInit["USER"] = handleUser;
    commandHandlersInit["PING"] = handlePing;
    commandHandlersInit["QUIT"] = handleQuit;
    commandHandlersInit["UNKNOWN"] = handleUnknown;
    commandHandlersInit["CAP"] = handleCap;
    commandHandlersInit["WHO"] = handleWho;
    commandHandlersInit["WHOIS"] = handleWhois;

    if (argc != 3)
    {
        std::fprintf(stderr, "Usage: %s <port> <password>\n", argv[0]);
        return 1;
    }
    const int port = std::atoi(argv[1]);
    serverPassword = argv[2];

    int listen_fd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd < 0)
    {
        perror("socket");
        return 1;
    }

    int yes = 1;
    if (::setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) < 0)
    {
        perror("setsockopt SO_REUSEADDR");
        return 1;
    }

    set_nonblock(listen_fd);

    sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port);

    if (::bind(listen_fd, (sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror("bind");
        return 1;
    }
    if (::listen(listen_fd, 128) < 0)
    {
        perror("listen");
        return 1;
    }
    pfds.reserve(1024);
    pfds.push_back((pollfd){listen_fd, POLLIN, 0});
#ifdef DEBUG
    std::puts("server: listening...");
#endif
    for (;;)
    {
        int n = ::poll(&pfds[0], pfds.size(), -1);
        if (n < 0)
        {
            if (errno == EINTR)
                continue;
            perror("poll");
            break;
        }

        for (size_t i = 0; i < pfds.size(); ++i)
        {
            pollfd &p = pfds[i];
            if (p.revents == 0)
                continue;
            if (p.revents & (POLLERR | POLLHUP | POLLNVAL))
            {
                if (p.fd == listen_fd)
                {
                    std::puts("server: listen socket error");
                    return 1;
                }
                cleanupClient(p.fd);
                pfds.erase(pfds.begin() + i);
                --i;
                continue;
            }

            if (p.fd == listen_fd)
            {
                for (;;)
                {
                    int cfd = ::accept(listen_fd, 0, 0);
                    if (cfd < 0)
                        break;
                    set_nonblock(cfd);
                    pfds.push_back((pollfd){cfd, POLLIN, 0});
                    Client::clients[cfd] = Client(cfd);
#ifdef DEBUG
                    std::printf("accept: fd=%d\n", cfd);
#endif
                }
                continue;
            }
            if (p.revents & POLLOUT)
            {
                Client &cl = Client::clients[p.fd];
                if (!cl.wbuf.empty())
                {
                    ssize_t s = ::send(p.fd, cl.wbuf.data(), cl.wbuf.size(), 0);
                    if (s < 0)
                    {
                        if (errno == EAGAIN || errno == EWOULDBLOCK)
                        {
#ifdef DEBUG
                            std::printf("SEND would block, keeping POLLOUT\n");
#endif
                        }
                        else
                        {
#ifdef DEBUG
                            std::printf("SEND failed: %s\n", strerror(errno));
#endif
                            ::close(p.fd);
                            Client::clients.erase(p.fd);
                            pfds.erase(pfds.begin() + i);
                            --i;
                        }
                    }
                    else if (s == 0)
                    {
#ifdef DEBUG
                        std::printf("SEND returned 0, closing connection\n");
#endif
                        ::close(p.fd);
                        Client::clients.erase(p.fd);
                        pfds.erase(pfds.begin() + i);
                        --i;
                    }
                    else
                    {
                        cl.wbuf.erase(0, (size_t)s);
                        if (cl.wbuf.empty())
                        {
                            pfds[i].events &= ~POLLOUT;
                            if (cl.logout)
                            {
#ifdef DEBUG
                                std::printf("Closing connection after flush: %s\n",
                                            cl.nickname.c_str());
#endif
                                ::close(p.fd);
                                Client::clients.erase(p.fd);
                                pfds.erase(pfds.begin() + i);
                                --i;
                            }
                        }
                    }
                }
            }

            if (p.revents & POLLIN)
            {
                char buf[4096];
                ssize_t r = ::recv(p.fd, buf, sizeof(buf), 0);
                if (r <= 0)
                {
#ifdef DEBUG
                    std::printf("close: fd=%d (recv=%zd errno=%d)\n", p.fd, r, errno);
#endif
                    ::close(p.fd);
                    Client::clients.erase(p.fd);
                    pfds.erase(pfds.begin() + i);
                    --i;
                    continue;
                }

                Client &cl = Client::clients[p.fd];
                cl.rbuf.append(buf, (size_t)r);

                if (line_too_long(cl.rbuf))
                {
#ifdef DEBUG
                    std::printf("close: fd=%d (line too long)\n", p.fd);
#endif
                    ::close(p.fd);
                    Client::clients.erase(p.fd);
                    pfds.erase(pfds.begin() + i);
                    --i;
                    continue;
                }

                std::string line;
                while (pop_line(cl.rbuf, line))
                {
                    Message msg;
                    if (!Parser::parse(line, msg))
                        continue;
#ifdef DEBUG
                    std::printf("CMD=%s ARGC=%zu\n", msg.cmd.c_str(), msg.args.size());
#endif
                    dispatchCommand(cl, msg);
                }

                if (!cl.wbuf.empty())
                {
                    pfds[i].events |= POLLOUT;
#ifdef DEBUG
                    std::printf("SET POLLOUT: fd=%d, wbuf.size=%zu\n", p.fd, cl.wbuf.size());
#endif
                }
            }
        }
    }

    for (size_t i = 0; i < pfds.size(); ++i)
        ::close(pfds[i].fd);
    return 0;
}