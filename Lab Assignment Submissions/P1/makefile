#setting the compiler
CC=gcc
#setting compiler flags
CFLAGS=-c
#setting the dependency programs
DEPS_PROGS = function.o main.o
#setting the debug flag
DFLAGS = -g
#setting the debugging files for debug-build
DEBUG_PROGS = function.o main.c
#setting the profile flag
GPROFILE = -pg

build: program

program: $(DEPS_PROGS) 
	$(CC)  $(DEPS_PROGS) -o program

debug-build: $(DEPS_PROGS)
	$(CC) $(DFLAGS) $(DEBUG_PROGS) -o debug-program

profile-build: $(DEPS_PROGS)
	$(CC) $(GPROFILE) $(DEBUG_PROGS) -o profile-program

run: build
	./program

run_profile: profile-build
	./profile-program

profile: run profile-build run_profile
	gprof -p -b profile-program > _profile.txt

clean: clean_txt clean_output clean_program clean_profile_program clean_debug_program clean_object

clean_txt: 
	rm *.txt
	

clean_object: 
	rm *.o

clean_program: program
	rm program 

clean_profile_program: profile-program 
	rm profile-program

clean_debug_program: debug-program
	rm debug-program

clean_output: 
	rm *.out
	
%.o: %.c
	$(CC) $(CFLAGS) $^ 


