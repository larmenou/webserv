CXX			:=	c++
CXXFLAGS	:=	-g -Wall -Wextra -Werror -std=c++98
INCS		:= -I./includes
OBJFLAGS 	:=	-c
SRCS_DIR	:= ./srcs

NAME		:=	webserv
PARSER_TEST	:= parser-test
REQUEST_TEST := req
CGI_TEST	:= cgi
LS_TEST		:= ls

SRCS_F		:= 	main

SRCS_F		+= $(addprefix $(SRCS_DIR)/Config/, \
					Config \
					Route \
					ServerConf \
					)

SRCS_F		+= $(addprefix $(SRCS_DIR)/CGI/, \
					CGI \
					)

SRCS_F		+= $(addprefix $(SRCS_DIR)/Request/, \
					Request \
					)

SRCS_F		+= $(addprefix $(SRCS_DIR)/Server/, \
					Server \
					RegisteredUsers \
					DirLister \
					)

SRCS		:=	$(addsuffix .cpp, $(SRCS_F))
OBJ			:=	$(addsuffix .o, $(SRCS_F))

DEP			:= Makefile

all: 		$(NAME)

%.o:		%.cpp $(DEP)
				$(CXX) $(CXXFLAGS) $(INCS) $(OBJFLAGS) $< -o $@

$(NAME):	$(OBJ)
				$(CXX) $(CXXFLAGS) $(INCS) $(OBJ) -o $(NAME)

$(PARSER_TEST): $(OBJ) srcs/Config/test.cpp
				$(CXX) $(CXXFLAGS) $(INCS) $(filter-out main.o,$(OBJ)) srcs/Config/test.cpp -o $@

$(REQUEST_TEST): $(OBJ) srcs/Request/test.cpp
				$(CXX) $(CXXFLAGS) $(INCS) $(filter-out main.o,$(OBJ)) srcs/Request/test.cpp -o $@

$(LS_TEST): $(OBJ) srcs/Server/test.cpp
				$(CXX) $(CXXFLAGS) $(INCS) $(filter-out main.o,$(OBJ)) srcs/Server/test.cpp -o $@

$(CGI_TEST): $(OBJ) srcs/CGI/test.cpp
				$(CXX) $(CXXFLAGS) $(INCS) $(filter-out main.o,$(OBJ)) srcs/CGI/test.cpp -o $@


clean:
				@rm -f $(OBJ)

fclean:		clean
				@rm -f $(NAME) $(PARSER_TEST) $(CGI_TEST) $(REQUEST_TEST) $(LS_TEST)

re:			fclean all

.PHONY:		all clean fclean re