# ------------ COLORS --------------

_END="\033[0m"
_RED="\033[0;31m"
_GREEN="\033[0;32m"
_YELLOW="\033[0;33m"
_CYAN="\033[0;36m"

# ----------------------------------

NAME = inquisitor

SRC_DIR = srcs
OBJ_DIR = objs

SRC = main.c

CC = clang
CFLAGS = -Wall -Werror -Wextra -g3
INC = -Iincludes
OBJ = $(addprefix $(OBJ_DIR)/, $(SRC:.c=.o))

all: $(NAME)

$(NAME): $(OBJ_DIR) $(OBJ)
	@echo $(_GREEN)Compiling $(OBJ)...$(_END)
	@$(CC) $(CFLAGS) $(OBJ) -o $@

$(OBJ_DIR):
	@mkdir -p $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	@echo $(_CYAN)Compiling $<...$(_END)
	@$(CC) -o $@ -c $< $(CFLAGS) $(INC)
	
# ---------- Public targets ----------

run: up
	@docker compose run --rm --build -it inquisitor ash || true

up:
	@docker compose up -d --build

clean:
	@echo $(_YELLOW)Cleaning $(OBJ)...$(_END)
	@rm -rf $(OBJ_DIR)

fclean: clean
	@echo $(_RED)Cleaning $(NAME)...$(_END)
	@rm -f $(NAME)

down: fclean
	@docker compose --profile tools down -v

re: fclean all

.PHONY: all clean fclean re run up down