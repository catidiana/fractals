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


V2 linear (V2 coord)
{
    V2 coord2;
    coord2.x = coord.x;
    coord2.y = coord.y;
    return coord2;
}


V2 sinusoidal (V2 coord)
{
    V2 coord2;
    coord2.x = sin(coord.x);
    coord2.y = sin(coord.y);
    return coord2;
}


V2 spherical (V2 coord)
{
    r64 R = coord.x*coord.x + coord.y*coord.y;
    V2 coord2;
    coord2.x = coord.x/R;
    coord2.y = coord.y/R;
    return coord2;
}


V2 swirl (V2 coord)
{
    V2 coord2;
    r64 R = sqrt(coord.x*coord.x + coord.y*coord.y);
    r64 alfa;
    if (coord.x != 0) alfa = atan (coord.y/coord.x);
    else if (coord.y>0) alfa = M_PI/2;
    else if (coord.y<0) alfa = - M_PI/2;
    else alfa = 0;
    coord2.x = R*cos(2*alfa+R);
    coord2.y = R*sin(2*alfa+R);
    return coord2;
}


V2 horseshoe (V2 coord)
{
    V2 coord2;
    r64 R = sqrt(coord.x*coord.x + coord.y*coord.y);
    r64 alfa;
    if (coord.x != 0) alfa = atan (coord.y/coord.x);
    else if (coord.y>0) alfa = M_PI/2;
    else if (coord.y<0) alfa = - M_PI/2;
    else alfa = 0;
    coord2.x = R*cos(4*alfa);
    coord2.y = R*sin(4*alfa);
    return coord2;
}


V2 polar (V2 coord)
{
    V2 coord2;
    r64 alfa;
    if (coord.x != 0) alfa = atan (coord.y/coord.x);
    else if (coord.y>0) alfa = M_PI/2;
    else if (coord.y<0) alfa = - M_PI/2;
    else alfa = 0;
    r64 R = sqrt(coord.x*coord.x + coord.y*coord.y);
    coord2.x = 2*alfa/M_PI;
    coord2.y = R - 1;
    return coord2;
}


V2 handkerchief (V2 coord)
{
    V2 coord2;
    r64 R = sqrt(coord.x*coord.x + coord.y*coord.y);
    r64 alfa;
    if (coord.x != 0) alfa = atan (coord.y/coord.x);
    else if (coord.y>0) alfa = M_PI/2;
    else if (coord.y<0) alfa = - M_PI/2;
    else alfa = 0;
    coord2.x = R*sin(2*alfa+R);
    coord2.y = R*cos(2*alfa-R);
    return coord2;
}


V2 heart (V2 coord)
{
    V2 coord2;
    r64 R = sqrt(coord.x*coord.x + coord.y*coord.y);
    r64 alfa;
    if (coord.x != 0) alfa = atan (coord.y/coord.x);
    else if (coord.y>0) alfa = M_PI/2;
    else if (coord.y<0) alfa = - M_PI/2;
    else alfa = 0;
    coord2.x = R*sin(2*R*alfa);
    coord2.y = - R*cos(2*R*alfa);
    return coord2;
}


V2 disc (V2 coord)
{
    V2 coord2;
    r64 R = sqrt(coord.x*coord.x + coord.y*coord.y);
    r64 alfa;
    if (coord.x != 0) alfa = atan (coord.y/coord.x);
    else if (coord.y>0) alfa = M_PI/2;
    else if (coord.y<0) alfa = - M_PI/2;
    else alfa = 0;
    coord2.x = (2*alfa/M_PI)*sin(R*M_PI);
    coord2.y = (2*alfa/M_PI)*cos(R*M_PI);
    return coord2;
}


V2 spiral (V2 coord)
{
    V2 coord2;
    r64 R = sqrt(coord.x*coord.x + coord.y*coord.y);
    r64 alfa;
    if (coord.x != 0) alfa = atan (coord.y/coord.x);
    else if (coord.y>0) alfa = M_PI/2;
    else if (coord.y<0) alfa = - M_PI/2;
    else alfa = 0;
    coord2.x = (cos(2*alfa) + sin(R))/R;
    coord2.y = (sin(2*alfa) - cos(R))/R;
    return coord2;
}


V2 hyperbolic (V2 coord)
{
    V2 coord2;
    r64 R = sqrt(coord.x*coord.x + coord.y*coord.y);
    r64 alfa;
    if (coord.x != 0) alfa = atan (coord.y/coord.x);
    else if (coord.y>0) alfa = M_PI/2;
    else if (coord.y<0) alfa = - M_PI/2;
    else alfa = 0;
    coord2.x = sin(2*alfa)/R;
    coord2.y = cos(2*alfa)*R;
    return coord2;
}


V2 diamond (V2 coord)
{
    V2 coord2;
    r64 R = sqrt(coord.x*coord.x + coord.y*coord.y);
    r64 alfa;
    if (coord.x != 0) alfa = atan (coord.y/coord.x);
    else if (coord.y>0) alfa = M_PI/2;
    else if (coord.y<0) alfa = - M_PI/2;
    else alfa = 0;
    coord2.x = sin(2*alfa)*cos(R);
    coord2.y = cos(2*alfa)*sin(R);
    return coord2;
}

V2 ex (V2 coord)
{
    V2 coord2;
    r64 R = sqrt(coord.x*coord.x + coord.y*coord.y);
    r64 alfa;
    if (coord.x != 0) alfa = atan (coord.y/coord.x);
    else if (coord.y>0) alfa = M_PI/2;
    else if (coord.y<0) alfa = - M_PI/2;
    else alfa = 0;
    coord2.x = R*pow(sin(2*alfa+R), 3);
    coord2.y = R*pow(cos(2*alfa-R), 3);
    return coord2;
}



V2 julia (V2 coord)
{
    V2 coord2;
    r64 R = sqrt(coord.x*coord.x + coord.y*coord.y);
    r64 alfa;
    if (coord.x != 0) alfa = atan (coord.y/coord.x);
    else if (coord.y>0) alfa = M_PI/2;
    else if (coord.y<0) alfa = - M_PI/2;
    else alfa = 0;
    r64 tau = (rand()%2)*M_PI;
    coord2.x = sqrt(R)*cos(alfa + tau);
    coord2.y = sqrt(R)*sin(alfa + tau);
    return coord2;
}


