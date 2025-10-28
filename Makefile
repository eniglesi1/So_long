SRC =	./srcs/parseo.c	./srcs/readmap.c	./srcs/movement.c	./srcs/mapa_is_funcional.c	./srcs/so_long.c
OBJS = $(SRC:.c=.o)
LIB = ./includes/printf/libftprintf.a
HEADERS = ./includes/so_long.h
NAME = so_long
FLAGS = -Wall -Wextra -Werror

all: $(NAME)

%.o: %.c
	$(CC) -c $(FLAGS) $< -o $@

$(NAME):
	@make -s -C ./includes/printf
	gcc $(FLAGS) -o $(NAME) $(SRC) $(LIB) -lXext -lX11 -lm -lz

clean:
	/bin/rm -f $(OBJS)
	@make clean -s -C ./includes/printf

fclean: clean
	/bin/rm -f $(NAME)
	@make fclean -s -C ./includes/printf
	
re: fclean all
