// Header file for chapter 3 - BLACK3.H

// D E F I N E S /////////////////////////////////////////////////////////////

#define ROM_CHAR_WIDTH      8     // width of ROM 8x8 character set in pixels
#define ROM_CHAR_HEIGHT     8     // height of ROM 8x8 character set in pixels

#define MODE13_WIDTH   (unsigned int)320 // mode 13h screen dimensions
#define MODE13_HEIGHT  (unsigned int)200

// M A C R O S ///////////////////////////////////////////////////////////////

#define SET_BITS(x,bits)   (x | bits)
#define RESET_BITS(x,bits) (x & ~bits)

// S T R U C T U R E S ///////////////////////////////////////////////////////

// this structure holds a RGB triple in three unsigned bytes
typedef struct RGB_color_typ
{
	unsigned char red;    // red   component of color 0-63
	unsigned char green;  // green component of color 0-63
	unsigned char blue;   // blue  component of color 0-63
} RGB_color, *RGB_color_ptr;

// this structure holds an entire color palette
typedef struct RGB_palette_typ
{
	int start_reg;   // index of the starting register that is save
	int end_reg;     // index of the ending registe that is saved
	RGB_color colors[256];   // the storage area for the palette
} RGB_palette, *RGB_palette_ptr;

// P R O T O T Y P E S ///////////////////////////////////////////////////////

// G L O B A L S //////////////////////////////////////////////////////////////

// F U N C T I O N S /////////////////////////////////////////////////////////

void Print_Char(int xc, int yc, char c, int color, int transparent)
{
	// this function is used to print a character on the screen. It uses the
	// internal 8x8 character set to do this. Note that each character is
	// 8 bytes where each byte represents the 8 pixels that make up the row
	// of pixels

	int offset,               // offset into video memory
		x,               // loop variable
		y;               // loop variable

	unsigned char *work_char; // pointer to character being printed

	unsigned char bit_mask;       // bitmask used to extract proper bit

	// compute starting offset in rom character lookup table
	// multiple the character by 8 and add the result to the starting address
	// of the ROM character set
	work_char = RETRO_FontData[c - 32];

	// compute offset of character in video buffer, use shifting to multiply
	offset = (yc << 8) + (yc << 6) + xc;

	// draw the character row by row
	for (y = 0; y < ROM_CHAR_HEIGHT; y++) {
		// reset bit mask
		bit_mask = 0x80;

		// draw each pixel of this row
		for (x = 0; x < ROM_CHAR_WIDTH; x++) {
			// test for transparent pixel i.e. 0, if not transparent then draw
			if (*work_char & (1 << x)) {
				RETRO.framebuffer[offset + x] = (unsigned char)color;
			} else if (!transparent) {               // takes care of transparency
				RETRO.framebuffer[offset + x] = 0; // make black part opaque
			}

			// shift bit mask
			bit_mask = (bit_mask >> 1);
		}

		// move to next line in video buffer and in rom character data area
		offset += MODE13_WIDTH;
		work_char++;
	}
}

//////////////////////////////////////////////////////////////////////////////

void Print_String(int x, int y, int color, const char *string, int transparent)
{
	// this function prints an entire string on the screen with fixed spacing
	// between each character by calling the Print_Char() function

	int index,   // loop index
		length;  // length of string

	// compute length of string
	length = strlen(string);

	// print the string a character at a time
	for (index = 0; index < length; index++) {
		Print_Char(x + (index << 3), y, string[index], color, transparent);
	}
}

///////////////////////////////////////////////////////////////////////////////

void Write_Pixel(int x, int y, int color)
{
	// plots the pixel in the desired color a little quicker using binary shifting
	// to accomplish the multiplications

	// use the fact that 320*y = 256*y + 64*y = y<<8 + y<<6
	RETRO.framebuffer[((y << 8) + (y << 6)) + x] = (unsigned char)color;
}

///////////////////////////////////////////////////////////////////////////////

int Read_Pixel(int x, int y)
{
	// this function read a pixel from the screen buffer

	// use the fact that 320*y = 256*y + 64*y = y<<8 + y<<6
	return((int)(RETRO.framebuffer[((y << 8) + (y << 6)) + x]));
}

///////////////////////////////////////////////////////////////////////////////

void Time_Delay(int clicks)
{
	// this function uses the internal timer to wait a specified number of "clicks"
	// the actual amount of real time is the number of clicks * (time per click)
	// usually the time per click is set to 1/18th of a second or 55ms

	SDL_Delay(55 * clicks);
}

///////////////////////////////////////////////////////////////////////////////

void Time_Delay_ms(int ms)
{
	// this function uses the internal timer to wait a specified number of milliseconds

	SDL_Delay(ms);
}

///////////////////////////////////////////////////////////////////////////////

void Line_H(int x1, int x2, int y, int color)
{
	// draw a horizontal line using the memset function
	// this function does not do clipping hence x1,x2 and y must all be within
	// the bounds of the screen

	int temp; // used for temporary storage during endpoint swap

	// sort x1 and x2, so that x2 > x1
	if (x1 > x2) {
		temp = x1;
		x1 = x2;
		x2 = temp;
	}

	// draw the row of pixels
	memset((char *)(RETRO.framebuffer + ((y << 8) + (y << 6)) + x1), (unsigned char)color, x2 - x1 + 1);
}

//////////////////////////////////////////////////////////////////////////////

void Line_V(int y1, int y2, int x, int color)
{
	// draw a vertical line, note that a memset function can no longer be
	// used since the pixel addresses are no longer contiguous in memory
	// note that the end points of the line must be on the screen

	unsigned char *start_offset; // starting memory offset of line

	int index, // loop index
		temp;  // used for temporary storage during swap

	// make sure y2 > y1
	if (y1 > y2) {
		temp = y1;
		y1 = y2;
		y2 = temp;
	}

	// compute starting position
	start_offset = RETRO.framebuffer + ((y1 << 8) + (y1 << 6)) + x;

	for (index = 0; index <= y2 - y1; index++) {
		// set the pixel
		*start_offset = (unsigned char)color;

		// move downward to next line
		start_offset += 320;
	}
}

///////////////////////////////////////////////////////////////////////////////

void Write_Color_Reg(int index, RGB_color_ptr color)
{
	// this function is used to write a color register with the RGB value
	// within "color"

	RETRO_Set6bitColor(index, color->red, color->green, color->blue);
}

///////////////////////////////////////////////////////////////////////////////

RGB_color_ptr Read_Color_Reg(int index, RGB_color_ptr color)
{
	// this function reads the RGB triple out of a palette register and places it
	// into "color"

	RETRO_Palette palette;
	
	palette = RETRO_Get6bitColor(index);

	color->red = palette.r;
	color->green = palette.g;
	color->blue = palette.b;

	// return a pointer to color so that the function can be used as an RVALUE
	return(color);
}

///////////////////////////////////////////////////////////////////////////////

void Read_Palette(int start_reg, int end_reg, RGB_palette_ptr the_palette)
{
	// this function will read the palette registers in the range start_reg to
	// end_reg and save them into the appropriate positions in colors

	int index; // used for looping

	RGB_color color;

	// read all the registers
	for (index = start_reg; index <= end_reg; index++) {
		// read the color register
		Read_Color_Reg(index, (RGB_color_ptr)&color);

		// save it in database
		the_palette->colors[index].red = color.red;
		the_palette->colors[index].green = color.green;
		the_palette->colors[index].blue = color.blue;
	}

	// save the interval of registers that were saved
	the_palette->start_reg = start_reg;
	the_palette->end_reg = end_reg;
}

///////////////////////////////////////////////////////////////////////////////

void Write_Palette(int start_reg, int end_reg, RGB_palette_ptr the_palette)
{
	// this function will write to the color registers using the data in the
	// sen palette structure

	int index; // used for looping

	// write all the registers
	for (index = start_reg; index <= end_reg; index++) {
		// write the color registers using the data from the sent palette
		Write_Color_Reg(index, (RGB_color_ptr) & (the_palette->colors[index]));
	}
}

///////////////////////////////////////////////////////////////////////////////

void Draw_Rectangle(int x1, int y1, int x2, int y2, int color)
{
	// this function will draw a rectangle from (x1,y1) - (x2,y2)
	// it is assumed that each endpoint is within the screen boundaries
	// and (x1,y1) is the upper left hand corner and (x2,y2) is the lower
	// right hand corner

	unsigned char *start_offset;   // starting memory offset of first row

	int width;   // width of rectangle

	// compute starting offset of first row
	start_offset = RETRO.framebuffer + ((y1 << 8) + (y1 << 6)) + x1;

	// compute width of rectangle
	width = 1 + x2 - x1; // the "1" to reflect the true width in pixels

	// draw the rectangle row by row
	while (y1++ <= y2) {
		// draw the line
		memset((char *)start_offset, (unsigned char)color, width);

		// move the memory pointer to the next line
		start_offset += 320;
	}
}

////////////////////////////////////////////////////////////////////////////////

void Fill_Screen(int color)
{
	// this function will fill the entire screen with the sent color

	memset((char *)RETRO.framebuffer, (unsigned char)color, 320 * 200);
}

//////////////////////////////////////////////////////////////////////////////
