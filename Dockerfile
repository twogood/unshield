FROM ams21/cmake:3 AS builder
WORKDIR /build

COPY . .
RUN apk update && apk upgrade --no-cache && apk add --no-cache zlib-dev openssl-dev
RUN cmake -DCMAKE_BUILD_TYPE=Release -DBUILD_STATIC=ON . && make && make install

FROM scratch AS unshield-dockerized
WORKDIR /data

COPY --from=builder /usr/local/bin/unshield /app/unshield
# copy all dynamically linked libs to the same absolute paths used at runtime
# COPY --from=builder /usr/local/lib/libunshield.so.1 /usr/local/lib/libunshield.so.1
COPY --from=builder /lib/ld-musl-x86_64.so.1 /lib/ld-musl-x86_64.so.1
COPY --from=builder /usr/lib/libz.so.1 /usr/lib/libz.so.1
COPY --from=builder /usr/lib/libcrypto.so.3 /usr/lib/libcrypto.so.3

ENTRYPOINT ["/app/unshield"]