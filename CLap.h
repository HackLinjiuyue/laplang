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
	Lnative
};

typedef struct LapObject{
	struct LapObject** Property;
	void *Value;
	int Type;
	int Size;
	int Protect;
	int MaxSize;
	int Ref;
}LapObject;

LapObject *CreateObject(int type,int size,void* value){
	LapObject *temp=malloc(sizeof(LapObject));
	temp->Type=type;
	temp->Size=size;
	temp->Value=value;
	temp->MaxSize=4;
	temp->Protect=0;
	temp->Ref=0;
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
		temp->Value=malloc(sizeof(char[size+3]));
		memset(temp->Value,0,sizeof(char[size+3]));
		break;
		case 3:
		temp->Value=malloc(sizeof(int));
		break;
		case 4:
		temp->Property=(LapObject**)malloc(sizeof(LapObject*[size]));
		for(;i<size;i++){
			temp->Property[i]=NULL;
		}
		temp->Protect=1;
		break;
		default:
		temp->Value=value;
		//case 5:文件指针 6动态库句柄 7:原生函数
		}
	}
	return temp;
}

LapObject *CreateObjectFromObject(LapObject *obj){
	LapObject* temp=NULL;
	if(obj!=NULL){
		int size=obj->Size,i=0;
		int type;
		if(obj->Protect){
			temp=obj;
			obj->Ref++;
		}
		else{
			temp=CreateObject(obj->Type,size,NULL);
			temp->Ref=1;
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
			break;
			case 4:
			for(;i<size;i++){
				if(obj->Property[i]==NULL){
					continue;
				}
				temp->Property[i]->Protect=0;
				temp->Property[i]=CreateObjectFromObject(obj->Property[i]);
				temp->Property[i]->Protect=1;
			}
			break;
			default:
			temp->Value=obj->Value;
			}
		}
	}
	return temp;
}

char* ConcatStr(char* s1,char* s2,int l1,int l2){
	int type=l1+l2;
	char* onstr=(char*)malloc(sizeof(char[type+3]));
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
		for(;i<obj->Size;i++){
			PrintData(obj->Property[i]);
			printf(",");
		}
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
		}
	}
}

void FreeObject(LapObject *obj){
	if(obj!=NULL){
		if(!obj->Ref){
									//system("pause");
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
				case 4:
				for(;i<size;i++){
					FreeObject(obj->Property[i]);
				}
				break;
				}
			}
			free(obj);
		}
		else{
			if(!--obj->Ref){
				FreeObject(obj);
			}
		}
	}
}
