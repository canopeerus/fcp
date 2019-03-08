#include <stdio.h>
#include <stdbool.h>

#define GREEN "\033[0;32m"
#define RESET "\033[0m"
#define BLU "\033[1;34m"

typedef enum
{
    FILE_OP,
    EXIT_OP,
    MSG_OP
} op_t;


typedef struct
{
    char filename[50];
    char buf[BUFSIZ];
    size_t b_size;
    char msg_str[100];
    op_t op;
    bool done;
} filechunk_t;
