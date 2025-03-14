#include <raylib.h>
#include <stdlib.h>
#include <string.h>

#define BUFF_SIZE 2048

int width = 800;
int height = 400;
int halfh = height / 2;
int quarterh = halfh / 2;

float buff[BUFF_SIZE];
float buff2[BUFF_SIZE];
int ptrCall = 0;

void callback(void *bufferData, unsigned int frames) {
    float *in = bufferData;
    for (int i = 0; i < frames * 2; i++) {
        if (ptrCall >= BUFF_SIZE) {
            break;
        }
        buff[ptrCall] = in[i];
        ptrCall++;
    }
}

int main() {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    SetConfigFlags(FLAG_WINDOW_TRANSPARENT);
    InitWindow(width, height, "Music Visualizer");
    SetWindowMinSize(200, 200);
    SetWindowMaxSize(BUFF_SIZE - 16, 1080);
    SetTargetFPS(30);
    InitAudioDevice();
    SetAudioStreamBufferSizeDefault(8192);
    
    float volume = 0.5;
    Music music = LoadMusicStream("/home/maks/Music/Tetrameth.mp3");
    PlayMusicStream(music);
    SetMusicVolume(music, volume);
    AttachAudioStreamProcessor(music.stream, callback);

    Color colorBG = {0, 0, 0, 63};
    Color colorMain = MAROON;
    Color colorSec = colorMain;
    colorSec.a = 127;
    Color colorLine = colorSec;
    Color colorDot = colorMain;
    int colmod = 0;

    while (!WindowShouldClose()) {
        UpdateMusicStream(music);

        if (IsKeyPressed(KEY_SPACE)) {
            if (IsMusicStreamPlaying(music)) {
                PauseMusicStream(music);
            } else {
                ResumeMusicStream(music);
            }
        }

        if (IsWindowResized()) {
            width = GetScreenWidth();
            height = GetScreenHeight();
            halfh = height / 2;
            quarterh = halfh / 2;
        }

        BeginDrawing();
        
        ClearBackground(colorBG);

        for (int i = 0; i < width; i++) {
            colorLine.a = colorSec.a;
            if (i % 16 < 2) {
                colorLine.a = 0;
            } else if (i % 2 == 0) {
                colorLine.a /= 2;
            }
            DrawLine(i, height - 1, i, height - abs(buff[i] * quarterh * 3) - 1 + buff[i+16] * quarterh, colorLine);

            colorDot.r /= 2;
            colorDot.g /= 2;
            colorDot.b /= 2;
            DrawPixel(i, quarterh + buff2[i] * quarterh + 5, GRAY);
            DrawPixel(i, quarterh + buff2[i] * quarterh + 5 + 1, colorDot);
            DrawPixel(i, quarterh + buff2[i] * quarterh + 5 + 3, colorDot);
            DrawPixel(i, quarterh + buff2[i] * quarterh + 5 + 5, colorDot);

            colorDot = colorMain;
            DrawPixel(i, quarterh + buff[i] * quarterh, WHITE);
            DrawPixel(i, quarterh + buff[i] * quarterh + 1, colorDot);
            DrawPixel(i, quarterh + buff[i] * quarterh + 3, colorDot);
            DrawPixel(i, quarterh + buff[i] * quarterh + 5, colorDot);
        }
        memcpy(buff2, buff, BUFF_SIZE * sizeof(buff[0]));       
        ptrCall = 0;
        EndDrawing();
    }
}
