CXX			:=	c++
CXXFLAGS	:=	-g -Wall -Wextra -Werror -std=c++98
INCS		:= -I./includes -I./srcs/Server/
OBJFLAGS 	:=	-c
SRCS_DIR	:= ./srcs

NAME		:=	webserv
PARSER_TEST	:= parser-test
REQUEST_TEST := req

SRCS_F		:= 	main

SRCS_F		+= $(addprefix $(SRCS_DIR)/Config/, \
					Config \
					Route \
					ServerConf \
					)

SRCS_F		+= $(addprefix $(SRCS_DIR)/CGI/, \
					PhpCGI \
					)

SRCS_F		+= $(addprefix $(SRCS_DIR)/Request/, \
					Request \
					)

SRCS_F		+= $(addprefix $(SRCS_DIR)/Server/, \
					Server \
					)

SRCS		:=	$(addsuffix .cpp, $(SRCS_F))
OBJ			:=	$(addsuffix .o, $(SRCS_F))

DEP			:= Makefile

all: 		$(NAME)

%.o:		%.cpp $(DEP)
				$(CXX) $(CXXFLAGS) $(INCS) $(OBJFLAGS) $< -o $@

$(NAME):	$(OBJ)
				$(CXX) $(CXXFLAGS) $(INCS) $(OBJ) -o $(NAME)

$(PARSER_TEST): $(OBJ) srcs/Config/test.o
				$(CXX) $(CXXFLAGS) $(INCS) $(filter-out main.o,$(OBJ)) srcs/Config/test.o -o $@

$(REQUEST_TEST): $(OBJ) srcs/Request/test.o
				$(CXX) $(CXXFLAGS) $(INCS) $(filter-out main.o,$(OBJ)) srcs/Request/test.o -o $@

clean:
				@rm -f $(OBJ)

fclean:		clean
				@rm -f $(NAME)
				@rm -f $(PARSER_TEST)

re:			fclean all

.PHONY:		all clean fclean re