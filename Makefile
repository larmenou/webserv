CXX			:=	c++
CXXFLAGS	:=	-g -Wall -Wextra -Werror -std=c++98 -MMD
INCS		:=	-I./includes
OBJFLAGS 	:=	-c
SRCS_DIR	:= ./srcs
OBJ_DIR		:= ./obj

NAME		:=	webserv

SRCS_F		:= 	$(addprefix $(SRCS_DIR)/, \
					main \
					)

SRCS_F		+= $(addprefix $(SRCS_DIR)/Config/, \
					Config \
					Route \
					ServerConf \
					)

SRCS_F		+= $(addprefix $(SRCS_DIR)/CGI/, \
					CGI \
					)


SRCS_F		+= $(addprefix $(SRCS_DIR)/Error/, \
					HTTPError \
					)

SRCS_F		+= $(addprefix $(SRCS_DIR)/Request/, \
					Request \
					Client \
					)

SRCS_F		+= $(addprefix $(SRCS_DIR)/Server/, \
					Server \
					RegisteredUsers \
					DirLister \
					)

SRCS		:=	$(addsuffix .cpp, $(SRCS_F))

OBJ			:=	$(SRCS:$(SRCS_DIR)/%.cpp=$(OBJ_DIR)/%.o)

DEPS		:= $(OBJ:%.o=%.d)

all: 		$(NAME)

-include $(DEPS)
$(OBJ): $(OBJ_DIR)/%.o : $(SRCS_DIR)/%.cpp
				@mkdir -p $(dir $@)
				$(CXX) $(CXXFLAGS) $(INCS) $(OBJFLAGS) $< -o $@

$(NAME):	$(OBJ)
				$(CXX) $(CXXFLAGS) $(INCS) $(OBJ) -o $(NAME)


clean:
				@rm -rf $(OBJ_DIR)

fclean:		clean
				@rm -f $(NAME)

re:			fclean all

.PHONY:		all clean fclean re