/*************************************************************************************************
** This function draws a single ASCII character at the coord and colour specified
** it optionally ERASES the background colour pixels to the background colour
** This means you can use this to erase characters
**
** e.g. writing a space character with Erase set to true will set all pixels in the
** character to the background colour
**
*************************************************************************************************/

#include "graphics.h"
#include "OutGraphicsCharFont.h"

extern const unsigned char Font5x7[95][7];
extern const unsigned short int Font10x14[][14];
extern const unsigned char Font16x27[];
extern const unsigned char Font22x40[];
extern const unsigned char Font38x59[];

void OutGraphicsCharFont(int x, int y, int fontcolour, int backgroundcolour, int c, int Erase, int fontWidth, int fontHeight){

	register int row, column, theX = x, theY = y ;
	register int pixels ;
	register char theColour = fontcolour  ;
	register int BitMask, theC = c ;

	// if x,y coord off edge of screen don't bother
	// XRES and YRES are #defined to be 800 and 480 respectively
    if(((short)(x) > (short)(XRES-1)) || ((short)(y) > (short)(YRES-1)))
        return ;

	// if printable character subtract hex 20
	if(((short)(theC) >= (short)(' ')) && ((short)(theC) <= (short)('~'))) {
		theC = theC - 0x20 ;
		for(row = 0; (char)(row) < (char)(fontHeight); row ++)	{

			// get the bit pattern for row 0 of the character from the software font

			if (fontWidth == 5){
				pixels = Font5x7[theC][row];
			}
			else if (fontWidth == 10){
				pixels = Font10x14[theC][row];
			}

			BitMask =  1 << fontWidth - 1;

			for(column = 0; (char)(column) < (char)(fontWidth); column ++)	{

				// if a pixel in the character display it
				if((pixels & BitMask))
					WriteAPixel(theX+column, theY+row, theColour) ;

				else {
					if(Erase == TRUE)

				// if pixel is part of background (not part of character)
				// erase the background to value of variable BackGroundColour

						WriteAPixel(theX+column, theY+row, backgroundcolour) ;
				}
				BitMask = BitMask >> 1 ;
			}
		}
	}
}

void OutGraphicsCharFont1(int x, int y, int fontcolour, int backgroundcolour, int c, int Erase)
{
	OutGraphicsCharFont(x, y, fontcolour, backgroundcolour, c, Erase, 5, 7);
}

void OutGraphicsCharFont2(int x, int y, int fontcolour, int backgroundcolour, int c, int Erase)
{
	OutGraphicsCharFont(x, y, fontcolour, backgroundcolour, c, Erase, 10, 14);
}

void OutGraphicsCharFont3(int x, int y, int fontcolour, int backgroundcolour, int c, int Erase)
{
	OutGraphicsCharFont(x, y, fontcolour, backgroundcolour, c, Erase, 16, 27);
}

void OutGraphicsCharFont4(int x, int y, int fontcolour, int backgroundcolour, int c, int Erase)
{
	OutGraphicsCharFont(x, y, fontcolour, backgroundcolour, c, Erase, 22, 40);
}

void OutGraphicsCharFont5(int x, int y, int fontcolour, int backgroundcolour, int c, int Erase)
{
	OutGraphicsCharFont(x, y, fontcolour, backgroundcolour, c, Erase, 38, 59);
}
