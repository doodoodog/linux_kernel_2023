Run :
$ gcc -Wall -o qsort qsort_mt.c -lpthread

usage: qsort-mt [-stv] [-f forkelements] [-h threads] [-n elements]
	-l	Run the libc version of qsort
	-s	Test with 20-byte strings, instead of integers
	-t	Print timing results
	-v	Verify the integer results
Defaults are 1e7 elements, 2 threads, 100 fork elements

Class Ref. https://hackmd.io/@sysprog/linux2023-summer-quiz0
Code Ref. https://gist.github.com/jserv/38bca4ebfea1c434e1dfc15337f80eeb