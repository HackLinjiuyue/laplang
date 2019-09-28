#include "CLap.h"

LapObject* System(LapObject* args){
	int *temp=malloc(sizeof(int));
	*temp=system((char*)args->Property[0]->Value);
	return CreateObject(Lint,0,temp,NULL);
}
