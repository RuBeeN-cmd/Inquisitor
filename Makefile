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

SRC = main.c \
		utils/endian.c \
		utils/log.c \
		utils/ansi_color.c \
		parsing/parsing.c \
		parsing/debug.c \
		capture.c

CC = clang
CFLAGS = -Wall -Werror -Wextra -g3
INC = -Iincludes
OBJ = $(addprefix $(OBJ_DIR)/, $(SRC:.c=.o))

# -------------- Libs --------------

LIBFT_DIR = libs/libft
LIBFT_INCLUDE = $(LIBFT_DIR)/includes
LIBFT = $(LIBFT_DIR)/libft.a

LIB = $(LIBFT)
LIBFLAGS = -L$(dir $(LIBFT))
LIBFLAGS += -lft -lpcap
INC += -I$(LIBFT_INCLUDE)

# ---------- Compilation -----------

all: $(NAME)

$(NAME): $(LIB) $(OBJ_DIR) $(OBJ)
	@echo $(_GREEN)Compiling $(OBJ)...$(_END)
	@$(CC) $(CFLAGS) $(OBJ) $(LIBFLAGS) -o $@

$(OBJ_DIR):
	@mkdir -p $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	@echo $(_CYAN)Compiling $<...$(_END)
	@$(CC) -o $@ -c $< $(CFLAGS) $(INC)

# ------- LIBFT -------

%.a:
	@make -C $(dir $@)

# ---------- Public targets ----------

run: up
	@docker compose run --rm --build -it inquisitor zsh || true

up:
	@docker compose --profile tools up -d --build

clean:
	@echo $(_YELLOW)Cleaning $(OBJ)...$(_END)
	@rm -rf $(OBJ_DIR)

fclean: clean
	@echo $(_RED)Cleaning $(NAME)...$(_END)
	@rm -f $(NAME)

down: fclean
	@docker compose --profile tools --profile prod down -v

re: fclean all

.PHONY: all clean fclean re run up down