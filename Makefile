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
LDFLAGS = -lstdc++ -lrt -lncurses -lhistory -lreadline

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^

$(OBJ_DIR)/%.o: %.cc
	$(CC) -c $(CXXFLAGS) $< -o $@

$(DEPEND): $(SRCS) $(HDRS)
	@echo $(OBJS)
	$(CC) -M $(CXXFLAGS) $(SRCS) > $@ 

clean:
	rm -rf $(OBJ_DIR)

-include $(DEPEND)

