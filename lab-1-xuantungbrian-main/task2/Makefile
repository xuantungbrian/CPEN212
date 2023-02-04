smoketest: smoketest.asm binsearch.asm
	nasm -felf64 binsearch.asm
	nasm -felf64 smoketest.asm
	ld -o smoketest smoketest.o binsearch.o

.PHONY: clean
clean:
	rm -f smoketest binsearch.o smoketest.o
