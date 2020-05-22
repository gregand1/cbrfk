
/*
*   greg androu
*   brainfuck interpreter
*/

#include<stdio.h>
#include <stdlib.h>
/*resize by that much the code/data arrays */
size_t block_size=4096;

/*code = instructions array*/
char *code=NULL;
size_t code_size=32768;
/*instruction ptr = program counter*/
char*cmd=NULL;

/*program memory */
char *data=NULL;
size_t data_size=32768;

/*program pointer*/
char*ptr=NULL;

int main(int argc,char**argv){
    code=malloc(code_size);
    data=malloc(data_size);


  return 0;
}
