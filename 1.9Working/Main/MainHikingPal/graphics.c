/*
 * "Hello World" example.
 *
 * This example prints 'Hello from Nios II' to the STDOUT stream. It runs on
 * the Nios II 'standard', 'full_featured', 'fast', and 'low_cost' example
 * designs. It runs with or without the MicroC/OS-II RTOS and requires a STDOUT
 * device in your system's hardware.
 * The memory footprint of this hosted application is ~69 kbytes by default
 * using the standard reference design.
 *
 * For a reduced footprint version of this template, and an explanation of how
 * to reduce the memory footprint for a given application, see the
 * "small_hello_world" template.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "graphics.h"
#include "Colours.h"

extern const unsigned int ColourPalletteData[256];

/**********************************************************************
* This function writes a single pixel to the x,y coords specified in the specified colour
* Note colour is a palette number (0-255) not a 24 bit RGB value
**********************************************************************/
void WriteAPixel (int x, int y, int Colour)
{
	WAIT_FOR_GRAPHICS;			// is graphics ready for new command

	GraphicsX1Reg = x;			// write coords to x1, y1
	GraphicsY1Reg = y;
	GraphicsColourReg = Colour;		// set pixel colour with a palette number
	GraphicsCommandReg = PutAPixel;		// give graphics a "write pixel" command
}

/*****************************************************************************************
* This function read a single pixel from x,y coords specified and returns its colour
* Note returned colour is a palette number (0-255) not a 24 bit RGB value
******************************************************************************************/
int ReadAPixel (int x, int y)
{
	WAIT_FOR_GRAPHICS;			// is graphics ready for new command

	GraphicsX1Reg = x;			// write coords to x1, y1
	GraphicsY1Reg = y;
	GraphicsCommandReg = GetAPixel;		// give graphics a "get pixel" command

	WAIT_FOR_GRAPHICS;			// is graphics done reading pixel
	return (int)(GraphicsColourReg) ;		// return the palette number (colour)
}

void DrawHorizontalLine(int x1, int x2, int y, int Colour)
{
	WAIT_FOR_GRAPHICS;

	GraphicsX1Reg = x1;
	GraphicsX2Reg = x2;
	GraphicsY1Reg = y;
	GraphicsColourReg = Colour;
	GraphicsCommandReg = DrawHLine;
}

void DrawVerticalLine(int y1, int y2, int x, int Colour)
{
	WAIT_FOR_GRAPHICS;

	GraphicsY1Reg = y1;
	GraphicsY2Reg = y2;
	GraphicsX1Reg = x;
	GraphicsColourReg = Colour;
	GraphicsCommandReg = DrawVLine;
}

void DrawBresenhamLine(int x1, int x2, int y1, int y2, int Colour)
{
	WAIT_FOR_GRAPHICS;

	GraphicsX1Reg = x1;
	GraphicsX2Reg = x2;
	GraphicsY1Reg = y1;
	GraphicsY2Reg = y2;
	GraphicsColourReg = Colour;
	GraphicsCommandReg = DrawLine;
}

void DrawString1(int x, int y, int colour, int background, char* string, int erase){
	int i;
	for (i = 0; string[i] != '\0'; ++i){
		OutGraphicsCharFont1(x, y, colour, background, string[i], erase);
		x += 10;
	}
}

void DrawString2(int x, int y, int colour, int background, char* string, int erase){
	int i;
	for (i = 0; string[i] != '\0'; ++i){
		OutGraphicsCharFont2(x, y, colour, background, string[i], erase);
		x += 14;
	}
}

void DrawRectangle(int x1, int x2, int y1, int y2, int colour){
	DrawHorizontalLine(x1, x2, y1, colour);
	DrawHorizontalLine(x1, x2, y2, colour);
	DrawVerticalLine(y1, y2, x1, colour);
	DrawVerticalLine(y1, y2, x2, colour);
}

void DrawFilledRectangle(int x1, int x2, int y1, int y2, int colour){
	WAIT_FOR_GRAPHICS;
	GraphicsX1Reg = x1;
	GraphicsX2Reg = x2;
	GraphicsY1Reg = y1;
	GraphicsY2Reg = y2;
	GraphicsColourReg = colour;
	GraphicsCommandReg = DrawFilledRect;
}

void DrawBresenhamCircle(int x1, int y1, int radius, int colour){
	WAIT_FOR_GRAPHICS;
	GraphicsX1Reg = x1;
	GraphicsY1Reg = y1;
	GraphicsX2Reg = radius;
	GraphicsColourReg = colour;
	GraphicsCommandReg = DrawCircle;
}

int MapToColour(int r, int g, int b){
	int r2 = ((r + ColourDiff / 2) / ColourDiff) * ColoursInRGB * ColoursInRGB;
	int g2 = ((g + ColourDiff / 2) / ColourDiff) * ColoursInRGB;
	int b2 = ((b + ColourDiff / 2) / ColourDiff);
	return r2 + g2 + b2 + CustomColorIndex;
}

int GetClosetColour(int r, int g, int b){
	Colour c = 0;
	int minDiff = 256 * 3;
	Colour minC = 0;
	for (c = BLACK; c <= WHITE_REPEAT; ++c){
		int currR = ColourPalletteData[c] >> 16;
		int currG = (ColourPalletteData[c] & 0x00FF00) >> 8;
		int currB = ColourPalletteData[c] & 0x0000FF;
		int currDiff = abs(r - currR) + abs(g - currG) + abs(b - currB);
		if (currDiff < minDiff){
			minDiff = currDiff;
			minC = c;
		}
		if (minDiff <= 70){
			return minC;
		}
	}

	return minC;
}

// Draws a map from a 24-bit bitmap where the lower left is at (x,y) and option to scale the image (stretch)
void DrawMap(char *fileName, int x, int y, int length, int width, int scale){
	 // super-simplified BMP read algorithm to pull out RGB data
	 // read image for coloring scheme
	 //int image[400*400][3]; // first number here is 1024 pixels in my image, 3 is for RGB values
	 FILE *streamIn;
	 streamIn = fopen(fileName, "r");
	 if (streamIn == (FILE *)0){
	   perror("File opening error ocurred. Exiting program.\n");
	   exit(0);
	 }

	 int byte;
	 int i;
	 for(i=0; i<54; i++) byte = getc(streamIn);  // strip out BMP header

	 int currX = x;
	 int currY = y;

	 for (i=0; i < length*width; i++){    // foreach pixel
	    int b = fgetc(streamIn);  // use BMP 24bit with no alpha channel
	    int g = fgetc(streamIn);  // BMP uses BGR but we want RGB, grab byte-by-byte
	    int r = fgetc(streamIn);  // reverse-order array indexing fixes RGB issue...

	    Colour c = GetClosetColour(r, g, b);
	    //printf("C: %x, Pixel %d : [%d,%d,%d]\n", ColourPalletteData[c], i+1,r,g,b);

	    if (i % length == 0){
	    	currX = x;
	    	currY -= scale;
	    }
	    else{
	    	currX += scale;
	    }

	    int scaleX;
	    int scaleY;
	    for (scaleY = currY; scaleY > currY - scale; --scaleY){
	    	for (scaleX = currX; scaleX < currX + scale; ++scaleX){
	    		WriteAPixel(scaleX, scaleY, c);
	    	}
	    }
	 }

	 fclose(streamIn);
}

// Same as DrawMap but with a more efficient bitmap decoding implementation
void DrawMap2(char *fileName, int x, int y, int length, int width, int scale){
	 // super-simplified BMP read algorithm to pull out RGB data
	 // read image for coloring scheme
	 //int image[400*400][3]; // first number here is 1024 pixels in my image, 3 is for RGB values
	 FILE *streamIn;
	 streamIn = fopen(fileName, "rb");
	 if (streamIn == (FILE *)0){
	   perror("File opening error ocurred. Exiting program.\n");
	   exit(0);
	 }

	 int i;

	 unsigned char info[54];
	 for(i=0; i<54; i++) getc(streamIn);  // strip out BMP header

	 int row_padded = (length*3 + 3) & (~3);
	 unsigned char data[width][row_padded];

	 int row, col;
	 for(row = 0; row < width; ++row){
		 fread(data[width - 1 - row], sizeof(unsigned char), row_padded, streamIn);
	 }
	 printf("boom\n");
	 int currX = x;
	 int currY = y;
	 for(row = 0; row < width; ++row){
		 for(col = 0; col < length * 3; col += 3){
			 //printf("Pixel: %d, R:%d, G:%d, B:%d\n", count++, (int)data[row][col], (int)data[row][col + 1], (int)data[row][col + 2]);
			 int c = MapToColour(data[row][col + 2], data[row][col + 1], data[row][col]);

			 int currX2, currY2;
			 for(currY2 = currY; currY2 < currY + scale; ++currY2){
				 for(currX2 = currX; currX2 < currX + scale; ++currX2){
					 WriteAPixel(currX2, currY2, c);
				 }
			 }
			 currX = currX2;
		 }

		 currX = x;
		 currY += scale;
	 }

	 fclose(streamIn);
}

void DrawMap3(char *fileName, int x, int y, int length, int width, int scale){
		 FILE *streamIn;
		 streamIn = fopen(fileName, "rb");
		 if (streamIn == (FILE *)0){
		   perror("File opening error ocurred. Exiting program.\n");
		   exit(0);
		 }

		 int i;

		 unsigned char info[54];
		 for(i=0; i<54; i++) getc(streamIn);  // strip out BMP header

		 int row_padded = (length*3 + 3) & (~3);
		 unsigned char data[row_padded * width];

		 int row, col, count;
		 row = col = count = 0;
		 fread(data, sizeof(unsigned char), row_padded * width, streamIn);

		 fclose(streamIn);
}

void TestShapes(){
	// draw a line across the screen in RED at y coord 100 and from x = 0 to 799
	//for(i = 0; i < 800; i ++)
		//WriteAPixel(i, 100, RED);

	// read the pixels back and make sure we read 2 (RED) to prove it's working
		//for(i = 0; i < 800; i ++)
			//printf("Colour value (i.e. pallette number) = %d at [%d, 100]\n", ReadAPixel(i, 100), i);

	DrawHorizontalLine(0, 800, 400, CYAN);
	DrawVerticalLine(0, 480, 400, MAGENTA);
	DrawBresenhamLine(0, 400, 0, 300, YELLOW);
	DrawBresenhamLine(0, 400, 300, 0, YELLOW);
	DrawBresenhamLine(600, 300, 0, 300, YELLOW);
	DrawBresenhamLine(600, 300, 300, 0, YELLOW);

	DrawString1(300, 150, RED, WHITE, "abcderfg", 1);
	DrawString2(400, 150, RED, WHITE, "abcderfg", 0);

	DrawRectangle(30, 200, 50, 150, CYAN);

	DrawFilledRectangle(100, 200, 300, 350, 12);

	DrawBresenhamCircle(400, 400, 50, BLUE);
}


/****************************************************************************************************
** subroutine to program a hardware (graphics chip) palette number with an RGB value
** e.g. ProgramPalette(RED, 0x00FF0000) ;
****************************************************************************************************/

void ProgramPalette(int PaletteNumber, int RGB)
{
    WAIT_FOR_GRAPHICS;
    GraphicsColourReg = PaletteNumber;
    GraphicsX1Reg = RGB >> 16   ;          // program red value in ls.8 bit of X1 reg
    GraphicsY1Reg = RGB ;                	 // program green and blue into 16 bit of Y1 reg
    GraphicsCommandReg = ProgramPaletteColour;	// issue command
}
