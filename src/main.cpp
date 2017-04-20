#include <arpa/inet.h>
#include <cstdio>
#include <encoder.h>
#include <logdef.h>
#include <thread>
#include <mutex>
#include <uWS.h>

#define WsMessageTypeNone 0x00000000
#define WsMessageTypePing 0x00000101
#define WsMessageTypePong 0x00000102
#define WsMessageTypeAuth 0x00000103
#define WsMessageTypeForbid 0x00000104
#define WsMessageTypeRecordStart 0x00000201
#define WsMessageTypeRecordStop 0x00000202
#define WsMessageTypeRecordStream 0x00000203
#define WsMessageTypePlaybackStart 0x00000211
#define WsMessageTypePlaybackStop 0x00000212
#define WsMessageTypePlaybackStream 0x00000213
#define WsMessageTypeH264Start 0x00000311
#define WsMessageTypeH264Stop 0x00000312
#define WsMessageTypeH264Stream 0x00000313

std::mutex ws_mutex;
void h264_onframe_cb(void *buf, size_t len, void *userdata) {
    // uv_poll_t *getPollHandle();
    //LOGD("h264_onframe_cb: %p %d %p", buf, len, userdata);
    uWS::WebSocket<uWS::SERVER> ws(
        uS::Socket(static_cast<uv_poll_t *>(userdata)));
    if (buf && len && userdata) {
        // TODO
        if (userdata) {
            char msg[8] = {0x00};
            uint32_t *type = (uint32_t *)msg;
            *type = htonl(WsMessageTypeH264Stream);
            uint32_t *plen = (uint32_t *)(msg + 4);
            *plen = htonl(len);
			std::lock_guard<std::mutex> lck(ws_mutex);
            ws.send(msg, 8, uWS::OpCode::BINARY);
            ws.send((const char *)buf, len, uWS::OpCode::BINARY);
        }
    } else if (userdata) {
        char msg[8] = {0x00};
        uint32_t *type = (uint32_t *)msg;
        *type = htonl(WsMessageTypeH264Stop);
        uint32_t *plen = (uint32_t *)(msg + 4);
        *plen = htonl(0);
        ws.send(msg, 8, uWS::OpCode::BINARY);
    }
}

int main() {
    uWS::Hub h;

    // uWS::WebSocket<uWS::SERVER> main_ws;
    h.onMessage([](uWS::WebSocket<uWS::SERVER> ws, char *message, size_t length,
                   uWS::OpCode opCode) {
        // if (!client) {
        uv_poll_t *client = nullptr;
        client = ws.getPollHandle();

        std::thread t([client]() {
            h265_encoder_run(360, 640, h264_onframe_cb, (void *)client);
            uWS::WebSocket<uWS::SERVER> ws(
                uS::Socket(static_cast<uv_poll_t *>(client)));
            ws.close(1000, "doclose", 7);
            // client = nullptr;
        });
        t.detach();
        //}
        // TODO
        uint32_t type = ntohl(*((uint32_t *)message));
        uint32_t len = ntohl(*((uint32_t *)(message + 4)));
        //LOGD("onMessage[%d]:%d type %04x len %d", opCode, length, type, len);
        switch (opCode) {
        case uWS::OpCode::TEXT:
            ws.close(1003, "error", 5);
            break;
        case uWS::OpCode::BINARY:
            break;
        case uWS::OpCode::CLOSE:
			LOGD("CLOSE: %s", message);
            break;
        case uWS::OpCode::PING:
            break;
        case uWS::OpCode::PONG:
            break;
        default:
            LOGE("bad OpCode: %d", opCode);
            ws.close(1002, "error", 5);
            break;
        }

        switch (type) {
        case WsMessageTypeNone:
            LOGD("NONE");
            break;
        case WsMessageTypePing:
            LOGD("PING");
            break;
        case WsMessageTypePong:
            LOGD("PONG");
            break;
        case WsMessageTypeAuth:
            LOGD("Auth: %s", std::string(message + 8, length - 8).c_str());
            break;
        case WsMessageTypeForbid:
            LOGD("Forbid");
            break;
        case WsMessageTypeRecordStart:
            LOGD("Audio Capture Start");
            break;
        case WsMessageTypeRecordStop:
            LOGD("Audio Capture Stop");
            break;
        case WsMessageTypeRecordStream:
            LOGD("Audio Capture Stream");
            break;
        case WsMessageTypePlaybackStart:
            LOGD("Audio Playback Start");
            break;
        case WsMessageTypePlaybackStop:
            LOGD("Audio Playback Stop");
            break;
        case WsMessageTypePlaybackStream:
            LOGD("Audio Playback Stream");
            break;
        case WsMessageTypeH264Start:
            LOGD("H264 Start");
            break;
        case WsMessageTypeH264Stop:
            LOGD("H264 Stop");
            break;
        case WsMessageTypeH264Stream:
            LOGD("H264 Stream");
            break;
        default:
            LOGD("UNKNOWN TYPE: %d", type);
            break;
        }

        ws.send(message, length, opCode);
    });

    h.listen(9000);

    h.run();
}
