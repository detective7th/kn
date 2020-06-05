FROM archlinux
MAINTAINER kid.Novalis

ENV LD_LIBRARY_PATH=/usr/local/lib:/usr/lib:$LD_LIBRARY_PATH
ENV PATH=/usr/local/bin:/usr/bin:$PATH

RUN pacman -Syyu --noconfirm
RUN pacman -S --noconfirm git gcc cmake protobuf boost boost-libs double-conversion google-glog snappy jemalloc libaio \
    	      		  libsodium libunwind libdwarf libevent gmock rapidjson librdkafka gtest gflags fmt make

RUN mkdir /data && mkdir /data/3rd

WORKDIR /data/3rd
RUN git clone https://github.com/mfontanini/cppkafka.git --recursive
WORKDIR /data/3rd/cppkafka
RUN  mkdir build && cd build && cmake .. -Wno-dev && make -j $(nproc) && make install && ldconfig

WORKDIR /data/3rd
RUN rm -rf cppkafka

RUN git clone https://github.com/jupp0r/prometheus-cpp.git --recursive
WORKDIR /data/3rd/prometheus-cpp
RUN mkdir _build && cd _build && cmake .. -Wno-dev -DBUILD_SHARED_LIBS=ON && make -j $(nproc) && make install && ldconfig

WORKDIR /data/3rd
RUN rm -rf prometheus-cpp

RUN git clone https://github.com/KjellKod/g3log.git
WORKDIR /data/3rd/g3log
RUN mkdir build && cd build && cmake .. -Wno-dev -DUSE_DYNAMIC_LOGGING_LEVELS=ON && make -j $(nproc) && make install && ldconfig

WORKDIR /data/3rd
RUN rm -rf g3log

RUN git clone https://github.com/KjellKod/g3sinks.git
WORKDIR /data/3rd/g3sinks/logrotate
RUN mkdir build && cd build && cmake .. -Wno-dev && make -j $(nproc) && make install && ldconfig

WORKDIR /data/3rd
RUN rm -rf g3sinks

RUN git clone https://github.com/facebook/folly.git
WORKDIR /data/3rd/folly
RUN mkdir build_ && cd build_ && cmake .. -Wno-dev && make -j $(nproc) && make install && ldconfig

WORKDIR /data/3rd
RUN git clone https://github.com/HowardHinnant/date.git
WORKDIR /data/3rd/date
RUN mkdir build && cd build && cmake .. -Wno-dev -DBUILD_SHARED_LIBS=ON -DBUILD_TZ_LIB=ON && make -j $(nproc) && make install \
    && cd /data/3rd/date && cp src/*.cpp /usr/local/include/date/ && cp include/date/*.h /usr/local/include/date

WORKDIR /data/3rd
RUN git clone https://github.com/Cyan4973/xxHash.git
WORKDIR /data/3rd/xxHash
RUN make -j $(nproc) && make install \
    && cp xxhash.c /usr/local/include/ \
    && cp xxh3.h /usr/local/include/ \
    && ldconfig

WORKDIR /data/3rd
RUN git clone https://github.com/yhirose/cpp-httplib.git
WORKDIR /data/3rd/cpp-httplib
RUN mkdir build_ && cd build_ && cmake .. -Wno-dev && make -j $(nproc) && make install && ldconfig

WORKDIR /
RUN rm -rf /data/