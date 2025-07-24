CC = c++

FLAGS = -Wall -Wextra -Werror -std=c++98

SRCS = main.cpp src/Parsing/parsing_request.cpp src/Parsing/utils/utils.cpp

OBJS = $(SRCS:.cpp=.o)

NAME = webserv
CC = c++  -std=c++98 -Wall -Wextra -Werror

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(FLAGS) -o $(NAME) $(OBJS)

$(NAME): $(SRC)
	$(CC) $(SRC) -o $(NAME)

run: $(NAME)
	@echo "$(GREEN)Running $(NAME)...$(RESET)"
	@./$(NAME) 

clean:
	rm -rf $(NAME)

fclean: clean

re: fclean all	

.PHONY: all clean fclean re run
.SECONDARY: $(OBJS)

