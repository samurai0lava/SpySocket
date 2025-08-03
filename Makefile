CC = c++

FLAGS = -Wall -Wextra -Werror -std=c++98

SRCS = main.cpp src/Parsing/parsing_request.cpp src/Config.cpp src/server.cpp src/singleserver.cpp src/Parsing/utils/utils.cpp src/MethodHandler.cpp src/Methods/Get.cpp src/utils/RespondError.cpp src/Methods/DELETE.cpp

OBJS = $(SRCS:.cpp=.o)

NAME = webserv


all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(FLAGS) -o $(NAME) $(OBJS)

%.o: %.cpp
	$(CC) $(FLAGS) -c $< -o $@

clean:
	@rm -rf	 $(OBJS)

fclean: clean
	@rm -rf $(NAME)

run: all clean
	@./$(NAME)

re: fclean all	

.PHONY: all clean fclean re run
.SECONDARY: $(OBJS)

