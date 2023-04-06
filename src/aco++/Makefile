# Makefile for ACO++

CPP= g++
CFLAGS= -O3 -w -std=c++11
EXE=acothop

all: thop.o utilities.o ants.o inout.o unix_timer.o ls.o parse.o acothop.o 
	$(CPP) $(CFLAGS) -o $(EXE) thop.o utilities.o ants.o inout.o unix_timer.o ls.o parse.o acothop.o 

thop.o: thop.cpp thop.h
	$(CPP) $(CFLAGS) -c thop.cpp

utilities.o: utilities.cpp utilities.h
	$(CPP) $(CFLAGS) -c utilities.cpp

ants.o: ants.cpp ants.h
	$(CPP) $(CFLAGS) -c ants.cpp

inout.o: inout.cpp inout.h
	$(CPP) $(CFLAGS) -c inout.cpp

unix_timer.o: unix_timer.cpp timer.h
	$(CPP) $(CFLAGS) -c unix_timer.cpp

ls.o: ls.cpp ls.h
	$(CPP) $(CFLAGS) -c ls.cpp

parse.o: parse.cpp parse.h
	$(CPP) $(CFLAGS) -c parse.cpp

acothop.o: ants.h utilities.h inout.h thop.h timer.h ls.h acothop.cpp
	$(CPP) $(CFLAGS) -c acothop.cpp

clean:
	rm -f $(EXE) *.o
