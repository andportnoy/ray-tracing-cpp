PROG = main
INCLUDE = common.hpp
CPPFLAGS += $(INCLUDE:%=-include %)
CPPFLAGS += -Ofast -march=native
CPPFLAGS += -Wall -Wextra -std=c++20 -pedantic
CPPFLAGS += -flto
CPPFLAGS += -fopenmp

main: graphics.o
all: $(PROG)

video.mp4: main
	./$< | ffmpeg -y -r 60 -f image2pipe -i - -vcodec libx264 \
		-pix_fmt yuv420p -preset ultrafast $@ 2> /dev/null
clean:
	rm -rf *.o $(PROG)
