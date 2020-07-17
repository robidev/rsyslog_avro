# rsyslog_avro
rsyslog imkafka and omkafka modification to support AVRO schema

Tryout rsyslog avro module for Apache kafka, on Docker container. Do NOT use this in production. It is only a PoC and prone to memory leaks.

## Build

```bash
$ docker build -t tcnksm/omkafka .
$ docker build -t tcnksm/imkafka .
```

## Usage

Start kafka broker,

```bash
$ docker run --rm -it \
    --name kafka \
    --publish 9092:9092 \
    --env ADVERTISED_HOST=${DOCKER_HOST} \
    --env ADVERTISED_PORT=9092 \
    tcnksm/single-kafka
```

Login to rsyslog docker container with link to kafka broker,

```bash
$ docker run --rm -it --link kafka:kafka tcnksm/omkafka bash
```

Check kafka broker host it's provided via env var,

```bash
# env
```

Run rsyslog and send log by hand,

```bash
# rsyslog
# logger 'Hello, world'
```

Check the log can be consumed, you can find `Hello world` message. 

```bash
$ kafkacat -C -t omkafka -b ${DOCKER_HOST}:9092
```

