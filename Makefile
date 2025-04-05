NAME = raypup

LIB = libraylib.a

INC_DIR = inc

INCLUDE = bones.h raymath.h raylib.h rlgl.h raygui.h gui.h

SRC = main.c bones.c gui.c

OBJ_DIR = obj
OBJ = $(SRC:%.c=$(OBJ_DIR)/%.o)

CC = cc

CFLAGS = -Wall -I$(INC_DIR)

LDFLAGS = -L. -lraylib -lm -ldl -lpthread -lGL -lX11 -g -fsanitize=address -fsanitize=leak

all: $(NAME)

$(NAME): $(OBJ)
	$(CC) $(CFLAGS) -o $(NAME) $(OBJ) $(LDFLAGS)

$(OBJ_DIR)/%.o: %.c	Makefile
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
