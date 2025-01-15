#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_ttf.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils/sdl_init.h"

#define MAXCHARACTERSARRAY 5000
#define MAXTEXTLINES 150

typedef enum { ALIGN_LEFT,
               ALIGN_CENTER,
               ALIGN_RIGHT } text_alignment_e;

SDL_Surface *theme_textboxSurface_High_Memory(const char *message,
                                              TTF_Font *font, SDL_Color fg,
                                              text_alignment_e align)
{
    SDL_Surface *lines[MAXTEXTLINES];
    int line_count = 0;
    int line_width = 0;
    int line_height = 1.2 * TTF_FontLineSkip(font);

    char *token = NULL;
    char *delim = "\n";
    char s[MAXCHARACTERSARRAY];
    strcpy(s, message);

    token = strtok(s, delim);
    while (token != NULL) {
        lines[line_count] = TTF_RenderUTF8_Blended(font, token, fg);
        SDL_SetAlpha(lines[line_count], 0, 0); /* important */
        if (lines[line_count]->w > line_width)
            line_width = lines[line_count]->w;
        line_count++;
        token = strtok(NULL, delim);
    }

    SDL_Surface *textbox = SDL_CreateRGBSurface(
        0, line_width, line_height * line_count, 32, 0x00FF0000, 0x0000FF00,
        0x000000FF, 0xFF000000); /* important */
    SDL_FillRect(textbox, NULL, 0x000000FF);

    SDL_Rect line_rect = {0, 0};

    int i;
    for (i = 0; i < line_count; i++) {
        if (align == ALIGN_CENTER)
            line_rect.x = (line_width - lines[i]->w) / 2;
        else if (align == ALIGN_RIGHT)
            line_rect.x = line_width - lines[i]->w;

        line_rect.y = line_height * i + (line_height - lines[i]->h) / 2;

        SDL_BlitSurface(lines[i], NULL, textbox, &line_rect);
        SDL_FreeSurface(lines[i]);
    }

    return textbox;
}

void logMessage(char *Message)
{
    FILE *file = fopen("/mnt/SDCARD/log_Easter_Message.txt", "a");

    char valLog[MAXCHARACTERSARRAY];
    sprintf(valLog, "%s %s", Message, "\n");
    fputs(valLog, file);
    fclose(file);
}

int main(int argc, char *argv[])
{
    printf("%s", "\n --------EASTER LAUNCH---------------- \n");
    // Initialize SDL
    SDL_InitDefault();

    // Load the icons
    SDL_Surface *logo1 = IMG_Load("/mnt/SDCARD/.tmp_update/res/easter1.png");
    SDL_Surface *logo2 = IMG_Load("/mnt/SDCARD/.tmp_update/res/easter2.png");
    SDL_Surface *logo3 = IMG_Load("/mnt/SDCARD/.tmp_update/res/easter3.png");
    SDL_Surface *endingScreen = IMG_Load("/mnt/SDCARD/.tmp_update/res/easter4");

    // Set the icon's initial position and velocity
    int x = 1;
    int y = 409;
    int xVel = 1;
    int yVel = 1;
    SDL_Rect rect;
    int bTouchWall1 = 0;
    int bTouchWall2 = 0;

    int mainLoop = 1;

    while (mainLoop) {

        bTouchWall1 = 0;
        bTouchWall2 = 0;

        // Handle events
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_KEYDOWN) {
                SDL_FillRect(screen, NULL, 0);
                SDL_Flip(screen);
                return 0;
            }
        }

        // Update the icon's position
        x += xVel;
        y += yVel;

        // If the icon hits a wall, bounce it and switch between the two icons
        if (x < 0 || x + logo1->w > screen->w) {
            bTouchWall1 = 1;
            xVel = -xVel;
            SDL_Surface *temp = logo1;
            logo1 = logo2;
            logo2 = temp;
        }
        if (y < 0 || y + logo1->h > screen->h) {
            bTouchWall2 = 1;
            yVel = -yVel;
            SDL_Surface *temp = logo1;
            logo1 = logo2;
            logo2 = temp;
        }

        rect.x = x;
        rect.y = y;

        if (bTouchWall1 && bTouchWall2) {
            // A corner is touched

            int loop = 1;
            int animationStep = 1;
            // Scrolling text
            // Text dimensions
            const int TEXT_WIDTH = 640;
            const int TEXT_HEIGHT = 480;

            // Scroll speed
            const int SCROLL_SPEED = 1;

            // The image we will load and show on the screen
            SDL_Surface *gTextSurface = NULL;

            // The color of the font
            SDL_Color gTextColor = {255, 255, 255};

            // The text to be rendered
            char gText[MAXCHARACTERSARRAY];
            int charIndex = 0;
            // Scroll position
            int scrollY = 560;

            // Initialize SDL_ttf
            TTF_Init();

            // Read the text from the text file
            sprintf(gText, "%s", "Onion ");
            charIndex = 6;

            FILE *file =
                fopen("/mnt/SDCARD/.tmp_update/onionVersion/version.txt", "r");
            char c;
            c = getc(file);
            while ((!feof(file)) && (charIndex < MAXCHARACTERSARRAY)) {

                gText[charIndex] = c;

                charIndex++;
                c = getc(file);
            }
            gText[charIndex] = '\n';
            charIndex++;
            gText[charIndex] = ' ';
            charIndex++;
            gText[charIndex] = '\n';
            charIndex++;

            file = fopen(
                "/mnt/SDCARD/.tmp_update/onionVersion/acknowledgments.txt",
                "r");
            c = getc(file);
            while ((!feof(file)) && (charIndex < MAXCHARACTERSARRAY)) {
                gText[charIndex] = c;
                charIndex++;
                c = getc(file);
            }

            gText[charIndex] = '\0';
            fclose(file);

            TTF_Font *font35 =
                TTF_OpenFont("/customer/app/Exo-2-Bold-Italic.ttf", 35);
            gTextSurface = theme_textboxSurface_High_Memory(
                gText, font35, gTextColor, ALIGN_LEFT);

            // DVD touching corner + text sliding Animation
            int cptFrames = 0;
            int moduloFrame;
            SDL_PixelFormat *fmt = screen->format;
            while (loop) {
                moduloFrame = cptFrames % moduloFrame;

                switch (animationStep) {
                case 1:
                    // Glitch animation
                    SDL_FillRect(screen, NULL,
                                 SDL_MapRGB(fmt, rand() % 256, rand() % 256,
                                            rand() % 256));

                    switch (moduloFrame) {
                    case 0:
                        SDL_BlitSurface(logo3, NULL, screen, &rect);
                        break;
                    case 1:
                        SDL_BlitSurface(logo1, NULL, screen, &rect);
                        break;
                    case 2:
                        SDL_BlitSurface(logo2, NULL, screen, &rect);
                        break;
                    }

                    if (cptFrames > 100) {
                        animationStep = 2;
                        cptFrames = 0;
                    }
                    cptFrames++;
                    SDL_Delay(2);
                    break;

                case 2:
                    // text sliding Animation
                    rect.x = 0;
                    rect.y = 0;
                    SDL_FillRect(screen, NULL, 0);
                    SDL_BlitSurface(endingScreen, NULL, screen, &rect);

                    // Render the text
                    SDL_Rect textRect = {15, scrollY, TEXT_WIDTH, TEXT_HEIGHT};
                    SDL_BlitSurface(gTextSurface, NULL, screen, &textRect);
                    // Move the text up by the scroll speed
                    scrollY -= SCROLL_SPEED;
                    if (-scrollY > gTextSurface->h) {
                        loop = 0;
                        mainLoop = 0;
                    }
                    SDL_Delay(10);
                    break;
                }

                SDL_BlitSurface(screen, NULL, video, NULL);
                SDL_Flip(video);

                while (SDL_PollEvent(&event)) {
                    if (event.type == SDL_KEYDOWN) {
                        loop = 0;
                        mainLoop = 0;
                    }
                }
            }
        }

        // Blit the icon onto the screen
        if (mainLoop) {
            SDL_BlitSurface(logo1, NULL, screen, &rect);
            SDL_BlitSurface(screen, NULL, video, NULL);
            SDL_Flip(video);
            SDL_Delay(2);
        }
    }
    // Clean up
    SDL_FreeSurface(logo1);
    SDL_FreeSurface(logo2);
    SDL_FreeSurface(logo3);
    SDL_FreeSurface(endingScreen);
    SDL_FreeSurface(screen);
    SDL_FreeSurface(video);
    SDL_Quit();
}
