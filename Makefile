PROGRAM_NAME = thumbs
CC = gcc
CCFLAGS = -Wall -g
srcdir = .
all: thumbs

thumbs: $(srcdir)/main.o \
	functions.o \
	args.o \
	image_processing.o
	${CC} ${CCFLAGS} --std=c99 -ggdb -framework littleCMS -framework ApplicationServices -framework Accelerate -o ${PROGRAM_NAME} args.o functions.o image_processing.o main.o

.c.o:
	$(CC) $(CFLAGS) --std=c99 -ggdb -c $< -o $@

clean: 
	rm -rf *.o thumbs
