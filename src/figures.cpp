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
draw_pixel (Image image, r64 x, r64 y, V3 color)
{
    u32 x1 = floor(x);
    u32 y1 = floor(y);
    u32 x2 = ceil(x);
    u32 y2 = ceil(y);

    r64 c1 = (x-floor(x))*(y-floor(y));
    V3 existing_color1 = image.pixels[y1 * image.w + x1];
    V3 new_color1 = {};
    new_color1.r = (1-c1)*existing_color1.r + c1*color.r;
    new_color1.g = (1-c1)*existing_color1.g + c1*color.g;
    new_color1.b = (1-c1)*existing_color1.b + c1*color.b;
    image.pixels[y1 * image.w + x1] = new_color1;

    r64 c2 = (floor(x))*(y-floor(y));
    V3 existing_color2 = image.pixels[y1 * image.w + x2];
    V3 new_color2 = {};
    new_color2.r = (1-c2)*existing_color2.r + c2*color.r;
    new_color2.g = (1-c2)*existing_color2.g + c2*color.g;
    new_color2.b = (1-c2)*existing_color2.b + c2*color.b;
    image.pixels[y1 * image.w + x2] = new_color2;

    r64 c3 = (x - floor(x))*(floor(y));
    V3 existing_color3 = image.pixels[y2 * image.w + x1];
    V3 new_color3 = {};
    new_color3.r = (1-c3)*existing_color3.r + c3*color.r;
    new_color3.g = (1-c3)*existing_color3.g + c3*color.g;
    new_color3.b = (1-c3)*existing_color3.b + c3*color.b;
    image.pixels[y2 * image.w + x1] = new_color3;

    r64 c4 = (floor(x))*(floor(y));
    V3 existing_color4 = image.pixels[y2 * image.w + x2];
    V3 new_color4 = {};
    new_color4.r = (1-c4)*existing_color4.r + c4*color.r;
    new_color4.g = (1-c4)*existing_color4.g + c4*color.g;
    new_color4.b = (1-c4)*existing_color4.b + c4*color.b;
    image.pixels[y2 * image.w + x2] = new_color4;




}


static void
uniform_fill (Image image, u32 hex_color)
{
  V3 color = to_color (hex_color);

  for (u32 y = 0; y < image.h; y++)
    {
      for (u32 x = 0; x < image.w; x++)
        {
          image.pixels[y * image.w + x] = color;
        }
    }
}


static void
draw_square (Image image, s32 center_x, s32 center_y, u32 square_w, u32 hex_color = 0x000000)
{
    V3 color = to_color (hex_color);
    s32 image_w = image.w;
    s32 image_h = image.h;

    s32 start_x = center_x - square_w/2;
    s32 finish_x = center_x + square_w/2 + square_w%2;
    if (start_x < 0) start_x = 0;
    if (start_x > image_w) return;
    if (finish_x < 0) return;
    if (finish_x > image_w) finish_x = image_w;

    s32 start_y = center_y - square_w/2;
    s32 finish_y = center_y + square_w/2 + square_w%2;
    if (start_y < 0) start_y = 0;
    if (start_y > image_h) return;
    if (finish_y < 0) return;
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
draw_rectangle (Image image, s32 center_x, s32 center_y, u32 rect_w, u32 rect_h, u32 hex_color = 0x000000)
{
    V3 color = to_color (hex_color);
    s32 image_w = image.w;
    s32 image_h = image.h;

    s32 start_x = center_x - rect_w/2;
    s32 finish_x = center_x + rect_w/2 + rect_w%2;
    if (start_x < 0) start_x = 0;
    if (start_x > image_w) return;
    if (finish_x < 0) return;
    if (finish_x > image_w) finish_x = image_w;

    s32 start_y = center_y - rect_h/2;
    s32 finish_y = center_y + rect_h/2 + rect_h%2;
    if (start_y < 0) start_y = 0;
    if (start_y > image_h) return;
    if (finish_y < 0) return;
    if (finish_y > image_h) finish_y = image_h;

    for (s32 y = start_y; y < finish_y; y++)
    {
        for (s32 x = start_x; x < finish_x; x++)
        {
            image.pixels[y * image.w + x] = color;
        }
    }
}

