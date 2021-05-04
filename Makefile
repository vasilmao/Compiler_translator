SRC_DIR := src
OBJ_DIR := obj
DEP_DIR := dep
BIN_DIR := bin # or . if you want it in the current directory

SRC := $(wildcard $(SRC_DIR)/*)
OBJ := $(SRC:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)
DEP := $(OBJ:$(OBJ_DIR)/%.o=$(DEP_DIR)/%.d)

OUT := puton.out

CPPFLAGS := -I include -MMD -MP # -I is a preprocessor flag, not a compiler flag
CFLAGS   :=             # some warnings about bad code


.PHONY: all clean

all: $(OUT)

$(OUT): $(OBJ)
	g++ $^ -o $@

-include $(obj:.o=.d)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp  | $(OBJ_DIR)
	g++ $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(BIN_DIR) $(OBJ_DIR):
	mkdir -p $@

clean:
	rm -rv $(BIN_DIR) $(OBJ_DIR)
