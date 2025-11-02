FROM postgres:16-alpine

RUN apk update && apk upgrade

RUN apk add clang
RUN apk add cmake
RUN apk add make
RUN apk add git
RUN apk add boost-dev
RUN apk add openssl
RUN apk add spdlog spdlog-dev
RUN apk add fmt fmt-dev

RUN apk add postgresql postgresql-contrib postgresql-dev

WORKDIR /app

# pqxx
RUN apk add libpq-dev
RUN git clone https://github.com/jtv/libpqxx.git
WORKDIR /app/libpqxx
RUN cmake . -DCMAKE_INSTALL_PREFIX=/usr -DSKIP_BUILD_TEST=ON -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++
RUN make
RUN make install
WORKDIR /app

# libcrypt
RUN git clone https://github.com/trusch/libbcrypt
WORKDIR /app/libbcrypt
RUN cmake . -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++
RUN make
RUN make install
WORKDIR /app

# nlohmann/json
RUN git clone --depth=1 https://github.com/nlohmann/json.git /app/json \
    && mkdir -p /usr/local/include \
    && cp -r /app/json/single_include/nlohmann /usr/local/include

# jwt-cpp 
RUN git clone https://github.com/Thalhammer/jwt-cpp && \
    cp -r /app/jwt-cpp/include/* /usr/local/include

COPY . /app

# build
RUN cmake . -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ && make

CMD ["./src/sBeHappy"]