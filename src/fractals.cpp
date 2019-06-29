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
    INPUT_FREEZE_UNFREEZE,
    INPUT_ZOOM_IN,
    INPUT_ZOOM_OUT,
    INPUT_RESET_SCALE,
    INPUT_CONSTANT,
    INPUT_COLOR_1,
    INPUT_COLOR_2,
    INPUT_COLOR_3,
    INPUT_RESET_COLORS,
    INPUT_TOTAL_RESET,
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


//#include "mandelbrot.cpp"
#include "figures.cpp"
#include "text.cpp"


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


static void
redraw (Image image,
        V2_2 *coordinates,
        s32 shift_x, s32 shift_y,
        r64 scale,
        r64 R,
        V3 color_scheme)
{
    uniform_fill (image, 0x000000);
    r64 x_step = 1.0/(image.w* scale);
    r64 y_step = 1.0/(image.h* scale);

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


static void
set_colors (V3 *color_scheme, u32 hex_color1, u32 hex_color2, u32 hex_color3)
{
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
}

u32 color_modification (u32 hex_color)
{
    V3 set = to_color(hex_color);
    if (set.r == 255 && set.g == 0 && set.b < 255) set.b +=5;
    else if (set.r > 0 && set.g ==0 && set.b == 255) set.r = set.r - 5;
    else if (set.r == 0 && set.g < 255 && set.b == 255) set.g+=5;
    else if (set.r ==0 && set.g == 255 && set.b > 0) set.b = set.b - 5;
    else if (set.r < 255 && set.g == 255 && set.b == 0) set.r+=5;
    else if (set.r == 255 && set.g > 0 && set.b == 0) set.g = set.g - 5;
    else
    {set.r +=51; set.g+=51; set.b+=51;}
    hex_color = set.r*256*256 + set.g*256 +set.b;

    return hex_color;
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

    draw_image (images[1], "res/instr.data", 320, 760, 0, 0);
    draw_image (images[2], "res/low.data", 760, 40, 0, 0);

    V3 color_scheme[60];
    u32 hex_color1 = 0x0000ff;
    u32 hex_color2 = 0xffffff;
    u32 hex_color3 = 0xffa000;
    set_colors (color_scheme, hex_color1, hex_color2, hex_color3);


    draw_square (images[3], 45, 20, 32, 0x000000);
    draw_square (images[3], 85, 20, 32, 0x000000);
    draw_square (images[3], 125, 20, 32, 0x000000);
    draw_square (images[3], 45, 20, 30, hex_color1);
    draw_square (images[3], 85, 20, 30, hex_color2);
    draw_square (images[3], 125, 20, 30, hex_color3);

    r64 constant_pool[14][2] = {{0.285, 0.01},{0.28, 0.0113},{0.285, 0},
                                {0.45, 0.1428}, {-0.0085, 0.71},{-0.1, 0.651},
                                {-0.382, 0.618},{-0.618, 0},{-0.74543, 0.11301},
                                {- 0.8, 0.156},{-0.70176, -0.3842}, {-0.835, -0.2321},
                                { -0.7269, 0.1889}, {0, -0.8}};
    u32 constant_pool_num = 0;


    r64 constant_x = constant_pool[constant_pool_num][0];
    r64 constant_y = constant_pool[constant_pool_num][1];
    r32 scale = 0.25;
    s32 shift_x = 380;
    s32 shift_y = 380;
    s32 image_w = images[0].w;
    s32 image_h = images[0].h;
    r64 R = (1 + sqrt (1 + 4* sqrt (constant_x*constant_x + constant_y*constant_y))) + 1;




    V2_2 *coordinates = (V2_2 *) malloc (images[0].w*images[0].h * sizeof (V2_2));

    redraw (images[0], coordinates, shift_x, shift_y, scale, R, color_scheme[0]);

    u32 frame_time = 0;
    bool freeze_flag = false;

    draw_double (images[2], constant_x, 166, 13);
    draw_double (images[2], constant_y, 251, 13);
    draw_double (images[2], coordinates[0].xn, 375, 13);
    draw_double (images[2], coordinates[images[0].w - 1].xn + 1.0/(images[0].w* scale), 468, 13);
    draw_double (images[2], coordinates[0].yn, 589, 13);
    draw_double (images[2], coordinates[images[0].w * images[0].h - 1].yn + 1.0/(images[0].h* scale), 683, 13);

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
            } break;

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
                    case SDLK_f:            input = INPUT_FREEZE_UNFREEZE; break;
                    case SDLK_z:            input = INPUT_ZOOM_IN; break;
                    case SDLK_x:            input = INPUT_ZOOM_OUT; break;
                    case SDLK_q:            input = INPUT_RESET_SCALE; break;
                    case SDLK_c:            input = INPUT_CONSTANT; break;
                    case SDLK_1:            input = INPUT_COLOR_1; break;
                    case SDLK_2:            input = INPUT_COLOR_2; break;
                    case SDLK_3:            input = INPUT_COLOR_3; break;
                    case SDLK_0:            input = INPUT_RESET_COLORS; break;
                    case SDLK_r:            input = INPUT_TOTAL_RESET; break;
                    }
                }
            } break;
            }
        }

        switch (input)
        {
        case INPUT_NONE: break;
        case INPUT_REDRAW:
        {
            freeze_flag = false;
            redraw (images[0], coordinates, shift_x, shift_y, scale, R, color_scheme[0]);
            s = 0;
            draw_rectangle (images[2], 86, 19, 62, 18, 0xffffff);
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
            freeze_flag = false;
            shift_y += 190;
            redraw (images[0], coordinates, shift_x, shift_y, scale, R, color_scheme[0]);
            s = 0;
            draw_rectangle (images[2], 86, 19, 62, 18, 0xffffff);
            draw_rectangle (images[2], 627, 19, 83, 21, 0xffffff);
            draw_rectangle (images[2], 720, 19, 78, 21, 0xffffff);
            draw_double (images[2], coordinates[0].yn, 589, 13);
            draw_double (images[2], coordinates[images[0].w * images[0].h - 1].yn + 1.0/(images[0].h* scale), 683, 13);
        } break;
        case INPUT_SHIFT_DOWN:
        {
            freeze_flag = false;
            shift_y -= 190;
            redraw (images[0], coordinates, shift_x, shift_y, scale, R, color_scheme[0]);
            s = 0;
            draw_rectangle (images[2], 86, 19, 62, 18, 0xffffff);
            draw_rectangle (images[2], 627, 19, 83, 21, 0xffffff);
            draw_rectangle (images[2], 720, 19, 78, 21, 0xffffff);
            draw_double (images[2], coordinates[0].yn, 589, 13);
            draw_double (images[2], coordinates[images[0].w * images[0].h - 1].yn + 1.0/(images[0].h* scale), 683, 13);
        } break;
        case INPUT_SHIFT_LEFT:
        {
            freeze_flag = false;
            shift_x -= 190;
            redraw (images[0], coordinates, shift_x, shift_y, scale, R, color_scheme[0]);
            s = 0;
            draw_rectangle (images[2], 86, 19, 62, 18, 0xffffff);
            draw_rectangle (images[2], 411, 21, 80, 21, 0xffffff);
            draw_rectangle (images[2], 509, 21, 85, 21, 0xffffff);
            draw_double (images[2], coordinates[0].xn, 375, 13);
            draw_double (images[2], coordinates[images[0].w - 1].xn + 1.0/(images[0].w* scale), 468, 13);
        } break;
        case INPUT_SHIFT_RIGHT:
        {
            freeze_flag = false;
            shift_x += 190;
            redraw (images[0], coordinates, shift_x, shift_y, scale, R, color_scheme[0]);
            s = 0;
            draw_rectangle (images[2], 86, 19, 62, 18, 0xffffff);
            draw_rectangle (images[2], 411, 21, 80, 21, 0xffffff);
            draw_rectangle (images[2], 509, 21, 85, 21, 0xffffff);
            draw_double (images[2], coordinates[0].xn, 375, 13);
            draw_double (images[2], coordinates[images[0].w - 1].xn + 1.0/(images[0].w* scale), 468, 13);
        } break;
        case INPUT_FREEZE_UNFREEZE:
        {
            if (freeze_flag == false) freeze_flag = true;
            else freeze_flag = false;
        } break;
        case INPUT_ZOOM_IN:
        {
            freeze_flag = false;
            scale = scale*2;
            redraw (images[0], coordinates, shift_x, shift_y, scale, R, color_scheme[0]);
            s = 0;
            draw_rectangle (images[2], 86, 19, 62, 18, 0xffffff);
            draw_rectangle (images[2], 411, 21, 80, 21, 0xffffff);
            draw_rectangle (images[2], 509, 21, 85, 21, 0xffffff);
            draw_double (images[2], coordinates[0].xn, 375, 13);
            draw_double (images[2], coordinates[images[0].w - 1].xn + 1.0/(images[0].w* scale), 468, 13);
            draw_rectangle (images[2], 627, 19, 83, 21, 0xffffff);
            draw_rectangle (images[2], 720, 19, 78, 21, 0xffffff);
            draw_double (images[2], coordinates[0].yn, 589, 13);
            draw_double (images[2], coordinates[images[0].w * images[0].h - 1].yn + 1.0/(images[0].h* scale), 683, 13);

        } break;
        case INPUT_ZOOM_OUT:
        {
            freeze_flag = false;
            scale = scale/2;
            redraw (images[0], coordinates, shift_x, shift_y, scale, R, color_scheme[0]);
            s = 0;
            draw_rectangle (images[2], 86, 19, 62, 18, 0xffffff);
            draw_rectangle (images[2], 411, 21, 80, 21, 0xffffff);
            draw_rectangle (images[2], 509, 21, 85, 21, 0xffffff);
            draw_double (images[2], coordinates[0].xn, 375, 13);
            draw_double (images[2], coordinates[images[0].w - 1].xn + 1.0/(images[0].w* scale), 468, 13);
            draw_rectangle (images[2], 627, 19, 83, 21, 0xffffff);
            draw_rectangle (images[2], 720, 19, 78, 21, 0xffffff);
            draw_double (images[2], coordinates[0].yn, 589, 13);
            draw_double (images[2], coordinates[images[0].w * images[0].h - 1].yn + 1.0/(images[0].h* scale), 683, 13);
        } break;
        case INPUT_RESET_SCALE:
        {
            freeze_flag = false;
            if (scale != 0.25 || shift_x != 380 || shift_y != 380)
            {
                scale = 0.25;
                shift_x = 380;
                shift_y = 380;
                redraw (images[0], coordinates, shift_x, shift_y, scale, R, color_scheme[0]);
                s = 0;
                draw_rectangle (images[2], 86, 19, 62, 18, 0xffffff);
                draw_rectangle (images[2], 411, 21, 80, 21, 0xffffff);
                draw_rectangle (images[2], 509, 21, 85, 21, 0xffffff);
                draw_double (images[2], coordinates[0].xn, 375, 13);
                draw_double (images[2], coordinates[images[0].w - 1].xn + 1.0/(images[0].w* scale), 468, 13);
                draw_rectangle (images[2], 627, 19, 83, 21, 0xffffff);
                draw_rectangle (images[2], 720, 19, 78, 21, 0xffffff);
                draw_double (images[2], coordinates[0].yn, 589, 13);
                draw_double (images[2], coordinates[images[0].w * images[0].h - 1].yn + 1.0/(images[0].h* scale), 683, 13);
            }
        } break;
        case INPUT_CONSTANT:
        {
            freeze_flag = false;
            constant_pool_num = (constant_pool_num +1)%14;
            constant_x = constant_pool[constant_pool_num][0];
            constant_y = constant_pool[constant_pool_num][1];
            R = (1 + sqrt (1 + 4* sqrt (constant_x*constant_x + constant_y*constant_y))) + 1;
            redraw (images[0], coordinates, shift_x, shift_y, scale, R, color_scheme[0]);
            s = 0;
            draw_rectangle (images[2], 86, 19, 62, 18, 0xffffff);
            draw_rectangle (images[2], 197, 19, 75, 23, 0xffffff);
            draw_rectangle (images[2], 282, 22, 71, 23, 0xffffff);
            draw_double (images[2], constant_x, 166, 13);
            draw_double (images[2], constant_y, 251, 13);
        } break;
        case INPUT_COLOR_1:
        {

            hex_color1 = color_modification(hex_color1);
            draw_square (images[3], 45, 20, 30, hex_color1);
            set_colors (color_scheme, hex_color1, hex_color2, hex_color3);
        } break;
        case INPUT_COLOR_2:
        {
            hex_color2 = color_modification(hex_color2);
            draw_square (images[3], 85, 20, 30, hex_color2);
            set_colors (color_scheme, hex_color1, hex_color2, hex_color3);
        } break;
        case INPUT_COLOR_3:
        {
            hex_color3 = color_modification(hex_color3);
            draw_square (images[3], 125, 20, 30, hex_color3);
            set_colors (color_scheme, hex_color1, hex_color2, hex_color3);
        } break;
        case INPUT_RESET_COLORS:
        {
            hex_color1 = 0x0000ff;
            hex_color2 = 0xffffff;
            hex_color3 = 0xffa000;
            draw_square (images[3], 45, 20, 30, hex_color1);
            draw_square (images[3], 85, 20, 30, hex_color2);
            draw_square (images[3], 125, 20, 30, hex_color3);
            set_colors (color_scheme, hex_color1, hex_color2, hex_color3);
        } break;
        case INPUT_TOTAL_RESET:
        {
            freeze_flag = false;
            frame_time = 0;
            scale = 0.25;
            shift_x = 380;
            shift_y = 380;
            constant_pool_num = 0;
            constant_x = constant_pool[constant_pool_num][0];
            constant_y = constant_pool[constant_pool_num][1];
            R = (1 + sqrt (1 + 4* sqrt (constant_x*constant_x + constant_y*constant_y))) + 1;
            hex_color1 = 0x0000ff;
            hex_color2 = 0xffffff;
            hex_color3 = 0xffa000;
            draw_square (images[3], 45, 20, 30, hex_color1);
            draw_square (images[3], 85, 20, 30, hex_color2);
            draw_square (images[3], 125, 20, 30, hex_color3);
            set_colors (color_scheme, hex_color1, hex_color2, hex_color3);
            redraw (images[0], coordinates, shift_x, shift_y, scale, R, color_scheme[0]);
            s = 0;
            draw_rectangle (images[2], 86, 19, 62, 18, 0xffffff);
            draw_rectangle (images[2], 411, 21, 80, 21, 0xffffff);
            draw_rectangle (images[2], 509, 21, 85, 21, 0xffffff);
            draw_double (images[2], coordinates[0].xn, 375, 13);
            draw_double (images[2], coordinates[images[0].w - 1].xn + 1.0/(images[0].w* scale), 468, 13);
            draw_rectangle (images[2], 627, 19, 83, 21, 0xffffff);
            draw_rectangle (images[2], 720, 19, 78, 21, 0xffffff);
            draw_double (images[2], coordinates[0].yn, 589, 13);
            draw_double (images[2], coordinates[images[0].w * images[0].h - 1].yn + 1.0/(images[0].h* scale), 683, 13);

        } break;
    }



    glClear (GL_COLOR_BUFFER_BIT);

    if (freeze_flag == false)
    {
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


        draw_integer(images[2], s);
        ++s;
    }

    for (u32 i = 0; i < images_count; ++i)
    {
        update_image_texture (images[i]);
        show_image           (images[i]);
    }



    SDL_Delay(frame_time);
    SDL_GL_SwapWindow (main_window);
}

return 0;
}


//  LocalWords:  InputType
