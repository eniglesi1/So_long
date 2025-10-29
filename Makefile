SRC =	./srcs/parseo.c	./srcs/readmap.c	./srcs/movement.c	./srcs/mapa_is_funcional.c	./srcs/so_long.c
OBJS = $(SRC:.c=.o)
LIB = ./includes/printf/libftprintf.a ./minilibx-linux/libmlx.a ./minilibx-linux/libmlx_Linux.a
HEADERS = ./includes/so_long.h
NAME = so_long
FLAGS = -Wall -Wextra -Werror
CC = cc

all: $(NAME)

%.o: %.c
	$(CC) $(FLAGS) -c $< -o $@

$(NAME): $(OBJS)
	@make -s -C ./includes/printf
	$(CC) $(FLAGS) -o $(NAME) $(OBJS) $(LIB) -lXext -lX11 -lm -lz

clean:
	/bin/rm -f $(OBJS)
	@make clean -s -C ./includes/printf

fclean: clean
	/bin/rm -f $(NAME)
	@make fclean -s -C ./includes/printf

re: fclean all
