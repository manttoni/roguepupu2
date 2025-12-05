# Compiler and flags
CXX := c++
CXXFLAGS := -Wall -Wextra -Werror -g -Iheaders -MMD -MP -O3# -std=c++20

LDFLAGS := -lncursesw -lpanelw -lm

# Directories
SRC_DIR := sources
SYSTEMS_DIR := sources/systems
OBJ_DIR := build
BIN := roguepupu2

# Find all .c files in sources directory
SRCS := $(wildcard $(SRC_DIR)/*.cpp)
# Add files in sources/systems/
SRCS += $(wildcard $(SYSTEMS_DIR)/*.cpp)

# Create corresponding .o filenames in build/
OBJS := $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SRCS))

# Depending on headers
DEP := $(OBJS:.o=.d)

ENTITY_HEADER = headers/entity_enums.hpp
ENTITY_JSON = data/entities.json
ENTITY_ENUM = scripts/enumerate_entities.py

# Default target
all: $(ENTITY_HEADER) $(BIN)

$(ENTITY_HEADER) : $(ENTITY_JSON) $(ENTITY_ENUM)
	python3 $(ENTITY_ENUM)

# Link object files into binary
$(BIN): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)
	@mkdir -p logs

# Compile source files into object files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(OBJ_DIR)
	@mkdir -p $(OBJ_DIR)/systems
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean build files
clean:
	rm -rf $(OBJ_DIR)

fclean: clean
	rm -f $(BIN)
	rm -rf logs/
	rm -f *.log

re: fclean all

.PHONY: all clean fclean re

-include $(DEP)
