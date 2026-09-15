FROM emscripten/emsdk AS builder

WORKDIR /app

RUN apt-get update
RUN apt-get install python3

COPY . .

RUN emcmake cmake -B embuild
WORKDIR /app/embuild
RUN make

FROM nginx as runtime

COPY --from=builder /app/embuild/*.js /usr/share/nginx/html/
COPY --from=builder /app/embuild/chess.html /usr/share/nginx/html/index.html
COPY --from=builder /app/embuild/*.data /usr/share/nginx/html/
COPY --from=builder /app/embuild/*.wasm /usr/share/nginx/html/
