# cbrfk
a simple brainfuck interpreter written in C

#usage
compile with gcc
run with -v parameter for verbosity

brainfuck language synopsis:
<  ptr--
>  ptr++
- *ptr--
+ *ptr++
[ while(*ptr>0){....
] ...}while(*ptr>0)
, *ptr = getchar();
. putchar(*ptr);
: stdout <- (int)*ptr;
