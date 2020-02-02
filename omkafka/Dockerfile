FROM buildpack-deps:buster

RUN apt-get update && apt-get install -y \
              git \
              libtool \
              libz-dev \
              libjson-c-dev \
              libgcrypt-dev \
              libestr-dev \
              flex \
              bison \
              python-docutils \
              libavro-dev \
              libfastjson-dev \
              libcurl4-gnutls-dev \
         && rm -rf /var/lib/apt/lists/*

RUN git clone https://github.com/edenhill/librdkafka /tmp/librdkafka
RUN cd /tmp/librdkafka && ./configure --libdir=/usr/lib \
                                      --includedir=/usr/include \
                       && make \
                       && make install

RUN git clone https://github.com/rsyslog/rsyslog /tmp/rsyslog
RUN cd /tmp/rsyslog  && git checkout -b v8.1911.0 refs/tags/v8.1911.0 

COPY ./omkafka /tmp/rsyslog/plugins/omkafka

RUN cd /tmp/rsyslog  && ./autogen.sh --enable-omkafka \
                                     --enable-imkafka \
                                     --disable-uuid \
                                     --disable-liblogging_stdlog \
                                     --disable-generate-man-pages \
                                     --prefix=/usr \
                     && make \
                     && make install

ADD etc/rsyslog.conf /etc/rsyslog.conf
ADD etc/rsyslog.d/00-omkafka.conf /etc/rsyslog.d/00-omkafka.conf
ENV RSYSLOG_DEBUG Debug
CMD ["rsyslogd","-n"]

