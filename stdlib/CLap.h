#include<stdlib.h>
#include<stdio.h>
#include<string.h>

#define MAX_STACK 256
#define MAX_ARG_NUM 10

enum LapType{
	Lint=0,
	Lfloat,
	Lstring,
	Lbool,
	Lobject,
	Lfile,
	Lhandle,
	Lnative,
	Lfunc
};

typedef struct LapObject{
	struct LapObject** Property;
	void *Value;
	int Type;
	int Size;
	int MaxSize;
	int Ref;
	void* Ori;
}LapObject;

LapObject *CreateObject(int type,int size,void* value){
	LapObject *temp=malloc(sizeof(LapObject));
	temp->Type=type;
	temp->Size=size;
	temp->Value=value;
	temp->MaxSize=4;
	temp->Ref=0;
	temp->Ori=NULL;
	char onstr[1];
	int i=0;
	if(value==NULL){
		switch(type){
		case 0:
		temp->Value=malloc(sizeof(int));
		break;
		case 1:
		temp->Value=malloc(sizeof(double));
		break;
		case 2:
		temp->Value=calloc(size+3,sizeof(char));
		break;
		case 3:
		temp->Value=malloc(sizeof(int));
		break;
		case 4:
		temp->Property=(LapObject**)calloc(size,sizeof(LapObject*));
		temp->Ref=1;
		break;
		//case 5:文件指针 6动态库句柄 7:原生函数
		}
	}
	return temp;
}

LapObject *CreateObjectFromObject(LapObject *obj){
	LapObject* temp=NULL;
	if(obj!=NULL){
		int size=obj->Size,i=0;
		int type=obj->Type;
		if(type==2||(type>3)){
			temp=obj;
			obj->Ref++;
		}
		else{
			temp=CreateObject(obj->Type,size,NULL);
			temp->Ori=obj->Value;
			temp->Ref=1;
			switch(type){
			case 0:
			*(int*)temp->Value=*(int*)obj->Value;
			break;
			case 1:
			*(double*)temp->Value=*(double*)obj->Value;
			break;
			case 3:
			*(int*)temp->Value=*(int*)obj->Value;
			break;
			}
		}
	}
	return temp;
}

char* ConcatStr(char* s1,char* s2,int l1,int l2){
	int type=l1+l2;
	char* onstr=(char*)calloc(type+3,sizeof(char));
	memset(onstr,0,sizeof(char[type+3]));
	strcpy(onstr,s1);
	strcpy(onstr+l1,s2);
	return onstr;
}

void PrintData(LapObject *obj){
	if(obj==NULL){
		printf("null");
	}
	else{
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
		printf("[");
		for(;i<obj->Size-1;i++){
			PrintData(obj->Property[i]);
			printf(",");
		}
		if(obj->Size){
			PrintData(obj->Property[i]);
		}
		printf("]");
		break;
		case 5:
		printf("File");
		break;
		case Lhandle:
		printf("DLL");
		break;
		case Lnative:
		printf("Lap NativeFunction");
		break;
		case Lfunc:
		printf("Lap Function at %d",*(int*)obj->Value);
		break;
		}
	}
}

void FreeObject(LapObject *obj){
	if(obj!=NULL){
		if(!obj->Ref){
			int type=obj->Type,size=obj->Size,i=0;
			if(obj->Value!=NULL){
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
				case Lfunc:
				free((int*)obj->Value);
				break;
				}
			}
			else{
				if(obj->Type==4){
					for(;i<size;i++){
					if(obj->Property[i]==NULL){
						continue;
					}
					FreeObject(obj->Property[i]);
					}
					free(obj->Property);
				}
			}
			free(obj);
		}
		else{
			--obj->Ref;
			if(!obj->Ref){
				FreeObject(obj);
			}
		}
	}
}
