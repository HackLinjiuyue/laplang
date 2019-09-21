#ifdef __MINGW32__
	#include<windows.h>
#else
	#include<dlfcn.h>
#endif

#include "CLap.h"

//#include<time.h>

int r_size=0;

typedef struct{
	LapObject*** VarStacks;
	LapObject** Stack;
	LapObject** ConstVars;
	int** Commands;
	char* Onstr;
	LapObject* Argv;
	int Index,ConstNum,*VarNum,*MaxVar,MaxIndex,PC,*PCStack,TruePC,Err,StackPC,MaxStackPC;
}LapState;

long long QuickPower(long long x,long long y)
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

LapObject *args=NULL;

int StrLen(const char* str){
	int i=0;
	while(str[i]!=0){
		++i;
	}
    return i;
}

void ExtendStack(LapState *env){
    if(env->MaxIndex==env->Index){
		env->MaxIndex+=20;
		env->Stack=(LapObject**)realloc(env->Stack,sizeof(LapObject*[env->MaxIndex]));
    }
}

int StartVM(LapState *env);

int GetIns(int*** on,FILE* fp,int ref){
    int TruePC=0,*on_i;
    int MaxPC;
    fread(&MaxPC,sizeof(int),1,fp);
    *on=calloc(MaxPC,sizeof(int*));
    while(1){
    	fread(&MaxPC,sizeof(int),1,fp);
    	if(feof(fp)){
    		break;
    	}
        if(MaxPC>63){
			printf("Err:unrecognized ins %d\n",MaxPC);
			return 0;
        }
        (*on)[TruePC]=calloc(3,sizeof(int));
        on_i=(*on)[TruePC++];
        on_i[0]=MaxPC;
        fread(&MaxPC,sizeof(int),1,fp);
        on_i[1]=MaxPC;
        fread(&MaxPC,sizeof(int),1,fp);
        on_i[2]=MaxPC;
    }
    return TruePC;
}

LapState *InitVM(char* path,LapObject* arg){
	FILE *fp=fopen(path,"rb+");
	if(fp==NULL){
		printf("File %s Not Found\n",path);
		return NULL;
	}
	LapState *temp=(LapState*)malloc(sizeof(LapState));
	temp->Argv=arg;
	temp->Stack=(LapObject**)calloc(20,sizeof(LapObject*));
    temp->VarStacks=(LapObject***)malloc(sizeof(LapObject**[8]));
    temp->VarStacks[0]=(LapObject**)calloc(20,sizeof(LapObject*));
    temp->MaxVar=(int*)malloc(sizeof(int[8]));
    temp->VarNum=(int*)malloc(sizeof(int[8]));
    temp->PCStack=(int*)malloc(sizeof(int[8]));
    temp->StackPC=1;
    temp->MaxStackPC=8;
    int i=0;
	temp->MaxVar[0]=20;
	temp->VarNum[0]=0;
    temp->Index=0;
    temp->PC=0;
    temp->MaxIndex=20;
    temp->Err=0;
    int c_size;
    fread(&c_size,sizeof(int),1,fp);
    temp->ConstVars=(LapObject**)malloc(sizeof(LapObject*[c_size]));
    temp->ConstNum=c_size;
    void *p=NULL;
    int i_temp;
    for(;i<c_size;i++){
    	fread(&i_temp,sizeof(int),1,fp);
		switch(i_temp){
		case 0:
			p=malloc(sizeof(int));
			fread(p,sizeof(int),1,fp);
			temp->ConstVars[i]=CreateObject(0,0,p);
			break;
		case 1:
			p=malloc(sizeof(double));
			fread(p,sizeof(double),1,fp);
			temp->ConstVars[i]=CreateObject(1,0,p);
			break;
		case 2:
            fread(&i_temp,sizeof(int),1,fp);
            p=calloc(i_temp+1,sizeof(char));
            fread(p,sizeof(char),i_temp,fp);
            temp->ConstVars[i]=CreateObject(2,i_temp,p);
			break;
		case 3:
			p=malloc(sizeof(int));
			fread(p,sizeof(int),1,fp);
			temp->ConstVars[i]=CreateObject(3,0,p);
			break;
		}
    }
    temp->TruePC=GetIns(&temp->Commands,fp,0);
    fclose(fp);
	return temp;
}

void DeleteState(LapState *state,int is_main){
	int i=0,max=state->ConstNum;
    for(;i<max;++i){
		FreeObject(state->ConstVars[i]);
    }
    free(state->ConstVars);
    max=state->VarNum[0];
    i=0;
	for(;i<state->VarNum[0];++i){
		if(state->VarStacks[0][i]!=NULL){
			FreeObject(state->VarStacks[0][i]);
		}
	}
	free(state->VarStacks[0]);
    i=0,max=state->TruePC;
    for(;i<max;++i){
        free(state->Commands[i]);
    }
    free(state->VarStacks);
    free(state->VarNum);
    free(state->MaxVar);
    free(state->PCStack);
    free(state->Commands);
    free(state->Stack);
    if(!is_main){
    	FreeObject(state->Argv);
    }
    free(state);
}

int StringCmp(const char* str1,const char* str2){
	int i=0;
	int l1=StrLen(str1);
	int l2=StrLen(str2);
	if(l1!=l2){
		return 0;
	}
	if(!l2&&!l1){
		return 1;
	}
	while(1){
		if(!str1[i]||!str2[i]){
			return 1;
		}
		if(str1[i]!=str2[i]){
			return 0;
		}
		++i;
	}
	return 1;
}

double* ParseFloat(char* str){
    int i=0,sign=1,l=0;
	double *temp=(double*)malloc(sizeof(double));
    *temp=0;
	int max=StrLen(str);
    if(str[0]=='-'){
		i=1,sign=-1;
    }
    while(str[l]!='.'){
		++l;
    }
    --l;
    while(str[i]!='.'){
		*temp+=(str[i]-'0')*QuickPower(10,l-i);
		++i;
    }
    ++i;
    l=1;
    while(str[i]>0){
		*temp+=(str[i]-'0')/QuickPower(10,l);
		++l;
		++i;
    }
    *temp*=sign;
    return temp;
}

void PushConst(LapState *env){
    int* ins=env->Commands[env->PC];
    ExtendStack(env);
    env->Stack[env->Index]=CreateObjectFromObject(env->ConstVars[ins[1]]);
    ++env->Index;
}

void PushVarLocal(LapState *env){
    ExtendStack(env);
    env->Stack[env->Index++]=CreateObjectFromObject(env->VarStacks[env->StackPC-1][env->Commands[env->PC][1]]);
}

void PushVarGlobal(LapState *env){
    ExtendStack(env);
    env->Stack[env->Index++]=CreateObjectFromObject(env->VarStacks[0][env->Commands[env->PC][1]]);
}

void Pop(LapState *env){
	if(env->Index>0){
		FreeObject(env->Stack[--env->Index]);
		env->Stack[env->Index]=NULL;
	}
}

void Add(LapState *env){
	LapObject *op2=env->Stack[--env->Index];
	LapObject *op1=env->Stack[--env->Index];
	LapObject *temp=NULL;
	int type=op1->Type;
	switch(type){
	case 0:
		temp=CreateObject(0,0,malloc(sizeof(int)));
		*(int*)temp->Value=*(int*)op1->Value+*(int*)op2->Value;
		break;
	case 1:
		temp=CreateObject(1,0,malloc(sizeof(double)));
		*(double*)temp->Value=*(double*)op1->Value+*(double*)op2->Value;
		break;
	case 2:
		temp=CreateObject(2,op1->Size+op2->Size,ConcatStr((char*)op1->Value,(char*)op2->Value,op1->Size,op2->Size));
		break;
	}
	env->Stack[env->Index++]=temp;
	FreeObject(op1);
	FreeObject(op2);
}

void Sub(LapState *env){
	LapObject *op2=env->Stack[--env->Index];
	LapObject *op1=env->Stack[--env->Index];
	LapObject *temp=NULL;
	int type=op1->Type;
	switch(type){
	case 0:
		temp=CreateObject(0,0,malloc(sizeof(int)));
		*(int*)temp->Value=*(int*)op1->Value-*(int*)op2->Value;
		break;
	case 1:
		temp=CreateObject(1,0,malloc(sizeof(double)));
		*(double*)temp->Value=*(double*)op1->Value-*(double*)op2->Value;
		break;
	}
	env->Stack[env->Index++]=temp;
	FreeObject(op1);
	FreeObject(op2);
}

void Mul(LapState *env){
	LapObject *op2=env->Stack[--env->Index];
	LapObject *op1=env->Stack[--env->Index];
	LapObject *temp=NULL;
	int type=op1->Type;
	switch(type){
	case 0:
		temp=CreateObject(0,0,malloc(sizeof(int)));
		*(int*)temp->Value=*(int*)op1->Value**(int*)op2->Value;
		break;
	case 1:
		temp=CreateObject(1,0,malloc(sizeof(double)));
		*(double*)temp->Value=*(double*)op1->Value**(double*)op2->Value;
		break;
	}
	env->Stack[env->Index++]=temp;
	FreeObject(op1);
	FreeObject(op2);
}

void Div(LapState *env){
	LapObject *op2=env->Stack[--env->Index];
	LapObject *op1=env->Stack[--env->Index];
	LapObject *temp=NULL;
	int type=op1->Type;
	if(op2->Type){
		if(!*(double*)op2->Value){
			env->Err=1;
			return;
		}
	}
	else{
		if(!*(int*)op2->Value){
			env->Err=1;
			return;
		}
	}
	switch(type){
	case 0:
		temp=CreateObject(0,0,malloc(sizeof(int)));
		*(int*)temp->Value=*(int*)op1->Value% *(int*)op2->Value;
		break;
	case 1:
		temp=CreateObject(1,0,malloc(sizeof(double)));
		*(double*)temp->Value=*(double*)op1->Value/ *(double*)op2->Value;
		break;
	}
	env->Stack[env->Index++]=temp;
	FreeObject(op1);
	FreeObject(op2);
}

void Mod(LapState *env){
	LapObject *op2=env->Stack[--env->Index];
	LapObject *op1=env->Stack[--env->Index];
	LapObject *temp=NULL;
	int type=op1->Type;
	double d1,d2;
	if(op2->Type){
		if(!*(double*)op2->Value){
			env->Err=1;
			return;
		}
	}
	else{
		if(!*(int*)op2->Value){
			env->Err=1;
			return;
		}
	}
	switch(type){
	case 0:
		temp=CreateObject(0,0,malloc(sizeof(int)));
		*(int*)temp->Value=*(int*)op1->Value/ *(int*)op2->Value;
		break;
	case 1:
		temp=CreateObject(1,0,malloc(sizeof(double)));
		d1=(*(double*)op1->Value);
		d2=(*(double*)op2->Value);
		*((double*)temp->Value)=d1-(int)(d1/d2)*d2;
		break;
	}
	env->Stack[env->Index++]=temp;
	FreeObject(op1);
	FreeObject(op2);
}

void MoveLeft(LapState* env){
	LapObject *op2=env->Stack[--env->Index];
	LapObject *op1=env->Stack[--env->Index];
	LapObject *temp=CreateObject(0,0,NULL);
	(*(int*)temp->Value)=*(int*)op1->Value<<*(int*)op2->Value;
	env->Stack[env->Index++]=temp;
	FreeObject(op1);
	FreeObject(op2);
}

void MoveRight(LapState* env){
	LapObject *op2=env->Stack[--env->Index];
	LapObject *op1=env->Stack[--env->Index];
	LapObject *temp=CreateObject(0,0,NULL);
	(*(int*)temp->Value)=*(int*)op1->Value>>*(int*)op2->Value;
	env->Stack[env->Index++]=temp;
	FreeObject(op1);
	FreeObject(op2);
}

void BitXor(LapState* env){
	LapObject *op2=env->Stack[--env->Index];
	LapObject *op1=env->Stack[--env->Index];
	LapObject *temp=CreateObject(0,0,NULL);
	(*(int*)temp->Value)=*(int*)op1->Value^*(int*)op2->Value;
	env->Stack[env->Index++]=temp;
	FreeObject(op1);
	FreeObject(op2);
}

void BitAnd(LapState* env){
	LapObject *op2=env->Stack[--env->Index];
	LapObject *op1=env->Stack[--env->Index];
	LapObject *temp=CreateObject(0,0,NULL);
	(*(int*)temp->Value)=*(int*)op1->Value&*(int*)op2->Value;
	env->Stack[env->Index++]=temp;
	FreeObject(op1);
	FreeObject(op2);
}

void BitOr(LapState* env){
	LapObject *op2=env->Stack[--env->Index];
	LapObject *op1=env->Stack[--env->Index];
	LapObject *temp=CreateObject(0,0,NULL);
	(*(int*)temp->Value)=*(int*)op1->Value|*(int*)op2->Value;
	env->Stack[env->Index++]=temp;
	FreeObject(op1);
	FreeObject(op2);
}

void Or(LapState* env){
	LapObject *op2=env->Stack[--env->Index];
	LapObject *op1=env->Stack[--env->Index];
	LapObject *temp=CreateObject(Lbool,0,NULL);
	(*(int*)temp->Value)=*(int*)op1->Value||*(int*)op2->Value;
	env->Stack[env->Index++]=temp;
	FreeObject(op1);
	FreeObject(op2);
}

void And(LapState* env){
	LapObject *op2=env->Stack[--env->Index];
	LapObject *op1=env->Stack[--env->Index];
	LapObject *temp=CreateObject(Lbool,0,NULL);
	(*(int*)temp->Value)=*(int*)op1->Value&&*(int*)op2->Value;
	env->Stack[env->Index++]=temp;
	FreeObject(op1);
	FreeObject(op2);
}

void Bigger(LapState *env){
	LapObject *op2=env->Stack[--env->Index];
	LapObject *op1=env->Stack[--env->Index];
	LapObject *temp=CreateObject(Lbool,0,malloc(sizeof(int)));
	int type=op1->Type;
	switch(type){
	case 0:
		*(int*)temp->Value=*(int*)op1->Value>*(int*)op2->Value;
		break;
	case 1:
		*(int*)temp->Value=*(double*)op1->Value>*(double*)op2->Value;
		break;
	}
	env->Stack[env->Index++]=temp;
	FreeObject(op1);
	FreeObject(op2);
}

void Smaller(LapState *env){
	LapObject *op2=env->Stack[--env->Index];
	LapObject *op1=env->Stack[--env->Index];
	LapObject *temp=CreateObject(Lbool,0,malloc(sizeof(int)));
	int type=op1->Type;
	switch(type){
	case 0:
		*(int*)temp->Value=*(int*)op1->Value<*(int*)op2->Value;
		break;
	case 1:
		*(int*)temp->Value=*(double*)op1->Value<*(double*)op2->Value;
		break;
	}
	env->Stack[env->Index++]=temp;
	FreeObject(op1);
	FreeObject(op2);
}

void PushFile(LapState* env){
	LapObject *op2=env->Stack[--env->Index];
	LapObject *op1=env->Stack[--env->Index];
	LapObject *temp=CreateObject(Lfile,0,fopen(op1->Value,op2->Value));
	env->Stack[env->Index++]=temp;
	FreeObject(op1);
	FreeObject(op2);
}

void Equal(LapState *env){
	LapObject *op2=env->Stack[--env->Index];
	LapObject *op1=env->Stack[--env->Index];
	LapObject *temp=CreateObject(Lbool,0,malloc(sizeof(int)));
	int type=op1->Type;
	switch(type){
	case 0:
		*(int*)temp->Value=*(int*)op1->Value==*(int*)op2->Value;
		break;
	case 1:
		*(int*)temp->Value=*(double*)op1->Value==*(double*)op2->Value;
		break;
	case 2:
		(*(int*)temp->Value)=StringCmp(op1->Value,op2->Value);
		break;
	case 3:
		(*(int*)temp->Value)=*(int*)op1->Value==*(int*)op2->Value;
		break;
	}
	env->Stack[env->Index++]=temp;
	FreeObject(op1);
	FreeObject(op2);
}

void Index(LapState *env){
	LapObject *op2=env->Stack[--env->Index];
	LapObject *op1=env->Stack[--env->Index];
	LapObject *temp=NULL;
	char* onstr=NULL;
	int n=*(int*)op2->Value;
	if(n>op1->Size-1){
		env->Err=2;
		return;
	}
	if(n<0){
		env->Err=9;
		return;
	}
	int type=op1->Type;
	switch(type){
	case 2:
		onstr=(char*)calloc(2,sizeof(char));
		onstr[0]=((char*)op1->Value)[n];
		temp=CreateObject(2,1,onstr);
		break;
	case 4:
		temp=CreateObjectFromObject(op1->Property[n]);
		break;
	}
	env->Stack[env->Index++]=temp;
	FreeObject(op1);
	FreeObject(op2);
}

void Not(LapState *env){
	LapObject *op1=env->Stack[env->Index-1];
	int type=op1->Type;
	switch(type){
	case 0:
		*(int*)op1->Value=!*(int*)op1->Value;
		break;
	case 1:
		*(double*)op1->Value=!*(double*)op1->Value;
		break;
	case 3:
		*(int*)op1->Value=!*(int*)op1->Value;
		break;
	}
}

void Inc(LapState *env){
	++*(int*)env->Stack[env->Index-1]->Ori;
}

void Dec(LapState *env){
	--*(int*)env->Stack[env->Index-1]->Ori;
}

void IsNull(LapState *env){
	LapObject *op1=env->Stack[env->Index-1];
	int *p=malloc(sizeof(int));
	if(op1==NULL){
		*p=1;
	}
	else{
		FreeObject(op1);
		*p=0;
	}
	env->Stack[env->Index-1]=CreateObject(3,0,p);
}

void Ops(LapState *env){
	LapObject *op1=env->Stack[env->Index-1];
	int type=op1->Type;
	switch(type){
	case 0:
		*(int*)op1->Value=-*(int*)op1->Value;
		break;
	case 1:
		*(double*)op1->Value=-*(double*)op1->Value;
		break;
	}
}

void Print(LapState *env){
    PrintData(env->Stack[--env->Index]);
    FreeObject(env->Stack[env->Index]);
    env->Stack[env->Index]=NULL;
}

void GetCommandArg(LapState *env){
    ExtendStack(env);
    env->Stack[env->Index]=CreateObjectFromObject(env->Argv);
    ++env->Index;
}

void StoreVarLocal(LapState *env){
    int* ins=env->Commands[env->PC];
    int i=ins[1],PC=env->StackPC-1;
	if(i>env->MaxVar[PC]-1){
		env->MaxVar[PC]+=20;
		env->VarStacks[PC]=(LapObject**)realloc(env->VarStacks[PC],sizeof(LapObject*[env->MaxVar[PC]+1]));
	}
	++env->VarNum[PC];
	env->VarStacks[PC][i]=CreateObjectFromObject(env->Stack[--env->Index]);
	FreeObject(env->Stack[env->Index]);
	env->Stack[env->Index]=NULL;
}

void StoreVarGlobal(LapState *env){
    int* ins=env->Commands[env->PC];
    int i=ins[1];
	if(i>env->MaxVar[0]-1){
		env->MaxVar[0]+=20;
		env->VarStacks[0]=(LapObject**)realloc(env->VarStacks[0],sizeof(LapObject*[env->MaxVar[0]+1]));
	}
	++env->VarNum[0];
	env->VarStacks[0][i]=CreateObjectFromObject(env->Stack[--env->Index]);
	FreeObject(env->Stack[env->Index]);
	env->Stack[env->Index]=NULL;
}

void SetVarLocal(LapState *env){
    int* ins=env->Commands[env->PC];
    int i=ins[1],PC=env->StackPC-1;
	if(env->VarStacks[PC][i]!=NULL){
		FreeObject(env->VarStacks[PC][i]);
	}
	env->VarStacks[PC][i]=CreateObjectFromObject(env->Stack[--env->Index]);
	FreeObject(env->Stack[env->Index]);
	env->Stack[env->Index]=NULL;
}

void SetVarGlobal(LapState *env){
    int* ins=env->Commands[env->PC];
    int i=ins[1];
	if(env->VarStacks[0][i]!=NULL){
		FreeObject(env->VarStacks[0][i]);
	}
	env->VarStacks[0][i]=CreateObjectFromObject(env->Stack[--env->Index]);
	FreeObject(env->Stack[env->Index]);
	env->Stack[env->Index]=NULL;
}

void True_Jump(LapState *env){
	int *line=NULL,value;
	LapObject *op1=env->Stack[--env->Index];
	if(op1->Type==1){
		value=(int)(*(double*)op1->Value);
	}
	else{
		value=*(int*)op1->Value;
	}
	if(value){
		env->PC=env->Commands[env->PC][1];
	}
	FreeObject(op1);
	env->Stack[env->Index]=NULL;
}

void False_Jump(LapState *env){
	int *line=NULL,value;
	LapObject *op1=env->Stack[--env->Index];
	if(op1->Type==1){
		value=(int)(*(double*)op1->Value);
	}
	else{
		value=*(int*)op1->Value;
	}
	if(!value){
		env->PC=env->Commands[env->PC][1];
	}
	FreeObject(op1);
	env->Stack[env->Index]=NULL;
}

void Goto(LapState *env){
	int* ins=env->Commands[env->PC];
	int n=ins[2],v=0;
	if(n>MAX_ARG_NUM){
		env->Err=3;
		return;
	}
	env->PCStack[env->StackPC]=env->PC;
	++env->StackPC;
	if(env->StackPC>MAX_STACK){
		env->Err=4;
		return;
	}
	if(env->StackPC==env->MaxStackPC){
		int i=env->MaxStackPC-1;
		env->MaxStackPC+=8;
		int max=i+8;
		size_t size=sizeof(int[max]);
		env->VarStacks=(LapObject***)realloc(env->VarStacks,sizeof(LapObject**[max]));
		env->PCStack=(int*)realloc(env->PCStack,size);
		env->VarNum=(int*)realloc(env->VarNum,size);
		env->MaxVar=(int*)realloc(env->MaxVar,size);
		for(;i<max;++i){
			env->VarNum[i]=0;
			env->MaxVar[i]=12;
			env->VarStacks[i]=NULL;
		}
	}
	env->VarStacks[env->StackPC-1]=(LapObject**)calloc(n+1,sizeof(LapObject*));
	env->VarNum[env->StackPC-1]=n;
	env->MaxVar[env->StackPC-1]=n;
	LapObject** var=env->VarStacks[env->StackPC-1];
	v=0;
	env->PC=ins[1];
    for(;v<n;++v){
		--env->Index;
		if(env->Stack[env->Index]!=NULL){
			var[n-v-1]=CreateObjectFromObject(env->Stack[env->Index]);
			FreeObject(env->Stack[env->Index]);
			env->Stack[env->Index]=NULL;
		}
    }
    //printf("%p\n",env->Stack[0]);
}

void Return(LapState *env){
	int PC=--env->StackPC;
	env->PC=env->PCStack[PC];
	int i=0,max=env->VarNum[PC];
	LapObject **var=env->VarStacks[PC];
	for(;i<max;++i){
		if(var[i]!=NULL){
			FreeObject(var[i]);
		}
	}
	free(var);
	env->VarNum[PC]=0;
	//printf("%p\n",env->Stack[0]);
}

void Asc(LapState *env){
	LapObject *obj=env->Stack[env->Index-1];
	int *x=malloc(sizeof(int));
	*x=((char*)obj->Value)[0];
	FreeObject(obj);
	env->Stack[env->Index-1]=CreateObject(0,0,x);
}

void Len(LapState *env){
	LapObject *obj=env->Stack[env->Index-1];
	env->Stack[env->Index-1]=CreateObject(0,0,NULL);
	*(int*)env->Stack[env->Index-1]->Value=obj->Size;
	FreeObject(obj);
}

void Fgetc(LapState *env){//所有文件操作确保至少绑定在一个变量上
	LapObject *obj=env->Stack[env->Index-1];
	char *x=malloc(sizeof(char[2]));
	x[0]=fgetc((FILE*)obj->Value);
	x[1]=0;
	env->Stack[env->Index-1]=CreateObject(2,1,x);
}

void Fwrite(LapState *env){
	LapObject *op3=env->Stack[--env->Index];//v
	LapObject *op1=env->Stack[--env->Index];//f
	int type=op3->Type;
	void* value=op3->Value;
	switch(type){
	case 0:
	fwrite(value,sizeof(int),1,op1->Value);
	break;
	case 1:
	fwrite(value,sizeof(double),1,op1->Value);
	break;
	case 2:
	fwrite(value,sizeof(char),op3->Size,op1->Value);
	break;
	case 3:
	fwrite(value,sizeof(int),1,op1->Value);
	break;
	}
	FreeObject(op1);
	free(op3);
	env->Stack[env->Index]=NULL;
}

void CloseFile(LapState *env){
	LapObject *obj=env->Stack[--env->Index];
	fclose((FILE*)obj->Value);
	free(obj);
	env->Stack[env->Index]=NULL;
}

void PushObj(LapState *env){
    ExtendStack(env);
    env->Stack[env->Index++]=CreateObject(4,env->Commands[env->PC][1],NULL);
}

void SetProperty(LapState *env){//引用设置
	int* ins=env->Commands[env->PC];
	LapObject *op2=env->Stack[--env->Index];
	int i=ins[1];
	LapObject *op1=env->Stack[--env->Index];
	if(op1->Property[i]!=NULL){
		FreeObject(op1->Property[i]);
	}
	op1->Property[i]=CreateObjectFromObject(op2);
	FreeObject(op2);
	env->Stack[env->Index]=NULL;
}

void SetIndex(LapState *env){//引用设置
	LapObject *op3=env->Stack[--env->Index];
	LapObject *op2=env->Stack[--env->Index];
	LapObject *op1=env->Stack[--env->Index];
	int index=*(int*)op2->Value;
	if(index>op1->Size-1){
		env->Err=2;
		return;
	}
	if(op1->Property[index]!=NULL){
		FreeObject(op1->Property[index]);
	}
	op1->Property[index]=CreateObjectFromObject(op3);
	FreeObject(op2);
	FreeObject(op3);
	env->Stack[env->Index]=NULL;
}

void ArrayPush(LapState *env){//配合引用使用
	int p=env->Commands[env->PC][1];
	LapObject *op1=env->Stack[env->Index-p-1],*op2=NULL;
	if(op1->Size+p>=op1->MaxSize){
		op1->MaxSize+=p+4;
		int m=op1->MaxSize,k=op1->Size;
		op1->Property=(LapObject**)realloc(op1->Property,sizeof(LapObject*[m]));
		for(;k<m;++k){
			op1->Property[k]=NULL;
		}
	}
	int i=0,sum=env->Index-p;
	env->Index-=p+1;
	for(;i<p;++i){
		op2=env->Stack[sum+i];
		op1->Property[op1->Size++]=CreateObjectFromObject(op2);
		FreeObject(op2);
	}
	env->Stack[env->Index]=NULL;
}

void ArrayPop(LapState *env){//配合引用使用
	LapObject *op1=env->Stack[--env->Index];
	if(!op1->Size){
		env->Err=6;
		return;
	}
	--op1->Size;
	FreeObject(op1->Property[op1->Size]);
	env->Stack[env->Index]=NULL;
}

void ArrayFill(LapState *env){//配合引用使用
	LapObject *op2=env->Stack[--env->Index];
	LapObject *op1=env->Stack[--env->Index];
	int max=op1->Size,i=0;
	for(;i<max;++i){
		if(op1->Property[i]!=NULL){
			FreeObject(op1->Property[i]);
		}
		op1->Property[i]=CreateObjectFromObject(op2);
	}
	FreeObject(op2);
	env->Stack[env->Index]=NULL;
}

void ArrayInsert(LapState *env){//配合引用使用
	LapObject *op3=env->Stack[--env->Index];
	LapObject *op2=env->Stack[--env->Index];
	LapObject *op1=env->Stack[--env->Index];
	if(op1->Size+1>=op1->MaxSize){
		op1->MaxSize+=4;
		int m=op1->MaxSize,k=op1->Size;
		op1->Property=(LapObject**)realloc(op1->Property,sizeof(LapObject*[m]));
		for(;k<m;++k){
			op1->Property[k]=NULL;
		}
	}
    int index=*(int*)op2->Value,max=op1->Size;
    if(index<0||index>op1->Size-1){
		env->Err=2;
		return;
    }
    for(;max>index;--max){
		op1->Property[max]=op1->Property[max-1];
    }
    op1->Property[index]=CreateObjectFromObject(op3);
    ++op1->Size;
	FreeObject(op2);
	FreeObject(op3);
	env->Stack[env->Index]=NULL;
}

void ArrayRemove(LapState *env){//配合引用使用
	LapObject *op2=env->Stack[--env->Index];
	LapObject *op1=env->Stack[--env->Index];
    int index=*(int*)op2->Value,max=op1->Size-1;
    if(index<0||index>op1->Size-1){
		env->Err=2;
		return;
    }
    FreeObject(op1->Property[index]);
    for(;max>index;--max){
		op1->Property[max-1]=op1->Property[max];
    }
    --op1->Size;
	FreeObject(op2);
	env->Stack[env->Index]=NULL;
}

void Dlopen(LapState *env){//动态库系列全部结合引用使用
	LapObject *op1=env->Stack[--env->Index];
	#ifdef __MINGW32__
		void* temp=LoadLibrary((char*)op1->Value);
	#else
		void* temp=dlopen((char*)op1->Value,RTLD_LAZY);
	#endif
	if(temp==NULL){
		env->Err=5;
		env->Onstr=op1->Value;
		free(op1);
		return;
	}
	FreeObject(env->Stack[env->Index]);
	env->Stack[env->Index]=CreateObject(Lhandle,0,temp);
	++env->Index;
}

void Dlsym(LapState *env){
	LapObject *op2=env->Stack[--env->Index];
	LapObject *op1=env->Stack[--env->Index];
	char* str=(char*)op2->Value;
	#ifdef __MINGW32__
		void* temp=GetProcAddress(op1->Value,str);//函数的Size必须写明参数 不支持动态参数
	#else
		void* temp=dlsym(op1->Value,str);
	#endif
	if(temp==NULL){
		env->Err=7;
		env->Onstr=op2->Value;
		free(op2);
		return;
	}
	FreeObject(op2);
	int type=env->Commands[env->PC][1];
	env->Stack[env->Index]=CreateObject(type,0,temp);
	++env->Index;
}

void CallNative(LapState *env){
	LapObject *op1=env->Stack[--env->Index];//函数本体先入栈
	int max=env->Commands[env->PC][1];
	LapObject *arg=CreateObject(Lobject,max,NULL);
	int i=0;
	for(;i<max;++i){
		--env->Index;
		if(env->Stack[env->Index]==NULL){
			continue;
		}
		arg->Property[max-i-1]=CreateObjectFromObject(env->Stack[env->Index]);
		FreeObject(env->Stack[env->Index]);
	}
	env->Stack[env->Index]=((LapObject*(*)(LapObject*))(op1->Value))(arg);
	++env->Index;
	FreeObject(arg);
}

void Dlclose(LapState *env){
	LapObject *op1=env->Stack[--env->Index];
	#ifdef __MINGW32__
		FreeLibrary(op1->Value);
	#else
		dlclose(op1->Value);
	#endif
	free(op1);
	env->Stack[env->Index]=NULL;
}

void PushEmptyStr(LapState *env){
    ExtendStack(env);
    env->Stack[env->Index]=CreateObject(2,0,NULL);
    ++env->Index;
}

void PushArray(LapState *env){
    ExtendStack(env);
	LapObject *op1=env->Stack[env->Index-1];
	if(*(int*)op1->Value<0){
		env->Err=9;
		return;
	}
	LapObject *op2=CreateObject(4,*(int*)op1->Value,NULL);
	FreeObject(op1);
    env->Stack[env->Index-1]=op2;
}

void Delete(LapState *env){
	env->VarStacks[env->StackPC-1][env->Commands[env->PC][1]]=NULL;
}

void Exec(LapState *env){
	LapObject *op2=env->Stack[--env->Index];
	LapObject *op1=env->Stack[--env->Index];
	LapState *state=InitVM((char*)op1->Value,op2);
	FreeObject(op1);
	if(state==NULL){
		printf("Init Failed\n");
		env->Err=-1;
		return;
	}

	StartVM(state);
	env->Err=state->Err;
	DeleteState(state,0);
	env->Stack[env->Index]=NULL;
}

void Int(LapState *env){
	LapObject *op1=env->Stack[env->Index-1];
	op1->Type=0;
	int *on=malloc(sizeof(int));
	*on=(int)(*(double*)op1->Value);
	free((double*)op1->Value);
	op1->Value=on;
}

void Float(LapState *env){
	LapObject *op1=env->Stack[env->Index-1];
	op1->Type=1;
	double *on=malloc(sizeof(double));
	*on=*(int*)op1->Value;
	free((int*)op1->Value);
	op1->Value=on;
}

void Type(LapState *env){
	LapObject *op1=env->Stack[env->Index-1];
	env->Stack[env->Index-1]=CreateObject(0,0,NULL);
	*(int*)env->Stack[env->Index-1]->Value=op1->Type;
	FreeObject(op1);
}

/*
void Input(LapState *env){
	ExtendStack(env);
	char onget=getch();
	char *onstr=calloc(8,sizeof(char));
	int onsize=0;
	int max=8;
	while(onget!='\n'&&onget!='\r'){
		if(onsize==max-1){
			max+=8;
			onstr=realloc(onstr,sizeof(char[max]));
			memset(onstr+max-8,0,8);
		}
		if(onget==8&&!onsize){
			onstr[--onsize]=0;
		}
		else{
			onstr[onsize++]=onget;
		}
		onget=getch();
	}
	env->Stack[env->Index++]=CreateObject(2,onsize,onstr);
}*/

void Input(LapState *env){
	ExtendStack(env);
	char *onstr=calloc(300,sizeof(char));
	fgets(onstr,300,stdin);
	int size=StrLen(onstr);
	char *temp=calloc(size,sizeof(char));
	onstr[size-1]=0;
	strcpy(temp,onstr);
	free(onstr);
	env->Stack[env->Index++]=CreateObject(2,size-1,temp);
}

void Jump(LapState *env){
	env->PC=env->Commands[env->PC][1];
}

void PushNULL(LapState *env){
	ExtendStack(env);
	env->Stack[env->Index++]=NULL;
}

void Fseek(LapState *env){
	LapObject *op2=env->Stack[--env->Index];
	LapObject *op1=env->Stack[--env->Index];
	fseek(op1->Value,*(int*)op2->Value,SEEK_CUR);
	FreeObject(op1);
	FreeObject(op2);
}

void(*Ins[64])(LapState*)={PushConst,PushVarLocal,PushVarGlobal,
Pop,Add,Sub,Mul,Div,Mod,MoveLeft,MoveRight,BitXor,BitAnd,BitOr,
Or,And,Bigger,Smaller,PushFile,Equal,Index,Not,Inc,Dec,IsNull,
Ops,Print,GetCommandArg,StoreVarLocal,StoreVarGlobal,SetVarLocal,
SetVarGlobal,True_Jump,False_Jump,Goto,Return,Asc,Len,Fgetc,Fwrite,
CloseFile,PushObj,SetProperty,SetIndex,ArrayPush,ArrayPop,
ArrayFill,ArrayInsert,ArrayRemove,Dlopen,Dlsym,CallNative,Dlclose,
PushEmptyStr,PushArray,Delete,Exec,Int,Float,Type,Input,Jump,PushNULL,Fseek
};
/*
char dbg_str[63][20]={"PushConst","PushVarLocal","PushVarGlobal","Pop","Add","Sub","Mul","Div","Mod",
"MoveLeft","MoveRight","BitXor","BitAnd","BitOr","Or","And","Bigger","Smaller","PushFile","Equal","Index",
"Not","Inc","Dec","IsNull","Ops","Print","GetCommandArg","StoreVarLocal","StoreVarGlobal","SetVarLocal",
"SetVarGlobal","True_Jump","False_Jump","Goto","Return","Asc","Len","Fgetc","Fwrite","CloseFile","PushObj",
"SetProperty","SetIndex","ArrayPush","ArrayPop","ArrayFill","ArrayInsert","ArrayRemove","Dlopen","Dlsym",
"CallNative","Dlclose","PushEmptyStr","PushArray","Delete","Exec","Int","Float","Type","Input","Jump","PushNULL","Fseek"};*/


int StartVM(LapState *env){
	int max=env->TruePC;
	for(;env->PC<max;++env->PC){
		//printf("%d %s %d %d\n",env->PC,dbg_str[env->Commands[env->PC][0]],env->Commands[env->PC][1],env->Commands[env->PC][2]);
        (Ins[env->Commands[env->PC][0]])(env);
        if(env->Err){
			break;
        }
	}
	return env->Err;
}


int main(int argc,char* argv[]){
	args=CreateObject(4,argc-1,NULL);
	int i=1;
	for(;i<argc;++i){
		args->Property[i-1]=CreateObject(2,StrLen(argv[i]),argv[i]);
	}
	LapState *env=InitVM(argv[1],args);
	if(env==NULL){
		printf("Init Failed\n");
		args->Ref=0;
		FreeObject(args);
		return -1;
	}
/*
	clock_t start, finish;
	double  duration;
	start = clock();
*/
	if(StartVM(env)){
		i=env->Err;
		printf("%d ",env->PC+1);
		switch(i){
		case 1:
			printf("Err:Div 0!\n");
			break;
		case 2:
			printf("Err:Index out of range!\n");
			break;
		case 3:
			printf("Err:Too many parms are given!\n");
			break;
		case 4:
			printf("Err:Stack Overflow Max:%d\n",MAX_STACK);
			break;
		case 5:
			printf("Err:File '%s' Not Exist!\n",env->Onstr);
			free(env->Onstr);
			break;
		case 6:
			printf("Err:Array is empty,can not pop!\n");
			break;
		case 7:
			printf("Err:DLL symbol '%s' Not Exist!\n",env->Onstr);
			free(env->Onstr);
			break;
		case 8:
			printf("Err:Index item is null!\n");
			break;
		case 9:
			printf("Err:Index can not be negative!\n");
			break;
		}
	}
/*
	finish = clock();
	duration = (double)(finish - start) / CLOCKS_PER_SEC;
	printf("%f\n",duration);*/
	DeleteState(env,1);
	free(args);
}
