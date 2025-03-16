#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFF_SIZE 2048

int width = 800;
int height = 400;
int halfh;
int quarterh;

int channels = 1;
float volume = 0.5;
float buff[BUFF_SIZE];
float buff2[BUFF_SIZE];
int ptrCall = 0;

char musicFile[2048] = "Drag&Drop Music Here";

void callback(void *bufferData, unsigned int frames) {
    float *in = bufferData;
    for (int i = 0; i < frames * channels; i++) {
        if (ptrCall >= BUFF_SIZE) {
            break;
        }
        buff[ptrCall] = in[i];
        ptrCall++;
    }
}

int main(int argc, char *argv[]) {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    SetConfigFlags(FLAG_WINDOW_TRANSPARENT);
    InitWindow(width, height, "Music Visualizer");
    SetWindowMinSize(200, 200);
    SetWindowMaxSize(BUFF_SIZE - 16, 1080);
    SetTargetFPS(20);
    InitAudioDevice();
    SetAudioStreamBufferSizeDefault(4096);

    int barHeight = 24;
    // Font font = GetFontDefault(); //LoadFont("JetBrainsMono-Regular.ttf");

    float musicPlayed;
    Music music;

    Color colorMain = VIOLET;
    if (argc > 4) {
        colorMain = (Color){atoi(argv[1]), atoi(argv[2]), atoi(argv[3]), atoi(argv[4])};
    }
    Color colorBG = {0, 0, 0, 63};
    Color colorSec = colorMain;
    colorSec.a = 127;
    Color colorBar = {0, 0, 0, 127};
    Color colorBarFull = colorSec;
    Color colorLine = colorSec;
    Color colorDot = colorMain;
    int colmod = 0;

    while (!WindowShouldClose()) {

        // Check for dropped files
        if (IsFileDropped()) {
            if (IsMusicValid(music)) {
                DetachAudioStreamProcessor(music.stream, callback);
            }
            FilePathList files = LoadDroppedFiles();
            TextCopy(musicFile, files.paths[0]);
            printf("Loading music: %s\n", GetFileNameWithoutExt(musicFile));
            music = LoadMusicStream(musicFile);
            if (IsMusicValid(music)) {
                PlayMusicStream(music);
                channels = music.stream.channels;
                SetMusicVolume(music, volume);
                AttachAudioStreamProcessor(music.stream, callback);
            }
            UnloadDroppedFiles(files);
        }

        // Draw everything
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
            DrawLine(i, height, i,
                     height - abs(buff[i] * quarterh * 3) +
                         buff[i + 16] * quarterh,
                     colorLine);

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

        if (IsCursorOnScreen()) {
            if (IsMusicValid(music)) {
                DrawRectangle(0, height - barHeight,
                              (float)width * GetMusicTimePlayed(music) /
                                  GetMusicTimeLength(music),
                              barHeight, colorBarFull);
            }
            DrawRectangle(0, height - barHeight, width, barHeight, colorBar);
            // DrawTextEx(font, GetFileNameWithoutExt(musicFile), (Vector2){2,
            // height - barHeight + 2}, 20, 0, WHITE);
            DrawText(GetFileNameWithoutExt(musicFile), 2,
                     height - barHeight + 2, 20, WHITE);
        }

        EndDrawing();

        if (!IsMusicValid(music)) {
            continue;
        }

        UpdateMusicStream(music);

        // Pausing
        if (IsKeyPressed(KEY_SPACE)) {
            if (IsMusicStreamPlaying(music)) {
                PauseMusicStream(music);
            } else {
                ResumeMusicStream(music);
            }
        }

        // Volume control
        if (IsKeyPressed(KEY_UP)) {
            volume += 0.1;
            if (volume > 1.0) {
                volume = 1.0;
            }
            SetMusicVolume(music, volume);
        } else if (IsKeyPressed(KEY_DOWN)) {
            volume -= 0.1;
            if (volume < 0.0) {
                volume = 0.0;
            }
            SetMusicVolume(music, volume);
        }

        // Skip and rewind
        if (IsKeyPressed(KEY_RIGHT)) {
            musicPlayed = GetMusicTimePlayed(music);
            musicPlayed += 5.0;
            if (musicPlayed > GetMusicTimeLength(music)) {
                musicPlayed = GetMusicTimeLength(music);
            }
            SeekMusicStream(music, musicPlayed);
            UpdateMusicStream(music);
        } else if (IsKeyPressed(KEY_LEFT)) {
            musicPlayed = GetMusicTimePlayed(music);
            musicPlayed -= 5.0;
            if (musicPlayed < 0.0) {
                musicPlayed = 0.0;
            }
            SeekMusicStream(music, musicPlayed);
            UpdateMusicStream(music);
        }

        // Mouse input
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            if (GetMouseY() >= height - barHeight) {
                musicPlayed = GetMusicTimeLength(music) * GetMouseX() / width;
                SeekMusicStream(music, musicPlayed);
                UpdateMusicStream(music);
            }
        }
    }
}
