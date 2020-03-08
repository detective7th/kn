FROM archlinux
MAINTAINER kid.Novalis

RUN pacman -Syyu --noconfirm
RUN pacman -S --noconfirm git gcc cmake protobuf boost boost-libs double-conversion google-glog snappy jemalloc libaio \
    	      		  libsodium libunwind libdwarf libevent gmock rapidjson librdkafka gtest gflags fmt make

RUN mkdir /data && mkdir /data/3rd

WORKDIR /data/3rd
RUN git clone https://github.com/mfontanini/cppkafka.git --recursive
WORKDIR /data/3rd/cppkafka
RUN  mkdir build && cd build && cmake .. && make -j $(nproc) && make install && ldconfig

WORKDIR /data/3rd
RUN rm -rf cppkafka

RUN git clone https://github.com/jupp0r/prometheus-cpp.git --recursive
WORKDIR /data/3rd/prometheus-cpp
RUN mkdir _build && cd _build && cmake .. -DBUILD_SHARED_LIBS=ON && make -j $(nproc) && make install && ldconfig

WORKDIR /data/3rd
RUN rm -rf prometheus-cpp

RUN git clone https://github.com/KjellKod/g3log.git
WORKDIR /data/3rd/g3log
RUN mkdir build && cd build && cmake .. -DUSE_DYNAMIC_LOGGING_LEVELS=ON && make -j $(nproc) && make install && ldconfig

WORKDIR /data/3rd
RUN rm -rf g3log

RUN git clone https://github.com/KjellKod/g3sinks.git
WORKDIR /data/3rd/g3sinks/logrotate
RUN mkdir build && cd build && cmake .. && make -j $(nproc) && make install && ldconfig

WORKDIR /data/3rd
RUN rm -rf g3sinks

RUN git clone https://github.com/facebook/folly.git
WORKDIR /data/3rd/folly
RUN mkdir build_ && cd build_ && cmake .. && make -j $(nproc) && make install && ldconfig

WORKDIR /data/3rd
RUN git clone https://github.com/HowardHinnant/date.git
WORKDIR /data/3rd/date
RUN mkdir build && cd build && cmake .. -DBUILD_SHARED_LIBS=ON -DBUILD_TZ_LIB=ON && make -j $(nproc) && make install \
    && cd /data/3rd/date && cp src/*.cpp /usr/local/include/date/ && cp include/date/*.h /usr/local/include/date

WORKDIR /data/3rd
RUN git clone https://github.com/Cyan4973/xxHash.git
WORKDIR /data/3rd/xxHash
RUN make -j $(nproc) && make install && ldconfig

WORKDIR /data
RUN mkdir -p /data/kn
ADD kn /data/kn
WORKDIR /data/kn
RUN mkdir -p build && cd build && cmake .. -DCMAKE_BUILD_TYPE=Release && make -j $(nproc) && make install && ldconfig

#RUN rm -rf /data/3rd
CMD git --version