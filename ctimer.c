#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/ioctl.h>
#endif

#include "style.h"

#define max(a, b) (((a) > (b)) ? (a) : (b))

void csleep(int seconds)
{
#ifdef _WIN32
  Sleep(seconds * 1000);
#else
  sleep(seconds);
#endif
}

int get_terminal_height()
{
  int height = 0;

#ifdef _WIN32
  CONSOLE_SCREEN_BUFFER_INFO csbi;
  if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi))
  {
    height = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
  }
#else
  struct winsize w;
  if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == 0)
  {
    height = w.ws_row;
  }
#endif

  return height;
}

int get_terminal_width()
{
  int width = 0;

#ifdef _WIN32
  CONSOLE_SCREEN_BUFFER_INFO csbi;
  if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi))
  {
    width = csbi.srWindow.Right - csbi.srWindow.Left + 1;
  }
#else
  struct winsize w;
  if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == 0)
  {
    width = w.ws_col;
  }
#endif

  return width;
}

#define FONT_WIDTH 3
#define FONT_HEIGHT 5
#define FONT_COUNT 11
#define FONT_PADDING 2
int FONT[] = {31599, 19812, 14479, 31207, 23524, 29411, 29679, 30866, 31727, 31719, 1040}; // https://x.com/aemkei/status/1795762928399880680 bitmap from here

void usage()
{
  printf("ctimer <time> - run timer that count from time to 0. Time provides in secs, but count will be in format HH:MM:SS\n");
  printf("ctimer        - show current time\n");
}

void err(char *message)
{
  printf("%s\n", message);
  usage();
  exit(69);
}

void printBuffered(const char *str)
{
  // fwrite(str, sizeof(char), strlen(str), stdout);
  // write(STDOUT_FILENO, str, strlen(str));
  printf("%s", str);
}

void draw_symbols(int len, int *buffer, int xPad, int yPad, int sym_thickness, char *bg_sym, char *bg_color, char *fg_sym, char *fg_color)
{
  printBuffered("\033[H\033[J");

  int w = (FONT_WIDTH + FONT_PADDING) * len;
  int h = FONT_HEIGHT;
  int tw = get_terminal_width();
  int th = get_terminal_height();
  for (size_t i = 0; i < yPad; i++)
  {
    for (size_t j = 0; j < tw; j++)
    {
      /* code */
      printBuffered(bg_sym);
      printBuffered(bg_color);
      printBuffered(RESET);
    }
    printBuffered("\n");
  }
  for (size_t y = 0; y < h; y++)
  {
    for (size_t i = 0; i < xPad; i++)
    {
      printBuffered(bg_sym);
      printBuffered(bg_color);
      printBuffered(RESET);
    }

    for (size_t x = 0; x < w; x++)
    {
      int i = x / (FONT_WIDTH + FONT_PADDING);
      int dx = x % (FONT_WIDTH + FONT_PADDING);
      int idx = (FONT_HEIGHT - y - 1) * FONT_WIDTH + dx;
      int ch = FONT[buffer[i]];

      if (dx < FONT_WIDTH && (ch >> idx) & 1)
      {
        for (size_t i = 0; i < sym_thickness; i++)
        {
          printBuffered(fg_sym);
          printBuffered(fg_color);
          printBuffered(RESET);
        }
      }
      else
      {
        for (size_t i = 0; i < sym_thickness; i++)
        {
          printBuffered(bg_sym);
          printBuffered(bg_color);
          printBuffered(RESET);
        }
      }
    }

    int text_width = len * (FONT_WIDTH + FONT_PADDING);
    text_width *= sym_thickness;
    for (size_t x = 0; x < tw - text_width - xPad; x++)
    {
      printBuffered(bg_sym);
      printBuffered(bg_color);
      printBuffered(RESET);
    }

    printBuffered("\n");
  }
  int text_height = FONT_HEIGHT;
  for (size_t i = 0; i < th - text_height - yPad - 1; i++)
  {
    for (size_t j = 0; j < tw; j++)
    {
      /* code */
      printBuffered(bg_sym);
      printBuffered(bg_color);
      printBuffered(RESET);
    }
    printBuffered("\n");
  }
}

void draw_time(struct tm *t, char *bg_sym, char *bg_color, char *fg_sym, char *fg_color)
{
  int dig[8];
  dig[0] = t->tm_hour / 10;
  dig[1] = t->tm_hour % 10;
  dig[2] = 10;
  dig[3] = t->tm_min / 10;
  dig[4] = t->tm_min % 10;
  dig[5] = 10;
  dig[6] = t->tm_sec / 10;
  dig[7] = t->tm_sec % 10;
  int padX = 0;
  int padY = 0;
  int sym_thickness = 2;

  int text_width = 8 * (FONT_WIDTH + FONT_PADDING);
  text_width *= sym_thickness;
  int term_width = get_terminal_width();
  padX = term_width / 2 - text_width / 2;
  padX = max(0, padX);

  int text_height = FONT_HEIGHT;
  int term_height = get_terminal_height();
  int height_pad = 2;
  padY = term_height / 2 - text_height / 2 - height_pad;
  padY = max(0, padY);

  draw_symbols(8, dig, padX, padY, sym_thickness, bg_sym, bg_color, fg_sym, fg_color);
}

int main(int argc, char **argv)
{
  if (argc > 2)
  {
    err("Invalid arguments");
  }

  if (argc == 1)
  {
    while (1)
    {
      time_t cur_time = time(0);
      struct tm *t = localtime(&cur_time);
      draw_time(t, BG_SYM, BG_COLOR, FG_SYM, FG_COLOR);
      csleep(1);
    }
  }
  else
  {
    char *arg = argv[1];
    time_t time = atoi(arg);
    if (time == 0)
    {
      err("Invalid time provided");
    }
    while (time >= 0)
    {
      struct tm *t = gmtime(&time);
      draw_time(t, BG_SYM, BG_COLOR, FG_SYM, FG_COLOR);
      time--;
      csleep(1);
    }
  }
  printBuffered("\033[H\033[J");
  fflush(stdout);
  return 0;
}
