FROM harbor.yijinin.biz/base/kn_base:v0.0.11
MAINTAINER kid.Novalis

ARG build_type=Release
RUN mkdir -p /data/kn
ADD kn /data/kn
WORKDIR /data/kn
RUN mkdir -p build && cd build && cmake .. -DCMAKE_BUILD_TYPE=$build_type && make -j $(nproc) && make install && ldconfig

WORKDIR /
RUN if [ "$build_type" == "Release" ]; then rm -rf /data; fi
