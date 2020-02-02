#include <avro.h>
#include <stdio.h>
#include <stdlib.h>
#include "insertavro.h"

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
int init_writer(avro_writer_t* w, char ** buf, int size)
{
        /* Initialize the schema structure from JSON */
        if (avro_schema_from_json_literal(SYSLOG_SCHEMA, &syslog_schema)) {
                fprintf(stderr, "Unable to parse syslog schema\n");
                return -1;
        }

        /*malloc data for writing */
        if(size == 0) {
                fprintf(stderr, "Invalid buffer Size\n");
                return -1;
        }

        /* check buffer allocation prerequisites */
        if(buf == 0) {
                fprintf(stderr, "Invalid buffer pointer\n");
                return -1;
        }

        /* allocate buffer */
        *buf = calloc(size);
        if(*buf == 0){
                fprintf(stderr, "Could not allocate memory\n");
                return -1;
        }

        /* create avro data writer */
        *w = avro_writer_memory(*buf, size);
        if(*w == 0){
                fprintf(stderr, "Could not allocate avro writer\n");
                return -1;
        }
        return 0;
}

void destroy_writer(avro_writer_t w, char ** buf)
{
        /* free the writer */
		    avro_writer_free(w);        
        /* free the buffer */
        free(*buf);
        /* We don't need this schema anymore */
        avro_schema_decref(syslog_schema);
}


/* Create a datum to match the syslog schema and save it */
int convert_to_avro(avro_writer_t w, const char *msg)
{
        int size = 0;
        avro_datum_t syslog_record = avro_record(syslog_schema);
        avro_datum_t msg_datum = avro_string(msg);
        avro_datum_t time_datum = avro_string("00:00:00");

        if (avro_record_set(syslog_record, "syslogmessage", msg_datum)) {
                fprintf(stderr, "Unable to create syslog datum structure\n");
                return 0;
        }
        if (avro_record_set(syslog_record, "time", time_datum)) {
                fprintf(stderr, "Unable to create syslog datum structure\n");
                return 0;
        }

        if (avro_write_data(w, syslog_schema, syslog_record) != 0) {
                fprintf(stderr,
                        "Unable to write syslog datum to memory buffer\nMessage: %s\n", avro_strerror());
                return 0;
        }
        
        size = avro_writer_tell(w);

        /* Decrement all our references to prevent memory from leaking */
        avro_datum_decref(time_datum);
        avro_datum_decref(msg_datum);
        avro_datum_decref(syslog_record);
        
        return size;
}

