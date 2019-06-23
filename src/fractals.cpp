/* Graphics drawing program
 *
 * Copyright (C) 2019 Martin & Diana
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#define MAIN_WINDOW_INIT_WIDTH  1080
#define MAIN_WINDOW_INIT_HEIGHT 800
#define M_PI 3.14159265358979323846

#include <SDL.h>
#include <GL/gl.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <math.h>
#include <string.h>

typedef float    r32;
typedef double   r64;

typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t  u8;
typedef int16_t s32;
typedef int16_t s16;


enum InputType {
  INPUT_NONE,
  INPUT_REDRAW,
  INPUT_INCREASE_SPEED,
  INPUT_DECREASE_SPEED,
  INPUT_RESET_SPEED,
  INPUT_SHIFT_UP,
  INPUT_SHIFT_DOWN,
  INPUT_SHIFT_LEFT,
  INPUT_SHIFT_RIGHT,
};


struct V2 {
    r32 x, y;
};

struct V2_2 {
    r64 xn, yn;
};
union V3 {
    struct {u8 r, g, b;};
    struct {u8 x, y, z;};
};


struct Image {
    V3 *pixels;
    u32 w, h;
    u32 x, y;
    GLuint texture;
};


static V3
to_color (u32 hex_color)
{
    u8 r = (hex_color & 0xff0000) >> 16;
    u8 g = (hex_color & 0x00ff00) >> 8;
    u8 b = (hex_color & 0x0000ff);
    return {r, g, b};
}


#include "mandelbrot.cpp"
#include "figures.cpp"
#include "lines.cpp"


static void
set_window_transform (int window_w, int window_h)
{
    r32 w = 2.0f / window_w;
    r32 h = 2.0f / window_h;
    r32 x = 0;
    r32 y = 0;

    r32 transform[] = {
        w, 0, 0, 0,
        0, h, 0, 0,
        0, 0, 1, 0,
        x, y, 0, 1,
    };

    glLoadMatrixf (transform);
    glViewport (0, 0, window_w, window_h);
}


static Image
new_image (u32 w, u32 h, int x, int y)
{
    Image image = {};
    image.x = x;
    image.y = y;
    image.w = w;
    image.h = h;
    image.pixels = (V3 *) malloc (sizeof (V3) * image.w * image.h);

    glGenTextures (1, &image.texture);
    glBindTexture (GL_TEXTURE_2D, image.texture);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    return image;
}


static void
update_image_texture (Image image)
{
    glBindTexture (GL_TEXTURE_2D, image.texture);
    glTexImage2D (GL_TEXTURE_2D, 0, GL_RGB8,
                  image.w, image.h, 0,
                  GL_RGB, GL_UNSIGNED_BYTE,
                  image.pixels);
}


static void
show_image (Image image)
{
    int w = image.w / 2;
    int h = image.h / 2;

    int x0 = image.x - w;
    int x1 = image.x + w;
    int y0 = image.y - h;
    int y1 = image.y + h;

    glBindTexture (GL_TEXTURE_2D, image.texture);

    glBegin (GL_TRIANGLE_STRIP);

    glTexCoord2f (0, 0);
    glVertex2f (x0, y0);
    glTexCoord2f (1, 0);
    glVertex2f (x1, y0);
    glTexCoord2f (0, 1);
    glVertex2f (x0, y1);
    glTexCoord2f (1, 1);
    glVertex2f (x1, y1);

    glEnd ();
}

struct Image load_image (const char *Filename)
{
  FILE *file = fopen (Filename, "rb");
  assert (file);

  Image image_src;
  image_src.w = 320;
  image_src.h = 760;

  u32 pixels_count = image_src.w * image_src.h;
  image_src.pixels = (V3 *) malloc (pixels_count * sizeof (V3));

  u32 filesize = pixels_count;
  u8 *file_contents = (u8 *) malloc (filesize);
  u32 bytes_read = fread (file_contents, 1, filesize, file);
  assert (bytes_read == filesize);
  fclose (file);

  for (size_t i = 0; i < pixels_count; ++i)
    {
      u8 byte = file_contents[i];
      V3 p = {byte, byte, byte};
      image_src.pixels[i] = p;
    }

   free (file_contents);
   return image_src;
}


static void
draw_image (Image image_des, const char *Filename, u32 x_start, u32 y_start)
{
   Image image_src = load_image (Filename) ;
   for (u32 y = y_start; y < y_start + image_src.h; y++)
   {
       for (u32 x = x_start; x < x_start + image_src.w; x++)
       {
          image_des.pixels[y*image_des.w + x] = image_src.pixels[y*image_src.w + x];
       }
   }
}


static void
redraw (Image image,
        V2_2 *coordinates,
        s32 shift_x, s32 shift_y,
        r64 x_step, r64 y_step,
        r64 R,
        V3 color_scheme)
{
  uniform_fill (image, 0x000000);

  for (u32 y = 0; y < image.h; y++)
    {
      for (u32 x = 0; x < image.w; x++)
        {
          coordinates[y * image.w + x].xn = (r64) ((s32) x - shift_x) * x_step;
          coordinates[y * image.w + x].yn = (r64) ((s32) y - shift_y) * y_step;
          if (coordinates[y * image.w + x].xn *
              coordinates[y * image.w + x].xn +
              coordinates[y * image.w + x].yn *
              coordinates[y * image.w + x].yn > R)
            {
              image.pixels[y * image.w + x] = color_scheme;
            }
        }
    }
}


int
main (int argc, char **argv)
{
    SDL_Init (SDL_INIT_VIDEO);
    SDL_Window *main_window = SDL_CreateWindow ("Graphics", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, MAIN_WINDOW_INIT_WIDTH, MAIN_WINDOW_INIT_HEIGHT, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    assert (main_window);
    SDL_GLContext glcontext = SDL_GL_CreateContext (main_window);
    assert (glcontext);

    set_window_transform (MAIN_WINDOW_INIT_WIDTH, MAIN_WINDOW_INIT_HEIGHT);

    size_t points_max = 1024;
    //size_t points_count = 0;


    V2    *points = (V2 *) malloc (points_max * sizeof (*points));

    u32   images_count = 4;
    Image images[4] = {
        new_image (760, 760,  -160,    20),
        new_image (320, 760,   380,    20),
        new_image (760, 40,   -160,  -380),
        new_image (320, 40,    380,  -380),
    };

    glEnable (GL_TEXTURE_2D);
    glClearColor (0.20, 0.25, 0.30, 1.0);

    int window_w = MAIN_WINDOW_INIT_WIDTH;
    int window_h = MAIN_WINDOW_INIT_WIDTH;

    uniform_fill (images[1], 0xffffff);
    uniform_fill (images[2], 0xffffff);
    uniform_fill (images[3], 0xffffff);

    draw_image (images[1], "instr.data", 0, 0);

    V3 color_scheme[60];
    u32 hex_color1 = 0x0000ff;
    u32 hex_color2 = 0xffffff;
    u32 hex_color3 = 0xffa000;
    V3 c1 = to_color (hex_color1);
    V3 c2 = to_color (hex_color2);
    V3 c3 = to_color (hex_color3);
    u8 r1 = c1.r;
    u8 g1 = c1.g;
    u8 b1 = c1.b;
    u8 r2 = c2.r;
    u8 g2 = c2.g;
    u8 b2 = c2.b;
    u8 r3 = c3.r;
    u8 g3 = c3.g;
    u8 b3 = c3.b;
    for (u8 i = 0; i<60; i++)
    {
        if (i < 20)
        {
            r64 t = i/20.0;
            color_scheme[i].r = (1-t)*r1 + t*r2;
            color_scheme[i].g = (1-t)*g1 + t*g2;
            color_scheme[i].b = (1-t)*b1 + t*b2;
        }
        else if (i >= 20 && i < 40)
        {
            r64 t = (i-20.0)/20.0;
            color_scheme[i].r = (1-t)*r2 + t*r3;
            color_scheme[i].g = (1-t)*g2 + t*g3;
            color_scheme[i].b = (1-t)*b2 + t*b3;
        }
        else
        {
            r64 t = (i-40.0)/20.0;
            color_scheme[i].r = (1-t)*r3 + t*r1;
            color_scheme[i].g = (1-t)*g3 + t*g1;
            color_scheme[i].b = (1-t)*b3 + t*b1;
        }

    }

    r64 constant_x = 0.285;
    r64 constant_y = 0.01;
    r32 scale = 0.3;
    s32 shift_x = 380;
    s32 shift_y = 380;
    s32 image_w = images[0].w;
    s32 image_h = images[0].h;
    r64 R = (1 + sqrt (1 + 4* sqrt (constant_x*constant_x + constant_y*constant_y))) + 1;


    r64 x_step = 1.0/(images[0].w* scale);
    r64 y_step = 1.0/(images[0].h* scale);

    V2_2 *coordinates = (V2_2 *) malloc (images[0].w*images[0].h * sizeof (V2_2));

    redraw (images[0], coordinates, shift_x, shift_y, x_step, y_step, R, color_scheme[0]);

    u32 frame_time = 0;

    for (int keep_running = 1; keep_running; )
      {
        static int s = 0;
        InputType input = INPUT_NONE;

        for (SDL_Event event; SDL_PollEvent (&event);)
          {
            switch (event.type)
              {

              case SDL_WINDOWEVENT:
                {
                  switch (event.window.event)
                    {
                    case SDL_WINDOWEVENT_SIZE_CHANGED:
                      window_w = event.window.data1;
                      window_h = event.window.data2;
                      set_window_transform (window_w, window_h);
                      break;
                    }
                } break;
              case SDL_QUIT:
                {
                  keep_running = 0;
                  break;
                }
              case SDL_KEYDOWN:
                {
                  if (event.type == SDL_KEYDOWN)
                    {
                      switch (event.key.keysym.sym)
                        {
                        case SDLK_BACKSPACE:    input = INPUT_REDRAW; break;
                        case SDLK_LEFTBRACKET:  input = INPUT_INCREASE_SPEED; break;
                        case SDLK_RIGHTBRACKET: input = INPUT_DECREASE_SPEED; break;
                        case SDLK_BACKSLASH:    input = INPUT_RESET_SPEED; break;
                        case SDLK_w:            input = INPUT_SHIFT_UP; break;
                        case SDLK_s:            input = INPUT_SHIFT_DOWN; break;
                        case SDLK_a:            input = INPUT_SHIFT_LEFT; break;
                        case SDLK_d:            input = INPUT_SHIFT_RIGHT; break;
                        }
                    }
                }
              }
          }

        switch (input)
          {
          case INPUT_NONE: break;
          case INPUT_REDRAW:
            {
              redraw (images[0], coordinates, shift_x, shift_y, x_step, y_step, R, color_scheme[0]);
              s = 0;
            } break;
          case INPUT_INCREASE_SPEED:
            {
              if (frame_time > 0) frame_time = frame_time - 10;
            } break;
          case INPUT_DECREASE_SPEED:
            {
              frame_time = frame_time + 10;
            } break;
          case INPUT_RESET_SPEED:
            {
              frame_time = 0;
            } break;
          case INPUT_SHIFT_UP:
            {
              shift_y += 190;
              redraw (images[0], coordinates, shift_x, shift_y, x_step, y_step, R, color_scheme[0]);
              s = 0;
            } break;
          case INPUT_SHIFT_DOWN:
            {
              shift_y -= 190;
              redraw (images[0], coordinates, shift_x, shift_y, x_step, y_step, R, color_scheme[0]);
              s = 0;
            } break;
          case INPUT_SHIFT_LEFT:
            {
              shift_x -= 190;
              redraw (images[0], coordinates, shift_x, shift_y, x_step, y_step, R, color_scheme[0]);
              s = 0;
            } break;
          case INPUT_SHIFT_RIGHT:
            {
              shift_x += 190;
              redraw (images[0], coordinates, shift_x, shift_y, x_step, y_step, R, color_scheme[0]);
              s = 0;
            } break;
          }

        glClear (GL_COLOR_BUFFER_BIT);

        V3 *pointer = images[0].pixels;
        for (s32 y = 0; y < image_h; y++)
          {
            for (s32 x = 0; x < image_w; x++)
              {
                if (coordinates[y*image_w+x].xn*coordinates[y*image_w+x].xn + coordinates[y*image_w+x].yn*coordinates[y*image_w+x].yn <= R)
                  {
                    r64 x_test = coordinates[y*image_w+x].xn;
                    r64 y_test = coordinates[y*image_w+x].yn;
                    coordinates[y*image_w+x].yn = 2*x_test * y_test + constant_y;
                    coordinates[y*image_w+x].xn = x_test*x_test - y_test*y_test + constant_x;

                    if (coordinates[y*image_w+x].xn*coordinates[y*image_w+x].xn + coordinates[y*image_w+x].yn*coordinates[y*image_w+x].yn > R)
                      {
                        u32 draw_step = s % 60;
                        *pointer = color_scheme[draw_step];
                      }
                  }

                pointer++;
              }
          }

        for (u32 i = 0; i < images_count; ++i)
          {
            update_image_texture (images[i]);
            show_image           (images[i]);
          }

        ++s;
        SDL_Delay(frame_time);
        SDL_GL_SwapWindow (main_window);
      }

    return 0;
}


//  LocalWords:  InputType
