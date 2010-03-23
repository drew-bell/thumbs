PROGRAM_NAME = thumbs
COMPILER = gcc
CFLAGS =  -no-cpp-precomp
CCFLAGS = -Wall -O2 -lgd -lexif

thumbs: main.c 
	${COMPILER} ${CCFLAGS} main.c -o ${PROGRAM_NAME}

clean: 
	rm -rf *.o thumbs
