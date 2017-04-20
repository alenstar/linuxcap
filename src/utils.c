#include <logdef.h>
#include <stdio.h>
#include <string.h>
#ifndef WIN32
#include <stdint.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned long DWORD;

#endif

size_t argb2rgb(const uint8_t *in, size_t in_len, uint8_t *out,
                size_t out_len) {
    if (in_len > ((out_len / 3) * 4)) {
        LOGD("Buffer is to small");
        return 0;
    }

    if (!(in && out)) {
        LOGD("Bad buffer");
        return 0;
    }

    out_len = 0;
    for (int i = 0; i < in_len;) {
        out[out_len + 2] = in[i + 2]; // B
        out[out_len + 1] = in[i + 1]; // G
        out[out_len + 0] = in[i + 0]; // R
        // out[out_len + 3] = 0x00; //A
        i += 4;
        out_len += 3;
    }

    return out_len;
}

size_t argb2bgr(const uint8_t *in, size_t in_len, uint8_t *out,
                size_t out_len) {
    if (in_len > ((out_len / 3) * 4)) {
        LOGD("Buffer is to small");
        return 0;
    }

    if (!(in && out)) {
        LOGD("Bad buffer");
        return 0;
    }

    out_len = 0;
    for (int i = 0; i < in_len;) {
        out[out_len + 2] = in[i + 0]; // B
        out[out_len + 1] = in[i + 1]; // G
        out[out_len + 0] = in[i + 2]; // R
        // out[out_len + 3] = 0x00; //A
        i += 4;
        out_len += 3;
    }

    return out_len;
}

size_t rgb2argb(const uint8_t *in, size_t in_len, uint8_t *out,
                size_t out_len) {
    if (out_len < ((in_len / 3) * 4)) {
        LOGD("Buffer is to small");
        return 0;
    }

    if (!(in && out)) {
        LOGD("Bad buffer");
        return 0;
    }

    out_len = 0;
    for (int i = 0; i < in_len;) {
        out[out_len + 0] = in[i + 0]; // B
        out[out_len + 1] = in[i + 1]; // G
        out[out_len + 2] = in[i + 2]; // R
        out[out_len + 3] = 0x00;      // A
        i += 3;
        out_len += 4;
    }

    return out_len;
}

#include <math.h>
#define WIDTHBYTES(bits) ((DWORD)(((bits) + 31) & (~31)) / 8)

unsigned char *load_file(const char *path, size_t *data_size) {
    FILE *fd;
    struct stat sb;
    unsigned char *buffer;
    size_t size;
    size_t n;

    if (stat(path, &sb)) {
        LOGE("stat:");
        // exit(EXIT_FAILURE);
        return NULL;
    }
    size = sb.st_size;

    fd = fopen(path, "rb");
    if (!fd) {
        perror(path);
        LOGE("fopen");
        return NULL;
    }

    buffer = (unsigned char *)malloc(size);
    if (!buffer) {
        LOGE("Unable to allocate %lld bytes", (long long)size);
        // exit(EXIT_FAILURE);
        fclose(fd);
        return NULL;
    }

    n = fread(buffer, 1, size, fd);
    if (n != size) {
        // perror(path);
        LOGE("fread");
        fclose(fd);
        return NULL;
    }

    fclose(fd);

    *data_size = size;
    return buffer;
}
