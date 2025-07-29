#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define BUFF_SIZE 1920

int width = 800;
int height = 400;
int halfh;
int quarterh;

int frame = 0;

int channels = 1;
float volume = 0.5;
float buff[12][BUFF_SIZE];
int ptrCall = 0;

char musicFile[2048] = "Drag&Drop Music Here";

void callback(void *bufferData, unsigned int frames) {
    float *in = bufferData;
    for (int i = 0; i < frames * channels; i++) {
        if (ptrCall >= BUFF_SIZE) {
            break;
        }
        buff[0][ptrCall] = in[i];
        ptrCall++;
    }
}

int main(int argc, char *argv[]) {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    SetConfigFlags(FLAG_WINDOW_TRANSPARENT);
    InitWindow(width, height, "Music Visualizer");
    halfh = height / 2;
    quarterh = halfh / 2;
    SetWindowMinSize(200, 200);
    SetWindowMaxSize(BUFF_SIZE, 1080);
    SetTargetFPS(60);
    InitAudioDevice();
    SetAudioStreamBufferSizeDefault(4096);

    int barHeight = 24;
    Font font = LoadFontEx("Iosevka-Regular.ttf", 20, NULL, 1280);

    float musicPlayed;
    Music music;

    Color color0 = VIOLET;
    if (argc > 3) {
        color0 = (Color){atoi(argv[1]), atoi(argv[2]), atoi(argv[3]), 255};
    }
    Color color1 = color0;
    Color color2 = color0;
    color1.a /= 2;
    color2.a /= 4;
    Color colorBG = {0, 0, 0, 127};
    Color colorBar = {0, 0, 0, 127};
    Color colorBarFull = color1;
    Color colorLine = color1;

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
            colorLine.a = color1.a;
            if (i % 16 < 2) {
                colorLine.a = 0;
            } else if (i % 2 == 0) {
                colorLine.a /= 2;
            }
            DrawLine(i, height, i,
                     height - fabs(buff[1][i]) * quarterh * 3,
                     colorLine); // what the fuck does this do i don't remember

            for (int j = 11; j > 1; --j) {
                DrawPixel(i, quarterh + buff[j][i] * quarterh + j * 2 - 2,
                          (Color){color0.r, color0.g, color0.b, color0.a * (12 - j) / 12});
            }
            DrawPixel(i, quarterh + buff[1][i] * quarterh, WHITE);
        }

        for (int i = 11; i > 0; --i) {
            memcpy(buff[i], buff[i - 1], sizeof(buff[0]));
        }

        frame++;
        if (frame == 3) {
            frame = 0;
            ptrCall = 0;
        }

        if (IsCursorOnScreen()) {
            if (IsMusicValid(music)) {
                DrawRectangle(0, height - barHeight,
                              (float)width * GetMusicTimePlayed(music) /
                                  GetMusicTimeLength(music),
                              barHeight, colorBarFull);
            }
            DrawRectangle(0, height - barHeight, width, barHeight, colorBar);
            DrawTextEx(font, GetFileNameWithoutExt(musicFile),
                        (Vector2){2, height - barHeight + 2}, font.baseSize,
                        0, WHITE);
            //DrawText(GetFileNameWithoutExt(musicFile), 2,
            //         height - barHeight + 2, 20, WHITE);
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
