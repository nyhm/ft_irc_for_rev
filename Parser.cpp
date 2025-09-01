/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Parser.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hnagashi <hnagashi@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/12 19:22:32 by hnagashi          #+#    #+#             */
/*   Updated: 2025/09/02 06:39:59 by hnagashi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Parser.hpp"
#include <cctype>

static void trim_spaces(std::string &s)
{
    std::string::size_type a = 0, b = s.size();
    while (a < b && (s[a] == ' ' || s[a] == '\t'))
        ++a;
    while (b > a && (s[b - 1] == ' ' || s[b - 1] == '\t'))
        --b;
    s.assign(s, a, b - a);
}

void Parser::to_upper(std::string &s)
{
    for (size_t i = 0; i < s.size(); ++i)
        s[i] = std::toupper(static_cast<unsigned char>(s[i]));
}

bool Parser::parse(const std::string &lineRaw, Message &out)
{
    out.cmd.clear();
    out.args.clear();

    std::string line = lineRaw;
    trim_spaces(line);
    if (line.empty())
        return false;
    std::string::size_type pos = 0;
    std::string::size_type sp = line.find_first_of(" \t");
    if (sp == std::string::npos)
    {
        out.cmd = line;
        to_upper(out.cmd);
        return !out.cmd.empty();
    }
    out.cmd = line.substr(0, sp);
    to_upper(out.cmd);
    pos = sp;

    while (pos < line.size())
    {
        while (pos < line.size() && (line[pos] == ' ' || line[pos] == '\t'))
            ++pos;
        if (pos >= line.size())
            break;

        if (line[pos] == ':')
        {
            out.args.push_back(line.substr(pos + 1));
            break;
        }
        else
        {
            std::string::size_type next = line.find_first_of(" \t", pos);
            if (next == std::string::npos)
            {
                out.args.push_back(line.substr(pos));
                break;
            }
            else
            {
                out.args.push_back(line.substr(pos, next - pos));
                pos = next;
            }
        }
    }
    return !out.cmd.empty();
}
