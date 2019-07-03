#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#define ERR_CODE_TOO_SHORT "error:File too short!\n"
#define ERR_FILE_NOT_FOUND "error:File not found!\n"
#define ERR_FILE_NOT_FIT "error:This file can not run on the lap platform!\n"
#define ERR_OUT_OF_MEM "error:Out of memory!\n"
#define ERR_DIV_ZERO "error:Div 0!\n"
#define ERR_INIT_VM_FAIL "error:VM Initialize failed!\n"

#define MAX_STACK_SIZE 20
#define MAX_CALL_STACK_DEPTH 256

#define GLOBAL_REG_NUM 100

#define version 0


/*enum Ins{
null,add,SUB,mul,div,mod,LT,EQ,GT,NOTLT,NOTGT,NOTEQ,NOT,OR,AND,mov,
set_var,true_jump,jmp,GOTO,RETURN,set_arr,set_str,POP,PUSH,PRINT,
repeat,dispose,save_local_var,save_global_var,load_local_var,load_global_var,
load_const,get_arg,shut_down,push_str
};*/

const char *header="lapl";

typedef struct{
	void **regs,**local_pool;
	int *pool_type,*reg_type,*pool_is_ref,*reg_is_ref,pool_size,reg_size;
}Pool;

typedef struct{
	int pc,*pc_stack,*reg_id_stack,const_size,
	pool_stack_size,*stack_type,*const_type,ins_max,stack_size,on_stack,reg_size;
	void **stack,**const_pool;
	Pool **pool_stack;
	char** ins;
}LapVM;

typedef struct{
	char* str;
	int length;
}String;

bool is_err=0;

long long quickpower(long long x,long long y)
{
    long long ans=1,cnt=x;
    while(y)
    {
        if(y&1)
        {
            ans*=cnt;
        }
        cnt*=cnt;
        y>>=1;
    }
    return ans;
}

int htoi(char *s,int len){
	int temp=0;
	for(int i=0;i<len;i++){
		if(s[i]<='9'){
			temp+=quickpower(16,len-i-1)*(s[i]-'0');
		}
		else{
			temp+=quickpower(16,len-i-1)*(s[i]-'a'+10);
		}
	}
	return temp;
}

void debug_log(const char* s){
	FILE *fp=fopen(".\\log.txt","a+");
	fputs(s,fp);
	fclose(fp);
}

Pool* Initialize_Pool(int pool_size,int reg_size){
	Pool* temp=(Pool*)malloc(sizeof(Pool));
	temp->local_pool=(void**)malloc(sizeof(void*[pool_size]));
	temp->pool_is_ref=(int*)malloc(sizeof(int[pool_size]));
	temp->reg_is_ref=(int*)malloc(sizeof(int[reg_size]));
	for(int i=0;i<pool_size;i++){
		temp->local_pool[i]=NULL;
		temp->pool_is_ref[i]=0;
	}
	temp->regs=(void**)malloc(sizeof(void*[reg_size]));
	for(int i=0;i<reg_size;i++){
		temp->regs[i]=NULL;
		temp->reg_is_ref[i]=0;
	}
	temp->pool_type=(int*)malloc(sizeof(int[pool_size]));
	temp->reg_type=(int*)malloc(sizeof(int[reg_size]));
	temp->pool_size=pool_size;
	temp->reg_size=reg_size;
	return temp;
}

void free_str(void** data){
	String *on=(String*)*data;
	free(on->str);
	free(on);
}

void free_data(void** data,int type){
	switch(type){
		case 0:free((int*)*data);break;
		case 1:free_str(data);break;
		case 2:free((float*)*data);break;
		case 3:free((double*)*data);break;
		case 4:free((int*)*data);break;//bool
	}
	*data=NULL;
}

int get_len(char s){
	if(s=='0') return 7;
	if(s=='2') return 8;
	if(s=='3') return 8;
	if(s=='4') return 1;
	return 0;
}

void* Initialize_Var(char t){
	void *temp=NULL;
	size_t size;
	switch(t){
		case 0:size=sizeof(int);break;
		case 1:size=sizeof(String);break;
		case 2:size=sizeof(float);break;
		case 3:size=sizeof(double);break;
		case 4:size=sizeof(int);break;
		default:
		size=0;
	}
	temp=malloc(size);
	if(temp==NULL){
		is_err=1;
	}
	return temp;
}

void set_var_mem(void** onset,void* on,char t){
	if(*onset==NULL&&t<5){
		*onset=Initialize_Var(t);
		if(*onset==NULL){
			return;
		}
	}
	switch(t){
		case 0:*(int*)(*onset)=*(int*)on;break;
		case 1:*(String*)(*onset)=*(String*)on;break;
		case 2:*(float*)(*onset)=*(float*)on;break;
		case 3:*(double*)(*onset)=*(double*)on;break;
		case 4:*(int*)(*onset)=*(int*)on;break;
		default:
		*onset=on;//之后考虑内存泄露问题
	}
}

char* substr(char** on,int s,int e){
	int max=e-s;
	char* temp=(char*)malloc(sizeof(char[max]));
	for(int i=0;i<max;i++){
		temp[i]=(*on)[s];
		s++;
	}
	return temp;
}

void* Parse_data(FILE *fp,char onget){
	char *onstr=NULL,*s=(char*)malloc(sizeof(char));
	void* temp=NULL;
	int len,is_free=0;
	switch(onget){
		case '1':
		onstr=(char*)malloc(sizeof(char[3]));
		for(int i=0;i<3;i++){
			onget=fgetc(fp);
			if(onget==EOF){
				break;
			}
			if(onget==' '||onget=='\n'){
				i--;
				continue;
			}
			onstr[i]=onget;
		}
		len=htoi(onstr,3);
		free(s);
		s=(char*)malloc(sizeof(char[len]));
		temp=malloc(sizeof(String));
		((String*)temp)->length=len;
		len*=2;
		for(int i=0;i<len;i+=2){
			onget=fgetc(fp);
			if(onget==EOF){
				break;
			}
			if(onget=='\n'||onget==' '){
				i-=2;
				continue;
			}
			onstr[0]=onget;
			onget=fgetc(fp);
			if(onget==EOF){
				break;
			}
			if(onget=='\n'||onget==' '){
				i-=2;
				continue;
			}
			onstr[1]=onget;
			s[i/2]=htoi(onstr,2);
		}
		((String*)temp)->str=s;
		break;
		default:
		is_free=1;
		len=get_len(onget);
		onstr=(char*)malloc(sizeof(char[len]));
		*s=onget;
		for(int i=0;i<len;i++){
			onget=fgetc(fp);
			if(onget==EOF){
				break;
			}
			if(onget=='\n'||onget==' '){
				i--;
				continue;
			}
			onstr[i]=onget;
		}
		if(*s=='0'){
			temp=(int*)malloc(sizeof(int));
			*(int*)temp=htoi(onstr,7);
		}
		else if(*s=='2'){
			temp=(float*)malloc(sizeof(float));
			free(s);
			s=(char*)malloc(sizeof(char[7]));
			onget=onstr[0];
			s=substr(&onstr,1,8);
			*(float*)temp=(float)htoi(s,7)/quickpower(10,htoi(onstr,1));
		}
		else if(*s=='3'){
			temp=(double*)malloc(sizeof(double));
			free(s);
			s=(char*)malloc(sizeof(char[7]));
			onget=onstr[0];
			s=substr(&onstr,1,8);
			*(double*)temp=(double)htoi(s,7)/quickpower(10,htoi(onstr,1));
		}
		else if(*s=='4'){
			temp=(int*)malloc(sizeof(int));
			*(int*)temp=onstr[0]-'0';
		}
	}
	if(is_free){
		free(s);
	}
	free(onstr);
	return temp;
}

void free_Pool(Pool **pool){
	Pool *on=*pool;
	for(int i=0;i<on->reg_size;i++){
		if(on->regs[i]!=NULL&&!on->reg_is_ref[i]){
			free_data(&on->regs[i],on->reg_type[i]);
		}
	}
	free(on->regs);
	//free(on->reg_type);
	for(int i=0;i<on->pool_size;i++){
		if(on->local_pool[i]!=NULL&&!on->pool_is_ref[i]){
			free_data(&on->local_pool[i],on->pool_type[i]);
		}
	}
	free(on->local_pool);
	free(on->pool_type);
	free(on->reg_is_ref);
	free(on->pool_is_ref);
	free(on);
}

void free_VM(LapVM **vm){
	LapVM *on=*vm;
	//free(on->pool_stack[0]->reg_type);
	free_Pool(&on->pool_stack[0]);
	free(on->pool_stack);
	for(int i=0;i<on->ins_max;i++){
		free(on->ins[i]);
	}
	free(on->ins);
	for(int i=0;i<on->const_size;i++){
		if(on->const_pool[i]!=NULL){
			free_data(&on->const_pool[i],on->const_type[i]);			
		}
	}
	free(on->const_type);
	free(on->const_pool);
	free(on);
}


int strcompare(const char* s1,const char* s2){
	int i=0,temp=1;
	while(s1[i]!=0){
		if(s1[i]==0||s2[i]==0){
			break;
		}
		if(s1[i]!=s2[i]){
			temp=0;
			break;
		}
		i++;
	}
	return temp;
}

LapVM* Initialize_VM(char path[]){
	FILE *fp=fopen(path,"r");
	if(fp==NULL){
		printf(ERR_FILE_NOT_FOUND);
		return NULL;
	}
	char *magic_num=(char*)malloc(sizeof(char[26])),onget,*onstr=NULL,**ins=NULL;
	int is_ok=1,reg,global,pool,pc=0,count,*const_type=NULL;
	void **const_pool=NULL;
	for(int i=0;i<22;i++){
		onget=fgetc(fp);
		if(onget==EOF){
			is_ok=0;
			break;
		}
		if(onget==' '){
			i--;
			continue;
		}
		magic_num[i]=onget;
	}
	if(!is_ok){
		printf(ERR_CODE_TOO_SHORT);
		return NULL;
	}
	onstr=substr(&magic_num,0,4);
	if(!strcompare(onstr,header)){
		printf(ERR_FILE_NOT_FIT);
		return NULL;
	}
	free(onstr);
	onstr=substr(&magic_num,8,12);
	is_ok=htoi(onstr,4);
	if(is_ok>version){
		printf("error：LapVM version is too low Current:%d|Code:%d\n",version,is_ok);
		return NULL;
	}
	free(onstr);
	onstr=substr(&magic_num,12,14);
	reg=htoi(onstr,2);
	free(onstr);
	onstr=substr(&magic_num,14,18);
	pool=htoi(onstr,4);
	free(onstr);
	onstr=substr(&magic_num,18,22);
	global=htoi(onstr,4);
	free(onstr);
	free(magic_num);
	const_pool=(void**)malloc(sizeof(void*[pool]));
	const_type=(int*)malloc(sizeof(int[pool]));
	for(int i=0;i<pool;i++){
		onget=fgetc(fp);
		if(onget==EOF){
			break;
		}
		if(onget==' '||onget=='\n'){
			continue;
		}
		const_pool[i]=Parse_data(fp,onget);
		const_type[i]=onget-'0';
	}
	magic_num=(char*)malloc(sizeof(char[8]));
	count=0;
	ins=(char**)malloc(sizeof(char*[40]));
	int up=38;
	char** getmem,*on_ins;
	while(onget!=EOF){
		onget=fgetc(fp);
		if(onget==EOF){
			break;
		}
		if(onget==' '||onget=='\n'){
			continue;
		}
		if(count<7){
			magic_num[count++]=onget;
		}
		else{
			if(pc>up){
				up+=40;
				getmem=(char**)realloc(ins,sizeof(char*[up]));
				if(getmem==NULL){
					printf(ERR_OUT_OF_MEM);
					is_err=1;
					break;
				}
				ins=getmem;
			}
			on_ins=(char*)malloc(sizeof(char[4]));
			if(on_ins==NULL){
				printf(ERR_OUT_OF_MEM);
				is_err=1;
				break;
			}
			ins[pc]=on_ins;
			count=0;
			onstr=substr(&magic_num,0,2);
			ins[pc][0]=htoi(onstr,2);
			free(onstr);
			onstr=substr(&magic_num,2,4);
			ins[pc][1]=htoi(onstr,2);
			free(onstr);
			onstr=substr(&magic_num,4,6);
			ins[pc][2]=htoi(onstr,2);
			free(onstr);
			onstr=substr(&magic_num,6,8);
			ins[pc][3]=htoi(onstr,2);
			free(onstr);
			pc++;
		}
	}
	fclose(fp);
	if(is_err){
		pc--;//wait to make sure it is right
		for(int i=0;i<pc;i++){
			free(ins[i]);
		}
		free(ins);
		return NULL;
	}
	LapVM *temp=(LapVM*)malloc(sizeof(LapVM));
	temp->pc=0;
	temp->ins_max=pc;
	temp->pool_stack_size=1;
	temp->const_size=pool;
	temp->ins=ins;
	temp->const_pool=const_pool;
	temp->const_type=const_type;
	temp->stack_size=0;
	temp->stack=NULL;
	temp->pc_stack=NULL;
	temp->pool_stack=(Pool**)malloc(sizeof(Pool*[1]));
	temp->pool_stack[0]=Initialize_Pool(0,reg);
	temp->pool_stack[0]->pool_size=global;
	temp->reg_id_stack=NULL;
	temp->on_stack=0;
	temp->reg_size=reg;
	return temp;
}

void Sadd(String *s1,String *s2){
	int max=s1->length+s2->length;
	char* onstr=(char*)realloc(s1->str,sizeof(char[max]));
	if(onstr==NULL){
		printf(ERR_OUT_OF_MEM);
		return;
	}
	strcat(s1->str,s2->str);
}
void add (LapVM* env){
	char* on_ins=env->ins[env->pc];
	int op1=on_ins[1],op2=on_ins[2];
	Pool *pool=env->pool_stack[env->on_stack];
	char type1=pool->reg_type[op1],type2=pool->reg_type[op2];
	if(type1==1){
		Sadd((String*)pool->regs[op1],(String*)pool->regs[op2]);
	}
	else if(type1==0&&type1==type2){
		*(int*)pool->regs[op1]+=*(int*)pool->regs[op2];
	}
	else if(type1==2){
		*(float*)pool->regs[op1]+=*(float*)pool->regs[op2];
	}
	else if(type1==3){
		*(double*)pool->regs[op1]+=*(double*)pool->regs[op2];
	}
	free_data(&pool->regs[op2],pool->reg_type[op2]);
	pool->reg_is_ref[op2]=1;
}
void SUB (LapVM* env){
	char* on_ins=env->ins[env->pc];
	int op1=on_ins[1],op2=on_ins[2];
	Pool *pool=env->pool_stack[env->on_stack];
	char type1=pool->reg_type[op1],type2=pool->reg_type[op2];
	if(type1==0&&type1==type2){
		*(int*)pool->regs[op1]-=*(int*)pool->regs[op2];
	}
	else if(type1==2){
		*(float*)pool->regs[op1]-=*(float*)pool->regs[op2];
	}
	else if(type1==3){
		*(double*)pool->regs[op1]-=*(double*)pool->regs[op2];
	}
	free_data(&pool->regs[op2],pool->reg_type[op2]);
	pool->reg_is_ref[op2]=1;
}
void mul (LapVM* env){
	char* on_ins=env->ins[env->pc];
	int op1=on_ins[1],op2=on_ins[2];
	Pool *pool=env->pool_stack[env->on_stack];
	char type1=pool->reg_type[op1],type2=pool->reg_type[op2];
	if(type1==0&&type1==type2){
		*(int*)pool->regs[op1]*=*(int*)pool->regs[op2];
	}
	else if(type1==2){
		*(float*)pool->regs[op1]*=*(float*)pool->regs[op2];
	}
	else if(type1==3){
		*(double*)pool->regs[op1]*=*(double*)pool->regs[op2];
	}
	free_data(&pool->regs[op2],pool->reg_type[op2]);
	pool->reg_is_ref[op2]=1;
}
void div (LapVM* env){
	char* on_ins=env->ins[env->pc];
	int op1=on_ins[1],op2=on_ins[2];
	Pool *pool=env->pool_stack[env->on_stack];
	char type1=pool->reg_type[op1],type2=pool->reg_type[op2];
	if(type1==0&&type1==type2){
		if(*(int*)pool->regs[op2]==0){
			printf(ERR_DIV_ZERO);
			is_err=1;
			return;
		}
		*(int*)pool->regs[op1]/=*(int*)pool->regs[op2];
	}
	else if(type1==2){
		if(*(float*)pool->regs[op2]==0){
			printf(ERR_DIV_ZERO);
			is_err=1;
			return;
		}
		*(float*)pool->regs[op1]/=*(float*)pool->regs[op2];
	}
	else if(type1==3){
		if(*(double*)pool->regs[op2]==0){
			printf(ERR_DIV_ZERO);
			is_err=1;
			return;
		}
		*(double*)pool->regs[op1]/=*(double*)pool->regs[op2];
	}
	free_data(&pool->regs[op2],pool->reg_type[op2]);
	pool->reg_is_ref[op2]=1;
}
void mod (LapVM* env){
	char* on_ins=env->ins[env->pc];
	int op1=on_ins[1],op2=on_ins[2];
	double d1,d2;
	float f1,f2;
	Pool *pool=env->pool_stack[env->on_stack];
	char type1=pool->reg_type[op1],type2=pool->reg_type[op2];
	if(type1==0&&type1==type2){
		if(*(int*)pool->regs[op2]==0){
			printf(ERR_DIV_ZERO);
			is_err=1;
			return;
		}
		*(int*)pool->regs[op1]%=*(int*)pool->regs[op2];
	}
	else if(type1==2){
		f1=*(float*)pool->regs[op1];
		f2=*(float*)pool->regs[op2];
		if(d2==0){
			printf(ERR_DIV_ZERO);
			is_err=1;
			return;
		}
		*(float*)pool->regs[op1]=f1-(int)(f1/f2)*f2;
	}
	else if(type1==3){
		d1=*(double*)pool->regs[op1];
		d2=*(double*)pool->regs[op2];
		if(d2==0){
			printf(ERR_DIV_ZERO);
			is_err=1;
			return;
		}
		*(double*)pool->regs[op1]=d1-(int)(d1/d2)*d2;
	}
	free_data(&pool->regs[op2],pool->reg_type[op2]);
	pool->reg_is_ref[op2]=1;
}
void LT (LapVM* env){
	char* on_ins=env->ins[env->pc];
	int op1=on_ins[1],op2=on_ins[2];
	Pool *pool=env->pool_stack[env->on_stack];
	char type1=pool->reg_type[op1],type2=pool->reg_type[op2];
	if(type1==0&&type1==type2){
		*(int*)pool->regs[op1]=*(int*)pool->regs[op1]<*(int*)pool->regs[op2];
	}
	else if(type1==2){
		*(float*)pool->regs[op1]=*(float*)pool->regs[op1]<*(float*)pool->regs[op2];
	}
	else if(type1==3){
		*(double*)pool->regs[op1]=*(double*)pool->regs[op1]<*(double*)pool->regs[op2];
	}
	free_data(&pool->regs[op2],pool->reg_type[op2]);
	pool->reg_type[op1]=4;
	pool->reg_is_ref[op2]=1;
}
void EQ (LapVM* env){
	char* on_ins=env->ins[env->pc];
	int op1=on_ins[1],op2=on_ins[2];
	Pool *pool=env->pool_stack[env->on_stack];
	char type1=pool->reg_type[op1],type2=pool->reg_type[op2];
	if(type1==0&&type1==type2){
		*(int*)pool->regs[op1]=*(int*)pool->regs[op1]==*(int*)pool->regs[op2];
	}
	else if(type1==2){
		*(float*)pool->regs[op1]=*(float*)pool->regs[op1]==*(float*)pool->regs[op2];
	}
	else if(type1==3){
		*(double*)pool->regs[op1]=*(double*)pool->regs[op1]==*(double*)pool->regs[op2];
	}
	free_data(&pool->regs[op2],pool->reg_type[op2]);
	pool->reg_type[op1]=4;
	pool->reg_is_ref[op2]=1;
}
void GT (LapVM* env){
	char* on_ins=env->ins[env->pc];
	int op1=on_ins[1],op2=on_ins[2];
	Pool *pool=env->pool_stack[env->on_stack];
	char type1=pool->reg_type[op1],type2=pool->reg_type[op2];
	if(type1==0&&type1==type2){
		*(int*)pool->regs[op1]=*(int*)pool->regs[op1]>*(int*)pool->regs[op2];
	}
	else if(type1==2){
		*(float*)pool->regs[op1]=*(float*)pool->regs[op1]>*(float*)pool->regs[op2];
	}
	else if(type1==3){
		*(double*)pool->regs[op1]=*(double*)pool->regs[op1]>*(double*)pool->regs[op2];
	}
	free_data(&pool->regs[op2],pool->reg_type[op2]);
	pool->reg_type[op1]=4;
	pool->reg_is_ref[op2]=1;
}
void NOT (LapVM* env){
	char* on_ins=env->ins[env->pc];
	int op1=on_ins[1];
	Pool *pool=env->pool_stack[env->on_stack];
	char type1=pool->reg_type[op1];
	if(type1==0||type1==4){
		*(int*)pool->regs[op1]=!*(int*)pool->regs[op1];
	}
	else if(type1==2){
		*(float*)pool->regs[op1]=!*(float*)pool->regs[op1];
	}
	else if(type1==3){
		*(double*)pool->regs[op1]=!*(double*)pool->regs[op1];
	}
}
void NOTLT (LapVM* env){
	LT(env);
	NOT(env);
}
void NOTGT (LapVM* env){
	GT(env);
	NOT(env);
	
}
void NOTEQ (LapVM* env){
	EQ(env);
	NOT(env);
}
void OR (LapVM* env){
	char* on_ins=env->ins[env->pc];
	int op1=on_ins[1],op2=on_ins[2];
	Pool *pool=env->pool_stack[env->on_stack];
	char type1=pool->reg_type[op1],type2=pool->reg_type[op2];
	if(type1==4){
		*(int*)pool->regs[op1]=*(int*)pool->regs[op1]||*(int*)pool->regs[op2];
	}
	else if(type1==2){
		*(float*)pool->regs[op1]=*(float*)pool->regs[op1]||*(float*)pool->regs[op2];
	}
	else if(type1==3){
		*(double*)pool->regs[op1]=*(double*)pool->regs[op1]||*(double*)pool->regs[op2];
	}
	free_data(&pool->regs[op2],pool->reg_type[op2]);
	pool->reg_type[op1]=4;
	pool->reg_is_ref[op2]=1;
}
void AND (LapVM* env){
	char* on_ins=env->ins[env->pc];
	int op1=on_ins[1],op2=on_ins[2];
	Pool *pool=env->pool_stack[env->on_stack];
	char type1=pool->reg_type[op1],type2=pool->reg_type[op2];
	if(type1==4){
		*(int*)pool->regs[op1]=*(int*)pool->regs[op1]&&*(int*)pool->regs[op2];
	}
	else if(type1==2){
		*(float*)pool->regs[op1]=*(float*)pool->regs[op1]&&*(float*)pool->regs[op2];
	}
	else if(type1==3){
		*(double*)pool->regs[op1]=*(double*)pool->regs[op1]&&*(double*)pool->regs[op2];
	}
	free_data(&pool->regs[op2],pool->reg_type[op2]);
	pool->reg_type[op1]=4;
	pool->reg_is_ref[op2]=1;
}
void set_var (LapVM* env){
	
}
void true_jump (LapVM* env){
	
}
void jmp (LapVM* env){
	env->pc=*(int*)env->const_pool[env->ins[env->pc][1]];
}
void GOTO (LapVM* env){//参数个数 寄存器末尾id(+1为常量索引) 最大255个局部量
	char* ins=env->ins[env->pc];
	int *on=NULL,max=ins[1],reg=ins[2];
	env->stack=(void**)malloc(sizeof(void*[max-1]));
	env->stack_type=(int*)malloc(sizeof(int[max-1]));
	env->reg_id_stack=(int*)realloc(env->reg_id_stack,sizeof(int[max-1]));
	on=(int*)realloc(env->pc_stack,sizeof(int[max--]));
	if(on==NULL){
		is_err=1;
		printf(ERR_OUT_OF_MEM);
		return;
	}
	env->pc_stack=on;
	Pool* pool=env->pool_stack[env->on_stack];
	env->pc_stack[env->on_stack]=env->pc;
	env->pc=*(int*)(pool->regs[reg]);
	free_data(&pool->regs[reg--],0);
	for(int i=max;i>-1;i--){//检查编译器的bug
		env->stack[i]=pool->regs[reg];
		env->stack_type[i]=pool->reg_type[reg--];
	}
	env->reg_id_stack[env->on_stack++]=reg;
	env->pool_stack[env->on_stack]=Initialize_Pool(255,env->reg_size);
	pool=env->pool_stack[env->on_stack];
	max++;
	for(int i=0;i<max;i++){
		set_var_mem(&pool->local_pool[i],env->stack[i],env->stack_type[i]);
		pool->pool_type[i]=env->stack_type[i];
	}
	free(env->stack);
	free(env->stack_type);
}
void RETURN (LapVM* env){
	env->pc=env->pc_stack[env->on_stack-1];
	Pool *pool1=env->pool_stack[env->on_stack-1],*pool2=env->pool_stack[env->on_stack--];
	int id=env->reg_id_stack[env->on_stack]+1;
	pool1->regs[id]=pool2->regs[0],pool1->reg_type[id]=pool2->reg_type[0];
	pool2->reg_is_ref[0]=1;
	free_Pool(&pool2);
	free(pool2->reg_type);
	free(pool2);
}
void set_arr (LapVM* env){
	
}
void set_str (LapVM* env){
	
}
void POP (LapVM* env){
	
}
void PUSH (LapVM* env){
}
void PRINT (LapVM* env){
	Pool *pool=env->pool_stack[env->on_stack];
	char type=pool->reg_type[0];
	void *value=pool->regs[0];
	switch(type){
		case 0:
		printf("%d\n",*(int*)value);break;
		case 1:printf("%s\n",((String*)value)->str);break;
		case 2:printf("%f\n",*(float*)value);break;
		case 3:printf("%lf\n",*(double*)value);break;
		case 4:
		if(*(int*)value){
			printf("true\n");
		}
		else{
			printf("false\n");
		}
		break;
	}
	free_data(&value,type);
}
void repeat (LapVM* env){
}
void dispose (LapVM* env){
	
}
void save_local_var (LapVM* env){
	
}
void save_global_var (LapVM* env){
	
}
void load_local_var (LapVM* env){
	char* on_ins=env->ins[env->pc];
	int index=on_ins[1],to=on_ins[2];
	Pool *pool=env->pool_stack[env->on_stack];
	char type=pool->pool_type[index];
	if(type<5){
		if(pool->regs[to]!=NULL&&!pool->reg_is_ref[to]){
			free_data(&pool->regs[to],pool->reg_type[to]);
		}
		set_var_mem(&pool->regs[to],pool->local_pool[index],type);
	}
	else{
		pool->regs[to]=pool->local_pool[index];
	}
	pool->reg_type[to]=type;
}
void load_global_var (LapVM* env){
	
}
void load_const (LapVM* env){//索引 寄存器
	char* on_ins=env->ins[env->pc];
	int index=on_ins[1],to=on_ins[2];
	char type=env->const_type[index];
	Pool *pool=env->pool_stack[env->on_stack];
	if(type<5){
		if(pool->regs[to]!=NULL&&!pool->reg_is_ref[to]){
			free_data(&pool->regs[to],pool->reg_type[to]);
		}
		set_var_mem(&pool->regs[to],env->const_pool[index],type);
	}
	else{
		pool->regs[to]=env->const_pool[index];
	}
	pool->reg_type[to]=type;
}
void get_arg (LapVM* env){
	
}
void shut_down (LapVM* env){
	
}
void push_str (LapVM* env){
	
}

void(*ins[35])(LapVM*)={NULL,add,SUB,mul,div,mod,LT,EQ,GT,NOTLT,
NOTGT,NOTEQ,NOT,OR,AND,set_var,true_jump,jmp,GOTO,RETURN,
set_arr,set_str,POP,PUSH,PRINT,repeat,dispose,save_local_var,
save_global_var,load_local_var,load_global_var,load_const,get_arg,shut_down,push_str};

void Run_code(LapVM *env,char* argv[]){
	char on_id;
	int max=env->ins_max;
	for(int i=0;i<max;i++){
		env->pc=i;
		on_id=env->ins[i][0];
		//printf("ins %d\n",on_id);
		if(on_id!=0){
			ins[on_id](env);
			if(is_err){
				break;
			}
			i=env->pc;
		}
	}
}

int main(int argc,char* argv[]){
	LapVM *env=Initialize_VM(argv[1]);
	if(env!=NULL){
		Run_code(env,argv);
		free_VM(&env);
	}
	else{
		printf(ERR_INIT_VM_FAIL);
	}
	system("pause");
}
