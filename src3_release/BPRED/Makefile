# Pick your favorite compiler
CC = g++
OPT = -O3

TOP = ../..
INC = -I. \
      -I../ \
      -I$(TOP)/LibSS_smt/include

SIM_LINUX = -DSIM_LINUX
ENDIAN = -DBYTES_LITTLE_ENDIAN -DWORDS_LITTLE_ENDIAN

CFLAGS = $(OPT) $(INC) $(SIM_LINUX) $(ENDIAN)

BP_SRC	= bpred_interface.cc
BP_OBJ	= bpred_interface.o


all:	libBP_little.a

libBP_little.a:	$(BP_OBJ)
	ar r libBP_little.a  $(BP_OBJ)

.cc.o:
	$(CC) $(CFLAGS) -c $*.cc


clean:
	rm -f *.o libBP_little.a core
