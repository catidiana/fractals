/* BMP Drawing program
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


// scale scales figure, keep it from 0 to 2; shift not more than image
// size; can shift to negative direction a bit
static void
draw_pure_mandelbrot (Image image, u32 hex_color, r32 scale, s32 shift_x, s32 shift_y, u32 max_steps=100)
{
    V3 pixel = to_color (hex_color);

    s32 image_w = image.w;
    s32 image_h = image.h;

    for (s32 y = 0; y < image_h; y++)
    {
        for (s32 x = 0; x < image_w; x++)
        {
            r64 cor_x = 0.0;
            r64 cor_y = 0.0;
            r64 x_c = (r64) (x-shift_x) / (image_w * scale);
            r64 y_c = (r64) (y-shift_y) / (image_h * scale);

            for (u32 steps = 0; ; ++steps) {
                r64 x_test = cor_x;
                r64 y_test = cor_y;
                cor_x = x_test*x_test - y_test*y_test + x_c;
                cor_y = 2 * x_test * y_test + y_c;

                if (cor_x*cor_x + cor_y*cor_y > 4) break;

                if (steps > max_steps)
                {
                    image.pixels[y * image.w + x] = pixel;
                    break;
                }
            }
        }
    }

}


static void
draw_mandelbrot_convergence (Image image, u32 hex_color1, u32 hex_color2, r32 scale, s32 shift_x, s32 shift_y, u32 max_steps=100)
{
    V3 c1 = to_color (hex_color1);
    V3 c2 = to_color (hex_color2);
    u8 r1 = c1.r;
    u8 g1 = c1.g;
    u8 b1 = c1.b;
    u8 r2 = c2.r;
    u8 g2 = c2.g;
    u8 b2 = c2.b;
    
    s32 image_w = image.w;
    s32 image_h = image.h;
    
    for (s32 y = 0; y < image_h; y++)
    {
        V3 pixel = {};
        for (s32 x = 0; x < image_w; x++)
        {
            r64 cor_x = 0.0;
            r64 cor_y = 0.0;
            r64 x_c = (r64) (x-shift_x) / (image_w * scale);
            r64 y_c = (r64) (y-shift_y) / (image_h * scale);
            
            for (u32 steps = 0; ; ++steps) {
                r64 x_test = cor_x;
                r64 y_test = cor_y;
                cor_x = x_test*x_test - y_test*y_test + x_c;
                cor_y = 2 * x_test * y_test + y_c;

                if (cor_x*cor_x+cor_y*cor_y > 4)
                {
                    u32 draw_step = steps % 60;

                    if (draw_step < 20)
                    {
                        r64 transit = draw_step/20.0;
                        pixel.r = r1 + transit*(255-r1);
                        pixel.g = g1 + transit*(255-g1);
                        pixel.b = b1 + transit*(255-b1);
                        image.pixels[y * image.w + x] = pixel;
                        break;
                    }
                    else if (draw_step >= 20 && draw_step < 40)
                    {
                        r64 transit = (draw_step-20.0)/20.0;
                        pixel.r = 255 - transit*(255-r2);
                        pixel.g = 255 - transit*(255-g2);
                        pixel.b = 255 - transit*(255-b2);
                        image.pixels[y * image.w + x] = pixel;
                        break;
                    }
                    else
                    {
                        r64 transit = (draw_step-40.0)/20.0;
                        pixel.r = r2*(1-transit) + transit*r1;
                        pixel.g = g2*(1-transit) + transit*g1;
                        pixel.b = b2*(1-transit) + transit*b1;
                        image.pixels[y * image.w + x] = pixel;
                        break;
                    }

                }
                if (steps > max_steps)
                {
                    pixel.r = 0;
                    pixel.g = 0;
                    pixel.b = 0;
                    image.pixels[y * image.w + x] = pixel;
                    break;
                }
            }
        }
    }
    
}


//by default try constant x = 0,28, constant y = 0,0113
static void
draw_pure_julia (Image image, u32 hex_color, r64 constant_x, r64 constant_y, r32 scale, s32 shift_x, s32 shift_y, u32 max_steps=100)
{
    V3 pixel = to_color (hex_color);

    s32 image_w = image.w;
    s32 image_h = image.h;
    r64 R = (1 + sqrt (1 + 4* sqrt (constant_x*constant_x + constant_y*constant_y)));

    for (s32 y = 0; y < image_h; y++)
    {
        for (s32 x = 0; x < image_w; x++)
        {
            r64 x_scale = (r64) (x-shift_x) / (image_w * scale);
            r64 y_scale = (r64) (y-shift_y) / (image_h * scale);

            for (u32 steps = 0; ; ++steps) {
                r64 x_test = x_scale;
                r64 y_test = y_scale;
                x_scale = x_test*x_test - y_test*y_test + constant_x;
                y_scale = 2 * x_test * y_test + constant_y;

                if (x_scale*x_scale + y_scale*y_scale > R) break;

                if (steps > max_steps)
                {
                    image.pixels[y * image.w + x] = pixel;
                    break;
                }
            }
        }
    }

}



static void
draw_julia_convergence (Image image, V3 *color_scheme, r64 constant_x, r64 constant_y, r32 scale, s32 shift_x, s32 shift_y, u32 max_steps=100)
{

    s32 image_w = image.w;
    s32 image_h = image.h;
    r64 R = (1 + sqrt (1 + 4* sqrt (constant_x*constant_x + constant_y*constant_y))) + 1;
    V3 *pointer = image.pixels;
    r64 x_step = 1.0/(image_w * scale);
    r64 y_step = 1.0/(image_h * scale);

    for (s32 y = -shift_y; y < image_h-shift_y; y++)
    {
        V3 pixel = {};
        for (s32 x = -shift_x; x < image_w-shift_x; x++)
        {
            r64 x_scale = (r64) x*x_step;
            r64 y_scale = (r64) y*y_step;

            for (u32 steps = 0; ; ++steps) {

                r64 x_square = x_scale*x_scale;
                r64 y_square = y_scale*y_scale;

                if (x_square + y_square > R)
                {
                    u32 draw_step = steps % 60;
                    *pointer = color_scheme[draw_step];
                     break;
                                    }

                if (steps > max_steps)
                {
                    pixel.r = 0;
                    pixel.g = 0;
                    pixel.b = 0;
                    *pointer = pixel;
                    break;
                }
                y_scale = 2 * x_scale * y_scale + constant_y;
                x_scale = x_square - y_square + constant_x;

            }
            pointer++;
        }
    }

}

