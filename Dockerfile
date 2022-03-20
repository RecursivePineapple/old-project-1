
FROM ubuntu:latest

COPY lib/uSockets.deb lib/uWebSockets.deb lib/rabbitmq-c-0.12.0-Linux.deb /tmp/

RUN apt-get update && \
    apt-get install -y libpq5 openssl && \
    dpkg -i /tmp/uSockets.deb && \
    dpkg -i /tmp/uWebSockets.deb && \
    dpkg -i /tmp/rabbitmq-c-0.12.0-Linux.deb && \
    apt-get clean

RUN mkdir -p /opt/server /opt/server/bin
COPY bin/server /opt/server/bin/

ENTRYPOINT [ "/opt/server/bin/server" ]
