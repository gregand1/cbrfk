# cbrfk
a simple brainfuck interpreter written in C

#usage
compile with gcc
run with -v parameter for verbosity

#brainfuck language synopsis:
<> move ptr ;
+- increase/decrease *ptr ;
,.: *ptr=getchar(),putchar(*ptr),putnumber(*ptr);
[...]:  while(*ptr!=0){....} ;
