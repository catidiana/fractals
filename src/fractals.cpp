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
//#define M_PI 3.14159265358979323846
#define _USE_MATH_DEFINES

#include <SDL.h>
#include <GL/gl.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <math.h>
#include <string.h>
#include <time.h>

typedef float    r32;
typedef double   r64;

typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t  u8;
typedef int16_t s32;
typedef int16_t s16;


enum InputType {
    INPUT_NONE,
    INPUT_CORRECT,
    INPUT_BRIGHTEN,
    INPUT_INCREASE_AFFINE,
    INPUT_DECREASE_AFFINE,
    INPUT_GEN_AFFINE,
    INPUT_SHIFT_UP,
    INPUT_SHIFT_DOWN,
    INPUT_SHIFT_LEFT,
    INPUT_SHIFT_RIGHT,
    INPUT_ZOOM_IN,
    INPUT_ZOOM_OUT,
    INPUT_RESET_SCALE,
    INPUT_LINEAR,
    INPUT_SINUSOIDAL,
    INPUT_SPHERICAL,
    INPUT_SWIRL,
    INPUT_HORSESHOE,
    INPUT_POLAR,
    INPUT_HANDKERCHIEF,
    INPUT_HEART,
    INPUT_DISC,
    INPUT_SPIRAL,
    INPUT_HYPERBOLIC,
    INPUT_DIAMOND,
    INPUT_EX,
    INPUT_JULIA,
    INPUT_RANDOM,
};


struct V2 {
    r64 x, y;
};


union V3 {
    struct {u8 r, g, b;};
    struct {u8 x, y, z;};
};

struct affine {
    r64 a, b1, c, d, e, f;
    u8 r, g, b;
};


struct Image {
    V3 *pixels;
    u32 *counter;
    r64 *normal;
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


#include "transformations.cpp"
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
    image.counter = (u32*) malloc (sizeof (u32) * image.w * image.h);
    image.normal = (r64*) malloc (sizeof (r64) * image.w * image.h);
    for (u32 i = 0; i< image.w*image.h; i++)
    {
        image.counter[i] = 0;
        image.normal[i] = 0;
    }
    glGenTextures (1, &image.texture);
    glBindTexture (GL_TEXTURE_2D, image.texture);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    return image;
}

static void
reset_image (Image image)
{
    for (u32 i = 0; i< image.w*image.h; i++)
    {
        image.counter[i] = 0;
        image.normal[i] = 0;
    }
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
correction(Image image)
{
    r64 max=0.0;
    r64 gamma=2.2;
    for (u32 x = 0; x < image.w; x++) {
        for (u32 y = 0; y < image.h; y++)
            if (image.counter[y*image.w + x] != 0)
            {
                image.normal[y*image.w + x]=log10(image.counter[y*image.w + x]);
                if (image.normal[y*image.w + x]>max)
                    max = image.normal[y*image.w + x];
            }
    }
    for (u32 x = 0; x < image.w; x++)
    {
        for (u32 y = 0; y < image.h; y++)
        {
            image.normal[y*image.w + x]/=max;
            V3 exist_color = image.pixels[y*image.w + x];
            V3 new_color;
            r64 corr_coef = pow(image.normal[y*image.w + x],(1.0 / gamma));
            new_color.r =exist_color.r*corr_coef;
            new_color.g =exist_color.g*corr_coef;
            new_color.b =exist_color.b*corr_coef;
            image.pixels[y*image.w + x] = new_color;
        }
    }
}


static void
brighten(Image image)
{
    for (u32 x = 0; x < image.w; x++)
    {
        for (u32 y = 0; y < image.h; y++)
        {
            r64 corr_coef = 1.1;
            V3 exist_color = image.pixels[y*image.w + x];
            V3 new_color;
            r64 R = (r64)exist_color.r/255.0;
            r64 G = (r64)exist_color.g/255.0;
            r64 B = (r64)exist_color.b/255.0;
            r64 min = R;
            if (G < min) min = G;
            if (B < min) min = B;
            r64 max = R;
            if (G > max) max = G;
            if (B > max) max = B;
            r64 Lum = (max + min)/2.0;
            if (Lum == 1) break;
            r64 Sat;
            if (Lum < 0.5) Sat = (max-min)/(max+min);
            else Sat = (max-min)/(2.0-max-min);
            r64 Hue;
            if (max == R) Hue = (G-B)/(max-min);
            else if (max == G) Hue = 2.0 + (B-R)/(max-min);
            else Hue = 4.0 + (R-G)/(max-min);
            Hue = Hue*60.0;
            if (Hue < 0) Hue = Hue + 360;

            if (Lum < 0.9) Lum = Lum* corr_coef;
            else Lum = 1;

            r64 temporary_1;
            if (Lum < 0.5) temporary_1 = Lum * (1.0+Sat);
            else temporary_1 = Lum + Sat - Lum * Sat;
            r64 temporary_2 = 2 * Lum - temporary_1;
            Hue = Hue/360;
            r64 temporary_R = Hue + 1.0/3.0;
            if (temporary_R > 1) temporary_R = temporary_R - 1;
            r64 temporary_G = Hue;
            r64 temporary_B = Hue - 1.0/3.0;
            if (temporary_B < 0) temporary_B = temporary_B + 1;

            if (6*temporary_R < 1) new_color.r = (temporary_2 + (temporary_1 - temporary_2) * 6 * temporary_R)*255;
            else if (2*temporary_R < 1)  new_color.r= temporary_1*255;
            else if (3*temporary_R < 2) new_color.r = (temporary_2 + (temporary_1 - temporary_2) * (2.0/3.0 - temporary_R) * 6)*255;
            else new_color.r = temporary_2*255;
            if (6*temporary_G < 1) new_color.g = (temporary_2 + (temporary_1 - temporary_2) * 6 * temporary_G)*255;
            else if (2*temporary_G < 1)  new_color.g= temporary_1*255;
            else if (3*temporary_G < 2) new_color.g = (temporary_2 + (temporary_1 - temporary_2) * (2.0/3.0 - temporary_G) * 6)*255;
            else new_color.g = temporary_2*255;
            if (6*temporary_B < 1) new_color.b = (temporary_2 + (temporary_1 - temporary_2) * 6 * temporary_B)*255;
            else if (2*temporary_B < 1)  new_color.b= temporary_1*255;
            else if (3*temporary_B < 2) new_color.b = (temporary_2 + (temporary_1 - temporary_2) * (2.0/3.0 - temporary_B) * 6)*255;
            else new_color.b = temporary_2*255;

            image.pixels[y*image.w + x] = new_color;
        }
    }
}


static void
generate_affine (affine *trans, u32 eqCount)
{
    for (u32 i = 0;i < eqCount; i++) {
        do {
            do {
                trans[i].a = (rand()%1001)/500.0 - 1.0;
                trans[i].d = (rand()%1001)/500.0 - 1.0;
            }
            while (trans[i].a*trans[i].a + trans[i].d*trans[i].d >=1);
            do {
                trans[i].b1 = (rand()%2001)/1000.0 - 1.0;
                trans[i].e = (rand()%2001)/1000.0 - 1.0;
            }
            while (trans[i].b1*trans[i].b1 + trans[i].e*trans[i].e >=1);
        }
        while (trans[i].a*trans[i].a + trans[i].d*trans[i].d + trans[i].b1*trans[i].b1 + trans[i].e*trans[i].e >= 1 + (trans[i].a*trans[i].e - trans[i].b1*trans[i].d)*(trans[i].a*trans[i].e - trans[i].b1*trans[i].d));
        trans[i].c = (rand()%1001)/5000.0 - 0.25;
        trans[i].f = (rand()%1001)/5000.0 - 0.25;
        trans[i].r = rand()%256;
        trans[i].g = rand()%256;
        trans[i].b = rand()%256;
    }
}


static void
fill_pool (V2 *start, affine *trans, u32 eqCount)
{
    for(u32 g=0; g <760; g++)
    {
        for (u32 j=0; j <760; j++)
        {
            start[g*760 + j].x = (g-380)/190;
            start[g*760 + j].y = (j-380)/190;
            for(u32 k=0; k<40; k++)
            {
                u32 i=rand()%eqCount;
                r64 x1=trans[i].a*start[g*760 + j].x+trans[i].b1*start[g*760 + j].y+trans[i].c;
                r64 y1=trans[i].d*start[g*760 + j].x+trans[i].e*start[g*760 + j].y+trans[i].f;
                start[g*760 + j].x = x1;
                start[g*760 + j].y = y1;
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

    uniform_fill (images[0], 0x000000);
    uniform_fill (images[1], 0xffffff);
    uniform_fill (images[2], 0xffffff);
    uniform_fill (images[3], 0xffffff);

    draw_image (images[1], "res/instr.data", 320, 760, 0, 0);
    draw_image (images[2], "res/low.data", 760, 40, 0, 0);

    srand(time(0));
    u32 eqCount = rand()%10 + 1;
    draw_integer(images[2], eqCount);
    affine trans[30];
    generate_affine (trans, eqCount);

    u32 pool = 760*760;
    V2 *start = (V2*)malloc(pool*sizeof (V2));
    fill_pool (start, trans, eqCount);


    r64 scale = 1;
    r64 x_shift = 0;
    r64 y_shift = 0;
    u32 trans_flag = rand()%14;
    bool corr_flag = false;
    bool rand_flag = false;



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
                    case SDLK_BACKSPACE:    input = INPUT_CORRECT; break;
                    case SDLK_BACKSLASH:    input = INPUT_BRIGHTEN; break;
                    case SDLK_RIGHTBRACKET: input = INPUT_INCREASE_AFFINE; break;
                    case SDLK_LEFTBRACKET:  input = INPUT_DECREASE_AFFINE; break;
                    case SDLK_0:            input = INPUT_GEN_AFFINE; break;
                    case SDLK_w:            input = INPUT_SHIFT_UP; break;
                    case SDLK_s:            input = INPUT_SHIFT_DOWN; break;
                    case SDLK_a:            input = INPUT_SHIFT_LEFT; break;
                    case SDLK_d:            input = INPUT_SHIFT_RIGHT; break;
                    case SDLK_z:            input = INPUT_ZOOM_IN; break;
                    case SDLK_x:            input = INPUT_ZOOM_OUT; break;
                    case SDLK_q:            input = INPUT_RESET_SCALE; break;
                    case SDLK_l:            input = INPUT_LINEAR; break;
                    case SDLK_u:            input = INPUT_SINUSOIDAL; break;
                    case SDLK_p:            input = INPUT_SPHERICAL; break;
                    case SDLK_h:            input = INPUT_SWIRL; break;
                    case SDLK_o:            input = INPUT_HORSESHOE; break;
                    case SDLK_b:            input = INPUT_POLAR; break;
                    case SDLK_f:            input = INPUT_HANDKERCHIEF; break;
                    case SDLK_m:            input = INPUT_HEART; break;
                    case SDLK_c:            input = INPUT_DISC; break;
                    case SDLK_i:            input = INPUT_SPIRAL; break;
                    case SDLK_y:            input = INPUT_HYPERBOLIC; break;
                    case SDLK_n:            input = INPUT_DIAMOND; break;
                    case SDLK_e:            input = INPUT_EX; break;
                    case SDLK_j:            input = INPUT_JULIA; break;
                    case SDLK_r:            input = INPUT_RANDOM; break;
                    }
                }
            } break;
            }
        }

        switch (input)
        {
        case INPUT_NONE: break;
        case INPUT_CORRECT:
        {
            corr_flag = true;
            correction(images[0]);
        } break;
        case INPUT_BRIGHTEN:
        {
            if (corr_flag == true) brighten(images[0]);
        } break;
        case INPUT_GEN_AFFINE:
        {
            corr_flag = false;
            reset_image (images[0]);
            uniform_fill (images[0], 0x000000);
            draw_image (images[2], "res/low.data", 760, 40, 0, 0);
            eqCount = rand()%10 + 1;
            draw_integer(images[2], eqCount);
            generate_affine (trans, eqCount);
            fill_pool (start, trans, eqCount);
        } break;
        case INPUT_INCREASE_AFFINE:
        {
            if (eqCount < 30)
            {
                corr_flag = false;
                reset_image (images[0]);
                uniform_fill (images[0], 0x000000);
                draw_image (images[2], "res/low.data", 760, 40, 0, 0);
                do {
                    do {
                        trans[eqCount].a = (rand()%1001)/500.0 - 1.0;
                        trans[eqCount].d = (rand()%1001)/500.0 - 1.0;
                    }
                    while (trans[eqCount].a*trans[eqCount].a + trans[eqCount].d*trans[eqCount].d >=1);
                    do {
                        trans[eqCount].b1 = (rand()%2001)/1000.0 - 1.0;
                        trans[eqCount].e = (rand()%2001)/1000.0 - 1.0;
                    }
                    while (trans[eqCount].b1*trans[eqCount].b1 + trans[eqCount].e*trans[eqCount].e >=1);
                }
                while (trans[eqCount].a*trans[eqCount].a + trans[eqCount].d*trans[eqCount].d + trans[eqCount].b1*trans[eqCount].b1 + trans[eqCount].e*trans[eqCount].e >= 1 + (trans[eqCount].a*trans[eqCount].e - trans[eqCount].b1*trans[eqCount].d)*(trans[eqCount].a*trans[eqCount].e - trans[eqCount].b1*trans[eqCount].d));
                trans[eqCount].c = (rand()%1001)/5000.0 - 0.25;
                trans[eqCount].f = (rand()%1001)/5000.0 - 0.25;
                trans[eqCount].r = rand()%256;
                trans[eqCount].g = rand()%256;
                trans[eqCount].b = rand()%256;
                eqCount = eqCount + 1;
                draw_integer(images[2], eqCount);
                fill_pool (start, trans, eqCount);
            }
        } break;
        case INPUT_DECREASE_AFFINE:
        {
            if (eqCount > 1)  {
                corr_flag = false;
                reset_image (images[0]);
                uniform_fill (images[0], 0x000000);
                draw_image (images[2], "res/low.data", 760, 40, 0, 0);
                eqCount = eqCount - 1;
                draw_integer(images[2], eqCount);
                fill_pool (start, trans, eqCount);
            }
            } break;
        case INPUT_ZOOM_IN:
            {
                corr_flag = false;
                reset_image (images[0]);
                uniform_fill (images[0], 0x000000);
                fill_pool (start, trans, eqCount);
                scale = scale/2;
            } break;
            case INPUT_ZOOM_OUT:
            {
                corr_flag = false;
                reset_image (images[0]);
                uniform_fill (images[0], 0x000000);
                fill_pool (start, trans, eqCount);
                scale = scale*2;
            } break;
            case INPUT_SHIFT_UP:
            {
                corr_flag = false;
                uniform_fill (images[0], 0x000000);
                fill_pool (start, trans, eqCount);
                y_shift = y_shift + 0.5;
            } break;
            case INPUT_SHIFT_DOWN:
            {
                corr_flag = false;
                reset_image (images[0]);
                uniform_fill (images[0], 0x000000);
                fill_pool (start, trans, eqCount);
                y_shift = y_shift - 0.5;
            } break;
            case INPUT_SHIFT_RIGHT:
            {
                corr_flag = false;
                reset_image (images[0]);
                uniform_fill (images[0], 0x000000);
                fill_pool (start, trans, eqCount);
                x_shift = x_shift + 0.5;
            } break;
            case INPUT_SHIFT_LEFT:
            {
                corr_flag = false;
                reset_image (images[0]);
                uniform_fill (images[0], 0x000000);
                fill_pool (start, trans, eqCount);
                x_shift = x_shift - 0.5;
            } break;
            case INPUT_RESET_SCALE:
            {
                corr_flag = false;
                reset_image (images[0]);
                uniform_fill (images[0], 0x000000);
                fill_pool (start, trans, eqCount);
                x_shift = 0;
                y_shift = 0;
                scale = 1.0;
            } break;
            case INPUT_LINEAR:
            {
                corr_flag = false;
                rand_flag = false;
                reset_image (images[0]);
                trans_flag = 0;
                uniform_fill (images[0], 0x000000);
                fill_pool (start, trans, eqCount);
                x_shift = 0;
                y_shift = 0;
                scale = 1.0;
            } break;
            case INPUT_SINUSOIDAL:
            {
                corr_flag = false;
                rand_flag = false;
                reset_image (images[0]);
                trans_flag = 1;
                uniform_fill (images[0], 0x000000);
                fill_pool (start, trans, eqCount);
                x_shift = 0;
                y_shift = 0;
                scale = 1.0;
            } break;
            case INPUT_SPHERICAL:
            {
                corr_flag = false;
                rand_flag = false;
                reset_image (images[0]);
                trans_flag = 2;
                uniform_fill (images[0], 0x000000);
                fill_pool (start, trans, eqCount);
                x_shift = 0;
                y_shift = 0;
                scale = 1.0;
            } break;
            case INPUT_SWIRL:
            {
                corr_flag = false;
                rand_flag = false;
                reset_image (images[0]);
                trans_flag = 3;
                uniform_fill (images[0], 0x000000);
                fill_pool (start, trans, eqCount);
                x_shift = 0;
                y_shift = 0;
                scale = 1.0;
            } break;
            case INPUT_HORSESHOE:
            {
                corr_flag = false;
                rand_flag = false;
                reset_image (images[0]);
                trans_flag = 4;
                uniform_fill (images[0], 0x000000);
                fill_pool (start, trans, eqCount);
                x_shift = 0;
                y_shift = 0;
                scale = 1.0;
            } break;
            case INPUT_POLAR:
            {
                corr_flag = false;
                rand_flag = false;
                reset_image (images[0]);
                trans_flag = 5;
                uniform_fill (images[0], 0x000000);
                fill_pool (start, trans, eqCount);
                x_shift = 0;
                y_shift = 0;
                scale = 1.0;
            } break;
            case INPUT_HANDKERCHIEF:
            {
                corr_flag = false;
                rand_flag = false;
                reset_image (images[0]);
                trans_flag = 6;
                uniform_fill (images[0], 0x000000);
                fill_pool (start, trans, eqCount);
                x_shift = 0;
                y_shift = 0;
                scale = 1.0;
            } break;
            case INPUT_HEART:
            {
                corr_flag = false;
                rand_flag = false;
                reset_image (images[0]);
                trans_flag = 7;
                uniform_fill (images[0], 0x000000);
                fill_pool (start, trans, eqCount);
                x_shift = 0;
                y_shift = 0;
                scale = 1.0;
            } break;
            case INPUT_DISC:
            {
                corr_flag = false;
                rand_flag = false;
                reset_image (images[0]);
                trans_flag = 8;
                uniform_fill (images[0], 0x000000);
                fill_pool (start, trans, eqCount);
                x_shift = 0;
                y_shift = 0;
                scale = 1.0;
            } break;
            case INPUT_SPIRAL:
            {
                corr_flag = false;
                rand_flag = false;
                reset_image (images[0]);
                trans_flag = 9;
                uniform_fill (images[0], 0x000000);
                fill_pool (start, trans, eqCount);
                x_shift = 0;
                y_shift = 0;
                scale = 1.0;
            } break;
            case INPUT_HYPERBOLIC:
            {
                corr_flag = false;
                rand_flag = false;
                reset_image (images[0]);
                trans_flag = 10;
                uniform_fill (images[0], 0x000000);
                fill_pool (start, trans, eqCount);
                x_shift = 0;
                y_shift = 0;
                scale = 1.0;
            } break;
            case INPUT_DIAMOND:
            {
                corr_flag = false;
                rand_flag = false;
                reset_image (images[0]);
                trans_flag = 11;
                uniform_fill (images[0], 0x000000);
                fill_pool (start, trans, eqCount);
                x_shift = 0;
                y_shift = 0;
                scale = 1.0;
            } break;
            case INPUT_EX:
            {
                corr_flag = false;
                rand_flag = false;
                reset_image (images[0]);
                trans_flag = 12;
                uniform_fill (images[0], 0x000000);
                fill_pool (start, trans, eqCount);
                x_shift = 0;
                y_shift = 0;
                scale = 1.0;
            } break;
            case INPUT_JULIA:
            {
                corr_flag = false;
                rand_flag = false;
                reset_image (images[0]);
                trans_flag = 13;
                uniform_fill (images[0], 0x000000);
                fill_pool (start, trans, eqCount);
                x_shift = 0;
                y_shift = 0;
                scale = 1.0;
            } break;
            case INPUT_RANDOM:
            {
                corr_flag = false;
                rand_flag = true;
                reset_image (images[0]);
                uniform_fill (images[0], 0x000000);
                fill_pool (start, trans, eqCount);
                x_shift = 0;
                y_shift = 0;
                scale = 1.0;
            } break;

            }



            glClear (GL_COLOR_BUFFER_BIT);


            //uniform_fill (images[0], 0x000000);
            if (corr_flag == false)
            {
                for(u32 num=0; num<pool; num++)
                {
                    u32 i=rand()%eqCount;
                    r64 x1=trans[i].a*start[num].x+trans[i].b1*start[num].y+trans[i].c;
                    r64 y1=trans[i].d*start[num].x+trans[i].e*start[num].y+trans[i].f;
                    start[num].x = x1;
                    start[num].y = y1;
                    if (rand_flag == true) trans_flag = rand()%14;
                    switch (trans_flag) {
                    case 0: start[num] = linear(start[num]); break;
                    case 1: start[num] = sinusoidal(start[num]); break;
                    case 2: start[num] = spherical(start[num]); break;
                    case 3: start[num] = swirl(start[num]); break;
                    case 4: start[num] = horseshoe(start[num]); break;
                    case 5: start[num] = polar(start[num]); break;
                    case 6: start[num] = handkerchief(start[num]); break;
                    case 7: start[num] = heart(start[num]); break;
                    case 8: start[num] = disc(start[num]); break;
                    case 9: start[num] = spiral(start[num]); break;
                    case 10: start[num] = hyperbolic(start[num]); break;
                    case 11: start[num] = diamond(start[num]); break;
                    case 12: start[num] = ex(start[num]); break;
                    case 13: start[num] = julia(start[num]); break;

                    }
                    r64 x_coord = (start[num].x+(2.0+x_shift)*scale)*(190/scale);
                    r64 y_coord = (start[num].y+(2.0+y_shift)*scale)*(190/scale);
                    u32 x_c = floor(x_coord);
                    u32 y_c = floor (y_coord);
                    if(x_c < images[0].w && y_c < images[0].h)
                    {
                        if(images[0].counter[y_c*images[0].w + x_c]==0)
                        {
                            V3 to_color;
                            to_color.r = trans[i].r;
                            to_color.g = trans[i].g;
                            to_color.b = trans[i].b;
                            images[0].pixels[y_c*images[0].w + x_c] = to_color;

                        } else
                        {
                            V3 to_color;
                            V3 current = images[0].pixels[y_c*images[0].w + x_c];
                            to_color.r = (trans[i].r + current.r)/2;
                            to_color.g = (trans[i].g + current.g)/2;
                            to_color.b = (trans[i].b + current.b)/2;
                            images[0].pixels[y_c*images[0].w + x_c] = to_color;

                        }
                        images[0].counter[y_c*images[0].w + x_c]++;
                    }
                }
            }



            ++s;


            for (u32 i = 0; i < images_count; ++i)
            {
                update_image_texture (images[i]);
                show_image           (images[i]);
            }

            //SDL_Delay(10000);


            SDL_GL_SwapWindow (main_window);
        }

        return 0;
    }


    //  LocalWords:  InputType
