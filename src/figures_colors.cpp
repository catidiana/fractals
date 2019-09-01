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

static void
draw_square (Image image, s32 center_x, s32 center_y, u32 square_w, u32 hex_color = 0x000000)
{
draw_rectangle (image, center_x, center_y, square_w, square_w, hex_color);
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




