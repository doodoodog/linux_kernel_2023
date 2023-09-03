Run :
$ gcc -O2 -Wall -std=c11 -o work-steal work-steal.c -lpthread
$ ./work-steal
...
work ter 1 finished
work item 10 finished
work item 11 finished
work item 14 finished
work item 6 finished
work item 16 finished
Expect 506 lines of output (including this one)

Class Ref. https://hackmd.io/@sysprog/linux2023-summer-quiz0
Code Ref. https://gist.github.com/jserv/4dfaf78cf516cc20f4bc55ce388c195d