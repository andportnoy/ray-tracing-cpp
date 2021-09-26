PROG = main
INCLUDE = common.hpp
CPPFLAGS += $(INCLUDE:%=-include %)
CPPFLAGS += -O3 -march=native
CPPFLAGS += -Wall -Wextra -std=c++20 -pedantic

main: graphics.o
all: $(PROG)
clean:
	rm -rf *.o $(PROG)
