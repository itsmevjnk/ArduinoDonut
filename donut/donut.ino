/*
 * An Arduino port of the famous donut.c
 * Original code by Andy Sloane (a1k0n)
 * Ported code   by Thanh Vinh Nguyen (itsmevjnk)
 *
 * This sketch is in the public domain; refer to the LICENSE file for more information.
 */

/* donut rendering options */
#define PRECISION             10      // number of bits of precision (min 7)
#define DISTANCE              5       // donut distance

#define OUT_WIDTH             40      // output buffer width
#define OUT_HEIGHT            22      // output buffer height
#define OUT_PAD               15       // number of spaces to pad on the left of output

#define DONUT_WIDTH           25      // donut width
#define DONUT_HEIGHT          12      // donut height

/* output options */
#define BAUD                  115200  // change this to the baud rate you prefer
#define DELAY                 0       // delay duration (in milliseconds) between frames

/* intermediaries */
#define BUF_SIZE              (OUT_WIDTH * OUT_HEIGHT)
#define PRECISION_MUL         (1 << PRECISION)
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
long int rotate_tmp;
#define R(mul, shift, x, y) \
  rotate_tmp = x; \
  x -= mul * y >> shift; \
  y += mul * rotate_tmp >> shift; \
  rotate_tmp = (3 << (PRECISION << 1)) - x*x - y*y >> (PRECISION + 1); \
  x = x * rotate_tmp >> PRECISION; \
  y = y * rotate_tmp >> PRECISION;

int16_t sinA = PRECISION_MUL, cosA = 0, sinB = PRECISION_MUL, cosB = 0;

/* render function */
void render() {
  memset(outbuf, ' ', BUF_SIZE);
  memset(zbuf, 127, BUF_SIZE);

  int16_t sinJ = 0, cosJ = PRECISION_MUL;
  for(uint8_t j = 0; j < 90; j++) {
    int16_t sinI = 0, cosI = PRECISION_MUL;
    for(uint16_t i = 0; i < 314; i++) {
      int16_t x0 = cosJ + R2;
      int16_t x1 = cosI * x0 >> PRECISION;
      int16_t x2 = cosA * sinJ >> PRECISION;
      int16_t x3 = sinI * x0 >> PRECISION;
      int16_t x4 = x2 - (sinA * x3 >> PRECISION);
      int16_t x5 = sinA * sinJ >> PRECISION;
      int32_t x6 = K2 + PRECISION_MUL * x5 + cosA * x3;
      int16_t x7 = cosJ * sinI >> PRECISION;

      int8_t x = (OUT_WIDTH >> 1) + DONUT_WIDTH * (cosB * x1 - sinB * x4) / x6;
      int8_t y = (OUT_HEIGHT >> 1) + DONUT_HEIGHT * (cosB * x4 + sinB * x1) / x6;
      int8_t N = (-cosA * x7 - cosB * ((-sinA * x7 >> PRECISION) + x2) - cosI * (cosJ * sinB >> PRECISION) >> PRECISION) - x5 >> (PRECISION - 3);

      uint16_t off = y * OUT_WIDTH + x;

      int8_t z = (x6 - K2) >> (DISTANCE + PRECISION);
      if(x >= 0 && x < OUT_WIDTH && y >= 0 && y < OUT_HEIGHT && z - zbuf[off]) {
        zbuf[off] = z;
        outbuf[off] = luminance_map[(N > 0) ? ((N < 12) ? N : 11) : 0];
      }

      R(5, 8, cosI, sinI);
    }

    R(9, 7, cosJ, sinJ);
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  render();

  uint16_t off = 0;
  for(uint8_t y = 0; y < OUT_HEIGHT; y++) {
    for(uint8_t x = 0; x < OUT_PAD; x++) Serial.print(' ');
    for(uint8_t x = 0; x < OUT_WIDTH; x++, off++) Serial.print(outbuf[off]);
    Serial.println();
  }

  R(5, 7, cosA, sinA);
  R(5, 8, cosB, sinB);

  Serial.print("\x1b[23A"); // reset to home position

#if DELAY > 0
  delay(DELAY);
#endif
}
