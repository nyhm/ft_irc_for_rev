# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: hnagashi <hnagashi@student.42.fr>          +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2025/08/06 09:18:03 by hnagashi          #+#    #+#              #
#    Updated: 2025/08/21 10:38:25 by hnagashi         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME := ircserv
CXX := c++
CXXFLAGS := -std=c++98 -Wall -Wextra -Werror
CPPFLAGS := -I.

SRC := main.cpp Parser.cpp Client.cpp Util.cpp Channel.cpp Command.cpp Server.cpp
OBJ := $(SRC:.cpp=.o)

all: $(NAME)
$(NAME): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJ)
%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c $< -o $@
clean:
	rm -f $(OBJ)
fclean: clean
	rm -f $(NAME)
re: fclean all
.PHONY: all clean fclean re
