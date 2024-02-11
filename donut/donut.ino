/*
 * An Arduino port of the famous donut.c
 * Original code by Andy Sloane (a1k0n)
 * Ported code   by Thanh Vinh Nguyen (itsmevjnk)
 *
 * This sketch is in the public domain; refer to the LICENSE file for more information.
 */

/* donut rendering options */
#define PRECISION             10      // number of bits of precision (min 7)
#define DISTANCE              5UL     // donut distance

#define OUT_WIDTH             40      // output buffer width
#define OUT_HEIGHT            22      // output buffer height
#define OUT_PAD               15      // number of spaces to pad on the left of output

#define DONUT_WIDTH           25      // donut width
#define DONUT_HEIGHT          12      // donut height

/* output options */
#define BAUD                  115200  // change this to the baud rate you prefer
#define DELAY                 0       // delay duration (in milliseconds) between frames

/* intermediaries */
#define BUF_SIZE              (OUT_WIDTH * OUT_HEIGHT)
#define PRECISION_MUL         (1UL << PRECISION)
// R1 is assumed to be 1; any other value seems to break the code
#define R2                    (PRECISION_MUL << 1)
#define K2                    (DISTANCE << (PRECISION << 1))

const char luminance_map[] = ".,-~:;=!*#$@"; // NOTE: this is small enough to be stored in RAM so we'll do that to improve perf

void setup() {
  // put your setup code here, to run once:
  Serial.begin(BAUD);
  while(!Serial);
  Serial.print("\x1b[2J"); // clear screen
}

char outbuf[BUF_SIZE]; // output buffer (each element stores 2 luminance values to save more space)
char zbuf[BUF_SIZE]; // Z buffer

/* rotate macro */
int32_t rotate_tmp;
#define R(mul, shift, x, y) \
  rotate_tmp = x; \
  x -= mul * (int32_t)y >> shift; \
  y += mul * rotate_tmp >> shift; \
  rotate_tmp = (3UL << (PRECISION << 1)) - (int32_t)x*x - (int32_t)y*y >> (PRECISION + 1); \
  x = (int32_t)x * rotate_tmp >> PRECISION; \
  y = (int32_t)y * rotate_tmp >> PRECISION;

int16_t sinA = PRECISION_MUL, cosA = 0, sinB = PRECISION_MUL, cosB = 0;

/* render function */
void render() {
  memset(outbuf, ' ', BUF_SIZE);
  memset(zbuf, 127, BUF_SIZE);

  int16_t sinJ = 0, cosJ = PRECISION_MUL;
  for(uint8_t j = 0; j < 90; j++) {
    int16_t sinI = 0, cosI = PRECISION_MUL;
    for(uint16_t i = 0; i < 314; i++) {
      // Serial.print(sinI, DEC); Serial.print(',');
      // Serial.print(cosI, DEC); Serial.print(',');
      // Serial.print(sinJ, DEC); Serial.print(',');
      // Serial.print(cosJ, DEC); Serial.print(',');

      int16_t x0 = cosJ + R2;
      int16_t x1 = (int32_t)cosI * x0 >> PRECISION;
      int16_t x2 = cosA * sinJ >> PRECISION;
      int16_t x3 = (int32_t)sinI * x0 >> PRECISION;
      int16_t x4 = x2 - ((int32_t)sinA * x3 >> PRECISION);
      int16_t x5 = (int32_t)sinA * sinJ >> PRECISION;
      int32_t x6 = K2 + PRECISION_MUL * (int32_t)x5 + (int32_t)cosA * x3;
      int16_t x7 = (int32_t)cosJ * sinI >> PRECISION;

      int8_t x = (OUT_WIDTH >> 1) + DONUT_WIDTH * ((int32_t)cosB * x1 - (int32_t)sinB * x4) / x6;
      int8_t y = (OUT_HEIGHT >> 1) + DONUT_HEIGHT * ((int32_t)cosB * x4 + (int32_t)sinB * x1) / x6;
      int8_t N = (-(int32_t)cosA * x7 - (int32_t)cosB * ((-sinA * x7 >> PRECISION) + x2) - (int32_t)cosI * ((int32_t)cosJ * sinB >> PRECISION) >> PRECISION) - (int32_t)x5 >> (PRECISION - 3);

      uint16_t off = y * OUT_WIDTH + x;

      int8_t z = (x6 - K2) >> (DISTANCE + PRECISION);

      // Serial.print(x0, DEC); Serial.print(',');
      // Serial.print(x1, DEC); Serial.print(',');
      // Serial.print(x2, DEC); Serial.print(',');
      // Serial.print(x3, DEC); Serial.print(',');
      // Serial.print(x4, DEC); Serial.print(',');
      // Serial.print(x5, DEC); Serial.print(',');
      // Serial.print(x6, DEC); Serial.print(',');
      // Serial.print(x7, DEC); Serial.print(',');
      // Serial.print(x, DEC); Serial.print(',');
      // Serial.print(y, DEC); Serial.print(',');
      // Serial.print(N, DEC); Serial.print(',');
      // Serial.println(z, DEC);

      if(x >= 0 && x < OUT_WIDTH && y >= 0 && y < OUT_HEIGHT && z < zbuf[off]) {
        zbuf[off] = z;
        outbuf[off] = luminance_map[(N > 0) ? ((N < 12) ? N : 11) : 0];
      }

      R(5UL, 8, cosI, sinI);
    }

    R(9UL, 7, cosJ, sinJ);
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  render();
  // Serial.println();

  uint16_t off = 0;
  for(uint8_t y = 0; y < OUT_HEIGHT; y++) {
    for(uint8_t x = 0; x < OUT_PAD; x++) Serial.print(' ');
    for(uint8_t x = 0; x < OUT_WIDTH; x++, off++) Serial.print(outbuf[off]);
    Serial.println();
  }

  R(5UL, 7, cosA, sinA);
  R(5UL, 8, cosB, sinB);

  Serial.print("\x1b[23A"); // reset to home position

#if DELAY > 0
  delay(DELAY);
#endif
}
