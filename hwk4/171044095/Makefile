#file for compiling
FILES = main.c utils.c parsearg.c queue.c
#G++ compiler
Gcc = gcc 
#output
OUTPUT = program

all: $(FILES)	
	$(Gcc) $(FILES) -Wall -o $(OUTPUT) -lrt -pthread
