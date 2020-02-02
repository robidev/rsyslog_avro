#include <avro.h>
#include <stdio.h>
#include <stdlib.h>
#include "extractavro.h"

avro_schema_t syslog_schema;

/* syslog schema */
const char  SYSLOG_SCHEMA[] =
"{\"type\":\"record\",\
  \"name\":\"SyslogToITEvent\",\
  \"namespace\":\"alliander.events\",\
  \"doc\":\"Event production for OT syslog which needs to be send to IT, this event will trigger for each syslog message, data is not allowed to alter.\",\
  \"fields\":[{\
    \"name\":\"syslogmessage\",\
    \"type\":\"string\",\
    \"doc\":\"the raw syslog message\"},{\
    \"name\":\"time\",\
    \"type\":\"string\",\
    \"doc\":\"the timestamp\"\
  }]\
}";


/* returns 0 if ok */
int init_schema(void)
{
        /* Initialize the schema structure from JSON */
        if (avro_schema_from_json_literal(SYSLOG_SCHEMA, &syslog_schema)) {
                fprintf(stderr, "imkafka - avro: Unable to parse syslog schema\n");
                return -1;
        }
        return 0;
}

void destroy_schema(void)
{
        /* We don't need this schema anymore */
        avro_schema_decref(syslog_schema);
}


/* Create a datum to match the syslog schema and save it */
int convert_from_avro(char *buf, int size, char **msg, size_t * msg_size)
{
        avro_value_iface_t *reader_class;
        avro_reader_t r;
        avro_value_t nested;
        avro_value_t element;
        avro_type_t val_type;//array/element type

        const char * msg_temp = NULL;
        size_t length;//array length
        const char *unused;


        if(buf == NULL || size == 0 || msg == NULL) {
              fprintf(stderr, "imkafka - avro: invalid argument\n");
              return -1;
        }

        reader_class = avro_generic_class_from_schema(syslog_schema);

        if(avro_generic_value_new( reader_class, &nested ) != 0) {
              fprintf(stderr, "imkafka - avro: Error creating instance of nested array" );
        }

        /* create avro data writer */
        r = avro_reader_memory(buf, size);
        if(r == 0){
              fprintf(stderr, "imkafka - avro: Could not allocate avro reader\n");
              return -1;
        }

        if(avro_value_read(r, &nested ) != 0) {
              fprintf(stderr, "imkafka - avro: Could not read value from memory\n");
        }
        
        //print_array_fields( &nested );
        val_type = avro_value_get_type( &nested );
        if(val_type != AVRO_RECORD) { // AVRO_RECORD=8
             fprintf(stderr, "imkafka - avro: wrong array type\n" );
             return -1;
        }

        if( avro_value_get_size( &nested, &length ) != 0){
             fprintf(stderr, "imkafka - avro: Couldn't get array size\n" );
             return -1;
        }
        if( length != 2) {
             fprintf(stderr, "imkafka - avro: Incorrect array length\n" );
             return -1;
        }
        
        if ( avro_value_get_by_index( &nested, 0, &element, &unused  ) != 0){
              fprintf(stderr, "imkafka - avro: Couldn't get array element\n" );
              return -1;
        }

        val_type = avro_value_get_type( &element );
        if(val_type != AVRO_STRING) { // AVRO_STRING=0
             fprintf(stderr, "imkafka - avro: wrong type, string expected\n" );
             return -1;
        }

        if ( avro_value_get_string( &element, &msg_temp, msg_size ) != 0){
              fprintf(stderr, "imkafka - avro: Couldn't get array element value\n" );
              return -1;
        }
        fprintf(stdout, "imkafka - avro: value = %s, size = %d\n", msg_temp, (unsigned int)*msg_size );
        
        *msg = calloc(*msg_size,sizeof(msg_temp));
        if(*msg == NULL) {
              fprintf(stderr, "imkafka - avro: Couldn't allocate memory\n" );
              return -1;
        }
        memcpy(*msg,msg_temp, *msg_size);        

        avro_value_decref( &element ); 
        avro_value_decref( &nested );      
        avro_reader_free( r );
        avro_value_iface_decref( reader_class );
        return 0;
}


