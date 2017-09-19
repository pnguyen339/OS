
#include <string.h>
#include <stdlib.h>
#include "vector.h"

#define MIN_SIZE 8




Vector* vector_constructor(int size) 
{
	if(size <= MIN_SIZE)
	{
		size = MIN_SIZE;
	}

	Vector* vec = malloc(sizeof(Vector));
	vec -> len = 0;
	vec -> size = size;
	vec-> data = malloc(sizeof(void*) * size);
	for(int i = 0; i < size; i++)
    {
        vec->data[i] = NULL;    // Null init
    }

    return vec;

}

void vector_realloc(Vector *this,  int size) {
	if(size <= this -> size)
	{
		return;
	}

	void** oldData = this -> data;
	int oldSize = this -> size;

	void** newData = malloc(sizeof(void*) * size);
	int i = 0;

	for(; i < oldSize; i++)
    {
        newData[i] = oldData[i];  //set to old void*
    }
    for(; i < size; i++)
    {
        newData[i] = NULL;    //null init
    }

    this -> data = newData;
    this -> size = size;
    free(oldData);
}



void vector_appendE(Vector *vec,void *data)
{
	if(vec -> len == vec -> size)
	{
		vector_realloc(vec, vec -> size * 2);
	}

	vec -> data[vec -> len] = data;
	vec -> len = vec -> len + 1;

}

int vector_insertE(Vector *vec,void *data, int index)
{
	if(index >= vec -> len)
	{
		return -1;
	}

	vec -> data[index] = data;
	return 0;

}

void* vector_get(Vector* this, int index)
{
	if(index < 0 && index >= this -> len)
		return NULL;
	else
		return this -> data[index];
}

int vector_len(Vector* this)
{
	return this -> len;
}

void vector_delete(Vector* this)
{
    free(this->data);   //free the array
    free(this);          //free the pointer
}



