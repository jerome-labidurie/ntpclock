#ifndef __ECDISPLAY_H__
#define __ECDISPLAY_H__

#define NBCHAR 10+26+3
extern uint8_t cars[NBCHAR][2];
uint16_t interleave (uint8_t x, uint8_t y);

#endif

