all: memtest_sys memtest_dev

memtest_sys: 
	gcc -o memtest_sys mem_test.c -DSYSTEM_MALLOC

memtest_dev:
	gcc -o memtest_dev malloc.c mem_test.c

clean:
	rm -f memtest_sys memtest_dev

