"""
This is a simple test adapter to introspect received messages
"""

import os

from io import BytesIO
from abr.adapters.base import INFO, ERROR

from abr.adapters.kafka import (
    KafkaInput,
    KafkaConfiguration
)

from avro.schema import Parse

from avro.io import (
    DatumReader,
    BinaryDecoder
)

from pyspark.sql import DataFrame

# --- LOAD CONFIGURATION ---

KAFKA_CONFIG = KafkaConfiguration.load()
TOPIC = os.getenv('TOPIC')
SYSLOG_AVRO_SCHEMA = Parse(open(os.getenv("SYSLOG_AVRO_SCHEMA"), "r").read())

class Adapter(KafkaInput):
    """
    Performs the transformation of input to output
    """

    def __init__(self):
        super().__init__(KAFKA_CONFIG, TOPIC)

    def filter(self, dataframes: [DataFrame]) -> [DataFrame]:
        """
        Filters and converts the given data to the proper output dataframe
        model

        :param dataframes: Containing the dataframes to transform
        :return: Dataframes to log
        """
        return dataframes

    def output(self, dataframes: [DataFrame]):
        """
        Emits the transformed data from the filter to a log

        :param dataframes: The dataframes to log
        """
        syslog_reader = DatumReader(SYSLOG_AVRO_SCHEMA, SYSLOG_AVRO_SCHEMA)

        for dataframe in dataframes:
            for row in dataframe.collect():
                buffer = BytesIO()
                buffer.write(row.value)
                buffer.flush()
                try:
                    buffer.seek(0)
                    decoder = BinaryDecoder(buffer)
                    output = syslog_reader.read(decoder)
                    self.log(INFO, f"Received AVRO syslog -> { output }")
                    #self.log(INFO, f"Received AVRO syslog -> { row.value }")
                except BaseException as e:
                    try:
                        buffer.seek(0)
                        decoder = BinaryDecoder(buffer)
                        output = syslog_reader.read(decoder)
                        self.log(INFO, f"ERROR Received AVRO feedback -> {output}")
                    except BaseException as e:
                        self.log(ERROR, f"{e}")

if __name__ == '__main__':
    Adapter().process()
