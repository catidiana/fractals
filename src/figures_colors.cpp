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


static void
uniform_fill (Image image, u32 hex_color)
{
  V3 color = to_color (hex_color);

  for (u32 i = 0; i < image.h*image.w; i++)
    {
          image.pixels[i] = color;
    }
}


static void
draw_rectangle (Image image, s32 center_x, s32 center_y, u32 rect_w, u32 rect_h, u32 hex_color = 0x000000)
{
    V3 color = to_color (hex_color);
    s32 image_w = image.w;
    s32 image_h = image.h;

    s32 start_x = center_x - rect_w/2;
    s32 finish_x = center_x + rect_w/2 + rect_w%2;
    if (start_x < 0) start_x = 0;
    if (start_x > image_w || finish_x < 0) return;
    if (finish_x > image_w) finish_x = image_w;

    s32 start_y = center_y - rect_h/2;
    s32 finish_y = center_y + rect_h/2 + rect_h%2;
    if (start_y < 0) start_y = 0;
    if (start_y > image_h || finish_y < 0) return;
    if (finish_y > image_h) finish_y = image_h;

    for (s32 y = start_y; y < finish_y; y++)
    {
        for (s32 x = start_x; x < finish_x; x++)
        {
            image.pixels[y * image.w + x] = color;
        }
    }
}


static void
draw_square (Image image, s32 center_x, s32 center_y, u32 square_w, u32 hex_color = 0x000000)
{
draw_rectangle (image, center_x, center_y, square_w, square_w, hex_color);
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
    {set.r +=17; set.g+=17; set.b+=17;}
    hex_color = set.r*256*256 + set.g*256 +set.b;

    return hex_color;
}



