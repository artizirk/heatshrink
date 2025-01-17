#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>

#include "../include/heatshrink_encoder.h"

void poll_data(heatshrink_encoder *hse, int fddest) {
    HSE_poll_res pres;
    do {
        uint8_t out_buf[512];
        size_t poll_sz;
        pres = heatshrink_encoder_poll(hse, out_buf, 512, &poll_sz);
        if (pres < 0) exit(1);
        write(fddest, out_buf, poll_sz);
    } while (pres == HSER_POLL_MORE);
}


int main(int argc, char *argv[]) {
    int fdorig = STDIN_FILENO;
    int fddest = STDOUT_FILENO;

    // alloc encoder using 8,4
    heatshrink_encoder *hse = heatshrink_encoder_alloc(8, 4);

    // read input file in 512 bytes blocks
    uint8_t buff[512];
    while (1) {
        int br = read(fdorig, buff, 512);
        if (br < 0) {
            printf("Error reading from STDIN: %s\n", strerror(errno));
            return 1;
        } else if (br == 0) // end of file
            break;

        size_t sunk = 0;
        do {
            // loop until buff is entire consumed, pushing data to compression
            size_t consumed = 0;
            HSE_sink_res res = heatshrink_encoder_sink(hse, &buff[sunk], br - sunk, &consumed);
            if (res < 0) {
                printf("Compressing error %d\n", res);
                return 1;
            }
            sunk += consumed;

            // retrieve compressed data
            poll_data(hse, fddest);

        } while (sunk < br);
    }

    // tell encoder that's all
    HSE_finish_res fres = heatshrink_encoder_finish(hse);

    // retrieve the remaining compressed data
    if (fres == HSER_FINISH_MORE)
        poll_data(hse, fddest);

    // cleanup
    heatshrink_encoder_free(hse);
    close(fdorig);
    close(fddest);
}
