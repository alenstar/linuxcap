#ifndef LOGDEF_H_
#define LOGDEF_H_
#include <stdio.h>
#ifdef DEBUG
#define LOGD(fmt, ...) do{fprintf(stderr, "%s(%3d) [D]: ", __FILE__, __LINE__); fprintf(stderr, fmt,  ##__VA_ARGS__), fprintf(stderr, "\n");}while(0)
#define LOGE(fmt, ...) do{fprintf(stderr, "%s(%3d) [E]: ", __FILE__, __LINE__);fprintf(stderr, fmt,  ##__VA_ARGS__), fprintf(stderr, "\n");}while(0)
#define LOGW(...)
#define LOGI(...)
#else
#define LOGD(...)
#define LOGE(...)
#define LOGW(...)
#define LOGI(...)
#endif 


#endif // LOGDEF_H_
