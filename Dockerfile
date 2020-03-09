FROM detective7th/kn_base
MAINTAINER kid.Novalis

ARG build_type=Release
RUN mkdir -p /data/kn
ADD kn /data/kn
WORKDIR /data/kn
RUN mkdir -p build && cd build && cmake .. -DCMAKE_BUILD_TYPE=$build_type && make -j $(nproc) && make install && ldconfig

RUN if [ "$build_type" == "Release" ]; then rm -rf /data; fi