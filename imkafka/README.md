# imkafka

Tryout rsyslog input avro module for Apache kafka, [imkafka](http://www.rsyslog.com/doc/master/configuration/modules/imkafka.html) on Docker container. Do NOT use this in production, it is only a very basic PoC!

## Build

```bash
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

Login to rsyslog omkafka docker container with link to kafka broker,

```bash
$ docker run --rm -it --link kafka:kafka tcnksm/omkafka bash
```

Check kafka broker host it's provided via env var,

```bash
# env
```

Run rsyslog and send log by hand in omkafka,

```bash
# rsyslog
# logger 'Hello, world'
```

Check the log can be consumed, you can find `Hello world` message. 

```bash
$ kafkacat -C -t omkafka -b ${DOCKER_HOST}:9092
```

