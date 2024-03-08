CXX			:=	c++
CXXFLAGS	:=	-g -Wall -Wextra -Werror -std=c++98
OBJFLAGS 	:=	-c

NAME		:=	webserv

SRCS_F		:= 	main \
				Server \

SRCS_F		+= $(addprefix ./Config/, \
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

parser-test: $(OBJ)
				$(CXX) $(CXXFLAGS) $(filter-out main.o,$(OBJ)) Config/test.cpp -o $@

clean:
				@rm -f $(OBJ)

fclean:		clean
				@rm -f $(NAME)

re:			fclean all

.PHONY:		all clean fclean re