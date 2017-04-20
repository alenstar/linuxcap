

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include <X11/extensions/XShm.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#ifdef _WIN32
#include <fcntl.h> /* _O_BINARY */
#include <io.h>    /* _setmode() */
#endif

#include <libyuv.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include <x264.h>
#include <x265.h>
#define FPS(start) (CLOCKS_PER_SEC / (clock() - start))

#include <logdef.h>
#include <nano_bmp.h>

struct screen_x11_s {
    int x;
    int y;
    int width;
    int height;
    Display *display;
    Screen *screen;
    XImage *ximg;
    Window root;
    // XWindowAttributes window_attributes;
    XShmSegmentInfo shminfo;
};

int screenshot_init(struct screen_x11_s *s, int x, int y, int width,
                    int height) {
    s->x = x;
    s->y = y;
    s->width = width;
    s->height = height;

    s->display = XOpenDisplay(NULL);
    s->root = DefaultRootWindow(s->display);
    XWindowAttributes window_attributes;
    int rc = XGetWindowAttributes(s->display, s->root, &window_attributes);
    s->screen = window_attributes.screen;
    s->ximg = XShmCreateImage(s->display, DefaultVisualOfScreen(s->screen),
                              DefaultDepthOfScreen(s->screen), ZPixmap, NULL,
                              &s->shminfo, width, height);

    s->shminfo.shmid =
        shmget(IPC_PRIVATE, s->ximg->bytes_per_line * s->ximg->height,
               IPC_CREAT | 0777);
    s->shminfo.shmaddr = s->ximg->data = (char *)shmat(s->shminfo.shmid, 0, 0);
    s->shminfo.readOnly = False;
    if (s->shminfo.shmid < 0) {
        LOGD("Fatal shminfo error!");
    }
    Status s1 = XShmAttach(s->display, &s->shminfo);
    if (!s1) {
        LOGD("XShmAttach() %s", s1 ? "success!" : "failure!");
    }

    return 0;
}
/*
    void operator() (cv::Mat& cv_img){
        if(init)
            init = false;

        XShmGetImage(display, root, ximg, 0, 0, 0x00ffffff);
        cv_img = cv::Mat(height, width, CV_8UC4, ximg->data);
    }
    */
void *screenshot_get(struct screen_x11_s *s) {
    if (XShmGetImage(s->display, s->root, s->ximg, s->x, s->y, 0x00ffffff)) {
        /*
        LOGD(
            "XShnGetImage: %dx%d depth %d bytes_per_line %d bits_per_pixel %d ",
            s->ximg->width, s->ximg->height, s->ximg->depth,
            s->ximg->bytes_per_line, s->ximg->bits_per_pixel);
            */
        return s->ximg->data;
    } else {
        LOGD("XhmGetImage: false");
    }
    return NULL;
}

void screenshot_destroy(struct screen_x11_s *s) {
    XDestroyImage(s->ximg);
    XShmDetach(s->display, &s->shminfo);
    shmdt(s->shminfo.shmaddr);
    XCloseDisplay(s->display);
}

int h264_encoder_run(int img_w, int img_h,
                     void (*cb)(void *buf, size_t len, void *userdata),
                     void *userdata) {
    struct screen_x11_s s;
    int rc = screenshot_init(&s, 0, 0, img_w, img_h);
    int i = 0;

    int width = img_w, height = img_h;
    x264_param_t param;
    x264_picture_t pic;
    x264_picture_t pic_out;
    x264_t *enc = NULL;
    int i_frame = 0;
    int i_frame_size = 0;
    x264_nal_t *nal = NULL;
    int i_nal = 0;

    /* Get default params for preset/tuning */ // medium
    if (x264_param_default_preset(&param, "superfast", "zerolatency") < 0) {
        // TODO
        LOGE("x264_encoder_open:");
        return -1;
    }

    /* Configure non-default params */
    param.i_csp = X264_CSP_I420;
    param.i_width = width;
    param.i_height = height;
    param.b_vfr_input = 0;
    param.b_repeat_headers = 1;
    param.b_annexb = 1;
    param.b_intra_refresh = 0;

    /* Apply profile restrictions. */ // Baseline main high
    if (x264_param_apply_profile(&param, "baseline") < 0) {
        // TODO
        LOGE("x264_encoder_open:");
        return -1;
    }

    if (x264_picture_alloc(&pic, param.i_csp, param.i_width, param.i_height) <
        0) {
        // TODO
        LOGE("x264_encoder_open:");
        return -1;
    }
    enc = x264_encoder_open(&param);
    if (!enc) {
        // TODO
        LOGE("x264_encoder_open:");
        return -1;
    }

    uint8_t *ybuf =
        pic.img.plane[0]; //(uint8_t *)malloc(img_w * img_h * 3 / 2);
    uint8_t *ubuf = pic.img.plane[1]; // ybuf + img_w * img_h;
    uint8_t *vbuf = pic.img.plane[2]; // ubuf + img_w * img_h / 4;
    bmp_t *bmp = NULL;
    for (;; ++i) {
        double start = clock();

        void *buf = screenshot_get(&s);
        bmp = create_bmp(img_w, img_h, 24);

        /*
        // Vertical flipping
        int k = 0;
        uint32_t *pixels = (uint32_t *)buf;
        uint32_t tmp_array[img_w];
        for (k = 0; k < (img_h / 2); k++) {
            uint32_t *a = pixels + k * img_w;
            uint32_t *b = pixels + (img_h - k - 1) * img_w;
            memcpy(tmp_array, a, sizeof(uint32_t) * img_w);
            memcpy(a, b, sizeof(uint32_t) * img_w);
            memcpy(b, tmp_array, sizeof(uint32_t) * img_w);
        }
        */

        // to I420
        int rc = ARGBToI420((const uint8_t *)buf, img_w * 4, ybuf, img_w, ubuf,
                            img_w / 2, vbuf, img_w / 2, img_w, img_h);
        if (rc != 0) {
            LOGE("ARGBToI420 failed");
        }
        i_frame_size = x264_encoder_encode(enc, &nal, &i_nal, &pic, &pic_out);
        // LOGD("i_frame_size: %d nal: %d", i_frame_size, i_nal);
        if (i_frame_size > 0) {
            // TODO
            cb(nal->p_payload, i_frame_size, userdata);
        }

        argb2rgb(buf, img_w * img_h * 4, bmp->pixels, img_w * img_h * 3);

        char *filename[128] = {0x00};
        sprintf(filename, "hello_%04d.bmp", i);
        write_bmp(filename, bmp);

        usleep(1000 * 33); // 66
        // if (!(i & 0b111111))
        // LOGD("fps %4.f  spf %.4f", FPS(start), 1 / FPS(start));
        if (i > 300) {
            break;
        }
    }

    /* Flush delayed frames */
    while (x264_encoder_delayed_frames(enc)) {
        i_frame_size = x264_encoder_encode(enc, &nal, &i_nal, NULL, &pic_out);
        if (i_frame_size < 0) {
            break;
        } else if (i_frame_size) {
            // TODO
            LOGD("Flush i_frame_size: %d", i_frame_size);
            cb(nal->p_payload, i_frame_size, userdata);
        }
    }

    screenshot_destroy(&s);
    destroy_bmp(bmp);
    x264_encoder_close(enc);
    x264_picture_clean(&pic);
    cb(NULL, 0, userdata);
    // free(ybuf);
    return 0;
}

// h265
int h265_encoder_run(int img_w, int img_h,
                     void (*cb)(void *buf, size_t len, void *userdata),
                     void *userdata) {
    int i, j;
    int y_size = 0;
    int buff_size = 0;
    char *buff = NULL;
    int ret = 0;
    x265_nal *pNals = NULL;
    uint32_t iNal = 0;

    x265_param *pParam = NULL;
    x265_encoder *pHandle = NULL;
    x265_picture *pPic_in = NULL;

    // Encode 50 frame
    // if set 0, encode all frame
    int frame_num = 300;
    int csp = X265_CSP_I420;
    int width = img_w, height = img_h;

    pParam = x265_param_alloc();
    x265_param_default(pParam);
    pParam->bRepeatHeaders = 1; // write sps,pps before keyframe
    pParam->internalCsp = csp;
    pParam->sourceWidth = width;
    pParam->sourceHeight = height;
    pParam->fpsNum = 25;
    pParam->fpsDenom = 1;
    // Init
    pHandle = x265_encoder_open(pParam);
    if (pHandle == NULL) {
        LOGE("x265_encoder_open err");
        return 0;
    }

    struct screen_x11_s s;
    int rc = screenshot_init(&s, 0, 0, img_w, img_h);

    y_size = pParam->sourceWidth * pParam->sourceHeight;

    pPic_in = x265_picture_alloc();
    x265_picture_init(pParam, pPic_in);
    switch (csp) {
    case X265_CSP_I444: {
        buff = (char *)malloc(y_size * 3);
        pPic_in->planes[0] = buff;
        pPic_in->planes[1] = buff + y_size;
        pPic_in->planes[2] = buff + y_size * 2;
        pPic_in->stride[0] = width;
        pPic_in->stride[1] = width;
        pPic_in->stride[2] = width;
        break;
    }
    case X265_CSP_I420: {
        buff = (char *)malloc(y_size * 3 / 2);
        pPic_in->planes[0] = buff;
        pPic_in->planes[1] = buff + y_size;
        pPic_in->planes[2] = buff + y_size * 5 / 4;
        pPic_in->stride[0] = width;
        pPic_in->stride[1] = width / 2;
        pPic_in->stride[2] = width / 2;
        break;
    }
    default: {
        LOGE("Colorspace Not Support.");
        return -1;
    }
    }

    // Loop to Encode
    for (i = 0; i < frame_num; i++) {
        void *buf = screenshot_get(&s);
        // to I420
        rc = ARGBToI420((const uint8_t *)buf, img_w * 4, pPic_in->planes[0],
                        img_w, pPic_in->planes[1], img_w / 2,
                        pPic_in->planes[2], img_w / 2, img_w, img_h);
        if (rc != 0) {
            LOGE("ARGBToI420 failed");
        }

        ret = x265_encoder_encode(pHandle, &pNals, &iNal, pPic_in, NULL);

        LOGD("Succeed encode %5d frames", i);

        for (j = 0; j < iNal; j++) {
            cb(pNals[j].payload, pNals[j].sizeBytes, userdata);
        }
    }
    // Flush Decoder
    while (1) {
        ret = x265_encoder_encode(pHandle, &pNals, &iNal, NULL, NULL);
        if (ret == 0) {
            break;
        }
        LOGD("Flush 1 frame.");

        for (j = 0; j < iNal; j++) {
            cb(pNals[j].payload, pNals[j].sizeBytes, userdata);
        }
    }

    x265_encoder_close(pHandle);
    x265_param_free(pParam);
    x265_picture_free(pPic_in);
    free(buff);
    screenshot_destroy(&s);
    return 0;
}
