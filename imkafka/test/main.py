"""
Starts the SYSLOG OT Test adapter
"""
import os
import uvicorn

from starlette.middleware.cors import CORSMiddleware
from pydantic import BaseModel
from fastapi import FastAPI

from pyspark.sql import DataFrame

from io import BytesIO

from typing import List

from abr.adapters.kafka import (
    KafkaOutputStream,
    KafkaConfiguration
)

from pyspark.sql.types import (
    BinaryType,
    StringType,
    StructType,
    StructField
)

from avro.io import (
    DatumWriter,
    BinaryEncoder
)

from avro.schema import Parse

from abr.adapters.base import INFO

SYSLOG_AVRO_SCHEMA = Parse(
    open(os.getenv("SYSLOG_AVRO_SCHEMA"), "r").read())

SPARK_SCHEMA = StructType([
    StructField('topic', StringType()),
    StructField('value', BinaryType())
])


# --- LOAD CONFIGURATION ---
KAFKA_CONFIG = KafkaConfiguration.load()
TOPIC = os.getenv("TOPIC")

class MessageModel(BaseModel):
    """
    Represents a message
    """
    syslogmessage: str
    time: str

    def to_avro_dict(self):
        return {
            "syslogmessage": self.syslogmessage , "time": self.time
        }

    def to_avro(self):
        writer = DatumWriter()
        writer.set_writer_schema(SYSLOG_AVRO_SCHEMA)
        buffer = BytesIO()
        encoder = BinaryEncoder(buffer)
        writer.write(self.to_avro_dict(), encoder)
        data = buffer.getbuffer().tobytes()
        return bytearray(data)


# --- THE ADAPTER ---
class Adapter(KafkaOutputStream):
    """
    This adapter takes data from syslog, and transforms it towards
    Kafka.
    """

    def __init__(self):
        self.log(INFO, '########## Starting Adapter ##########')
        super().__init__(KAFKA_CONFIG)
        self.current_data = []

    def input(self):
        rdd = self.spark.sparkContext.parallelize(self.current_data)
        self.current_data = []
        return [self.spark.createDataFrame(rdd, SPARK_SCHEMA)]

    def filter(self, dataframes: [DataFrame]) -> [DataFrame]:
        """
        Filters the dataframes, drops data which can not be processed properly
        into the Avro schema, this will be logged.

        :param dataframes: The `Spark` dataframes which are collected by the
                           input
        """
        for dataframe in dataframes:
            for data in dataframe.collect():
                self.log(INFO, f"Processing {data}")
        return dataframes


app = FastAPI(
    title="Syslog OT Adapter Test Driver API",
    description="Syslog OT Adapter Test Driver API",
    version="0",
    docs_url="/")

app.add_middleware(CORSMiddleware, allow_origins=["*"])

test_adapter = Adapter()


@app.post("/messages")
def post_syslog(msgs: List[MessageModel]):
    """
    Processes multiple messages, sends them to Kafka
    """
    test_adapter.current_data = [(TOPIC, msg.to_avro()) for msg in msgs]
    test_adapter.stream(test_adapter.input())


if __name__ == '__main__':
    uvicorn.run(app, host="0.0.0.0", port=5000, log_level="info")
