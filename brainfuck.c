#include<stdio.h>
#include<stdlib.h>

#define EXEC_CONTINUE
#define EXEC_STOPPED_OK
#define EXEC_STOPPED_ERR

void read_program();
int execute();
int exec_cmd();



int verbose=0;
char execution_state=EXEC_CONTINUE
/*extend by that much the code/data arrays when needed*/
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

    if(2==argc && '-'==argv[1][0] && 'v'==argv[1][1]){
        verbose=1;
    }
    
    while(1){
        zeros(data,data_size);
        printf("\nready\n");
        read_program();
        
        execute();
        printf("\nexecution complete\n");
        zeros(code,code_size);
    }

  return 0;
}


void read_program(){
   
    while(1){
        /*write instruction chars from input to code array*/
        *cmd=getchar();
        switch(*cmd){
            /*char was a closing char?replace with eof and return*/
            case EOF:
            case '\0':
            case '\r':
            case '\n':
                *cmd=EOF;
                code_size=cmd-code;
                return;
            default:
                /*char was any other? move to next instruction position*/
                 cmd++;
                 
                /*reached end of code array? extend*/
                if(cmd-code>=code_size){
                    code_size+=block_size;
                    code=realloc(code,code_size);
                }
        }
    }
}


/*
* execute the program placed in code array unless an instruction causes
* 
*/
int execute(){
    execution_state=EXEC_CONTINUE;

    while(EXEC_CONTINUE == execution_state){
        exec_cmd();
    }
    
    return execution_state;
}

/*
*  execute the instruction at current cmd pointer and move to next
*  returns execution state after completing instruction
*  (continue/finished ok/error)
*/
int exec_cmd(){

    switch(*cmd){
        /*instructions without problem return continue_execution*/
        case '>': exec_mem_right(); break;
        case '<': exec_mem_left();  break;
        case '+': exec_inc_mem();   break;
        case '-': exec_dec_mem();   break;
        case '.': exec_putchar();   break;
        case ',': exec_getchar();   break;
        case ':': exec_putnum();    break;
        case EOF:
            /*reached program end? return execution complete*/
            execution_state= EXEC_STOPPED_OK;

        default :;
           /*ignore other characters*/
    }
    /*move to next instruction*/
    cmd++;
    return execution_state;
}
