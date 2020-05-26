/*
*   grandr1
*   a c brainfuck interpreter
*
*   if anybody asks,cbrfk has gnu gpl v3 licence
*/
#include<stdio.h>
#include <stdlib.h>

#define EXEC_CONTINUE 0
#define EXEC_STOPPED_ERR 1
#define EXEC_STOPPED_OK 2


void read_program();
int execute();
int exec_cmd();
int exec_mem_right();
int exec_mem_left();
int exec_getchar();
int exec_putchar();
int exec_inc_mem();
int exec_dec_mem();
int exec_putnum();
int exec_from();
int exec_to();
char* seek_closing_bracket(char*);
char* seek_opening_bracket(char*);

int link_brackets(char*code,char**links);
void zeros(char*arr,size_t len);
void print_exec_state();

/*program state:  execute / finished ok / finished with error */
char execution_state=EXEC_CONTINUE;

/*-v sets verbose to 1 for detailed output about execution */
char verbose=0;


size_t block_size=4096;  /*array extension step*/


char *code=NULL;         /* "code segment" */
size_t code_size=32768;
char*cmd=NULL;           /*current instruction pointer*/



char**links=NULL;
int lp=0;

char *data=NULL;         /*"data segment*/
size_t data_size=32768;
char*ptr=NULL;           /*current memory pos. pointer*/




/*
*   allocate & init code/data arrays
*   and anything else that runs once at start of the interpreter
*/
void initialize(int argc,char**argv){
    code=malloc(code_size);
    data=malloc(data_size);
    links=malloc(sizeof(char*)*code_size);

    if(2==argc && '-'==argv[1][0] && 'v'==argv[1][1]){
        verbose=1;
    }
}


/*
*  initialize 
*  reset data & ptr ,read line, execute as bf code ,repeat
*/
int main(int argc,char**argv){
    /*initialize*/
    initialize(argc,argv);

    /*execcute every line of input as a separate bf program*/
    while(1){
        /* data memory<-0 */
        zeros(data,data_size);
        /*pointers point to start of arrays*/
        ptr=data;
        cmd=code;

        /*read bf program from input*/
        printf("\nready\n");

        read_program();
        
        lp=link_brackets(code,links);

        /*execute program stored in code array*/
        execute();
        printf("\nexecution complete\n");

        /*wipe code to prepare for next line*/
        zeros(code,code_size);
    }

    return 0;
}



/*
*  read a bf program from input to code array. extend code if necesary
*/
void read_program(){
    size_t i=0;
    while(1){
        /*write instr to array*/
        *cmd = getchar();
        switch(*cmd){
        /*if instr. end of bf program,set in code as end of program and return*/
            case '\0':
            case '\r':
            case '\n':
                *cmd=EOF;
                code_size = cmd-code;
                cmd=code;
                return;
            default:
                /*normal instruction? proceed to next mem.address*/
                cmd++;
                
                /*extend array if cmd >code array range */
                if(cmd-code >= code_size){
                    code_size+=block_size;
                    code=realloc(code,code_size);
                    links=realloc(links,sizeof(char*)*code_size);
                }
        }
    }
}



/*
* execute instructions in code while execution_state = exec_continue
*/
int execute(){
    execution_state=EXEC_CONTINUE;

    while(EXEC_CONTINUE==execution_state){
        exec_cmd();
    }
    return execution_state;
}




/*
*   execute a single instruction at *cmd
*   update and return execution state
*/
int exec_cmd(){

    switch(*cmd){
        /*cmd points to instruction? 
          exec accordingly*/
        case '>':exec_mem_right();break;
        case '<': exec_mem_left();break;
        case '+': exec_inc_mem(); break;
        case '-': exec_dec_mem(); break;
        case '.': exec_putchar(); break;
        case ',': exec_getchar(); break;
        case ':': exec_putnum();  break;
        case '[': exec_from();    break;
        case ']': exec_to();      break;
        case EOF:
        /*cmd points to end of program?
          complete. change exec state*/
            execution_state= EXEC_STOPPED_OK;
            break;
            
        default :;
       /*cmd points to any other character?
         ignore*/
            break;
    }
    
    /*move to next instruction*/
    cmd++;
    
    /*returns the execution state*/
    return execution_state;
}

/*
*   go forward to matching closing bracket,
*   returns null if reaches end of code array without finding it
*   returns address of closing bracket otherwise
*/
char* seek_closing_bracket_iter(){
    char*tmp=cmd;
    int scope=1;

    while(scope!=0){
        tmp++;
        switch(*tmp){
            case ']':scope--;break;
            case '[':scope++;break;
            case EOF:
            fprintf(stderr,"missing closing bracket (%ld);\n",(long)(cmd-code));
            return NULL;
        }
    }
    return tmp;
}



/*
*   go backwards to matching opening bracket,
*   returns null if passes start of code array without finding it
*   returns address of closing bracket otherwise
*/
char* seek_opening_bracket_iter(){
    char*tmp=cmd;
    int scope=1;

    while(scope!=0){
        tmp--;
        if(tmp<=code){
            fprintf(stderr,"missing opening bracket (%ld);\n",(long)(cmd-code));
            execution_state=EXEC_STOPPED_ERR;
            return NULL;
        }
        switch(*tmp){
            case ']':scope++;break;
            case '[':scope--;break;
        }
    }
    return tmp;
}


char* seek_opening_bracket(char*cmd_ptr){
    int i=1;
    while(i<lp && links[i]!=cmd_ptr)
        i+=2;
    
    
    if(i>=lp)
        return NULL;
    else
        return links[i-1];
}

char* seek_closing_bracket(char*cmd_ptr){
    int i=0;
    while(i<lp && links[i]!=cmd_ptr)
        i+=2;
    
    
    if(i>=lp)
        return NULL;
    else
        return links[i+1];
}

/*reset an array to zeros */
void zeros(char*arr,size_t len){
    while(len-->0){
        arr[len]=0;
    }
}



/*move data pointer right */
int exec_mem_right(){
    ++ ptr; 
    if(ptr-data >= data_size){
        data_size+=block_size;
        data=realloc(data,data_size);
    }

    if(verbose)
        printf("(%ld)< mem[%ld]:%c\n",(long)(cmd-code),(long)(ptr-data),*ptr);
    return EXEC_CONTINUE;           
}

/*move data pointer left*/
int exec_mem_left(){
    -- ptr;
    
    if(verbose)
        printf("(%ld)< mem[%ld]:%c\n",(long)(cmd-code),(long)(ptr-data),*ptr);
    
    if(ptr<data){
        printf("error:data underrun (%ld)< mem[%ld]\n",(long)(cmd-code),(long)(ptr-data));
        execution_state=EXEC_STOPPED_ERR;
        return EXEC_STOPPED_ERR;
    }
    return EXEC_CONTINUE;
}

/*increase value of selected mem.pos*/
int exec_inc_mem(){
    ++*ptr;
    return EXEC_CONTINUE;
}

/*decrease value of selected mem.pos*/
int exec_dec_mem(){
    --*ptr;
    return EXEC_CONTINUE;
}


/*curr mem <- getchar*/
int exec_getchar(){
    *ptr=getchar();
    return EXEC_CONTINUE;
}

/*putchar <- curr mem value*/
int exec_putchar(){
    putchar(*ptr);
    return EXEC_CONTINUE;
}

/*print mem value as a number*/
int exec_putnum(){
    printf("%d ",*ptr);
    return EXEC_CONTINUE;
}


/*skip forward to closing bracket if ptr value=zero */
int exec_from(){
    if(0==*ptr){
        cmd=seek_closing_bracket(cmd);
        if(NULL==cmd)
            return EXEC_STOPPED_ERR;
    }
    return EXEC_CONTINUE;
}


/*return back to opening bracket if ptr value=nonzero */
int exec_to(){
    if(0!=*ptr){
        cmd=seek_opening_bracket(cmd);
        if(NULL==cmd)
            return EXEC_STOPPED_ERR;
    }
    return EXEC_CONTINUE;
}



int link_brackets(char*code,char**links){
    char* stack[2000];
    int sp=0;
    int stack_size=0;
    int i=0;
    int lp=0;

    while(EOF != code[i]){
        switch(code[i]){
            case '[':
                stack[sp++]=code+i;
                break;

            case ']':
                links[lp]=stack[--sp];
                links[lp+1]=code+i;
                lp+=2;
                break;
        }
        i++;
    }
    return lp;
}
