/** main.c
 * affiches commandes serie pour buspirate pour controler la carte EC de led/afficheurs 7seg
 */

#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#include "clock.h"


int main (int argc, char** argv) {
    int n = 0, c = 0;

    printf (" [0xd0 0x0 0x1] [0xd0 0x82 0xF:16]\n"); // init+PWM indivs
    if (argc > 1) {
        // a string has been passed
        char* p=argv[1];
        printf (" [0xd0 0x94 0x0:4] "); // all off
        for (int i=0; i<5; i++) {
            if (p[i] == 0) {
                // EOL
                break;
            }
            n = (i%2 == 0)?1:0; // digit number
            // indice dans cars[]
            if (p[i] >= '0' && p[i] <= '9') {
                c = p[i] - '0';
            } else {
                // on considere minuscules /!\ pas de ctrl
                c = p[i] - 'a' + 10;
            }
            printf ("[0x%02X 0x%02X 0x%02X 0x%02X] ",
                    0xC0 + (i/2)*2, // adresse i2c c0,c2,c4
                    (n == 1)? 0x96 : 0x94, // @registres l2/3 ou l01 avec autoincrement
                    (interleave (cars[c][n], 0)&0xff00) >> 8,
                    (interleave (cars[c][n], 0)&0xff));

            }
            printf("\n");
        }
        else {
            // pas de param, dump table cars[]
            for (int i=0; i<NBCHAR; i++) {
                printf ("%c : [0xc0 0x96 0x%02X 0x%02X] [0xc0 0x94 0x%02X 0x%02X]\n",
                        (i<10)?i+'0':i-10+'a',
                        (interleave (cars[i][1], 0)&0xff00) >> 8,
                        (interleave (cars[i][1], 0)&0xff),
                        (interleave (cars[i][0], 0)&0xff00) >> 8,
                        (interleave (cars[i][0], 0)&0xff)
               );
            }
        }

//     if (argc < 2) {
//         printf ("usage : %s /dev/ttyUSB0\n", argv[0]);
//         return(1);
//     }
//
//     int fd = open (argv[1], O_RDWR | O_NOCTTY | O_SYNC);
//     if (fd < 0) {
//         printf ("error %d opening %s: %s\n", errno, argv[1], strerror (errno));
//         return (errno);
//     }
//
//     set_interface_attribs (fd, B115200, 0);  // set speed to 115,200 bps, 8n1 (no parity)
//     set_blocking (fd, 0);                // set no blocking
//
//     write (fd, "h\n", 7);           // send 7 character greeting
//
//     usleep ((7 + 25) * 100);             // sleep enough to transmit the 7 plus
//                                         // receive 25:  approx 100 uS per char transmit
//     char buf [1024];
//     int n = read (fd, buf, sizeof buf);  // read up to 100 characters if ready to read
//     printf ("%s\n", buf);
//
//     close(fd);
    return (0);

}
