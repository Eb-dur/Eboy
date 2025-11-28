binary_name = eboy


eboy: clean
	gcc -std=c99 -o $(binary_name) main.c cpu.c

clean:
	rm -f $(binary_name)
