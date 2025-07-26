SRC =  src/main.cpp src/Config.cpp src/singleserver.cpp src/server.cpp\
	POST/main.cpp
NAME = webserv
CC = c++  
#-std=c++98 -Wall -Wextra -Werror

all: $(NAME)

$(NAME): $(SRC)
	$(CC) $(SRC) -o $(NAME)

clean:
	rm -rf $(NAME)

fclean: clean

re: fclean all