.PHONY: clean all

CC = gcc

SRC_DIR = .
OBJ_DIR = obj

dummy := $(shell mkdir -p $(OBJ_DIR))

SRCS = $(wildcard *.cc)
HDRS = $(wildcard *.hh)
OBJS = $(foreach o, $(patsubst %.cc, %.o, $(SRCS)), $(OBJ_DIR)/$(o))

TARGET = obj/util
DEPEND = obj/depend.d

CXXFLAGS = -g

#LDFLAGS must be placed before object files, otherwise undefined reference occurs during ld
#-lreadline must be placed before -lhistory, otherwise segmentation fault occurs when pressing up arrow to search history command
LDFLAGS = -lstdc++ -lncurses -lreadline -lhistory 

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $^ $(LDFLAGS) -o $@

$(OBJ_DIR)/%.o: %.cc
	$(CC) -c $(CXXFLAGS) $< -o $@

# sed is used to add obj_dir prefix to target in depend.d
$(DEPEND): $(SRCS) $(HDRS)
	$(CC) -M $(CXXFLAGS) $(SRCS) > $@ 
	sed -i $@ -e 's/\(.*\.o\)/obj\/\1/'

clean:
	rm -rf $(OBJ_DIR)

-include $(DEPEND)
