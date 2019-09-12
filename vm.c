#include<windows.h>
#include<math.h>

#include "CLap.h"

typedef struct{
	LapObject*** VarStacks;
	LapObject** Stack;
	LapObject** ConstVars;
	char*** Commands;
	char* Onstr;
	LapObject* Argv;
	int Index,ConstNum,*VarNum,MaxConst,*MaxVar,MaxIndex,PC,*PCStack,MaxPC,TruePC,Err,StackPC,MaxStackPC;
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
		i++;
	}
    return i;
}

void ExtendStack(LapState *env){
    if(env->MaxIndex==env->Index){
		env->MaxIndex+=20;
		env->Stack=(LapObject**)realloc(env->Stack,sizeof(LapObject*[env->MaxIndex]));
    }
}

char** SplitIns(char* str){
	int len=StrLen(str)-1;
    char** temp=(char**)calloc(3,sizeof(char*)),*onstr=(char*)calloc(20,sizeof(char));
    int i=0,index=0,k=0,max=20;
    for(;i<len;i++){
		if(str[i]==' '){
			index=0;
			temp[k]=onstr;
			if(k==2){
				onstr=NULL;
				break;
			}
			onstr=(char*)calloc(5,sizeof(char));
			max=4;
			k++;
		}
		else{
			if(index==max){
				max+=20;
				onstr=(char*)realloc(onstr,sizeof(char[max+3]));
				memset(onstr+max-20,0,20);
			}
			onstr[index]=str[i];
			index++;
		}
    }
    if(onstr!=NULL){
		temp[k]=onstr;
    }
    return temp;
}

LapState *InitVM(char* path,LapObject* arg){
	FILE *fp=fopen(path,"r+");
	if(fp==NULL){
		printf("File %s Not Found\n",path);
		return NULL;
	}
	LapState *temp=(LapState*)malloc(sizeof(LapState));
	temp->Argv=arg;
	temp->Stack=(LapObject**)calloc(20,sizeof(LapObject*));
	temp->ConstVars=(LapObject**)malloc(sizeof(LapObject*[20]));
    temp->VarStacks=(LapObject***)malloc(sizeof(LapObject**[1]));
    temp->VarStacks[0]=(LapObject**)calloc(20,sizeof(LapObject*));
    temp->MaxVar=(int*)malloc(sizeof(int[8]));
    temp->VarNum=(int*)malloc(sizeof(int[8]));
    temp->PCStack=(int*)malloc(sizeof(int[8]));
    temp->Commands=(char***)malloc(sizeof(char**[20]));
    temp->StackPC=0;
    temp->MaxStackPC=1;
    int i=0;
	temp->MaxVar[0]=20;
	temp->VarNum[0]=0;
    temp->Index=0;
    temp->ConstNum=0;
    temp->MaxPC=20;
    temp->TruePC=0;
    temp->PC=0;
    temp->MaxConst=20;
    temp->MaxIndex=20;
    temp->Err=0;
    char* onstr=NULL;
    while(1){
        onstr=(char*)calloc(100,sizeof(100));
        if(fgets(onstr,100,fp)==NULL){
			free(onstr);
			break;
        }
        if(temp->TruePC==temp->MaxPC){
			temp->MaxPC+=20;
			temp->Commands=(char***)realloc(temp->Commands,sizeof(char**[temp->MaxPC]));
        }
        temp->Commands[temp->TruePC]=SplitIns(onstr);
        temp->TruePC++;
        free(onstr);
    }
    fclose(fp);
	return temp;
}

void DeleteState(LapState *state){
	int i=0,max=state->ConstNum;
    for(;i<max;i++){
		FreeObject(state->ConstVars[i]);
    }
    free(state->ConstVars);
    max=state->VarNum[0];
    int ii=0;
    for(;ii<state->MaxStackPC;ii++){
		if(state->VarStacks[ii]!=NULL){
			i=0;
			for(;i<state->VarNum[ii];i++){
				if(state->VarStacks[ii][i]!=NULL){
					state->VarStacks[ii][i]->Protect=0;
					FreeObject(state->VarStacks[ii][i]);
				}
			}
			free(state->VarStacks[ii]);
		}
    }
    i=0,max=state->TruePC;
    for(;i<max;i++){
		free(state->Commands[i][0]);
		if(state->Commands[i][1]!=NULL){
			free(state->Commands[i][1]);
		}
		if(state->Commands[i][2]!=NULL){
			free(state->Commands[i][2]);
		}
        free(state->Commands[i]);
    }
    free(state->VarStacks);
    free(state->VarNum);
    free(state->MaxVar);
    free(state->PCStack);
    free(state->Commands);
    free(state->Stack);
    FreeObject(state->Argv);
    free(state);
}

int StringCmp(const char* str1,const char* str2){
	int i=0,temp=1;
	while(1){
		if(str1[i]!=str2[i]){
			temp=0;
			break;
		}
		if(str2[i]==0||str1[i]==0){
			if(!i){
				temp=0;
			}
			break;
		}
		i++;
	}
	return temp;
}

int* ParseInt(const char* str){
    int max=StrLen(str);
    int i=0,sign=1;
    int *temp=malloc(sizeof(int));
    *temp=0;
    if(str[0]=='-'){
		i=1,sign=-1;
    }
    for(;i<max;i++){
		*temp+=(str[i]-'0')*QuickPower(10,max-i-1);
    }
    *temp*=sign;
    return temp;
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
		l++;
    }
    l--;
    while(str[i]!='.'){
		*temp+=(str[i]-'0')*QuickPower(10,l-i);
		i++;
    }
    i++;
    l=1;
    while(str[i]>0){
		*temp+=(str[i]-'0')*pow(0.1,l);
		l++;
		i++;
    }
    *temp*=sign;
    return temp;
}

char* ParseStr(const char* str){
	int max=StrLen(str);
	char* temp=malloc(sizeof(char[max+3]));
	memset(temp,0,sizeof(char[max+3]));
	int i=0,index=0;
	char lastc=-1;
	for(;i<max;i++){
		if(lastc=='\\'){
			if(str[i]=='n'){
				temp[index]='\n';
			}
			else if(str[i]=='b'){
				temp[index]=' ';
			}
			else{
				temp[index]=str[i];
			}
			index++;
		}
		else if(str[i]=='\\'){
			lastc='\\';
			continue;
		}
		else{
			temp[index]=str[i];
			index++;
		}
		lastc=str[i];
	}
	return temp;
}

void AddConst(LapState *env,char type){
	if(env->ConstNum==env->MaxConst){
		env->MaxConst+=20;
		env->ConstVars=(LapObject**)realloc(env->ConstVars,sizeof(LapObject*[env->MaxConst]));
	}
	char *temp=NULL;
	LapObject *obj=NULL;
	switch(type){
	case 0:
		env->ConstVars[env->ConstNum]=CreateObject(0,0,ParseInt(env->Commands[env->PC][1]));
	break;
	case 1:
		env->ConstVars[env->ConstNum]=CreateObject(1,0,ParseFloat(env->Commands[env->PC][1]));
	break;
	case 2:
		temp=ParseStr(env->Commands[env->PC][1]);
		env->ConstVars[env->ConstNum]=CreateObject(2,StrLen(temp),temp);
	break;
	case 3:
		obj=CreateObject(3,0,NULL);
		if(StringCmp(env->Commands[env->PC][1],"false")){
			*(int*)obj->Value=0;
		}
		else{
			*(int*)obj->Value=1;
		}
		env->ConstVars[env->ConstNum]=obj;
	break;
	}
	env->ConstNum++;
}

void PushConst(LapState *env){
    char** ins=env->Commands[env->PC];
    int *i=ParseInt(ins[1]);
    ExtendStack(env);
    env->Stack[env->Index]=CreateObjectFromObject(env->ConstVars[*i]);
    env->Index++;
    free(i);
}

void PushVar(LapState *env,int sign){
    char** ins=env->Commands[env->PC];
    int *i=ParseInt(ins[1]);
    ExtendStack(env);
    switch(sign){
	case 'l':
		env->Stack[env->Index]=CreateObjectFromObject(env->VarStacks[env->StackPC][*i]);
		break;
	case 'g':
		env->Stack[env->Index]=CreateObjectFromObject(env->VarStacks[0][*i]);
		break;
    }
    env->Index++;
    free(i);
}

void Pop(LapState *env){
	if(env->Index>0){
		env->Index--;
		FreeObject(env->Stack[env->Index]);
		env->Stack[env->Index]=NULL;
	}
}

void Calculate(LapState *env,int sign){
	env->Index--;
	LapObject *op2=env->Stack[env->Index];
	env->Index--;
	LapObject *op1=env->Stack[env->Index];
	int type=op1->Type;
	char* onstr=NULL;
	LapObject *temp=CreateObjectFromObject(op1);
	double d1,d2;
	int *onbool=NULL,n;
	FILE *fp=NULL;
	switch(sign){
		case '+':
		switch(type){
		case 0:
			*(int*)temp->Value=*(int*)op1->Value+*(int*)op2->Value;
			break;
		case 1:
			*(double*)temp->Value=*(double*)op1->Value+*(double*)op2->Value;
			break;
		case 2:
			type=op1->Size+op2->Size;
			free(temp->Value);
			temp->Value=ConcatStr((char*)op1->Value,(char*)op2->Value,op1->Size,op2->Size);
			temp->Size=type;
			break;
		}
		break;
		case '-':
		switch(type){
		case 0:
			*(int*)temp->Value=*(int*)op1->Value-*(int*)op2->Value;
			break;
		case 1:
			*(double*)temp->Value=*(double*)op1->Value-*(double*)op2->Value;
			break;
		}
		break;
		case '*':
		switch(type){
		case 0:
			*(int*)temp->Value=*(int*)op1->Value**(int*)op2->Value;
			break;
		case 1:
			*(double*)temp->Value=*(double*)op1->Value**(double*)op2->Value;
			break;
		}
		break;
		case '/':
		if(!*(int*)op2->Value){
			env->Err=1;
			break;
		}
		switch(type){
		case 0:
			*(int*)temp->Value=*(int*)op1->Value/ *(int*)op2->Value;
			break;
		case 1:
			*(double*)temp->Value=*(double*)op1->Value/ *(double*)op2->Value;
			break;
		}
		break;
		case '%':
		if(!*(double*)op2->Value){
			env->Err=1;
			break;
		}
		switch(type){
			case 0:
				*(int*)temp->Value=*(int*)op1->Value% *(int*)op2->Value;
				break;
			case 1:
				d1=(*(double*)op1->Value);
				d2=(*(double*)op2->Value);
				*((double*)temp->Value)=d1-(int)(d1/d2)*d2;
				break;
		}
		break;
		case 'l':
		switch(type){
		case 0:
			(*(int*)temp->Value)=*(int*)op1->Value<<*(int*)op2->Value;
			break;
		}
		break;
		case 'r':
		switch(type){
		case 0:
			(*(int*)temp->Value)=*(int*)op1->Value>>*(int*)op2->Value;
			break;
		}
		break;
		case '^':
		switch(type){
		case 0:
			(*(int*)temp->Value)=*(int*)op1->Value^*(int*)op2->Value;
			break;
		}
		break;
		case '&':
		switch(type){
		case 0:
			(*(int*)temp->Value)=*(int*)op1->Value&*(int*)op2->Value;
			break;
		}
		break;
		case '|':
		switch(type){
		case 0:
			(*(int*)temp->Value)=*(int*)op1->Value|*(int*)op2->Value;
			break;
		}
		break;
		case '>':
		switch(type){
		case 0:
			(*(int*)temp->Value)=*(int*)op1->Value>*(int*)op2->Value;
			break;
		case 1:
			(*(double*)temp->Value)=*(double*)op1->Value>*(double*)op2->Value;
			break;
		}
		temp->Type=3;
		break;
		case '<':
		switch(type){
		case 0:
			(*(int*)temp->Value)=*(int*)op1->Value<*(int*)op2->Value;
			break;
		case 1:
			(*(double*)temp->Value)=*(double*)op1->Value<*(double*)op2->Value;
			break;
		}
		temp->Type=3;
		break;
		case '=':
		switch(type){
		case 0:
			(*(int*)temp->Value)=*(int*)op1->Value==*(int*)op2->Value;
			break;
		case 1:
			(*(double*)temp->Value)=*(double*)op1->Value==*(double*)op2->Value;
			break;
		case 2:
			onbool=malloc(sizeof(int));
			*onbool=StringCmp((char*)op1->Value,(char*)op2->Value);
			FreeObject(temp);
			temp=CreateObject(3,0,onbool);
			break;
		case 3:
			(*(int*)temp->Value)=*(int*)op1->Value==*(int*)op2->Value;
			break;
		}
		temp->Type=3;
		break;
		case 'i':
		n=*(int*)op2->Value;
		if(n>op1->Size-1){
			env->Err=2;
			break;
		}
		switch(type){
		case 2:
			onstr=(char*)malloc(sizeof(char[2]));
			onstr[0]=((char*)op1->Value)[n];
			onstr[1]=0;
			free((char*)temp->Value);
			temp->Value=onstr;
			temp->Size=1;
			break;
		case 4:
			temp=CreateObjectFromObject(op1->Property[n]);
			break;
		}
		break;
		case 'a':
			(*(int*)temp->Value)=*(int*)op1->Value&&*(int*)op2->Value;
		break;
		case 'o':
			(*(int*)temp->Value)=*(int*)op1->Value||*(int*)op2->Value;
		break;
		case 'f':
			fp=fopen((char*)op1->Value,(char*)op2->Value);
			if(fp==NULL){
				env->Err=5;
				env->Onstr=op1->Value;
				free(op1);
				break;
			}
			FreeObject(temp);
			temp=CreateObject(5,0,fp);
		break;
	}
	FreeObject(op1);
	FreeObject(op2);
	env->Stack[env->Index]=temp;
	env->Index++;
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
	}
	op1->Type=3;
}

void IsNull(LapState *env){
	LapObject *op1=env->Stack[env->Index-1];
	int *p=malloc(sizeof(int));
	if(op1==NULL){
		*p=1;
	}
	else{
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
	env->Index--;
	LapObject *obj=NULL,*op1=env->Stack[env->Index];
	if(op1!=NULL){
		if(op1->Type>3){
			obj=CreateObjectFromObject(op1);
		}
		else{
			obj=op1;
		}
	}
    PrintData(obj);
    FreeObject(obj);
    env->Stack[env->Index]=NULL;
}

void GetCommandArg(LapState *env){
    char** ins=env->Commands[env->PC];
    ExtendStack(env);
    env->Stack[env->Index]=CreateObjectFromObject(env->Argv);
    env->Index++;
}

void StoreVar(LapState *env,int sign){
    char** ins=env->Commands[env->PC];
    int *i=ParseInt(ins[1]),PC=env->StackPC;
    switch(sign){
	case 'l':
		if(*i>env->MaxVar[PC]-1){
			env->MaxVar[PC]+=20;
			env->VarStacks[PC]=(LapObject**)realloc(env->VarStacks[PC],sizeof(LapObject*[env->MaxVar[PC]]));
		}
		env->VarNum[PC]++;
        env->VarStacks[PC][*i]=CreateObjectFromObject(env->Stack[0]);
		break;
	case 'g':
		if(*i>env->MaxVar[0]-1){
			env->MaxVar[0]+=20;
			env->VarStacks[0]=(LapObject**)realloc(env->VarStacks[0],sizeof(LapObject*[env->MaxVar[0]]));
		}
		env->VarNum[0]++;
        env->VarStacks[0][*i]=CreateObjectFromObject(env->Stack[0]);
		break;
    }
	if(env->Stack[0]->Type>3){
		env->Stack[0]->Ref++;
	}
    env->Index--;
    FreeObject(env->Stack[0]);
    env->Stack[0]=NULL;
    free(i);
}

void SetVar(LapState *env,int sign){
    char** ins=env->Commands[env->PC];
    int *i=ParseInt(ins[1]),PC=env->StackPC;
    switch(sign){
	case 'l':
		if(env->VarStacks[PC][*i]!=NULL){
			if(env->VarStacks[PC][*i]->Protect){
				env->VarStacks[PC][*i]=env->Stack[0];
			}
			else{
				FreeObject(env->VarStacks[PC][*i]);
				env->VarStacks[PC][*i]=CreateObjectFromObject(env->Stack[0]);
			}
		}
		break;
	case 'g':
		if(env->VarStacks[0][*i]!=NULL){
			if(env->VarStacks[0][*i]->Protect){
				env->VarStacks[0][*i]=env->Stack[0];
			}
			else{
				FreeObject(env->VarStacks[0][*i]);
				env->VarStacks[0][*i]=CreateObjectFromObject(env->Stack[0]);
			}
		}
		break;
    }
    env->Index--;
    FreeObject(env->Stack[0]);
    env->Stack[0]=NULL;
    free(i);
}

void Jump(LapState *env,int sign){
	int *line=NULL;
	if(sign){
		if(*(int*)env->Stack[0]->Value){
			line=ParseInt(env->Commands[env->PC][1]);
			env->PC=*line-2;
			free(line);
		}
	}
	else{
		if(!*(int*)env->Stack[0]->Value){
			line=ParseInt(env->Commands[env->PC][1]);
			env->PC=*line-2;
			free(line);
		}
	}
	env->Index--;
	FreeObject(env->Stack[0]);
	env->Stack[0]=NULL;
}

void Goto(LapState *env){
	char** ins=env->Commands[env->PC];
	int *line=ParseInt(ins[2]);
	int n=*line,v=0;
	free(line);
	if(n>MAX_ARG_NUM){
		env->Err=3;
		return;
	}
	env->PCStack[env->StackPC]=env->PC;
	env->StackPC++;
	if(env->StackPC>MAX_STACK){
		env->Err=4;
		return;
	}
	if(env->StackPC==env->MaxStackPC){
		int i=env->MaxStackPC;
		env->MaxStackPC+=8;
		int max=i+8;
		env->VarNum=(int*)realloc(env->VarNum,sizeof(int[max]));
		env->VarStacks=(LapObject***)realloc(env->VarStacks,sizeof(LapObject**[max]));
		env->MaxVar=(int*)realloc(env->MaxVar,sizeof(int[max]));
		env->PCStack=(int*)realloc(env->PCStack,sizeof(int[max]));
		for(;i<max;i++){
			env->VarNum[i]=0;
			env->MaxVar[i]=20;
			env->VarStacks[i]=NULL;
		}
	}
	env->VarStacks[env->StackPC]=(LapObject**)malloc(sizeof(LapObject*[20]));
	LapObject** var=env->VarStacks[env->StackPC];
	for(;v<20;v++){
		var[v]=NULL;
	}
	v=0;
	line=ParseInt(ins[1]);
	env->PC=*line-2;
	free(line);
    for(;v<n;v++){
		env->Index--;
		var[n-v-1]=CreateObjectFromObject(env->Stack[env->Index]);
		FreeObject(env->Stack[env->Index]);
		env->Stack[env->Index]=NULL;
    }
}

void Return(LapState *env){
	env->StackPC--;
	int PC=env->StackPC;
	env->PC=env->PCStack[PC];
	int i=0,max=env->MaxVar[PC];
	LapObject **var=env->VarStacks[PC];
	for(;i<max;i++){
		if(var[i]!=NULL){
			FreeObject(var[i]);
		}
	}
    //free(var);
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
	int *x=malloc(sizeof(int));
	*x=obj->Size;
	FreeObject(obj);
	env->Stack[env->Index-1]=CreateObject(0,0,x);
}

void Fgetc(LapState *env){//所有文件操作确保至少绑定在一个变量上
	LapObject *obj=env->Stack[env->Index-1];
	char *x=malloc(sizeof(char[2]));
	x[0]=fgetc((FILE*)obj->Value);
	x[1]=0;
	env->Stack[env->Index-1]=CreateObject(2,1,x);
}

void Fwrite(LapState *env){
	env->Index--;
	LapObject *op2=env->Stack[env->Index];
	env->Index--;
	LapObject *op1=env->Stack[env->Index];
	fputs((char*)op2->Value,(FILE*)op1->Value);
	free(op1);
	FreeObject(op2);
	env->Stack[env->Index]=NULL;
}

void CloseFile(LapState *env){
	env->Index--;
	LapObject *obj=env->Stack[env->Index];
	fclose((FILE*)obj->Value);
	free(obj);
	env->Stack[env->Index]=NULL;
}

void PushObj(LapState *env){
    ExtendStack(env);
    int *n=ParseInt(env->Commands[env->PC][1]);
    env->Stack[env->Index]=CreateObject(4,*n,NULL);
    env->Index++;
    free(n);
}

void SetProperty(LapState *env){//引用设置
	char** ins=env->Commands[env->PC];
	env->Index--;
	LapObject *op2=env->Stack[env->Index];
	int *i=ParseInt(ins[1]);
	env->Index--;
	LapObject *op1=env->Stack[env->Index];
	if(op1->Property[*i]!=NULL){
		FreeObject(op1->Property[*i]);
	}
	op1->Property[*i]=CreateObjectFromObject(op2);
	FreeObject(op2);
	free(i);
	env->Stack[env->Index]=NULL;
}

void SetIndex(LapState *env){//引用设置
	env->Index--;
	LapObject *op3=env->Stack[env->Index];
	env->Index--;
	LapObject *op2=env->Stack[env->Index];
	env->Index--;
	LapObject *op1=env->Stack[env->Index];
	int index=*(int*)op2->Value;
	if(index>op1->Size-1){
		env->Err=2;
		return;
	}
	if(op1->Property[index]!=NULL){
		FreeObject(op1->Property[index]);
	}
	op1->Property[index]=CreateObjectFromObject(op3);
	if(op3->Type>3){
		op3->Ref++;
	}
	FreeObject(op2);
	FreeObject(op3);
	env->Stack[env->Index]=NULL;
}

void ArrayPush(LapState *env){//配合引用使用
	int *p=ParseInt(env->Commands[env->PC][1]);
	LapObject *op1=env->Stack[env->Index-*p-1],*op2=NULL;
	if(op1->Size+*p>=op1->MaxSize){
		op1->MaxSize+=*p;
		int m=op1->MaxSize,k=op1->Size;
		op1->Property=(LapObject**)realloc(op1->Property,sizeof(LapObject*[m]));
		for(;k<m;k++){
			op1->Property[k]=NULL;
		}
	}
	int i=0,sum=env->Index-*p;
	env->Index-=*p+1;
	int m=*p;
	free(p);
	for(;i<m;i++){
		op2=env->Stack[sum+i];
		op1->Property[op1->Size++]=CreateObjectFromObject(op2);
		FreeObject(op2);
	}
	env->Stack[env->Index]=NULL;
}

void ArrayPop(LapState *env){//配合引用使用
	env->Index--;
	LapObject *op1=env->Stack[env->Index];
	if(!op1->Size){
		env->Err=6;
		return;
	}
	op1->Size--;
	FreeObject(op1->Property[op1->Size]);
	env->Stack[env->Index]=NULL;
}

void ArrayFill(LapState *env){//配合引用使用
	env->Index--;
	LapObject *op2=env->Stack[env->Index];
	env->Index--;
	LapObject *op1=env->Stack[env->Index];
	int max=op1->Size,i=0;
	for(;i<max;i++){
		if(op1->Property[i]!=NULL){
			FreeObject(op1->Property[i]);
		}
		op1->Property[i]=CreateObjectFromObject(op2);
	}
	FreeObject(op2);
	env->Stack[env->Index]=NULL;
}

void ArrayInsert(LapState *env){//配合引用使用
	env->Index--;
	LapObject *op3=env->Stack[env->Index];
	env->Index--;
	LapObject *op2=env->Stack[env->Index];
	env->Index--;
	LapObject *op1=env->Stack[env->Index];
	if(op1->Size+1>=op1->MaxSize){
		op1->MaxSize+=8;
		int m=op1->MaxSize,k=op1->Size;
		op1->Property=(LapObject**)realloc(op1->Property,sizeof(LapObject*[m]));
		for(;k<m;k++){
			op1->Property[k]=NULL;
		}
	}
    int index=*(int*)op2->Value,max=op1->Size;
    if(index<0||index>op1->Size-1){
		env->Err=2;
		return;
    }
    for(;max>index;max--){
		op1->Property[max]=op1->Property[max-1];
    }
    op1->Property[index]=CreateObjectFromObject(op3);
    op1->Size++;
	FreeObject(op2);
	FreeObject(op3);
	env->Stack[env->Index]=NULL;
}

void ArrayRemove(LapState *env){//配合引用使用
	env->Index--;
	LapObject *op2=env->Stack[env->Index];
	env->Index--;
	LapObject *op1=env->Stack[env->Index];
    int index=*(int*)op2->Value,max=op1->Size-1;
    if(index<0||index>op1->Size-1){
		env->Err=2;
		return;
    }
    FreeObject(op1->Property[index]);
    for(;max>index;max--){
		op1->Property[max-1]=op1->Property[max];
    }
    op1->Size--;
	FreeObject(op2);
	env->Stack[env->Index]=NULL;
}

void Dlopen(LapState *env){//动态库系列全部结合引用使用
	env->Index--;
	LapObject *op1=env->Stack[env->Index];
	void* temp=LoadLibrary((char*)op1->Value);
	if(temp==NULL){
		env->Err=5;
		env->Onstr=op1->Value;
		free(op1);
		return;
	}
	FreeObject(env->Stack[env->Index]);
	env->Stack[env->Index]=CreateObject(Lhandle,0,temp);
	env->Index++;
}

void Dlsym(LapState *env){
	env->Index--;
	LapObject *op2=env->Stack[env->Index];
	env->Index--;
	LapObject *op1=env->Stack[env->Index];
	char* str=(char*)op2->Value;
	void* temp=GetProcAddress(op1->Value,str);//函数的Size必须写明参数 不支持动态参数
	if(temp==NULL){
		env->Err=7;
		env->Onstr=op2->Value;
		free(op2);
		return;
	}
	FreeObject(op2);
	char** ins=env->Commands[env->PC];
	int *t=ParseInt(ins[1]),type=*t;
	free(t);
	t=ParseInt(ins[2]);
	env->Stack[env->Index]=CreateObject(type,*t,temp);
	free(t);
	env->Index++;
}

void CallNative(LapState *env){
	env->Index--;
	LapObject *op1=env->Stack[env->Index];//函数本体先入栈
	int max=op1->Size;
	LapObject *arg=CreateObject(Lobject,max,NULL);
	int i=0;
	for(;i<max;i++){
		env->Index--;
		arg->Property[max-i-1]=CreateObjectFromObject(env->Stack[env->Index]);
		FreeObject(env->Stack[env->Index]);
	}
	env->Stack[env->Index]=((LapObject*(*)(LapObject*))(op1->Value))(arg);
	env->Index++;
	FreeObject(arg);
}

void Dlclose(LapState *env){
	env->Index--;
	LapObject *op1=env->Stack[env->Index];
	FreeLibrary(op1->Value);
	free(op1);
	env->Stack[env->Index]=NULL;
}

void PushEmptyStr(LapState *env){
    ExtendStack(env);
    env->Stack[env->Index]=CreateObject(2,0,NULL);
    env->Index++;
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
	int *index=ParseInt(env->Commands[env->PC][1]);
	FreeObject(env->VarStacks[env->StackPC][*index]);
	env->VarStacks[env->StackPC][*index]=NULL;
	free(index);
}

void Exec(LapState *env){//配合引用使用
	env->Index--;
	LapObject *op2=env->Stack[env->Index];
	env->Index--;
	LapObject *op1=env->Stack[env->Index];
	LapState *state=InitVM((char*)op1->Value,op2);
	FreeObject(op1);
	if(state==NULL){
		printf("Init Failed\n");
		env->Err=-1;
		return;
	}
	StartVM(state);
	env->Err=state->Err;
	DeleteState(state);
	env->Stack[env->Index]=NULL;
}


void DoIns(LapState *env){//use ' ' to separate parms
	char** ins=env->Commands[env->PC];
	//printf("%s\n",ins[0]);
	//system("pause");
    if(StringCmp(ins[0],"add_const_int")){//1 str for int
        AddConst(env,0);
    }
    else if(StringCmp(ins[0],"add_const_float")){//1 str for double
        AddConst(env,1);
    }
    else if(StringCmp(ins[0],"add_const_str")){//1 str no"" \b->' ' \n->'\n'
        AddConst(env,2);
    }
    else if(StringCmp(ins[0],"add_const_bool")){//1 true/false
        AddConst(env,3);
    }
    else if(StringCmp(ins[0],"push_const")){//1 int for const id
		PushConst(env);
    }
    else if(StringCmp(ins[0],"push_var_local")){//1 int for var id 1 true for reference
		PushVar(env,'l');
    }
    else if(StringCmp(ins[0],"push_var_global")){//1 int for var id 1 true for reference
		PushVar(env,'g');
    }
    else if(StringCmp(ins[0],"pop")){
		Pop(env);
    }
    else if(StringCmp(ins[0],"add")){
		Calculate(env,'+');
    }
    else if(StringCmp(ins[0],"sub")){
		Calculate(env,'-');
    }
    else if(StringCmp(ins[0],"mul")){
		Calculate(env,'*');
    }
    else if(StringCmp(ins[0],"div")){
		Calculate(env,'/');
    }
    else if(StringCmp(ins[0],"mod")){
		Calculate(env,'%');
    }
    else if(StringCmp(ins[0],"index")){
		Calculate(env,'i');
    }
    else if(StringCmp(ins[0],"bigger")){
		Calculate(env,'>');
    }
    else if(StringCmp(ins[0],"smaller")){
		Calculate(env,'<');
    }
    else if(StringCmp(ins[0],"equal")){
		Calculate(env,'=');
    }
    else if(StringCmp(ins[0],"and")){
		Calculate(env,'a');
    }
    else if(StringCmp(ins[0],"or")){
		Calculate(env,'o');
    }
    else if(StringCmp(ins[0],"move_left")){
		Calculate(env,'l');
    }
    else if(StringCmp(ins[0],"move_right")){
		Calculate(env,'r');
    }
    else if(StringCmp(ins[0],"bit_and")){
		Calculate(env,'&');
    }
    else if(StringCmp(ins[0],"xor")){
		Calculate(env,'^');
    }
    else if(StringCmp(ins[0],"bit_or")){
		Calculate(env,'|');
    }
    else if(StringCmp(ins[0],"not")){
		Not(env);
    }
    else if(StringCmp(ins[0],"ops")){
		Ops(env);
    }
    else if(StringCmp(ins[0],"print")){
		Print(env);
    }
    else if(StringCmp(ins[0],"delete")){
		Delete(env);
    }
    else if(StringCmp(ins[0],"push_null")){
		ExtendStack(env);
		env->Index++;
    }
    else if(StringCmp(ins[0],"true_jump")){//1 int for line
		Jump(env,1);
    }
    else if(StringCmp(ins[0],"false_jump")){//1 int for line
		Jump(env,0);
    }
    else if(StringCmp(ins[0],"jump")){//1 int for line
		int *line=ParseInt(ins[1]);
		env->PC=*line-2;
		free(line);
    }
    else if(StringCmp(ins[0],"is_null")){
		IsNull(env);
    }
    else if(StringCmp(ins[0],"get_command_arg")){
		GetCommandArg(env);
    }
    else if(StringCmp(ins[0],"store_var_local")){//1 int for var index
		StoreVar(env,'l');
    }
    else if(StringCmp(ins[0],"store_var_global")){//1 int for var index
		StoreVar(env,'g');
    }
    else if(StringCmp(ins[0],"push_obj")){//1 int for property num
		PushObj(env);
    }
    else if(StringCmp(ins[0],"push_arr")){
		PushArray(env);
    }
    else if(StringCmp(ins[0],"push_empty_str")){
		PushEmptyStr(env);
    }
    else if(StringCmp(ins[0],"set_var_local")){//1 int for property index
		SetVar(env,'l');
    }
    else if(StringCmp(ins[0],"set_var_global")){//1 int for property index
		SetVar(env,'g');
    }
    else if(StringCmp(ins[0],"set_property")){//1 int for property index
		SetProperty(env);
    }
    else if(StringCmp(ins[0],"set_index")){
		SetIndex(env);
    }
    else if(StringCmp(ins[0],"goto")){//1 int for line  1 int for parm num
		Goto(env);
    }
    else if(StringCmp(ins[0],"return")){
		Return(env);
    }
    else if(StringCmp(ins[0],"asc")){
		Asc(env);
    }
    else if(StringCmp(ins[0],"len")){
		Len(env);
    }
    else if(StringCmp(ins[0],"exec")){
		Exec(env);
    }
    else if(StringCmp(ins[0],"open_file")){
		Calculate(env,'f');
    }
    else if(StringCmp(ins[0],"close_file")){
		CloseFile(env);
    }
    else if(StringCmp(ins[0],"fgetc")){
		Fgetc(env);
    }
    else if(StringCmp(ins[0],"fwrite")){
		Fwrite(env);
    }
    else if(StringCmp(ins[0],"arr_push")){
		ArrayPush(env);
    }
    else if(StringCmp(ins[0],"arr_pop")){
		ArrayPop(env);
    }
    else if(StringCmp(ins[0],"arr_insert")){
		ArrayInsert(env);
    }
    else if(StringCmp(ins[0],"arr_remove")){
		ArrayRemove(env);
    }
    else if(StringCmp(ins[0],"arr_fill")){
		ArrayFill(env);
    }
    else if(StringCmp(ins[0],"dlopen")){
		Dlopen(env);
    }
    else if(StringCmp(ins[0],"dlsym")){
		Dlsym(env);
    }
    else if(StringCmp(ins[0],"dlclose")){
		Dlclose(env);
    }
    else if(StringCmp(ins[0],"call_native")){
		CallNative(env);
    }
    else{//小于0虚拟机问题 大于0编程问题
		env->Err=-1;
		printf("%d Err:unrecognized ins:%s\n",env->PC+1,ins[0]);
    }
}

int StartVM(LapState *env){
	int max=env->TruePC;
	for(;env->PC<max;env->PC++){
        DoIns(env);
        if(env->Err){
			break;
        }
	}
	return env->Err;
}


int main(int argc,char* argv[]){
	args=CreateObject(4,argc-1,NULL);
	args->Protect=1;
	int i=1;
	for(;i<argc;i++){
		args->Property[i-1]=CreateObject(2,StrLen(argv[i]),argv[i]);
	}
	LapState *env=InitVM(argv[1],args);
	if(env==NULL){
		printf("Init Failed\n");
		FreeObject(args);
		return -1;
	}
	else if(StartVM(env)){
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
			printf("Err:Array length can not be less than zero!\n");
			break;
		}
	}
	args->Ref=0;
	DeleteState(env);
	system("pause");
}
