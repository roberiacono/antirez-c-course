all: toyforth

toyforth: toyforth.c
	$(CC) toyforth.c -Wall -W -o toyforth

clean: rm -rf toyforth