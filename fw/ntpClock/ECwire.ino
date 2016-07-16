
#include "ECwire.h"
#include "ECdisplay.h"

void ecInit(void) {
  Wire.begin(); // SDA:4, SCL:5
  Wire.setClock(100000);

// activation
    Wire.beginTransmission(0xd0>>1);
  Wire.write(byte(0x00));
  Wire.write(byte(0x01));
  Wire.endTransmission();

  // set global brightness
  uint8_t d[5] = {0x01, 0x00, 0, 0, 0};
  wReg (0xd0, 2, d);
  memset(d, 0xFF, 5);
  d[0] = 0x94;
  wReg (0xd0, 5, d);
  ecGBright(0xf);

  // set indiv brigtness to max
  ecIBright(0xFF);


}

/** set global brigthness
 *  @param[in] b brightness value
 */
void ecGBright(uint8_t b) {
  uint8_t d[2] = {0x12, b};
  wReg(0xd0, 2, d);
}

/** set indiv brigthness
 *  @param[in] b brightness value
 */
void ecIBright(uint8_t b) {
  Wire.beginTransmission(0xd0>>1);
  Wire.write(byte(0x82));
  for (uint8_t i = 0; i<16; i++) {
    Wire.write(byte(b));
  }
  Wire.endTransmission();
}
void wReg (uint8_t addr, uint8_t len, uint8_t* val) {
  Wire.beginTransmission(addr>>1);
  Wire.write(val, len);
  Wire.endTransmission();
}

void wCar (uint8_t chip, uint8_t reg, uint8_t v1, uint8_t v2) {
  Wire.beginTransmission(chip>>1);
  Wire.write(reg);
  Wire.write(v1);
  Wire.write(v2);
  Wire.endTransmission();  
}

void wScroll (char* p) {
  int l = strlen(p);
  char p2[l+7];
  char* pp = p2;

  p2[0]=0;
  strcat(p2, "     ");
  p2[6] = 0;
  strncat (p2, p, l);
  strcat(p2, " ");
  p2[l+7]=0;

  do {
    wChaine(pp);
    pp++;
    delay(500);
  } while (*pp != 0);
}

void wChaine (char* p) {
  uint8_t n = 0, c = 0;
  uint8_t i;
  
          for (i=0; i<5; i++) {
            if (p[i] == 0) {
                // EOL
                break;
            }
            n = (i%2 == 0)?1:0; // digit number
            // indice dans cars[]
            if (p[i] >= '0' && p[i] <= '9') {
                c = p[i] - '0';
            } else if (p[i] == ' ') {
              c = 36;
            } else if (p[i] == '_') {
              c = 37;
            } else if (p[i] == '-') {
              c = 38;
            } else {
                // on considere minuscules /!\ pas de ctrl
                c = p[i] - 'a' + 10;
            }
            wCar (0xC0 + (i/2)*2, // adresse i2c c0,c2,c4
                  (n == 1)? 0x96 : 0x94, // @registres l2/3 ou l01 avec autoincrement
                    (interleave (cars[c][n], cars[c][n])&0xff00) >> 8,
                    (interleave (cars[c][n], cars[c][n])&0xff));
            }
            // si besoin écrit des espaces jusqu'au bout
            for (;i<5;i++) {
              n = (i%2 == 0)?1:0;
              wCar(0xC0 + (i/2)*2,(n == 1)? 0x96 : 0x94,
              (interleave (cars[36][n], cars[36][n])&0xff00) >> 8,
              (interleave (cars[36][n], cars[36][n])&0xff));
            }
}

