OPTIMIZE = -ggdb -O0
#OPTIMIZE = -O3
CFLAGS += -I../include -I../src ${OPTIMIZE}

all: simple_encoder simple_decoder

simple_encoder: simple_encoder.c ../src/heatshrink_encoder.c
	${CC} -Wall ${CFLAGS} -o $@ $^

simple_decoder: simple_decoder.c ../src/heatshrink_decoder.c
	${CC} -Wall ${CFLAGS} -o $@ $^

clean:
	rm -rf simple_encoder simple_decoder
