#ifndef COMMON_TYPES
#define COMMON_TYPES

typedef void *Pointer;

typedef char *String;

typedef int (*CompareFunc)(Pointer a, Pointer b);   // bale const Pointer const a

typedef void (*DestroyFunc)(Pointer value);

#endif      // COMMON_TYPES