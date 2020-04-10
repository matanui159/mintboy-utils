#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <SDL2/SDL.h>

#define PIXEL_WIDTH 32
#define PIXEL_HEIGHT 32

static int width = 1;
static int height = 1;
static int realWidth = 8;
static int realHeight = 8;
static char* bitmap = NULL;

static void error(const char* msg) {
   fprintf(stderr, "%s\n", msg);
   exit(EXIT_FAILURE);
}

static void allocBitmap(void) {
   if (bitmap == NULL) {
      bitmap = malloc(realWidth * realHeight);
      memset(bitmap, '0', realWidth * realHeight);
   }
}

static bool readDigit(char c, int* digit, int* mul8) {
   if (!isdigit(c)) {
      return false;
   }
   *digit = c - '0';
   *mul8 = *digit * 8;
   return true;
}

static bool readSize(const char* input) {
   if (!readDigit(input[0], &width, &realWidth)) {
      return false;
   }
   if (input[1] != 'x') {
      return false;
   }
   if (!readDigit(input[2], &height, &realHeight)) {
      return false;
   }
   return true;
}

static bool readFile(FILE* file) {
   char buffer[5];
   if (fread(buffer, 4, 1, file) != 1) {
      return false;
   }
   if (buffer[0] != ';') {
      return false;
   }
   if (!readSize(buffer + 1)) {
      return false;
   }

   allocBitmap();
   for (int x = 0; x < width; ++x) {
      for (int y = 0; y < realHeight; ++y) {
         if (fread(buffer, 5, 1, file) != 1) {
            return false;
         }
         if (memcmp(buffer, "\ndw `", 4) != 0) {
            return false;
         }
         if (fread(bitmap + (y * width + x) * 8, 8, 1, file) != 1) {
            return false;
         }
      }
   }

   for (int i = 0; i < realWidth * realHeight; ++i) {
      if (bitmap[i] < '0' || bitmap[i] > '3') {
         return false;
      }
   }
   return true;
}

static void setPixel(Sint32 x, Sint32 y, char color) {
   int xx = x / PIXEL_WIDTH;
   int yy = y / PIXEL_HEIGHT;
   bitmap[yy * realWidth + xx] = color;
}

int main(int argc, char* argv[]) {
   if (argc < 2 || argc > 3) {
      error("Usage: mintboy-bitmap <file> [WxH]");
   }

   const char* filename = argv[1];
   FILE* file = fopen(filename, "rb");
   if (file != NULL) {
      if (argc == 3) {
         fputs("Cannot update bitmap size\n", stderr);
      }
      if (!readFile(file)) {
         fclose(file);
         error("Failed to read input file");
      }
      fclose(file);
   } else if (argc == 3 && !readSize(argv[2])) {
      error("Failed to read size argument");
   }
   allocBitmap();

   SDL_Window* window = SDL_CreateWindow(
      "Mintboy Bitmap",
      SDL_WINDOWPOS_CENTERED, 
      SDL_WINDOWPOS_CENTERED,
      realWidth * PIXEL_WIDTH,
      realHeight * PIXEL_HEIGHT,
      0
   );
   SDL_Surface* surface = SDL_GetWindowSurface(window);
   char color = '3';

   for (;;) {
      SDL_Event event;
      while (SDL_PollEvent(&event)) {
         switch (event.type) {
            case SDL_QUIT:
               goto exit;
            case SDL_KEYDOWN:;
               SDL_Keycode code = event.key.keysym.sym;
               if (code >= SDLK_1 && code <= SDLK_3) {
                  color = code - SDLK_1 + '1';
               } else if (code == SDLK_BACKQUOTE) {
                  color = '0';
               }
               break;
            case SDL_MOUSEBUTTONDOWN:
               if (event.button.button == SDL_BUTTON_LEFT) {
                  setPixel(event.button.x, event.button.y, color);
               }
               break;
            case SDL_MOUSEMOTION:
               if (event.motion.state & SDL_BUTTON_LMASK) {
                  setPixel(event.motion.x, event.motion.y, color);
               }
               break;
         }
      }

      for (int y = 0; y < realHeight; ++y) {
         for (int x = 0; x < realWidth; ++x) {
            const uint8_t grays[] = {0xFF, 0xAA, 0x55, 0x00};
            uint8_t gray = grays[bitmap[y * realWidth + x] - '0'];
            SDL_FillRect(
               surface,
               &(SDL_Rect){
                  .x = x * PIXEL_WIDTH,
                  .y = y * PIXEL_WIDTH,
                  .w = PIXEL_WIDTH,
                  .h = PIXEL_HEIGHT
               },
               SDL_MapRGBA(surface->format, gray, gray, gray, 0xFF)
            );
         }
      }
      SDL_UpdateWindowSurface(window);
   }

exit:
   SDL_DestroyWindow(window);

   file = fopen(filename, "wb");
   fprintf(file, ";%ix%i", width, height);
   for (int x = 0; x < width; ++x) {
      for (int y = 0; y < realHeight; ++y) {
         fputs("\ndw `", file);
         fwrite(bitmap + (y * width + x) * 8, 8, 1, file);
      }
   }
   return 0;
}
