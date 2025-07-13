SRC =  main.cpp Config.cpp singleserver.cpp print.cpp
NAME = web
CC = c++  -std=c++98

all: $(NAME)

$(NAME): $(SRC)
	$(CC) $(SRC) -o $(NAME)

clean:
	rm -rf $(NAME)

fclean: clean

re: fclean all