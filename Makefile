NAME = ft_ping

CC_FLAGS = -Wall -Wextra -Werror -I include/

SRC_DIR = src/

SRC = $(shell find $(SRC_DIR) -name '*.c')

OBJ_DIR = obj/

OBJ = $(patsubst $(SRC_DIR)%.c, $(OBJ_DIR)%.o, $(SRC))

all: $(NAME)

$(NAME): $(OBJ)
	@echo "Linking..."
	@gcc $(CC_FLAGS) -o $@ $^
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