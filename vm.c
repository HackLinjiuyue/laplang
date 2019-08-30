#include<stdio.h>
#include<stdlib.h>
#include<string.h>

typedef struct LapObject{
	struct LapObject** Property;
	void *Value;
	int Type;
	int Size;
}LapObject;

typedef struct{
	LapObject** GlobalVars;
	LapObject*** VarStacks;
	LapObject** Stack;
	LapObject** ConstVars;
	char*** Commands;
	int Index,ConstNum,GlobalNum,*VarNum,MaxConst,MaxGlobal,*MaxVar,MaxIndex,PC,*PCStack,MaxPC,TruePC,Err;
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

LapObject *CreateObject(int type,int size,void* value){
	LapObject *temp=malloc(sizeof(LapObject));
	temp->Type=type;
	temp->Size=size;
	temp->Value=value;
	if(value==NULL){
		switch(type){
		case 0:
		temp->Value=malloc(sizeof(int));
		break;
		case 1:
		temp->Value=malloc(sizeof(double));
		case 2:
		temp->Value=malloc(sizeof(char[size]));
		memset(temp->Value,0,sizeof((char*)temp->Value));
		break;
		case 3:
		temp->Value=malloc(sizeof(int));
		case 4:
		temp->Property=(LapObject**)malloc(sizeof(LapObject*[size]));
		break;
		}
	}
	return temp;
}

LapObject *CreateObjectFromObject(LapObject *obj){
	int size=obj->Size,i=0;
	LapObject* temp=CreateObject(obj->Type,size,NULL);
	switch(obj->Type){
	case 0:
	*(int*)temp->Value=*(int*)obj->Value;
	break;
	case 1:
	*(double*)temp->Value=*(double*)obj->Value;
	break;
	case 2:
	strcpy((char*)temp->Value,(char*)obj->Value);
	break;
	case 3:
	*(int*)temp->Value=*(int*)obj->Value;
	case 4:
	temp->Property=obj->Property;
	break;
	}
	return temp;
}

LapObject *args=NULL;

int StrLen(char* str){
	int i=0;
	while(str[i]>0){
		i++;
	}
    return i;
}

char** SplitIns(char* str){
	int len=StrLen(str)-1;
    char** temp=(char**)malloc(sizeof(char*[3])),*onstr=(char*)malloc(sizeof(char[20]));
    int i=0,index=0,k=0,max=20;
    for(;i<len;i++){
		if(str[i]==' '){
			index=0;
			temp[k]=onstr;
			if(k==2){
				break;
			}
			onstr=(char*)malloc(sizeof(char[4]));
			memset(onstr,0,4*sizeof(char));
			max=4;
			k++;
		}
		else{
			if(index==max){
				max+=20;
				onstr=(char*)realloc(onstr,sizeof(char[max]));
			}
			onstr[index]=str[i];
			index++;
		}
    }
    if(onstr[0]!=0){
		temp[k]=onstr;
    }
    return temp;
}//无参数指令请带上双空格

LapState *InitVM(char* path){
	FILE *fp=fopen(path,"r+");
	if(fp==NULL){
		printf("File %s Not Found\n",path);
		return NULL;
	}
	LapState *temp=(LapState*)malloc(sizeof(LapState));
	temp->GlobalVars=(LapObject**)malloc(sizeof(LapObject*[20]));
	temp->Stack=(LapObject**)malloc(sizeof(LapObject*[20]));
	temp->ConstVars=(LapObject**)malloc(sizeof(LapObject*[20]));
    temp->VarStacks=(LapObject***)malloc(sizeof(LapObject**[4]));
    temp->MaxVar=(int*)malloc(sizeof(int[4]));
    temp->VarNum=(int*)malloc(sizeof(int[4]));
    temp->PCStack=(int*)malloc(sizeof(int[4]));
    temp->Commands=(char***)malloc(sizeof(char**[20]));
    int i=0;
    for(;i<4;i++){
		temp->VarStacks[i]=(LapObject**)malloc(sizeof(LapObject*[20]));
		temp->MaxVar[i]=20;
		temp->VarNum[i]=0;
    }
    /*for(;i<20;i++){
		temp->Commands[i]=(char**)malloc(sizeof(char*[3]));
    }*/
    temp->Index=0;
    temp->ConstNum=0;
    temp->GlobalNum=0;
    temp->MaxPC=20;
    temp->TruePC=0;
    temp->PC=0;
    temp->MaxGlobal=20;
    temp->MaxConst=20;
    temp->MaxIndex=20;
    temp->Err=0;
    char* onstr=NULL;
    while(1){
        onstr=(char*)malloc(sizeof(char[40]));
        if(fgets(onstr,40,fp)==NULL){
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

void DeleteObject(LapObject *obj){
	int type=obj->Type,size=obj->Size,i=0;
	switch(type){
	case 0:
	free((int*)obj->Value);
	break;
	case 1:
	free((double*)obj->Value);
	break;
	case 2:
	free((char*)obj->Value);
	break;
	case 3:
	free((int*)obj->Value);
	break;
	case 4:
	for(;i<size;i++){
		DeleteObject(obj->Property[i]);
	}
	free(obj->Property);
	break;
	}
	free(obj);
}

void DeleteState(LapState *state){
	int i=0,max=state->ConstNum;
    for(;i<max;i++){
		DeleteObject(state->ConstVars[i]);
    }
    free(state->ConstVars);
    i=0,max=state->GlobalNum;
    for(;i<max;i++){
		DeleteObject(state->GlobalVars[i]);
    }
    free(state->GlobalVars);
    i=0,max=state->VarNum[0];
    for(;i<max;i++){
		DeleteObject(state->VarStacks[0][i]);
    }
    i=0,max=state->TruePC;
    for(;i<max;i++){
		free(state->Commands[i][0]);
		free(state->Commands[i][1]);
		free(state->Commands[i][2]);
        free(state->Commands[i]);
    }
    free(state->Commands);
    free(state->VarStacks[0]);
    free(state->VarStacks);
    free(state->VarNum);
    free(state->MaxVar);
    free(state->PCStack);
    free(state->Commands);
    free(state);
}

void PrintData(LapObject *obj){
	int type=obj->Type,i=0,size=obj->Size;
	void *data=obj->Value;
	char* onstr=NULL;
    switch(type){
	case 0:
	printf("%d",*(int*)data);
	break;
	case 1:
	printf("%f",*(double*)data);
	break;
	case 2:
	printf("%s",(char*)data);
	break;
	case 3:
	if(*(int*)data){
		printf("true");
	}
	else{
		printf("false");
	}
	break;
	case 4:
	printf("LapObject");
	break;
    }
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

int InsCmp(const char* str1,const char* str2){
	int i=0,temp=1,max=StrLen(str2);
	for(;i<max;i++){
		if(str1[i]!=str2[i]){
			temp=0;
			break;
		}
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
	char* temp=malloc(sizeof(char[max]));
	memset(temp,0,sizeof(temp));
	int i=0,index=0;
	char lastc=-1;
	for(;i<max;i++){
		if(str[i]==-60){
			break;
		}
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
	break;
	}
	env->ConstNum++;
}

void PushConst(LapState *env){
    char** ins=env->Commands[env->PC];
    int *i=ParseInt(ins[1]);
    if(env->MaxIndex==env->Index){
		env->MaxIndex+=20;
		env->Stack=(LapObject**)realloc(env->Stack,sizeof(LapObject*[env->MaxIndex]));
    }
    env->Stack[env->Index]=CreateObjectFromObject(env->ConstVars[*i]);
    env->Index++;
    free(i);
}

void Pop(LapState *env){
	env->Index--;
	DeleteObject(env->Stack[env->Index]);
}

void Calculate(LapState *env,int sign){
	env->Index--;
	LapObject *op2=env->Stack[env->Index];
	env->Index--;
	LapObject *op1=env->Stack[env->Index];
	int type=op1->Type;
	char* onstr=NULL;
	LapObject *temp=op1;
	double d1,d2;
	switch(sign){
		case '+':
		switch(type){
		case 0:
			(*(int*)op1->Value)+=*(int*)op2->Value;
			break;
		case 1:
			(*(double*)op1->Value)+=*(double*)op2->Value;
			break;
		case 2:
			type=op1->Size+op2->Size;
			onstr=(char*)malloc(sizeof(char[type]));
			memset(onstr,0,sizeof(char[type]));
			strcpy(onstr,(char*)op1->Value);
			strcpy(onstr+op1->Size,(char*)op2->Value);
			temp=CreateObject(2,type,onstr);
			DeleteObject(op1);
			break;
		}
		break;
		case '-':
		switch(type){
		case 0:
			(*(int*)op1->Value)-=*(int*)op2->Value;
			break;
		case 1:
			(*(double*)op1->Value)-=*(double*)op2->Value;
			break;
		}
		break;
		case '*':
		switch(type){
		case 0:
			(*(int*)op1->Value)*=*(int*)op2->Value;
			break;
		case 1:
			(*(double*)op1->Value)*=*(double*)op2->Value;
			break;
		}
		break;
		case '/':
		if(!*(double*)op2->Value){
			env->Err=1;
			break;
		}
		switch(type){
		case 0:
			(*(int*)op1->Value)/=*(int*)op2->Value;
			break;
		case 1:
			(*(double*)op1->Value)/=*(double*)op2->Value;
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
				(*(int*)op1->Value)%=*(int*)op2->Value;
				break;
			case 1:
				d1=(*(double*)op1->Value);
				d2=(*(double*)op2->Value);
				*((double*)op1->Value)=d1-(int)(d1/d2)*d2;
				break;
		}
		break;
	}
	DeleteObject(op2);
	env->Stack[env->Index]=temp;
	env->Index++;
}

void Print(LapState *env){
	env->Index--;
    PrintData(env->Stack[env->Index]);
    DeleteObject(env->Stack[env->Index]);
}

void GetCommandArg(LapState *env){
    char** ins=env->Commands[env->PC];
    int *i=ParseInt(ins[1]);
    if(*i>args->Size-1){
		env->Err=2;
		return;
    }
    if(env->MaxIndex==env->Index){
		env->MaxIndex+=20;
		env->Stack=(LapObject**)realloc(env->Stack,sizeof(LapObject*[env->MaxIndex]));
    }
    env->Stack[env->Index]=CreateObjectFromObject(args->Property[*i]);
    env->Index++;
    free(i);
}

void DoIns(LapState *env){
	char** ins=env->Commands[env->PC];
    if(InsCmp(ins[0],"add_const_int")){
        AddConst(env,0);
    }
    if(InsCmp(ins[0],"add_const_float")){
        AddConst(env,1);
    }
    else if(InsCmp(ins[0],"add_const_str")){
        AddConst(env,2);
    }
    else if(InsCmp(ins[0],"push_const")){
		PushConst(env);
    }
    else if(InsCmp(ins[0],"add")){
		Calculate(env,'+');
    }
    else if(InsCmp(ins[0],"sub")){
		Calculate(env,'-');
    }
    else if(InsCmp(ins[0],"mul")){
		Calculate(env,'*');
    }
    else if(InsCmp(ins[0],"div")){
		Calculate(env,'/');
    }
    else if(InsCmp(ins[0],"mod")){
		Calculate(env,'%');
    }
    else if(InsCmp(ins[0],"print")){
		Print(env);
    }
    else if(InsCmp(ins[0],"get_command_arg")){
		GetCommandArg(env);
    }
    else{//小于0虚拟机问题 大于0编程问题
		env->Err=-1;
		printf("unrecognized ins:%s\n",ins[0]);
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
	int i=1;
	for(;i<argc;i++){
		args->Property[i-1]=CreateObject(2,StrLen(argv[i]),argv[i]);
	}
	LapState *env=InitVM(argv[1]);
	if(env==NULL){
		printf("Init Failed\n");
		system("pause");
		DeleteObject(args);
		return -1;
	}
	else if(StartVM(env)){
		i=env->Err;
		switch(i){
		case 1:
			printf("Err:Div 0!\n");
			break;
		case 2:
			printf("Err:Index out of range!\n");
			break;
		}
	}
	DeleteObject(args);
	DeleteState(env);
	system("pause");
}
