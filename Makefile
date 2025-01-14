SRC = $(shell find ./src -iname "*.cpp")

NAME = webserv

CPPFLAGS = -Wall -Wextra -Werror -std=c++20 #-fsanitize=thread

BLUE =		\033[0;34m
GREEN =		\033[0;32m
RESET =		\033[0m


OBJ_DIR =	obj

OBJ =		$(SRC:%.cpp=$(OBJ_DIR)/%.o)

DEPS =		$(OBJ:.o=.d)

$(OBJ_DIR)/%.o: %.cpp
	@mkdir -p $(@D)
	c++ $(CPPFLAGS) -MMD -c $< -o $@


$(NAME) :	$(OBJ)
	@echo "$(BLUE)Building $(NAME)...$(RESET)"
	c++ $(CPPFLAGS) $(OBJ) $(LIBFT) -o $(NAME)
	@echo "$(GREEN)$(NAME) built$(RESET)"

all :		$(NAME)

-include $(DEPS)

clean:
	rm -f $(OBJ) $(DEPS)
	rm -rf $(OBJ_DIR)

fclean: clean
	rm -f log.txt
	rm -f $(NAME)

re: fclean all


.PHONY:
	all clean fclean re
