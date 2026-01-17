# Compiler and flags
CXX := g++
CXXFLAGS := -Wall -Wextra -Werror -Iheaders -MMD -MP -std=c++20 -flarge-source-files -g -O3
LDFLAGS := -lncursesw -lpanelw -lm

# Directories
SRC_DIR := sources
OBJ_DIR := build
BIN := roguepupu2

# Source files (recursive)
SRCS := $(shell find $(SRC_DIR) -name '*.cpp')

# Object files in build/
OBJS := $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRCS))

# Dependencies
DEP := $(OBJS:.o=.d)

# Default target
all: $(BIN)

# Link binary
$(BIN): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)
	@mkdir -p logs

# Compile object files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean
clean:
	rm -rf $(OBJ_DIR)

fclean: clean
	rm -f $(BIN)
	rm -rf logs/
	rm -f *.log

re: fclean all

.PHONY: all clean fclean re

-include $(DEP)

