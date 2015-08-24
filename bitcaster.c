/*
Copyright (c) 2004-2007, Lode Vandevenne
Copyright (c) 2015, Makapuf

All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

        * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
        * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <math.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h> // abs

#include "bitbox.h"

#include "terrain.h" // includes data


float posX = 22, posY = 12;  //x and y start position
float dirX = -1, dirY = 0; //initial direction vector
float planeX = 0, planeY = 0.66; //the 2d raycaster version of camera plane


// color and verical position from the top of each vertical line
uint16_t raycast_color [VGA_H_PIXELS]; 
uint8_t  raycast_y [VGA_H_PIXELS]; // assumes vertical resolution not more than 512

#define CEILING_COLOR RGB(40,60,80)
#define GROUND_COLOR RGB(40,40,40)


void _graph_line(void) 
{
    // draws elements 
    if (vga_line<=VGA_V_PIXELS/2) {
        for (int x=0;x<VGA_H_PIXELS;x++) {
                draw_buffer[x] = vga_line < raycast_y[x] ? CEILING_COLOR : raycast_color[x];
        }
    } else { // bottom
        for (int x=0;x<VGA_H_PIXELS;x++) {
            draw_buffer[x] = VGA_V_PIXELS - vga_line < raycast_y[x] ? GROUND_COLOR : raycast_color[x];
        }
    }
}


void graph_line(void) 
{
    uint32_t *y32 = (uint32_t *) raycast_y;
    uint32_t *buf32 = (uint32_t *) draw_buffer;
    uint32_t c32;
    // draws elements 
    if (vga_line<=VGA_V_PIXELS/2) { // top
        for (int x=0;x<VGA_H_PIXELS/4;x++,y32++) {
            c32  =  vga_line < (*y32     & 0xff) ? CEILING_COLOR : raycast_color[x*4];
            c32 |= (vga_line < (*y32>> 8 & 0xff) ? CEILING_COLOR : raycast_color[x*4+1]) << 16;
            *buf32++ = c32;

            c32  =  vga_line < (*y32>>16 & 0xff) ? CEILING_COLOR : raycast_color[x*4+2];
            c32 |= (vga_line < (*y32>>24 & 0xff) ? CEILING_COLOR : raycast_color[x*4+3]) << 16;
            *buf32++ = c32;
        }
    } else { // bottom
        int line = VGA_V_PIXELS - vga_line;
        for (int x=0;x<VGA_H_PIXELS/4;x++,y32++) {
            c32  =  line < (*y32    & 0xff) ? GROUND_COLOR : raycast_color[x*4];
            c32 |= (line < (*y32>>8 & 0xff) ? GROUND_COLOR : raycast_color[x*4+1]) << 16;
            *buf32++ = c32;

            c32  =  line < (*y32>>16 & 0xff) ? GROUND_COLOR : raycast_color[x*4+2];
            c32 |= (line < (*y32>>24 & 0xff) ? GROUND_COLOR : raycast_color[x*4+3])<<16;
            *buf32++ = c32;
        }
    }
}


void graph_frame(void) {}

void handle_userinput(void) {
    
    kbd_emulate_gamepad();

    // speed modifiers
    // asserts framerate of 60 fps, constant.
    float moveSpeed = 5.0f / 60.f; //the constant value is in squares/second
    float rotSpeed = 3.0f /60.f ; //the constant value is in radians/second

    // move forward if no wall in front of you
    if (GAMEPAD_PRESSED(0,up)) {
        if(worldMap[(int)(posX + dirX * moveSpeed)][(int)(posY)] == 0) 
            posX += dirX * moveSpeed;
        if(worldMap[(int)(posX)][(int)(posY + dirY * moveSpeed)] == 0) 
            posY += dirY * moveSpeed;
    }

    // move backwards if no wall behind you
    if (GAMEPAD_PRESSED(0,down)) {
        if(worldMap[(int)(posX - dirX * moveSpeed)][(int)(posY)] == 0) 
            posX -= dirX * moveSpeed;
        if(worldMap[(int)(posX)][(int)(posY - dirY * moveSpeed)] == 0) 
            posY -= dirY * moveSpeed;
    }

    // rotate to the right
    if (GAMEPAD_PRESSED(0,right))
    {
        //both camera direction and camera plane must be rotated
        float oldDirX = dirX;
        dirX = dirX * cosf(-rotSpeed) - dirY * sinf(-rotSpeed);
        dirY = oldDirX * sinf(-rotSpeed) + dirY * cosf(-rotSpeed);
        float oldPlaneX = planeX;
        planeX = planeX * cosf(-rotSpeed) - planeY * sinf(-rotSpeed);
        planeY = oldPlaneX * sinf(-rotSpeed) + planeY * cosf(-rotSpeed);
    }
    // rotate to the left
    if (GAMEPAD_PRESSED(0,left)) {
        //both camera direction and camera plane must be rotated
        float oldDirX = dirX;
        dirX = dirX * cosf(rotSpeed) - dirY * sinf(rotSpeed);
        dirY = oldDirX * sinf(rotSpeed) + dirY * cosf(rotSpeed);
        float oldPlaneX = planeX;
        planeX = planeX * cosf(rotSpeed) - planeY * sinf(rotSpeed);
        planeY = oldPlaneX * sinf(rotSpeed) + planeY * cosf(rotSpeed);
    }

    // strafe left
    if (GAMEPAD_PRESSED(0,L)) {
        if(worldMap[(int)(posX - dirY  * moveSpeed)][(int)(posY)] == 0) 
            posX -= dirY * moveSpeed;
        if(worldMap[(int)(posX)][(int)(posY + dirX * moveSpeed)] == 0) 
            posY += dirX * moveSpeed;
    }

    // strafe right
    if (GAMEPAD_PRESSED(0,R)) {
        if(worldMap[(int)(posX + dirY * moveSpeed)][(int)(posY)] == 0) 
            posX += dirY * moveSpeed;
        if(worldMap[(int)(posX)][(int)(posY - dirX * moveSpeed)] == 0) 
            posY -= dirX * moveSpeed;
    }

}


void game_init()
{
    //screen(512, 384, 0, "Raycaster");
}

void game_frame() 
{
    for(int x = 0; x < VGA_H_PIXELS; x++)
    {
        //calculate ray position and direction 
        float cameraX = 2.f * x / (float)(VGA_H_PIXELS) - 1.f; //x-coordinate in camera space
        float rayPosX = posX;
        float rayPosY = posY;
        float rayDirX = dirX + planeX * cameraX;
        float rayDirY = dirY + planeY * cameraX;
        //which box of the map we're in  
        int mapX = (int)(rayPosX);
        int mapY = (int)(rayPosY);
         
        //length of ray from current position to next x or y-side
        float sideDistX;
        float sideDistY;
         
         //length of ray from one x or y-side to next x or y-side
        float deltaDistX = sqrt(1 + (rayDirY * rayDirY) / (rayDirX * rayDirX));
        float deltaDistY = sqrt(1 + (rayDirX * rayDirX) / (rayDirY * rayDirY));
        float perpWallDist;
         
        //what direction to step in x or y-direction (either +1 or -1)
        int stepX;
        int stepY;

        int hit = 0; //was there a wall hit?
        int side =0; //was a NS or a EW wall hit?
        //calculate step and initial sideDist
        if (rayDirX < 0)
        {
            stepX = -1;
            sideDistX = (rayPosX - mapX) * deltaDistX;
        } else {
            stepX = 1;
            sideDistX = (mapX + 1.0f - rayPosX) * deltaDistX;
        }

        if (rayDirY < 0) {
            stepY = -1;
            sideDistY = (rayPosY - mapY) * deltaDistY;
        } else {
            stepY = 1;
            sideDistY = (mapY + 1.0f - rayPosY) * deltaDistY;
        }

        //perform DDA
        while (!hit)
        {
            //jump to next map square, OR in x-direction, OR in y-direction
            if (sideDistX < sideDistY)
            {
                sideDistX += deltaDistX;
                mapX += stepX;
                side = 0;
            }
            else
            {
                sideDistY += deltaDistY;
                mapY += stepY;
                side = 1;
            }
            //Check if ray has hit a wall
            if (worldMap[mapX][mapY] > 0) 
                hit = 1;
        } 

        //Calculate distance projected on camera direction (oblique distance will give fisheye effect!)
        if (side == 0)
            perpWallDist = fabsf((mapX - rayPosX + (1 - stepX) / 2) / rayDirX);
        else
            perpWallDist = fabsf((mapY - rayPosY + (1 - stepY) / 2) / rayDirY);
        
        //Calculate height of line to draw on screen
        int lineHeight = abs((int)(VGA_V_PIXELS / perpWallDist));
         
        // calculate pixel to fill in current stripe (will get other symmetrically for now. no need to clamp)
        int y = -lineHeight / 2 + VGA_V_PIXELS / 2;
        raycast_y[x] = y<0 ? 0 : y; // cast to unsigned 
        
        /*
        int drawStart = -lineHeight / 2 + VGA_V_PIXELS / 2;
        if(drawStart < 0)drawStart = 0;
        int drawEnd = lineHeight / 2 + h / 2;
        if(drawEnd >= h)drawEnd = h - 1;
        */


        // choose wall color
        uint16_t color = palette[worldMap[mapX][mapY]];

        // give x and y sides different brightness (? use a second palette ?)
        if (side) color &= 0b111001110011100;


        // will draw the pixels of the stripe as a vertical line
        raycast_color[x] = color;
    } // for x

    handle_userinput();
} // frame

