CC = g++
OPT = -O3
#OPT = -g

TOP = ..
INC = -I. \
      -I$(TOP)/LibSS_smt/include \
      -I./BPRED -I./MEM -I./UTILS 
LIB = -L$(TOP)/LibSS_smt/lib \
      -L./BPRED -L./MEM -L./UTILS 

SIM_LINUX = -DSIM_LINUX
ENDIAN = -DBYTES_LITTLE_ENDIAN -DWORDS_LITTLE_ENDIAN

#WARN = -Wall
CFLAGS = $(OPT) $(WARN) $(INC) $(LIB) $(SIM_LINUX) $(ENDIAN)

SIM_SRC = processor.cc \
	  renamer.cc \
	  fetch.cc decode.cc rename.cc dispatch.cc schedule.cc register_read.cc execute.cc writeback.cc retire.cc \
	  alu.cc squash.cc checker.cc \
	  fetch_queue.cc issue_queue.cc lane.cc pipeline_register.cc payload.cc \
	  simulator.cc parameters.cc globals.cc sim_input.cc sim_output.cc

SIM_OBJ = processor.o \
	  renamer.o \
	  fetch.o decode.o rename.o dispatch.o schedule.o register_read.o execute.o writeback.o retire.o \
	  alu.o squash.o checker.o \
	  fetch_queue.o issue_queue.o lane.o pipeline_register.o payload.o \
	  simulator.o parameters.o globals.o sim_input.o sim_output.o


all: little

little: $(SIM_OBJ)
	$(CC) -o sim_little $(CFLAGS) \
		$(SIM_OBJ) -lBP_little -lMEM_little -lUTILS_little \
		-lSS_smt_little
	@echo "-----------DONE WITH SIM FOR LITTLE ENDIAN-----------"

.cc.o:
	$(CC) $(CFLAGS) -c $*.cc


clean:
	rm -f *.o core Makefile.bak sim_little

clobber:
	rm -f *.o
