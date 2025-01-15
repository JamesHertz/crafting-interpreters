# dirs
BIN_DIR := bin

# files c:
SRC := $(wildcard src/*.c)
OBJ := $(SRC:src/%.c=$(BIN_DIR)/%.o)
EXE := $(BIN_DIR)/clox

FLAGS := -Wall
ifdef D
	FLAGS += -DDEBUG=1
endif

all: $(EXE)

$(EXE): $(OBJ)
	$(CC) $(FLAGS) -o $@ $^

$(BIN_DIR)/%.o: src/%.c | $(BIN_DIR)
	$(CC) $(FLAGS) -c -o $@ $<

$(BIN_DIR):
	mkdir -p $@

.PHONNY: clean
clean:
	@rm -rfv $(BIN_DIR)
