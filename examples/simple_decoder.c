#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>

#include "../include/heatshrink_decoder.h"

void poll_data(heatshrink_decoder *hsd, int fddest) {
    HSD_poll_res pres;
    do {
        uint8_t out_buf[512];
        size_t poll_sz;
        pres = heatshrink_decoder_poll(hsd, out_buf, 512, &poll_sz);
        if (pres < 0) exit(1);
        write(fddest, out_buf, poll_sz);
    } while (pres == HSDR_POLL_MORE);
}


int main(int argc, char *argv[]) {

    // open compressed filename
    int fdorig = STDIN_FILENO;
    int fddest = STDOUT_FILENO;

    // alloc decoder using 8,4
    heatshrink_decoder *hsd = heatshrink_decoder_alloc(512, 8, 4);

    // read input file in 512 bytes blocks
    uint8_t buff[512];
    while (1) {
        size_t br = read(fdorig, buff, 512);
        if (br < 0) {
            printf("Error reading from STDINT: %s\n", strerror(errno));
            return 1;
        } else if (br == 0) // end of file
            break;

        size_t sunk = 0;
        do {
            // loop until buff is entire consumed, pushing data to decompression
            size_t consumed = 0;
            HSD_sink_res res = heatshrink_decoder_sink(hsd, &buff[sunk], br - sunk, &consumed);
            if (res < 0) {
                printf("Decompressing error %d\n", res);
                return 1;
            }
            sunk += consumed;

            // retrieve uncompressed data
            poll_data(hsd, fddest);

        } while (sunk < br);
    }

    // tell decoder that's all
    HSD_finish_res fres = heatshrink_decoder_finish(hsd);

    // retrieve the remaining compressed data
    if (fres == HSDR_FINISH_MORE)
        poll_data(hsd, fddest);

    // cleanup
    heatshrink_decoder_free(hsd);
    close(fdorig);
    close(fddest);
}
