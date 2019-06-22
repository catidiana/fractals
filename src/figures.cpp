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

