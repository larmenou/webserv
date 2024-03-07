CC		= c++
FLAGS	= -Wall -Wextra -Werror -std=c++98 -MMD -g
SRC		= Config.cpp ServerConf.cpp Route.cpp
OBJ		= $(SRC:%.cpp=$(OBJ_DIR)/%.o)
OBJ_DIR = ./obj
NAME	= test
DEPS	= $(OBJ:%.o=%.d)

all: $(NAME)

-include $(DEPS)
$(OBJ_DIR)/%.o : %.cpp
	@mkdir -p $(OBJ_DIR)
	$(CC) $(FLAGS) -o obj/$(<:.cpp=.o) -c $<

$(NAME): $(OBJ)
	$(CC) $(FLAGS) $(OBJ) -o $(NAME)

clean :
	rm -rf ./obj 

fclean : clean
	rm -f $(NAME)

re : fclean all

.PHONY :
	clean fclean re all
