/*
 * An Arduino port of the famous donut.c
 * Original code by Andy Sloane (a1k0n) - https://gist.github.com/a1k0n/8ea6516b4946ab36348fb61703dc3194
 * Ported code   by Thanh Vinh Nguyen (itsmevjnk)
 *
 * This sketch is in the public domain; refer to the LICENSE file for more information.
 */

/* render options */
#define R1                    1UL
#define R2                    2UL
// #define USE_MULTIPLY                  // uncomment this to use multiplication operations - suitable for MCUs with hardware multiplication instructions

/* output options */
#define BAUD                  115200  // change this to the baud rate you prefer
#define DELAY                 0       // delay duration (in milliseconds) between frames

const char luminance_map[] = ".,-~:;=!*#$@"; // NOTE: this is small enough to be stored in RAM so we'll do that to improve perf

void setup() {
  // put your setup code here, to run once:
  Serial.begin(BAUD);
  while(!Serial);
  Serial.print("\x1b[H\x1b[J"); // clear screen and return to home
}

/* render function */
#define R(s, x, y)          x -= (y >> s); y += (x >> s)

int16_t length_cordic(int16_t x, int16_t y, int16_t* x2_, int16_t y2) {
  int16_t x2 = *x2_;
  if(x < 0) {
    x = -x;
    x2 = -x2;
  }
  for(uint8_t i = 0; i < 8; i++) {
    int16_t t = x, t2 = x2;
    if(y < 0) {
      x -= y >> i;
      y += t >> i;
      x2 -= y2 >> i;
      y2 += t2 >> i;
    } else {
      x += y >> i;
      y -= t >> i;
      x2 += y2 >> i;
      y2 -= t2 >> i;
    }
  }

  *x2_ = (x2 >> 1) + (x2 >> 3);
  return (x >> 1) + (x >> 3);
}

int16_t sB = 0, cB = 16384;
int16_t sA = 11583, cA = 11583;
int16_t sAsB = 0, cAsB = 0;
int16_t sAcB = 11583, cAcB = 11583;

#define p0(n)                   (((int32_t)n) + (((int32_t)n << 2)) >> 6) // dz * n >> 6 with dz = 5
#define r1i                     (R1 << 8) // r1 * 256
#define r2i                     (R2 << 8) // r2 * 256
void render() {
  int16_t x1_16 = cAcB << 2;
  
  int16_t p0x = p0(sB);
  int16_t p0y = p0(sAcB);
  int16_t p0z = -p0(cAcB);

  int16_t yincC = (cA >> 6) + (cA >> 5);      // 12*cA >> 8;
  int16_t yincS = (sA >> 6) + (sA >> 5);      // 12*sA >> 8;
  int16_t xincX = (cB >> 7) + (cB >> 6);      // 6*cB >> 8;
  int16_t xincY = (sAsB >> 7) + (sAsB >> 6);  // 6*sAsB >> 8;
  int16_t xincZ = (cAsB >> 7) + (cAsB >> 6);  // 6*cAsB >> 8;
  int16_t ycA = -((cA >> 1) + (cA >> 4));     // -12 * yinc1 = -9*cA >> 4;
  int16_t ysA = -((sA >> 1) + (sA >> 4));     // -12 * yinc2 = -9*sA >> 4;

  for(uint8_t j = 0; j < 23; j++, ycA += yincC, ysA += yincS) {
    int16_t xsAsB = (sAsB >> 4) - sAsB;  // -40*xincY
    int16_t xcAsB = (cAsB >> 4) - cAsB;  // -40*xincZ;

    int16_t vxi14 = (cB >> 4) - cB - sB; // -40*xincX - sB;
    int16_t vyi14 = ycA - xsAsB - sAcB;
    int16_t vzi14 = ysA + xcAsB + cAcB;

    for(uint8_t i = 0; i < 79; i++, vxi14 += xincX, vyi14 -= xincY, vzi14 += xincZ) {
      int16_t t = (5 - R1 - R2) << 8; // (256 * dz) - r2i - r1i = (dz - r2 - r1) * 256 w/ dz = 5

      int16_t px = p0x + (vxi14 >> 5); // assuming t = 512, t*vxi>>8 == vxi<<1
      int16_t py = p0y + (vyi14 >> 5);
      int16_t pz = p0z + (vzi14 >> 5);

      int16_t lx0 = sB >> 2;
      int16_t ly0 = sAcB - cA >> 2;
      int16_t lz0 = -cAcB - sA >> 2;

      while(1) {
        int16_t t0, t1, t2, d;
        int16_t lx = lx0, ly = ly0, lz = lz0;

        t0 = length_cordic(px, py, &lx, ly);
        t1 = t0 - r2i;
        t2 = length_cordic(pz, t1, &lz, lx);
        d = t2 - r1i;
        t += d;

        if(t > 8 * 256) {
          Serial.print(' ');
          break;
        } else if(d < 2) {
          int8_t N = lz >> 9;
          Serial.print(luminance_map[(N > 0) ? ((N < 12) ? N : 11) : 0]);
          break;
        }

#ifdef USE_MULTIPLY
        px += (int32_t)d * vxi14 >> 14;
        py += (int32_t)d * vyi14 >> 14;
        pz += (int32_t)d * vzi14 >> 14;
#else
        int16_t dx = 0, dy = 0, dz = 0;
        int16_t a = vxi14, b = vyi14, c = vzi14;
        while(d) {
          if(d & 1024) {
            dx += a;
            dy += b;
            dz += c;
          }

          d = (d & 1023) << 1;
          a >>= 1;
          b >>= 1;
          c >>= 1;
        }

        px += dx >> 4;
        py += dy >> 4;
        pz += dz >> 4;
#endif

      }
    }

    Serial.println();
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  render();

  R(5, cA, sA);
  R(5, cAsB, sAsB);
  R(5, cAcB, sAcB);
  R(6, cB, sB);
  R(6, cAcB, cAsB);
  R(6, sAcB, sAsB);

  Serial.print("\x1b[H"); // reset to home position

#if DELAY > 0
  delay(DELAY);
#endif
}
