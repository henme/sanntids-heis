SRC = src
OBJ = obj
BIN = bin

PROJECTS = $(patsubst $(SRC)/%, %, $(wildcard $(SRC)/*))

CC = g++
CFLAGS = -Wall -g -std=c++11
LINKFLAGS = -Wall -g -lboost_system -lboost_thread

default:
	@echo "Usage: make <PROJECT>|all"
	@echo "Projects: $(PROJECTS)"

all: $(PROJECTS)

$(BIN):
	@mkdir -p $@

clean:
	rm -rf $(OBJ)
	rm -rf $(BIN)

define PROJECT_template

SOURCES-$1 = $$(wildcard $$(SRC)/$1/*.cpp)
OBJECTS-$1 = $$(patsubst $$(SRC)/$1/%.cpp,$$(OBJ)/$1/%.o,$$(SOURCES-$1))

$1: $$(BIN)/$1

run-$1: $$(BIN)/$1
	@echo "\n\n===== RUNNING APPLICATION =====\n"
	@$$<

$$(OBJ)/$1/%.o: $$(SRC)/$1/%.cpp | $$(OBJ)/$1
	$$(CC) $$(CFLAGS) -c $$< -o $$@

$$(OBJ)/$1:
	@mkdir -p $$@

$$(BIN)/$1: $$(OBJECTS-$1) | $$(BIN)
	$$(CC) $$(OBJECTS-$1) $$(LINKFLAGS) -o $$@

endef

$(foreach p, $(PROJECTS), \
	$(eval $(call PROJECT_template,$(p)))\
)
