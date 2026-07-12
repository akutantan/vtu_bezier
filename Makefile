PROG := run
GCC := g++
INC_DIR := include
SRC_DIR := src
OBJ_DIR := obj
FLAGS := -std=c++17 -O3 -I$(INC_DIR)

SRCS := $(wildcard $(SRC_DIR)/*.cpp)
OBJS := $(notdir $(SRCS:.cpp=.o))
OBJSDIR := $(addprefix $(OBJ_DIR)/,$(OBJS))

all: $(PROG)

sim: $(PROG)
	./$(PROG)

$(PROG): $(OBJSDIR)
	$(GCC) $(FLAGS) -o $@ $^

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(OBJ_DIR)
	$(GCC) $(FLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR) $(PROG)

clean-results:
	rm -rf results

clean-all: clean clean-results
