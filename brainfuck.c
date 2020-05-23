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
char* seek_closing_bracket();
char* seek_opening_bracket();

void zeros(char*arr,size_t len);
void print_exec_state();

/*
* program state:execute/finished ok/finished with error
*/
char execution_state=EXEC_CONTINUE;
char verbose=0;

/*array extension step*/
size_t block_size=4096;

/* "code segment" */
char *code=NULL;
size_t code_size=32768;
/*current instruction pointer*/
char*cmd=NULL;

/*"data segment*/
char *data=NULL;
size_t data_size=32768;
/*current memory pos. pointer*/
char*ptr=NULL;




void initialize(){
    code=malloc(code_size);
    data=malloc(data_size);

    if(2==argc && '-'==argv[1][0] && 'v'==argv[1][1]){
        verbose=1;
    }
}

int main(int argc,char**argv){
    /*initialize*/
    initialize();

    while(1){
        zeros(data,data_size);
        
        ptr=data;
        cmd=code;

        printf("\nready\n");
        read_program();
        
        execute();
        printf("\nexecution complete\n");

        zeros(code,code_size);
    }

    return 0;
}

int execute(){
    execution_state=EXEC_CONTINUE;

    while(EXEC_CONTINUE==execution_state){
        exec_cmd();
    }
    return execution_state;
}

void read_program(){
    size_t i=0;
    while(1){
        *cmd = getchar();
        switch(*cmd){

            case '\0':
            case '\r':
            case '\n':
                *cmd=EOF;
                code_size = cmd-code;
                cmd=code;
                return;
            default:
                cmd++;
                if(cmd-code >= code_size){
                    code_size+=block_size;
                    code=realloc(code,code_size);
                }
        }
    }
}



int exec_cmd(){

    switch(*cmd){
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
            execution_state= EXEC_STOPPED_OK;

        default :;
           /*ignore other characters*/
    }
    cmd++;
    return EXEC_CONTINUE;
}


char* seek_closing_bracket(){
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

char* seek_opening_bracket(){
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


void zeros(char*arr,size_t len){
    while(len-->0){
        arr[len]=0;
    }
}




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

int exec_inc_mem(){
    ++*ptr;
    return EXEC_CONTINUE;
}
int exec_dec_mem(){
    --*ptr;
    return EXEC_CONTINUE;
}

int exec_getchar(){
    *ptr=getchar();
    return EXEC_CONTINUE;
}

int exec_putchar(){
    putchar(*ptr);
    return EXEC_CONTINUE;
}

int exec_putnum(){
    printf("%d ",*ptr);
    return EXEC_CONTINUE;
}

int exec_from(){
    if(0==*ptr){
        cmd=seek_closing_bracket(cmd);
        if(NULL==cmd)
            return EXEC_STOPPED_ERR;
    }
    return EXEC_CONTINUE;
}

int exec_to(){
    if(0!=*ptr){
        cmd=seek_opening_bracket(cmd);
        if(NULL==cmd)
            return EXEC_STOPPED_ERR;
        return EXEC_CONTINUE;
    }
}
