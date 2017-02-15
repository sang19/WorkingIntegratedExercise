#ifndef GRAPHICS_H_
#define GRAPHICS_H_

#define TRUE 1
#define FALSE 0

#define XRES 800
#define YRES 480

#define ColourDiff 86 // Difference between each colour in custom colors i.e 0x000000, 0x000056
#define ColoursInRGB 4  // Number of Colours in each RGB: 0x00, 0x56, 0xAC, 0xFF
#define CustomColorIndex 151 // Where the custom colors start in ColourPalleteData[]

// #defined constants representing values we write to the graphics 'command' register to get
// it to do something. You will add more values as you add hardware to the graphics chip
// Note DrawHLine, DrawVLine and DrawLine at the moment do nothing - you will modify these

#define DrawHLine		1
#define DrawVLine		2
#define DrawLine		3
#define DrawFilledRect  4
#define DrawCircle		5
#define PutAPixel		0xA
#define GetAPixel		0xB
#define ProgramPaletteColour   	0x10

/*******************************************************************************************
** This macro pauses until the graphics chip status register indicates that it is idle
*******************************************************************************************/

#define WAIT_FOR_GRAPHICS		while((GraphicsStatusReg & 0x0001) != 0x0001);

//Predefined Colour Values
//Use the symbolic constants below as the values to write to the Colour Register
//When you ask the graphics chip to draw something. These values are also returned
//By the read pixel command

// the header file "Colours.h" contains symbolic names for all 256 colours e.g. RED
// while the source file ColourPaletteData.c contains the 24 bit RGB data
// that is pre-programmed into each of the 256 palettes

#define GraphicsCommandReg   		(*(volatile unsigned short int *)(0x84000000))
#define GraphicsStatusReg   		(*(volatile unsigned short int *)(0x84000000))
#define GraphicsX1Reg   		(*(volatile unsigned short int *)(0x84000002))
#define GraphicsY1Reg   		(*(volatile unsigned short int *)(0x84000004))
#define GraphicsX2Reg   		(*(volatile unsigned short int *)(0x84000006))
#define GraphicsY2Reg			(*(volatile unsigned short int *)(0x84000008))
#define GraphicsColourReg		(*(volatile unsigned short int *)(0x8400000E))
#define GraphicsBackGroundColourReg   		(*(volatile unsigned short int *)(0x84000010))

void WriteAPixel (int x, int y, int Colour);
int ReadAPixel (int x, int y);
void DrawHorizontalLine(int x1, int x2, int y, int Colour);
void DrawVerticalLine(int y1, int y2, int x, int Colour);
void DrawBresenhamLine(int x1, int x2, int y1, int y2, int Colour);
void DrawString1(int x, int y, int colour, int background, char* string, int erase);
void DrawString2(int x, int y, int colour, int background, char* string, int erase);
void DrawRectangle(int x1, int x2, int y1, int y2, int colour);
void DrawFilledRectangle(int x1, int x2, int y1, int y2, int colour);
void DrawBresenhamCircle(int x1, int y1, int radius, int colour);
void ProgramPalette(int PaletteNumber, int RGB);
int GetClosetColour(int r, int g, int b);
void DrawMap(char *fileName, int x, int y, int length, int width, int scale);
void DrawMap2(char *fileName, int x, int y, int length, int width, int scale);
void TestShapes();

#endif
