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


struct Image load_image (const char *Filename, u32 width, u32 height)
{
    FILE *file = fopen (Filename, "rb");
    assert (file);

    Image image_src;
    image_src.w = width;
    image_src.h = height;

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
draw_image (Image image_des, const char *Filename, u32 width, u32 height, u32 x_start, u32 y_start)
{
    Image image_src = load_image (Filename, width, height) ;
    for (u32 y = 0; y < image_src.h; y++)
    {
        for (u32 x = 0; x < image_src.w; x++)
        {
            image_des.pixels[(y+y_start)*image_des.w + x + x_start] = image_src.pixels[(image_src.h - y -1)*image_src.w + x];
        }
    }
}



static void
draw_digit (Image image, u32 width, u32 height, u32 x_pos, u32 y_pos, u32 digit)
{
    switch (digit)
    {
    case 0: draw_image (image, "res/0.data",  width, height, x_pos, y_pos); break;
    case 1: draw_image (image, "res/1.data",  width, height, x_pos, y_pos); break;
    case 2: draw_image (image, "res/2.data",  width, height, x_pos, y_pos); break;
    case 3: draw_image (image, "res/3.data",  width, height, x_pos, y_pos); break;
    case 4: draw_image (image, "res/4.data",  width, height, x_pos, y_pos); break;
    case 5: draw_image (image, "res/5.data",  width, height, x_pos, y_pos); break;
    case 6: draw_image (image, "res/6.data",  width, height, x_pos, y_pos); break;
    case 7: draw_image (image, "res/7.data",  width, height, x_pos, y_pos); break;
    case 8: draw_image (image, "res/8.data",  width, height, x_pos, y_pos); break;
    case 9: draw_image (image, "res/9.data",  width, height, x_pos, y_pos); break;
    default: break;
    }
}



static void
draw_integer (Image image, u32 number)
{
    if (number < 1000000)
    {
        u32 i = 0;
        while (number>0)
        {
            u32 digit = number%10;
            number = number/10;
            draw_digit (image, 10, 16, 106 - i, 13, digit);
            i=i+10;
        }
    }
    else {
        u32 power= 0;
        u32 firts_digit;
        u32 second_digit;
        while (number>=100) {
            number = number/10;
            power++;
        }
        power = power + 1;
        second_digit = number%10;
        firts_digit = number/10;
        draw_digit (image, 10, 16, 56, 13, firts_digit);
        draw_image (image, "res/dot.data", 7, 16, 66, 13);
        draw_digit (image, 10, 16, 73, 13, second_digit);
        draw_image (image, "res/e.data", 10, 16, 83, 13);
        draw_image (image, "res/plus.data", 10, 16, 93, 13);
        if (power <10)  draw_digit (image, 10, 16, 103, 13, power);
        else draw_image (image, "res/plus.data", 10, 16, 103, 13);

    }
}


static void
draw_double (Image image, r64 number, u32 x_pos, u32 y_pos)
{
    if (number<0)
    {
        draw_image (image, "res/minus.data", 10, 16, x_pos, y_pos);
        x_pos = x_pos + 10;
        number = - number;
    }
    if (number >= 0.0001)
    {
        u32 digit = floor(number);
        draw_digit (image, 10, 16, x_pos, y_pos, digit);
        draw_image (image, "res/dot.data", 7, 16, x_pos + 10, y_pos);
        for (u32 i = 2; i < 6; i++) {
            number = number - digit;
            number = number * 10;
            digit = floor(number);
            draw_digit (image, 10, 16, x_pos - 3 + 10*i, y_pos, digit);
        }
    }
    else if ( number < 0.0001 && number >= 0.000000001)
    {
        u32 power = 0;
        u32 firts_digit;
        u32 second_digit;
        while (number < 10) {
            number = number*10;
            power++;
        }
        power = power - 1;
        u32 num = floor(number);
        second_digit = num%10;
        firts_digit = num/10;
        draw_digit (image, 10, 16, x_pos, y_pos, firts_digit);
        draw_image (image, "res/dot.data", 7, 16, x_pos + 10, y_pos);
        draw_digit (image, 10, 16, x_pos + 17, y_pos, second_digit);
        draw_image (image, "res/e.data", 10, 16, x_pos + 27, y_pos);
        draw_image (image, "res/minus.data", 10, 16, x_pos + 37, y_pos);
        draw_digit (image, 10, 16, x_pos + 47, y_pos, power);
    }
    else {
        draw_image (image, "res/0.data", 10, 16, x_pos, y_pos);
        draw_image (image, "res/dot.data", 7, 16, x_pos + 10, y_pos);
        for (u32 i = 2; i < 6; i++)
        {
            draw_image (image, "res/0.data", 10, 16, x_pos - 3 + i*10, y_pos);
        }
    }
}
