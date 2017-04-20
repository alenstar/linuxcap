#ifndef ENCODER__H
#define ENCODER__H
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif
int h264_encoder_run(int w, int h, void (*cb)(void *buf, size_t len, void *userdata), void* userdata);
int h265_encoder_run(int w, int h, void (*cb)(void *buf, size_t len, void *userdata), void* userdata);
#ifdef __cplusplus
}
#endif

#endif // ENCODER__H
