#ifndef UTILS__H
#define UTILS__H
#include <stdint.h>

unsigned char *load_file(const char *path, size_t *data_size);
size_t argb2rgb(const uint8_t *in, size_t in_len, uint8_t *out, size_t out_len);
size_t argb2bgr(const uint8_t *in, size_t in_len, uint8_t *out, size_t out_len);
size_t rgb2argb(const uint8_t *in, size_t in_len, uint8_t *out, size_t out_len);
size_t bgr2argb(const uint8_t *in, size_t in_len, uint8_t *out, size_t out_len);
#endif // UTILS__H
