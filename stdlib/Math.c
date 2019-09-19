#include<math.h>
#include "CLap.h"

LapObject* Floor(LapObject* args){
	LapObject* temp=CreateObject(1,0,malloc(sizeof(double)));
	*(double*)temp->Value=floor(*(double*)args->Property[0]->Value);
	return temp;
}
LapObject* Pow(LapObject* args){
	LapObject* temp=CreateObject(1,0,malloc(sizeof(double)));
	*(double*)temp->Value=pow(*(double*)args->Property[0]->Value,*(double*)args->Property[1]->Value);
	return temp;
}
LapObject* Sin(LapObject* args){
	LapObject* temp=CreateObject(1,0,malloc(sizeof(double)));
	*(double*)temp->Value=sin(*(double*)args->Property[0]->Value);
	return temp;
}
LapObject* Cos(LapObject* args){
	LapObject* temp=CreateObject(1,0,malloc(sizeof(double)));
	*(double*)temp->Value=cos(*(double*)args->Property[0]->Value);
	return temp;
}
LapObject* Tan(LapObject* args){
	LapObject* temp=CreateObject(1,0,malloc(sizeof(double)));
	*(double*)temp->Value=tan(*(double*)args->Property[0]->Value);
	return temp;
}
LapObject* aSin(LapObject* args){
	LapObject* temp=CreateObject(1,0,malloc(sizeof(double)));
	*(double*)temp->Value=asin(*(double*)args->Property[0]->Value);
	return temp;
}
LapObject* aCos(LapObject* args){
	LapObject* temp=CreateObject(1,0,malloc(sizeof(double)));
	*(double*)temp->Value=acos(*(double*)args->Property[0]->Value);
	return temp;
}
LapObject* aTan(LapObject* args){
	LapObject* temp=CreateObject(1,0,malloc(sizeof(double)));
	*(double*)temp->Value=atan(*(double*)args->Property[0]->Value);
	return temp;
}
LapObject* aTan2(LapObject* args){
	LapObject* temp=CreateObject(1,0,malloc(sizeof(double)));
	*(double*)temp->Value=atan2(*(double*)args->Property[0]->Value,*(double*)args->Property[1]->Value);
	return temp;
}
LapObject* Sinh(LapObject* args){
	LapObject* temp=CreateObject(1,0,malloc(sizeof(double)));
	*(double*)temp->Value=sinh(*(double*)args->Property[0]->Value);
	return temp;
}
LapObject* Cosh(LapObject* args){
	LapObject* temp=CreateObject(1,0,malloc(sizeof(double)));
	*(double*)temp->Value=cosh(*(double*)args->Property[0]->Value);
	return temp;
}
LapObject* Tanh(LapObject* args){
	LapObject* temp=CreateObject(1,0,malloc(sizeof(double)));
	*(double*)temp->Value=tanh(*(double*)args->Property[0]->Value);
	return temp;
}
LapObject* Log(LapObject* args){
	LapObject* temp=CreateObject(1,0,malloc(sizeof(double)));
	*(double*)temp->Value=log(*(double*)args->Property[0]->Value);
	return temp;
}
LapObject* Log10(LapObject* args){
	LapObject* temp=CreateObject(1,0,malloc(sizeof(double)));
	*(double*)temp->Value=log10(*(double*)args->Property[0]->Value);
	return temp;
}
LapObject* Exp(LapObject* args){
	LapObject* temp=CreateObject(1,0,malloc(sizeof(double)));
	*(double*)temp->Value=exp(*(double*)args->Property[0]->Value);
	return temp;
}
LapObject* Sqrt(LapObject* args){
	LapObject* temp=CreateObject(1,0,malloc(sizeof(double)));
	*(double*)temp->Value=sqrt(*(double*)args->Property[0]->Value);
	return temp;
}
LapObject* Ceil(LapObject* args){
	LapObject* temp=CreateObject(1,0,malloc(sizeof(double)));
	*(double*)temp->Value=ceil(*(double*)args->Property[0]->Value);
	return temp;
}
LapObject* Fabs(LapObject* args){
	LapObject* temp=CreateObject(1,0,malloc(sizeof(double)));
	*(double*)temp->Value=fabs(*(double*)args->Property[0]->Value);
	return temp;
}
LapObject* Abs(LapObject* args){
	LapObject* temp=CreateObject(0,0,malloc(sizeof(int)));
	*(int*)temp->Value=abs(*(int*)args->Property[0]->Value);
	return temp;
}
LapObject* Hypot(LapObject* args){
	LapObject* temp=CreateObject(1,0,malloc(sizeof(double)));
	*(double*)temp->Value=hypot(*(double*)args->Property[0]->Value,*(double*)args->Property[1]->Value);
	return temp;
}
LapObject* Ldexp(LapObject* args){
	LapObject* temp=CreateObject(1,0,malloc(sizeof(double)));
	*(double*)temp->Value=ldexp(*(double*)args->Property[0]->Value,*(double*)args->Property[1]->Value);
	return temp;
}
