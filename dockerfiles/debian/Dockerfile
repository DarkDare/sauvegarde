FROM debian:latest
MAINTAINER Olivier Delhomme <olivier.delhomme@free.fr>
ENV PKG_CONFIG_PATH=/usr/local/lib/pkgconfig/
RUN apt-get -y update; \
    apt-get -y upgrade; \
    apt-get -y install apt-utils; \
    apt-get -y install bash git autoconf automake libtool make libsqlite3-dev  libglib2.0-dev libmicrohttpd-dev libcurl4-gnutls-dev intltool; \
    git clone git://github.com/akheron/jansson.git; cd jansson; autoreconf -f -i; ./configure --enable-silent-rules; make; make install; cd ..; rm -fr jansson;

# Compiling sauvegarde project.
RUN git clone https://github.com/dupgit/sauvegarde.git; cd sauvegarde; ./autogen.sh && ./configure && make && make check && make install;
