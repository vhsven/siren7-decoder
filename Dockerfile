FROM gcc
RUN apt-get update && apt-get install libtool-bin
WORKDIR /app
RUN mkdir -p /app/share
RUN git clone https://github.com/kakaroto/libsiren.git
RUN cd libsiren && ./autogen.sh && make && make install
# hack because of improper install:
COPY libsiren /usr/local/include/libsiren
ENV LD_LIBRARY_PATH=/usr/local/lib
COPY main.c .
RUN libtool --mode=compile gcc -g -O -c main.c
RUN libtool --mode=link gcc -g -O -o myapp main.o \
                /usr/local/lib/libsiren.la
SHELL ["/bin/bash", "-c"]
CMD for file in ./share/*.wav; do ./myapp "$file" "${file/.wav/-raw.wav}"; done