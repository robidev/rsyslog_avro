#ifndef INSERTAVRO_H_INCLUDED
#define INSERTAVRO_H_INCLUDED

#include <avro.h>
#include <stdio.h>
#include <stdlib.h>

/* returns 0 if ok */
int init_writer(avro_writer_t* w, char ** buf, int size);

void destroy_writer(avro_writer_t w, char ** buf);

/* return size if ok, returns 0 if failed */
int convert_to_avro(avro_writer_t w, const char *msg);

#endif /* #ifndef INSERTAVRO_H_INCLUDED */