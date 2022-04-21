# Pick your favorite compiler
CC = g++
OPT = -O3
#OPT = -g

TOP = ../..
INC = -I. \
      -I../ \
      -I../UTILS \
      -I$(TOP)/LibSS_smt/include

SIM_LINUX = -DSIM_LINUX
ENDIAN = -DBYTES_LITTLE_ENDIAN -DWORDS_LITTLE_ENDIAN

CFLAGS = $(OPT) $(INC) $(SIM_LINUX) $(ENDIAN)

SRC	= dcache.cc mem_interface.cc
OBJ	= dcache.o mem_interface.o


all:	libMEM_little.a

libMEM_little.a: $(OBJ)
	ar r libMEM_little.a  $(OBJ)

.cc.o:
	$(CC) $(CFLAGS) -c $*.cc


clean:
	rm -f *.o libMEM_little.a core
