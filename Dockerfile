FROM alpine

RUN apk add --no-cache build-base git byacc flex pkgconfig libpng-dev cmake

RUN git clone https://github.com/rednex/rgbds.git -b v0.4.0 &&\
   make -C rgbds install &&\
   rm -rf rgbds

RUN git clone https://github.com/binji/binjgb.git -b v0.1.5 &&\
   cmake -S binjgb -B build -D CMAKE_BUILD_TYPE=Release &&\
   make -C build install &&\
   rm -rf binjgb build
