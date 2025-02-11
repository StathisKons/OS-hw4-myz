#include <stdlib.h>

#define DEF_CAP 5

struct vector{
	int curelements;
	int capacity;
	int* entries;
};

typedef struct vector vector;
typedef vector* Vector;


void vec_init(Vector vec)
{
	vec->capacity = DEF_CAP;
	vec->curelements = 0;
	vec->entries = (int*)malloc(sizeof(int) * DEF_CAP);
}

static void expand(Vector vec)
{
	vec->capacity *= 2;
	vec->entries = realloc(vec->entries, vec->capacity);
}

void vec_insert(Vector vec, int num)
{
	vec->entries[vec->curelements] = num;
	vec->curelements++;
	if(vec->curelements == vec->capacity)
		expand(vec);
}


void vec_destroy(Vector vec)
{
	free(vec->entries);
}



