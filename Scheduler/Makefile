build:
	gcc process_generator.c -g -o process_generator.out
	gcc clk.c -g -o clk.out
	gcc scheduler.c -g -o scheduler.out
	gcc process.c -g -o process.out
	gcc test_generator.c -g -o test_generator.out

clean:
	rm -f *.out  processes.txt

all: clean build

run:
	./process_generator.out
