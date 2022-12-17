###### Arguments for run ######

PROC_GEN_ARGS=processes.txt -sch 1 -q 2

SCHED_ARGS=

###### Compiler options  ######

CC=gcc
IDIR=../include
CFLAGS=-Wall -g -I$(IDIR)
LDFLAGS=-lm
OBJDIR=../obj
BINDIR=../bin
OBJ = $(addprefix $(OBJDIR)/, $(patsubst %.c, %.o, $(wildcard *.c)))
OBJ_PROC=$(OBJDIR)/data_structures.o $(OBJDIR)/process_generator.o
OBJ_SCHED=$(OBJDIR)/data_structures.o $(OBJDIR)/scheduler.o
OBJ_TEST=$(OBJDIR)/data_structures.o $(OBJDIR)/test.o

###### Rules ######

all: $(OBJDIR) $(BINDIR) process_generator test_generator scheduler clk process
	chmod 0777 $(BINDIR)/*

$(OBJDIR):
	mkdir $(OBJDIR)

$(BINDIR):
	mkdir $(BINDIR)


$(OBJDIR)/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

#all: process_generator test_generator scheduler clk process
test: $(OBJ_TEST)
	$(CC) $(CFLAGS) -o $(BINDIR)/$@.out $^

data_structures:
	$(CC) $(CFLAGS) -o $@ $^
#	$(CC) $(CFLAGS) data_structures.o

process_generator: $(OBJ_PROC)
	$(CC) $(CFLAGS) -o $(BINDIR)/$@.out $^
#	$(CC) $(CFLAGS) -o process_generator.out process_generator.o data_structures.o

test_generator: $(OBJDIR)/test_generator.o
	$(CC) $(CFLAGS) -o $(BINDIR)/$@.out $^

scheduler: $(OBJ_SCHED)
	$(CC) $(CFLAGS) -o $(BINDIR)/$@.out $^ $(LDFLAGS) 

clk: $(OBJDIR)/clk.o
	$(CC) $(CFLAGS) -o $(BINDIR)/$@.out $^
	
process: $(OBJDIR)/process.o
	$(CC) $(CFLAGS) -o $(BINDIR)/$@.out $^

run: clk process_generator scheduler
	$(BINDIR)/clk.out
	$(BINDIR)/process_generator.out $(PROC_GEN_ARGS)
	$(BINDIR)/scheduler.out	$(SCHED_ARGS)

run_test: test_generator
	$(BINDIR)/test_generator.out

clean:
	rm -f *.out *.o processes.txt $(OBJDIR)/*.o $(BINDIR)/*