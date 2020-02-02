#ifndef EXTRACTAVRO_H_INCLUDED
#define EXTRACTAVRO_H_INCLUDED

#include <avro.h>
#include <stdio.h>
#include <stdlib.h>

/* returns 0 if ok */
int init_schema(void);

void destroy_schema(void);

/* return size if ok, returns 0 if failed */
int convert_from_avro(char *buf, int size, char **msg, size_t * msg_size);

#endif /* #ifndef EXTRACTAVRO_H_INCLUDED */