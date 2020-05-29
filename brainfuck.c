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
void zerosc(char*arr,size_t len);
void print_exec_state();

/*program state:  execute / finished ok / finished with error */
char execution_state=EXEC_CONTINUE;

/*-v sets verbose to 1 for detailed output about execution */
char verbose=0;


size_t block_size=1024;  /*array extension step*/


char *code=NULL;         /* "code segment" */
size_t code_size=2048;
char*cmd=NULL;           /*current instruction pointer*/



char**links=NULL;
int lp=0;

int ptr=0;
char dirty_ptr=0;
struct block*blocks;
struct block*block_cursor;
int*block_mem=0;

struct block{
	int id;
	int*mem;
	struct block*next;
	struct block*prev;
};

void printstate(){printf("cmd:%ld\tptr:%d\n",(cmd-code),(int)ptr);}

struct block*new_block(int id);


int block_of_addr(int memptr){
	return memptr/block_size;
}

void set_cursor(struct block*b){
	block_cursor=b;
	block_mem=b->mem;
	dirty_ptr=0;
}



void zeros(int*ar,unsigned int len){
	unsigned int i=0;
	while(i<len)
		ar[i++]=0;
}

struct block*new_block(int id){
	struct block*res=malloc(sizeof(struct block));
	if(NULL==res)return NULL;

	res->mem=malloc(sizeof(int)*block_size);
	if(NULL==res->mem){
		free(res);
		return NULL;
	}

	zeros(res->mem,block_size);
	res->prev = res->next = NULL;
	res->id=id;
	return res;
}


struct block*insert_new_block(int blockid,struct block*prev,struct block*next){
	struct block* newblock=new_block(blockid);
	newblock->prev=prev;
	newblock->next=next;
	if(prev)prev->next=newblock;
	if(next)next->prev=newblock;
	return newblock;
}


struct block*get_block(int block_id){
	//lookup a bigger blockid?go up the ll
	if(block_id > block_cursor->id){
		//seek while next exists
		while(block_cursor->next){
			//cases:  id< nextid, id==nextid, id>nextid:
			//id < nextid? create a new block between curr and next 
			//             and return it
			if(block_id < block_cursor->next->id)
				return insert_new_block(block_id,block_cursor,block_cursor->next);
			//id==nextid? return nextblock
			if(block_id == block_cursor->next->id)
				return block_cursor->next;
			//id>nextid? next
			block_cursor=block_cursor->next;
		}
		//reached end without finding?add and return it
		return insert_new_block(block_id,block_cursor,NULL);
	}

	//lookup a smaller blockid?go down the ll
	if(block_id < block_cursor->id){
		//seek while next exists
		while(block_cursor->prev){
			if(block_id > block_cursor->prev->id)
				return insert_new_block(block_id, block_cursor->prev, block_cursor);
			//id==nextid? return prevblock
			if(block_id == block_cursor->prev->id)
				return block_cursor->prev;
			//id< previd? next
			block_cursor=block_cursor->prev;
		}
		//reached end without finding?add and return it
		return insert_new_block(block_id,block_cursor,NULL);
	}
	//id==cursor.id ? return it
	return block_cursor;
}


void verify_clean(){
	if(dirty_ptr){
		set_cursor(get_block(block_of_addr(ptr)));
	}
}




/*
*   allocate & init code/data arrays
*   and anything else that runs once at start of the interpreter
*/
void initialize(int argc,char**argv){
    
    blocks=new_block(0);
	set_cursor(blocks);
	ptr=0;
    cmd=code;
    code =malloc(code_size);
    links=malloc(sizeof(char*)*code_size);

    if( 2==argc && '-'==argv[1][0] && 'v'==argv[1][1] ){
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
        /*pointers point to start of arrays*/
        ptr=0;
        cmd=code;

        /*read bf program from input*/
        printf("\nready\n");

        read_program();
        
        lp=link_brackets(code,links);

        /*execute program stored in code array*/
        execute();
        printf("\nexecution complete\n");

        /*wipe code to prepare for next line*/
        zerosc(code,code_size);
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

    while(EXEC_CONTINUE == execution_state){
      //  printstate();
        exec_cmd();
    }
    return execution_state;
}




/*
*   execute a single instruction at *cmd
*   update and return execution state
*/
int exec_cmd(){
   // putchar(*cmd);
  //  putchar('\n');
    switch(*cmd){
        /*cmd points to instruction? 
          exec accordingly*/
        case '>': exec_mem_right(); break;
        case '<': exec_mem_left();  break;
        case '+': exec_inc_mem();   break;
        case '-': exec_dec_mem();   break;
        case '.': exec_putchar();   break;
        case ',': exec_getchar();   break;
        case ':': exec_putnum();    break;
        case '[': exec_from();      break;
        case ']': exec_to();        break;
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
void zerosc(char*arr,size_t len){
    while(len-->0){
        arr[len]=0;
    }
}



/*move data pointer right */
int exec_mem_right(){
    ++ ptr;
    dirty_ptr=1;

    return EXEC_CONTINUE;           
}

/*move data pointer left*/
int exec_mem_left(){
    -- ptr;
    dirty_ptr=1;

    return EXEC_CONTINUE;
}

/*increase value of selected mem.pos*/
int exec_inc_mem(){
	verify_clean();
	block_mem[ ptr%block_size ]++;
    return EXEC_CONTINUE;
}

/*decrease value of selected mem.pos*/
int exec_dec_mem(){
    verify_clean();
	block_mem[ ptr%block_size ]--;
    return EXEC_CONTINUE;
}


/*curr mem <- getchar*/
int exec_getchar(){
    verify_clean();
    block_mem[ ptr%block_size ]=getchar();
    return EXEC_CONTINUE;
}

/*putchar <- curr mem value*/
int exec_putchar(){
    verify_clean();
    putchar(block_mem[ ptr%block_size ]);
    return EXEC_CONTINUE;
}

/*print mem value as a number*/
int exec_putnum(){
    verify_clean();
    printf("%d ",block_mem[ ptr%block_size ]);
    return EXEC_CONTINUE;
}


/*skip forward to closing bracket if ptr value=zero */
int exec_from(){
    verify_clean();
    
    if( 0==block_mem[ ptr%block_size ]){
        cmd=seek_closing_bracket(cmd);
        if(NULL==cmd)
            return EXEC_STOPPED_ERR;
    }
    return EXEC_CONTINUE;
}


/*return back to opening bracket if ptr value=nonzero */
int exec_to(){
    verify_clean();
    
    if( 0 != block_mem[ ptr%block_size ] ){
        cmd=seek_opening_bracket(cmd);
        if(NULL==cmd)
            return EXEC_STOPPED_ERR;
    }
    
    return EXEC_CONTINUE;
}


/*  
 *  for each '[' in code,place its adress in links[i]
 *  for each ']' in code, place its address in links[i+1],
 *   where links[i] is the matching opening bracket
 *
 * */
int link_brackets(char*code,char**links){
    int buffsize=2048;
    char** stack=malloc(sizeof(char*)*buffsize);
    int sp=0;
    int stack_size=0;
    int i=0;
    int lp=0;

    while(EOF != code[i]){
        switch(code[i]){
            case '[':
		if(sp>=buffsize){
			buffsize+=2048;
			stack=realloc(stack,buffsize);
		}
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
    if(stack)free(stack);
    return lp;
}
