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

//draw pixel with coordinates (x,y) and transarency c


static void
draw_pixel (Image image, s32 x, s32 y, u32 hex_color, r64 c)
{
    s32 image_w = image.w;
    s32 image_h = image.h;
    if (x<0 || x >=image_w || y<0 || y>=image_h ) return;

    V3 color = to_color (hex_color);
    V3 existing_color = image.pixels[y * image.w + x];
    V3 new_color = {};
    new_color.r = (1-c)*existing_color.r + c*color.r;
    new_color.g = (1-c)*existing_color.g + c*color.g;
    new_color.b = (1-c)*existing_color.b + c*color.b;
    image.pixels[y * image.w + x] = new_color;

}



static void
draw_vertical_line (Image image, u32 line_position_x, u32 hex_color = 0x000000, u32 line_width = 1)
{
    V3 color = to_color (hex_color);

    u32 line_start;
    if (line_position_x<line_width/2) line_start=0;
    else line_start = line_position_x-line_width/2;
    if (line_start>image.w) return;
    u32 line_end = line_position_x+line_width/2+line_width%2;
    if (line_end>image.w) line_end = image.w;

    for (u32 y = 0; y < image.h; y++)
    {
        for (u32 x = line_start; x < line_end; x++)
        {

            image.pixels[y * image.w + x] = color;
        }
    }
}


static void
draw_horizontal_line (Image image, u32 line_position_y, u32 hex_color = 0x000000, u32 line_width = 1)
{
    V3 color = to_color (hex_color);

    u32 line_start;
    if (line_position_y<line_width/2) line_start=0;
    else line_start = line_position_y-line_width/2;
    if (line_start>image.h) return;
    u32 line_end = line_position_y+line_width/2+line_width%2;
    if (line_end>image.h) line_end = image.h;

    for (u32 x = 0; x < image.w; x++)
    {
        for (u32 y = line_start; y < line_end; y++)
        {

            image.pixels[y * image.w + x] = color;
        }
    }
}


//put angle in degrees; corner_x corner_y - coordinates of the dot that should belong to line
static void
draw_angle_line (Image image, u32 angle = 45, u32 hex_color = 0x000000, u32 corner_x = 0, u32 corner_y = 0, u32 line_width = 1)
{
    V3 color = to_color (hex_color);
    s32 image_w = image.w;
    s32 image_h = image.h;

    angle = angle%180;
    if (angle==0)
    {
        draw_horizontal_line (image, corner_y, hex_color, line_width);
        return;
    }
    else if (angle==90)
    {
        draw_vertical_line (image, corner_x, hex_color, line_width);
        return;
    }
    else if (angle<=45 || angle >=135)
    {
        double alfa = tan(angle*M_PI/180.0);
        line_width = line_width/abs(cos(angle*M_PI/180.0));
        for (s32 x = 0; x < image_w; x++)
        {
            s32 y_start = corner_y + alfa*x - alfa*corner_x - line_width/2;
            s32 y_finish = corner_y + alfa*x - alfa*corner_x + line_width/2 +line_width%2;
            for (s32 y = y_start; y < y_finish; y++)
            {
                if(y >= 0 && y< image_h)

                    image.pixels[y * image.w + x] = color;
            }
        }
    }
    else {
        double alfa = 1/tan(angle*M_PI/180.0);
        line_width = line_width/sin(angle*M_PI/180.0);
        for (s32 y = 0; y < image_h; y++)
        {
            s32 x_start = corner_x + alfa*y - alfa*corner_y - line_width/2;
            s32 x_finish = corner_x + alfa*y - alfa*corner_y + line_width/2 +line_width%2;
            for (s32 x = x_start; x < x_finish; x++)
            {
                if(x >= 0 && x < image_w)

                    image.pixels[y * image.w + x] = color;
            }
        }

    }
}



static void
linear_bezier_curve (Image image, s32 P0_x, s32 P0_y, s32 P1_x, s32 P1_y, u32 hex_color = 0x000000)
{

    if (P1_x == P0_x && P1_y == P0_y)
    {
        return;
    }
    else if (abs(P1_x-P0_x)>=abs(P1_y-P0_y))
    {
        s32 step_x;
        if (P1_x > P0_x) {step_x = 1;}
        else {step_x = -1;}
        r64 y = P0_y;
        r64 step_y = (r64)(P1_y-P0_y)/(r64)(P1_x-P0_x);
        for (s32 x = P0_x; x < P1_x; x=x+step_x)
        {
            u32 y_floor = floor (y);
            r64 y_dev = y - y_floor;
            draw_pixel (image, x, y_floor, hex_color, 1-y_dev);
            draw_pixel (image, x, y_floor+1, hex_color, y_dev);
            y = y + step_y;
        }
    }

    else
    {
        s32 step_y;
        if (P1_y > P0_y) step_y = 1;
        else step_y = -1;
        r64 x = P0_x;
        r64 step_x = (r64)(P1_x-P0_x)/(r64)(P1_y-P0_y);
        for (s32 y = P0_y; y < P1_y; y=y+step_y)
        {
            u32 x_floor = floor (x);
            r64 x_dev = x - x_floor;
            draw_pixel (image, x_floor, y, hex_color, 1-x_dev);
            draw_pixel (image, x_floor+1, y, hex_color, x_dev);
            x = x + step_x;
        }
    }

}

static void
quadratic_bezier_curve (Image image, r64 P0_x, r64 P0_y, r64 P1_x, r64 P1_y, r64 P2_x, r64 P2_y, u32 hex_color = 0x000000)
{
    r64 distance1 = sqrt((P0_x - P2_x)*(P0_x - P2_x) + (P0_y - P2_y)*(P0_y - P2_y));
    r64 distance2 = sqrt((P0_x - P1_x)*(P0_x - P1_x) + (P0_y - P1_y)*(P0_y - P1_y));
    if(distance2 < 1 && distance1 < 1)
        
    {
        r64 x = P0_x;
        r64 y = P0_y;
        r64 cx = x-floor(x);
        r64 cy = y-floor(y);
        r64 anti_density = sqrt(distance1);
        draw_pixel (image, floor(x), floor(y), hex_color, anti_density*(1-cx)*(1-cy));
        draw_pixel (image, floor(x)+1, floor(y), hex_color, anti_density*cx*(1-cy));
        draw_pixel (image, floor(x), floor(y)+1, hex_color, anti_density*(1-cx)*cy);
        draw_pixel (image, floor(x)+1, floor(y)+1, hex_color, anti_density*cx*cy);
        
    }

    else {
        quadratic_bezier_curve (image, P0_x, P0_y, (P0_x+P1_x)/2, (P0_y+P1_y)/2, (P0_x +2*P1_x + P2_x)/4, (P0_y +2*P1_y + P2_y)/4, hex_color);
        quadratic_bezier_curve (image, (P0_x +2*P1_x + P2_x)/4, (P0_y +2*P1_y + P2_y)/4, (P1_x+P2_x)/2, (P1_y+P2_y)/2, P2_x, P2_y, hex_color);
    }

}


static void
cubic_bezier_curve (Image image, r64 P0_x, r64 P0_y, r64 P1_x, r64 P1_y, r64 P2_x, r64 P2_y, r64 P3_x, r64 P3_y, u32 hex_color = 0x000000)
{
    r64 distance1 = sqrt((P0_x - P3_x)*(P0_x - P3_x) + (P0_y - P3_y)*(P0_y - P3_y));
    r64 distance2 = sqrt((P0_x - P1_x)*(P0_x - P1_x) + (P0_y - P1_y)*(P0_y - P1_y));
    r64 distance3 = sqrt((P0_x - P2_x)*(P0_x - P2_x) + (P0_y - P2_y)*(P0_y - P2_y));
    if(distance3 < 1 && distance2 < 1 && distance1 < 1)

    {
        r64 x = P0_x;
        r64 y = P0_y;
        r64 cx = x-floor(x);
        r64 cy = y-floor(y);
        r64 anti_density = sqrt(distance1);
        draw_pixel (image, floor(x), floor(y), hex_color, anti_density*(1-cx)*(1-cy));
        draw_pixel (image, floor(x)+1, floor(y), hex_color, anti_density*cx*(1-cy));
        draw_pixel (image, floor(x), floor(y)+1, hex_color, anti_density*(1-cx)*cy);
        draw_pixel (image, floor(x)+1, floor(y)+1, hex_color, anti_density*cx*cy);

    }

    else {
        cubic_bezier_curve (image, P0_x, P0_y, (P0_x+P1_x)/2, (P0_y+P1_y)/2, (P0_x +2*P1_x + P2_x)/4, (P0_y +2*P1_y + P2_y)/4, (P0_x+3*P1_x+3*P2_x+P3_x)/8, (P0_y+3*P1_y+3*P2_y+P3_y)/8, hex_color);
        cubic_bezier_curve (image, (P0_x+3*P1_x+3*P2_x+P3_x)/8, (P0_y+3*P1_y+3*P2_y+P3_y)/8, (P1_x +2*P2_x + P3_x)/4, (P1_y +2*P2_y + P3_y)/4, (P3_x+P2_x)/2, (P3_y+P2_y)/2, P3_x, P3_y, hex_color);
    }

}
