#ifndef VECTOR_H
#define VECTOR_H

typedef struct vector_str
{
	int len;
	int size;
	void** data;	
} Vector;

Vector* vector_constructor(int size);
void vector_realloc(Vector *this,  int size);
void vector_appendE(Vector *vec,void *data);
void* vector_get(Vector* this, int index);


#endif