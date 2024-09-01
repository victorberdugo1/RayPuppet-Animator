NAME = bones

LIB = libraylib.a

INC_DIR = inc

INCLUDE = bones.h raymath.h raylib.h rlgl.h raygui.h

SRC = main.c bones.c

OBJ = $(SRC:.c=.o)

CC = cc

CFLAGS = -Wall -I$(INC_DIR)

LDFLAGS = -L. -lraylib -lm -ldl -lpthread -lGL -lX11

all: $(NAME)

$(NAME): $(OBJ)
	$(CC) $(CFLAGS) -o $(NAME) $(OBJ) $(LDFLAGS)

%.o: %.c $(INCLUDE)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
