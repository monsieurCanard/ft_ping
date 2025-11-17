NAME = ft_ping

CC_FLAGS = -Wall -Wextra -Werror -I includes/

SRC_DIR = src/

SRC = $(SRC_DIR)main.c \
			$(SRC_DIR)builder.c \
			$(SRC_DIR)client.c \
			$(SRC_DIR)error.c \
			$(SRC_DIR)loop.c \
			$(SRC_DIR)parser.c \
			$(SRC_DIR)printer.c \
			$(SRC_DIR)timestamp.c \
			$(SRC_DIR)verifier.c \
			$(SRC_DIR)exit.c

OBJ_DIR = obj/

OBJ = $(patsubst $(SRC_DIR)%.c, $(OBJ_DIR)%.o, $(SRC))

all: $(NAME)

$(NAME): $(OBJ)
	@echo "Linking..."
	@gcc $(CC_FLAGS) -o $@ $^ -lm
	@echo "$(NAME) compiled successfully!"

$(OBJ_DIR):
	@mkdir -p $@

$(OBJ_DIR)%.o: $(SRC_DIR)%.c | $(OBJ_DIR)
	@echo "Compiling $<..."
	@gcc $(CC_FLAGS) -c $< -o $@
	@echo "$< compiled successfully!"

clean:
	@echo "Cleaning object files..."
	@rm -rf $(OBJ_DIR)
	@echo "Object files cleaned."

fclean: clean
	@echo "Removing executable..."
	@rm -f $(NAME)
	@echo "Executable removed."

re: fclean all