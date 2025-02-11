#include <stdlib.h>

struct vector{
	int curelements;
	int capacity;
	int* entries;
};

typedef struct vector vector;
typedef vector* Vector;


void vec_init(Vector vec);

void vec_insert(Vector vec, int num);

void vec_destroy(Vector vec);
