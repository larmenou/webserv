CXX			:=	c++
CXXFLAGS	:=	-g -Wall -Wextra -Werror -std=c++98
OBJFLAGS 	:=	-c
SRCS_DIR	:= ./srcs

NAME		:=	webserv
PARSER_TEST	:= parser-test

SRCS_F		:= 	main \
				Server \

SRCS_F		+= $(addprefix $(SRCS_DIR)/Config/, \
					Config \
					Route \
					ServerConf \
					)

SRCS		:=	$(addsuffix .cpp, $(SRCS_F))
OBJ			:=	$(addsuffix .o, $(SRCS_F))

DEP			:= Makefile Server.hpp

all: 		$(NAME)

%.o:		%.cpp $(DEP)
				$(CXX) $(CXXFLAGS) $(OBJFLAGS) $< -o $@ -I.

$(NAME):	$(OBJ)
				$(CXX) $(CXXFLAGS) $(OBJ) -o $(NAME)

$(PARSER_TEST): $(OBJ) srcs/Config/test.o
				$(CXX) $(CXXFLAGS) $(filter-out main.o,$(OBJ)) srcs/Config/test.o -o $@

clean:
				@rm -f $(OBJ)

fclean:		clean
				@rm -f $(NAME)
				@rm -f $(PARSER_TEST)

re:			fclean all

.PHONY:		all clean fclean re