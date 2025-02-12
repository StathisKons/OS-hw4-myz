#ifndef SYS_UTILS_H
#define SYS_UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define to_str(a) #a
#define to_string(a) to_str(a)      /* to expand __LINE__, else it would print __LINE__*/

#define safe_sys(FUNCTION_CALL)                               \
do{                                                           \
    if(FUNCTION_CALL == -1){                                  \
        char function_name[] = #FUNCTION_CALL;                \
        function_name[strcspn(function_name, "( \t")] = '\0'; \
        fprintf(stderr, "%s", function_name);                 \
        perror("@" __FILE__ ":" to_string(__LINE__) );        \
        exit(EXIT_FAILURE);                                   \
    }                                                         \
} while(0);   


// if successful assigned the return value of FUNCTION_CALL to result
#define safe_sys_assign(result, FUNCTION_CALL)                \
do{                                                           \
    (result) = (FUNCTION_CALL);                               \
    if((result) == -1){                                       \
        char function_name[] = #FUNCTION_CALL;                \
        function_name[strcspn(function_name, "( \t")] = '\0'; \
        fprintf(stderr, "%s", function_name);                 \
        perror("@" __FILE__ ":" to_string(__LINE__) );        \
        exit(EXIT_FAILURE);                                   \
    }                                                         \
} while(0);                                                   \


// #define safe_fork(PID) (safe_sys_assign(PID, fork())) 

void *safe_malloc(size_t size);

void *safe_realloc(void *ptr, size_t size);


#endif // SYS_UTILS_H