// Header file for chapter 4 - BLACK4.H

// D E F I N E S  ////////////////////////////////////////////////////////////

#define SCREEN_WIDTH      (unsigned int)320 // mode 13h screen dimensions
#define SCREEN_HEIGHT     (unsigned int)200

// screen transition commands
#define SCREEN_DARKNESS  0         // fade to black
#define SCREEN_WHITENESS 1         // fade to white
#define SCREEN_WARP      2         // warp the screen image
#define SCREEN_SWIPE_X   3         // do a horizontal swipe
#define SCREEN_SWIPE_Y   4         // do a vertical swipe
#define SCREEN_DISOLVE   5         // a pixel disolve

#define SPRITE_WIDTH      16       // default width of a sprite
#define SPRITE_HEIGHT     16       // default height of a sprite

#define MAX_SPRITE_FRAMES 32

// sprite states
#define SPRITE_DEAD       0
#define SPRITE_ALIVE      1
#define SPRITE_DYING      2

// M A C R O S ///////////////////////////////////////////////////////////////

#define SET_SPRITE_SIZE(w,h) { sprite_width = w; sprite_height = h; }

// S T R U C T U R E S ///////////////////////////////////////////////////////

// the PCX file structure

typedef struct pcx_header_typ
{
	char manufacturer;          // the manufacturer of the file
	char version;               // the file format version
	char encoding;              // type of compression
	char bits_per_pixel;        // number of bits per pixel
	short int x, y;                    // starting location of image
	short int width, height;           // size of image
	short int horz_res;               // resolution in DPI (dots per inch)
	short int vert_res;
	char ega_palette[48];       // the old EGA palette (usually ignored)
	char reserved;              // don't care
	char num_color_planes;      // number of color planes
	short int bytes_per_line;         // number of bytes per line of the image
	short int palette_type;           // 1 for color, 2 for grey scale palette
	char padding[58];           // extra bytes
} pcx_header, *pcx_header_ptr;

// this holds the PCX header and the actual image
typedef struct pcx_picture_typ
{
	pcx_header header;          // the header of the PCX file
	RGB_color palette[256];     // the palette data
	unsigned char *buffer;  // a pointer to the 64,000 byte buffer holding the decompressed image
} pcx_picture, *pcx_picture_ptr;

// this is a sprite structure
typedef struct sprite_typ
{
	int x, y;            // position of sprite
	int width, height;   // dimensions of sprite in pixels
	int counter_1;      // some counters for timing and animation
	int counter_2;
	int counter_3;
	int threshold_1;    // thresholds for the counters (if needed)
	int threshold_2;
	int threshold_3;
	unsigned char *frames[MAX_SPRITE_FRAMES]; // array of pointers to the images
	int curr_frame;                // current frame being displayed
	int num_frames;                // total number of frames
	int state;                     // state of sprite, alive, dead...
	unsigned char *background; // image under the sprite
	int x_clip, y_clip;             // clipped position of sprite
	int width_clip, height_clip;    // clipped size of sprite
	int visible;                   // used by sprite engine to flag
	// if a sprite was invisible last
	// time it was drawn hence the background
	// need not be replaced
} sprite, *sprite_ptr;

// this is the typedef for a bitmap
typedef struct bitmap_typ
{
	int x, y;                    // position of bitmap
	int width, height;           // size of bitmap
	unsigned char *buffer;  // buffer holding image
} bitmap, *bitmap_ptr;

// this is a typdef used for the layers in parallax scrolling
// note it is identical to a bitmap, but we'll make a separate typedef
// in the event we later need to add fields to it
typedef struct layer_typ
{
	int x, y;           // used to hold position information
	int width, height;  // size of layer,note:width must be divisible by 2
	unsigned char *buffer;  // the layer buffer
} layer, *layer_ptr;

// P R O T O T Y P E S ///////////////////////////////////////////////////////

// G L O B A L S  ////////////////////////////////////////////////////////////

// default sprite width and height

unsigned int sprite_width = SPRITE_WIDTH;
unsigned int sprite_height = SPRITE_HEIGHT;

// the default dimensions of the double buffer
unsigned int double_buffer_height = SCREEN_HEIGHT;

// size of double buffer
unsigned int double_buffer_size = SCREEN_WIDTH * SCREEN_HEIGHT;

// F U N C T I O N S /////////////////////////////////////////////////////////

int PCX_Init(pcx_picture_ptr image)
{
	// this function allocates the buffer that the image data will be loaded into
	// when a PCX file is decompressed

	if (!(image->buffer = (unsigned char *)malloc(SCREEN_WIDTH * SCREEN_HEIGHT + 1))) {
		printf("\nPCX SYSTEM - Couldn't allocate PCX image buffer");
		return(0);
	}

	// success
	return(1);
}

//////////////////////////////////////////////////////////////////////////////

int PCX_Load(const char *filename, pcx_picture_ptr image, int load_palette)
{
	// this function loads a PCX file into the image structure. The function
	// has three main parts: 1. load the PCX header, 2. load the image data and
	// decompress it and 3. load the palette data and update the VGA palette
	// note: the palette will only be loaded if the load_palette flag is 1

	FILE *fp;                // the file pointer used to open the PCX file

	int num_bytes,           // number of bytes in current RLE run
		index;               // loop variable

	long count;              // the total number of bytes decompressed
	unsigned char data;      // the current pixel data
	char *temp_buffer;   // working buffer

	// open the file, test if it exists
	if ((fp = fopen(filename, "rb")) == NULL) {
		printf("\nPCX SYSTEM - Couldn't find file: %s", filename);
		return(0);
	}

	// load the header
	temp_buffer = (char *)image;

	for (index = 0; index < 128; index++) {
		temp_buffer[index] = (char)getc(fp);
	}

	// load the data and decompress into buffer, we need a total of 64,000 bytes
	count = 0;

	// loop while 64,000 bytes haven't been decompressed
	while (count <= SCREEN_WIDTH * SCREEN_HEIGHT) {
		// get the first piece of data
		data = (unsigned char)getc(fp);

		// is this a RLE run?
		if (data >= 192 && data <= 255) {
			// compute number of bytes in run
			num_bytes = data - 192;

			// get the actual data for the run
			data = (unsigned char)getc(fp);

			// replicate data in buffer num_bytes times
			while (num_bytes-- > 0) {
				image->buffer[count++] = data;

			}
		} else {
			// actual data, just copy it into buffer at next location
			image->buffer[count++] = data;
		}
	}

	// load color palette
	// move to end of file then back up 768 bytes i.e. to begining of palette
	fseek(fp, -768L, SEEK_END);

	// load the PCX pallete into the VGA color registers
	for (index = 0; index < 256; index++) {
		// get the red component
		image->palette[index].red = (unsigned char)(getc(fp) >> 2);

		// get the green component
		image->palette[index].green = (unsigned char)(getc(fp) >> 2);

		// get the blue component
		image->palette[index].blue = (unsigned char)(getc(fp) >> 2);
	}

	// time to close the file
	fclose(fp);

	// change the palette to newly loaded palette if commanded to do so
	if (load_palette) {
		// for each palette register set to the new color values
		for (index = 0; index < 256; index++) {
			Write_Color_Reg(index, (RGB_color_ptr)&image->palette[index]);
		}
	}

	// success
	return(1);
}

/////////////////////////////////////////////////////////////////////////////

void PCX_Delete(pcx_picture_ptr image)
{
	// this function de-allocates the buffer region used for the pcx file load

	free(image->buffer);
}

//////////////////////////////////////////////////////////////////////////////

void PCX_Show_Buffer(pcx_picture_ptr image)
{
	// just copy he pcx buffer into the video buffer

	unsigned int index;               // loop variable

	for (index = 0; index < SCREEN_WIDTH * SCREEN_HEIGHT; index++) {
		RETRO.framebuffer[index] = image->buffer[index];
	}

	RETRO_Flip();
}

///////////////////////////////////////////////////////////////////////////////

void PCX_Copy_To_Buffer(pcx_picture_ptr image, unsigned char *buffer)
{
	// this function is used to copy the data in the PCX buffer to another buffer
	// usually the double buffer

	unsigned int index;               // loop variable

	for (index = 0; index < SCREEN_WIDTH * SCREEN_HEIGHT; index++) {
		buffer[index] = image->buffer[index];
	}
}

//////////////////////////////////////////////////////////////////////////////

void PCX_Get_Sprite(pcx_picture_ptr image, sprite_ptr sprite, int sprite_frame, int cell_x, int cell_y)
{
	// this function is used to load the images for a sprite into the sprite
	// frames array. It functions by using the size of the sprite and the
	// position of the requested cell to compute the proper location in the
	// pcx image buffer to extract the data from.

	int x_off,  // position of sprite cell in PCX image buffer
		y_off,
		y,      // looping variable
		width,  // size of sprite
		height;

	unsigned char *sprite_data;

	// extract width and height of sprite
	width = sprite->width;
	height = sprite->height;

	// first allocate the memory for the sprite in the sprite structure
	sprite->frames[sprite_frame] = (unsigned char *)malloc(width * height + 1);

	// create an alias to the sprite frame for ease of access
	sprite_data = sprite->frames[sprite_frame];

	// now load the sprite data into the sprite frame array from the pcx picture
	x_off = (width + 1) * cell_x + 1;
	y_off = (height + 1) * cell_y + 1;

	// compute starting y address
	y_off = y_off * 320; // 320 bytes per line

	// scan the data row by row
	for (y = 0; y < height; y++, y_off += 320) {
		// copy the row of pixels
		memcpy((void *)&sprite_data[y * width], (void *)&(image->buffer[y_off + x_off]), width);
	}

	// increment number of frames
	sprite->num_frames++;
}

//////////////////////////////////////////////////////////////////////////////

void Sprite_Init(sprite_ptr sprite, int x, int y, int width, int height, int c1, int c2, int c3, int t1, int t2, int t3)
{
	// this function initializes a sprite

	int index;

	sprite->x = x;
	sprite->y = y;
	sprite->width = width;
	sprite->height = height;
	sprite->visible = 1;
	sprite->counter_1 = c1;
	sprite->counter_2 = c2;
	sprite->counter_3 = c3;
	sprite->threshold_1 = t1;
	sprite->threshold_2 = t2;
	sprite->threshold_3 = t3;
	sprite->curr_frame = 0;
	sprite->state = SPRITE_DEAD;
	sprite->num_frames = 0;
	sprite->background = (unsigned char *)malloc(width * height + 1);

	// set all bitmap pointers to null
	for (index = 0; index < MAX_SPRITE_FRAMES; index++) {
		sprite->frames[index] = NULL;
	}
}

//////////////////////////////////////////////////////////////////////////////

void Sprite_Delete(sprite_ptr sprite)
{
	// this function deletes all the memory associated with a sprite

	int index;

	free(sprite->background);

	// now de-allocate all the animation frames
	for (index = 0; index < MAX_SPRITE_FRAMES; index++) {
		free(sprite->frames[index]);
	}
}

////////////////////////////////////////////////////////////////////////////////

void Sprite_Under(sprite_ptr sprite, unsigned char *buffer)
{
	// this function scans the background under a sprite so that when the sprite
	// is drawn the background isn't obliterated

	unsigned char *back_buffer; // background buffer for sprite

	int y,                          // current line being scanned
		width,                      // size of sprite
		height;

	// alias a pointer to sprite background for ease of access
	back_buffer = sprite->background;

	// alias width and height
	width = sprite->width;
	height = sprite->height;

	// compute offset of background in source buffer
	buffer = buffer + (sprite->y << 8) + (sprite->y << 6) + sprite->x;

	for (y = 0; y < height; y++) {
		// copy the next row out off image buffer into sprite background buffer
		memcpy((void *)back_buffer, (void *)buffer, width);

		// move to next line in source buffer and in sprite background buffer
		buffer += SCREEN_WIDTH;
		back_buffer += width;
	}
}

//////////////////////////////////////////////////////////////////////////////

void Sprite_Erase(sprite_ptr sprite, unsigned char *buffer)
{
	// replace the background that was behind the sprite
	// this function replaces the background that was saved from where a sprite
	// was going to be placed

	unsigned char *back_buffer; // background buffer for sprite

	int y,                          // current line being scanned
		width,                      // size of sprite
		height;

	// alias a pointer to sprite background for ease of access
	back_buffer = sprite->background;

	// alias width and height
	width = sprite->width;
	height = sprite->height;

	// compute offset in destination buffer
	buffer = buffer + (sprite->y << 8) + (sprite->y << 6) + sprite->x;

	for (y = 0; y < height; y++) {
		// copy the next from sprite background buffer to destination buffer
		memcpy((void *)buffer, (void *)back_buffer, width);

		// move to next line in destination buffer and in sprite background buffer
		buffer += SCREEN_WIDTH;
		back_buffer += width;
	}
}

//////////////////////////////////////////////////////////////////////////////

void Sprite_Draw(sprite_ptr sprite, unsigned char *buffer, int transparent)
{
	// this function draws a sprite on the screen row by row very quickly
	// note the use of shifting to implement multplication
	// if the transparent flag is true then pixels wil be draw one by one
	// else a memcpy will be used to draw each line

	unsigned char *sprite_data; // pointer to sprite data
	unsigned char *dest_buffer; // pointer to destination buffer

	int x, y,                        // looping variables
		width,                      // width of sprite
		height;                     // height of sprite

	unsigned char pixel;            // the current pixel being processed

	// alias a pointer to sprite for ease of access
	sprite_data = sprite->frames[sprite->curr_frame];

	// alias a variable to sprite size
	width = sprite->width;
	height = sprite->height;

	// compute number of bytes between adjacent video lines after a row of pixels
	// has been drawn

	// compute offset of sprite in destination buffer
	dest_buffer = buffer + (sprite->y << 8) + (sprite->y << 6) + sprite->x;

	// copy each line of the sprite data into destination buffer
	if (transparent) {
		for (y = 0; y < height; y++) {
			// copy the next row into the destination buffer
			for (x = 0; x < width; x++) {
				// test for transparent pixel i.e. 0, if not transparent then draw
				if ((pixel = sprite_data[x])) {
					dest_buffer[x] = pixel;
				}
			}

			// move to next line in desintation buffer and sprite image buffer
			dest_buffer += SCREEN_WIDTH;
			sprite_data += width;
		}
	} else {
		// draw sprite with transparency off
		for (y = 0; y < height; y++) {
			// copy the next row into the destination buffer
			memcpy((void *)dest_buffer, (void *)sprite_data, width);

			// move to next line in desintation buffer and sprite image buffer
			dest_buffer += SCREEN_WIDTH;
			sprite_data += width;
		}
	}
}

////////////////////////////////////////////////////////////////////////////////

void Sprite_Under_Clip(sprite_ptr sprite, unsigned char *buffer)
{
	// this function scans the background under a sprite, but only those
	// portions that are visible

	unsigned char *back_buffer;   // pointer to sprite background buffer
	unsigned char *source_buffer; // pointer to source buffer

	int y,                        // looping variable
		sx, sy,                      // position of sprite
		width,                      // width of sprite
		bitmap_width = 0,           // width and height of sub-bitmap
		bitmap_height = 0;

	// alias a variable to sprite size
	width = sprite->width;
	bitmap_width = width;
	bitmap_height = sprite->height;
	sx = sprite->x;
	sy = sprite->y;

	// perform trivial rejection tests
	if (sx >= (int)SCREEN_WIDTH || sy >= (int)SCREEN_HEIGHT ||
		(sx + width) <= 0 || (sy + bitmap_height) <= 0) {
		// sprite is totally invisible therefore don't scan

		// set invisible flag in strucuture so that the draw sub-function
		// doesn't do anything
		sprite->visible = 0;
		return;
	}

	// the sprite background region must be clipped before scanning
	// therefore compute visible portion

	// first compute upper left hand corner of clipped sprite background
	if (sx < 0) {
		bitmap_width += sx;
		sx = 0;
	} else if (sx + width >= (int)SCREEN_WIDTH) {
		bitmap_width = (int)SCREEN_WIDTH - sx;
	}

	// now process y
	if (sy < 0) {
		bitmap_height += sy;
		sy = 0;
	} else if (sy + bitmap_height >= (int)SCREEN_HEIGHT) {
		bitmap_height = (int)SCREEN_HEIGHT - sy;
	}

	// this point we know were to start scanning the bitmap i.e.
	// sx,sy
	// and we know the size of the bitmap to be scanned i.e.
	// width,height, so plug it all into the rest of function

	// compute number of bytes between adjacent video lines after a row of pixels
	// has been drawn

	// compute offset of sprite background in source buffer
	source_buffer = buffer + (sy << 8) + (sy << 6) + sx;

	// alias a pointer to sprite background
	back_buffer = sprite->background;

	for (y = 0; y < bitmap_height; y++) {
		// copy the next row into the destination buffer
		memcpy((void *)back_buffer, (void *)source_buffer, bitmap_width);

		// move to next line in desintation buffer and sprite image buffer
		source_buffer += SCREEN_WIDTH;
		back_buffer += width;  // note this width is the actual width of the entire bitmap NOT the visible portion
	}

	// set variables in strucuture so that the erase sub-function can operate
	// faster
	sprite->x_clip = sx;
	sprite->y_clip = sy;
	sprite->width_clip = bitmap_width;
	sprite->height_clip = bitmap_height;
	sprite->visible = 1;
}

//////////////////////////////////////////////////////////////////////////////

void Sprite_Erase_Clip(sprite_ptr sprite, unsigned char *buffer)
{
	// replace the background that was behind the sprite
	// this function replaces the background that was saved from where a sprite
	// was going to be placed

	unsigned char *back_buffer; // background buffer for sprite

	int y,                          // current line being scanned
		width,                      // size of sprite background buffer
		bitmap_height,              // size of clipped bitmap
		bitmap_width;

	// make sure sprite was visible
	if (!sprite->visible) {
		return;
	}

	// alias a pointer to sprite background for ease of access
	back_buffer = sprite->background;

	// alias width and height
	bitmap_width = sprite->width_clip;
	bitmap_height = sprite->height_clip;
	width = sprite->width;

	// compute offset in destination buffer
	buffer = buffer + (sprite->y_clip << 8) + (sprite->y_clip << 6) + sprite->x_clip;

	for (y = 0; y < bitmap_height; y++) {
		// copy the next row from sprite background buffer to destination buffer
		memcpy((void *)buffer, (void *)back_buffer, bitmap_width);

		// move to next line in destination buffer and in sprite background buffer
		buffer += SCREEN_WIDTH;
		back_buffer += width;
	}
}

//////////////////////////////////////////////////////////////////////////////

void Sprite_Draw_Clip(sprite_ptr sprite, unsigned char *buffer, int transparent)
{
	// this function draws a sprite on the screen row by row very quickly
	// note the use of shifting to implement multplication
	// if the transparent flag is true then pixels wil be draw one by one
	// else a memcpy will be used to draw each line
	// this function also performs clipping. It will test if the sprite
	// is totally visible/invisible and will only draw the portions that are visible

	unsigned char *sprite_data; // pointer to sprite data
	unsigned char *dest_buffer; // pointer to destination buffer

	int x, y,                        // looping variables
		sx, sy,                      // position of sprite
		width,                      // width of sprite
		bitmap_x = 0,           // starting upper left corner of sub-bitmap
		bitmap_y = 0,           // to be drawn after clipping
		bitmap_width = 0,           // width and height of sub-bitmap
		bitmap_height = 0;

	unsigned char pixel;            // the current pixel being processed

	// alias a variable to sprite size
	width = sprite->width;
	bitmap_width = width;
	bitmap_height = sprite->height;
	sx = sprite->x;
	sy = sprite->y;

	// perform trivial rejection tests
	if (sx >= (int)SCREEN_WIDTH || sy >= (int)SCREEN_HEIGHT ||
		(sx + width) <= 0 || (sy + bitmap_height) <= 0 || !sprite->visible) {
		// sprite is totally invisible therefore don't draw

		// set invisible flag in strucuture so that the erase sub-function
		// doesn't do anything
		sprite->visible = 0;

		return;
	}

	// the sprite needs some clipping or no clipping at all, so compute
	// visible portion of sprite rectangle

	// first compute upper left hand corner of clipped sprite
	if (sx < 0) {
		bitmap_x = -sx;
		sx = 0;
		bitmap_width -= bitmap_x;
	} else if (sx + width >= (int)SCREEN_WIDTH) {
		bitmap_x = 0;
		bitmap_width = (int)SCREEN_WIDTH - sx;
	}

	// now process y
	if (sy < 0) {
		bitmap_y = -sy;
		sy = 0;
		bitmap_height -= bitmap_y;
	} else if (sy + bitmap_height >= (int)SCREEN_HEIGHT) {
		bitmap_y = 0;
		bitmap_height = (int)SCREEN_HEIGHT - sy;
	}

	// this point we know were to start drawing the bitmap i.e.
	// sx,sy
	// and we know were in the data to extract the bitmap i.e.
	// bitmap_x, bitmap_y,
	// and finaly we know the size of the bitmap to be drawn i.e.
	// width,height, so plug it all into the rest of function

	// compute number of bytes between adjacent video lines after a row of pixels
	// has been drawn

	// compute offset of sprite in destination buffer
	dest_buffer = buffer + (sy << 8) + (sy << 6) + sx;

	// alias a pointer to sprite for ease of access and locate starting sub
	// bitmap that will be drawn
	sprite_data = sprite->frames[sprite->curr_frame] + (bitmap_y * width) + bitmap_x;

	// copy each line of the sprite data into destination buffer
	if (transparent) {
		for (y = 0; y < bitmap_height; y++) {
			// copy the next row into the destination buffer
			for (x = 0; x < bitmap_width; x++) {
				// test for transparent pixel i.e. 0, if not transparent then draw
				if ((pixel = sprite_data[x])) {
					dest_buffer[x] = pixel;
				}
			}

			// move to next line in desintation buffer and sprite image buffer
			dest_buffer += SCREEN_WIDTH;
			sprite_data += width;   // note this width is the actual width of the entire bitmap NOT the visible portion
		}
	} else {
		// draw sprite with transparency off
		for (y = 0; y < bitmap_height; y++) {
			// copy the next row into the destination buffer
			memcpy((void *)dest_buffer, (void *)sprite_data, bitmap_width);

			// move to next line in desintation buffer and sprite image buffer
			dest_buffer += SCREEN_WIDTH;
			sprite_data += width;  // note this width is the actual width of the entire bitmap NOT the visible portion
		}
	}

	// set variables in strucuture so that the erase sub-function can operate faster
	sprite->x_clip = sx;
	sprite->y_clip = sy;
	sprite->width_clip = bitmap_width;
	sprite->height_clip = bitmap_height;
	sprite->visible = 1;
}

//////////////////////////////////////////////////////////////////////////////

int Sprites_Collide(sprite_ptr sprite_1, sprite_ptr sprite_2)
{
	// this function tests if two sprites have intersected by testing their
	// bounding boxes for collision

	return(0);
}

///////////////////////////////////////////////////////////////////////////////

int Create_Double_Buffer(int num_lines)
{
	// set the height of the buffer and compute it's size
	double_buffer_height = num_lines;
	double_buffer_size = SCREEN_WIDTH * num_lines;

	// everything was ok
	return(1);
}

///////////////////////////////////////////////////////////////////////////////
void Screen_Transition(int effect)
{
	// this function can be called to perform a myraid of screen transitions
	// to the video buffer, note I have left one for you to create!

	int pal_reg;       //  used as loop counter
	long index;        // used as loop counter
	RGB_color color;   // temporary color

	// test which screen effect is being selected
	switch (effect) {
	case SCREEN_DARKNESS:
	{
		// fade to black
		for (index = 0; index < 20; index++) {
			// loop thru all palette registers
			for (pal_reg = 1; pal_reg < 255; pal_reg++) {
				// get the next color to fade
				Read_Color_Reg(pal_reg, (RGB_color_ptr)&color);

				// test if this color regisyer is already black
				if (color.red > 4) {
					color.red -= 3;
				} else {
					color.red = 0;
				}

				if (color.green > 4) {
					color.green -= 3;
				} else {
					color.green = 0;
				}

				if (color.blue > 4) {
					color.blue -= 3;
				} else {
					color.blue = 0;
				}

				// set the color to a diminished intensity
				Write_Color_Reg(pal_reg, (RGB_color_ptr)&color);
			}

			// wait a bit
			Time_Delay(1);
			RETRO_Flip();
		}
	} break;

	case SCREEN_WHITENESS:
	{
		// fade to white
		for (index = 0; index < 20; index++) {

			// loop thru all palette registers
			for (pal_reg = 0; pal_reg < 255; pal_reg++) {
				// get the color to fade
				Read_Color_Reg(pal_reg, (RGB_color_ptr)&color);

				color.red += 4;
				if (color.red > 63) {
					color.red = 63;
				}

				color.green += 4;
				if (color.green > 63) {
					color.green = 63;
				}

				color.blue += 4;
				if (color.blue > 63) {
					color.blue = 63;
				}

				// set the color to a brightend intensity
				Write_Color_Reg(pal_reg, (RGB_color_ptr)&color);
			}

			// wait a bit
			Time_Delay(1);
			RETRO_Flip();
		}
	} break;

	case SCREEN_WARP:
	{
		// this one you do!!!!
	} break;

	case SCREEN_SWIPE_X:
	{
		// do a screen wipe from right to left, left to right
		for (index = 0; index < 160; index += 2) {
			// draw two vertical lines at opposite ends of the screen
			Line_V(0, 199, 319 - index, 0);
			Line_V(0, 199, index, 0);
			Line_V(0, 199, 319 - (index + 1), 0);
			Line_V(0, 199, index + 1, 0);

			// wait a bit
			Time_Delay(1);
			RETRO_Flip();
		}
	} break;

	case SCREEN_SWIPE_Y:
	{
		// do a screen wipe from top to bottom, bottom to top
		for (index = 0; index < 100; index += 2) {
			// draw two horizontal lines at opposite ends of the screen
			Line_H(0, 319, 199 - index, 0);
			Line_H(0, 319, index, 0);
			Line_H(0, 319, 199 - (index + 1), 0);
			Line_H(0, 319, index + 1, 0);

			// wait a bit
			Time_Delay(1);
			RETRO_Flip();
		}
	} break;

	case SCREEN_DISOLVE:
	{
		// disolve the screen by plotting zillions of little black dots
		for (index = 0; index <= 300000; index++) {
			RETRO_PutPixel(rand() % 320, rand() % 200, 0);
		}
	} break;

	default:break;
	}
}

//////////////////////////////////////////////////////////////////////////////

void Bitmap_Put(bitmap_ptr image, unsigned char *destination, int transparent)
{
	// this fucntion will draw a bitmap on the destination buffer which could
	// be a double buffer or the video buffer

	int x, y,                        // looping variables
		width, height;               // size of bitmap

	unsigned char *bitmap_data; // pointer to bitmap buffer
	unsigned char *dest_buffer; // pointer to destination buffer
	unsigned char pixel;            // current pixel value being processed

	// compute offset of bitmap in destination buffer. note: all video or double
	// buffers must be 320 bytes wide!
	dest_buffer = destination + (image->y << 8) + (image->y << 6) + image->x;

	// create aliases to variables so the strucuture doesn't need to be
	// dereferenced continually
	height = image->height;
	width = image->width;
	bitmap_data = image->buffer;

	// test if transparency is on or off
	if (transparent) {
		// use version that will draw a transparent bitmap(slighlty slower)

		// draw each line of the bitmap
		for (y = 0; y < height; y++) {
			// copy the next row into the destination buffer
			for (x = 0; x < width; x++) {
				// test for transparent pixel i.e. 0, if not transparent then draw
				if ((pixel = bitmap_data[x])) {
					dest_buffer[x] = pixel;
				}
			}

			// move to next line in double buffer and in bitmap buffer
			dest_buffer += SCREEN_WIDTH;
			bitmap_data += width;
		}
	} else {
		// draw each line of the bitmap, note how each pixel doesn't need to be
		// tested for transparency hence a memcpy can be used (very fast!)
		for (y = 0; y < height; y++) {
			// copy the next row into the destination buffer using memcpy for speed
			memcpy((void *)dest_buffer, (void *)bitmap_data, width);

			// move to next line in destination buffer and in bitmap buffer
			dest_buffer += SCREEN_WIDTH;
			bitmap_data += width;
		}
	}
}

//////////////////////////////////////////////////////////////////////////////

void Bitmap_Get(bitmap_ptr image, unsigned char *source)
{
	// this function will scan a bitmap fomr the source buffer
	// could be a double buffer, video buffer or any other buffer with a
	// logical row width of 320 bytes

	unsigned int source_off,       // offsets into destination and source buffers
		bitmap_off;

	int y,                         // looping variable
		width, height;              // size of bitmap

	unsigned char *bitmap_data; // pointer to bitmap buffer

	// compute offset of bitmap in source buffer. note: all video or double
	// buffers must be 320 bytes wide!
	source_off = (image->y << 8) + (image->y << 6) + image->x;
	bitmap_off = 0;

	// create aliases to variables so the strucuture doesn't need to be
	// dereferenced continually
	height = image->height;
	width = image->width;
	bitmap_data = image->buffer;

	// draw each line of the bitmap, note how each pixel doesn't need to be
	// tested for transparency hence a memcpy can be used (very fast!)
	for (y = 0; y < height; y++) {
		// copy the next row into the bitmap buffer using memcpy for speed
		memcpy((void *)&bitmap_data[bitmap_off], (void *)&source[source_off], width);

		// move to next line in source buffer and in bitmap buffer
		source_off += SCREEN_WIDTH;
		bitmap_off += width;
	}
}

//////////////////////////////////////////////////////////////////////////////

int Bitmap_Allocate(bitmap_ptr image, int width, int height)
{
	// this function can be used to allocate the memory needed for a bitmap

	if ((image->buffer = (unsigned char *)malloc(width * height + 1)) == NULL) {
		return(0);
	} else {
		return(1);
	}
}

//////////////////////////////////////////////////////////////////////////////

void Bitmap_Delete(bitmap_ptr the_bitmap)
{
	// this function deletes the memory used by a bitmap

	if (the_bitmap->buffer) {
		free(the_bitmap->buffer);
	}
}

//////////////////////////////////////////////////////////////////////////////

void Layer_Draw(layer_ptr source_layer, int source_x, int source_y, unsigned char *dest_buffer, int dest_y, int dest_height, int transparent)
{
	// this function will map down a section of the layer onto the destination
	// buffer at the desired location, note the width of the destination buffer
	// is always assumed to be 320 bytes width. Also, the function will always
	// wrap around the layer

	int x, y,                        // looping variables
		layer_width,                // the width of the layer
		right_width,                // the width of the right and left half of
		left_width;                 // the layer to be drawn

	unsigned char *layer_buffer_l; // pointers to the left and right halves
	unsigned char *dest_buffer_l;  // of the layer buffer and destination
	unsigned char *layer_buffer_r; // buffer
	unsigned char *dest_buffer_r;
	unsigned char pixel;            // current pixel value being processed

	layer_width = source_layer->width;
	dest_buffer_l = dest_buffer + (dest_y << 8) + (dest_y << 6);
	layer_buffer_l = source_layer->buffer + layer_width * source_y + source_x;

	// test if wraping is needed
	if (((layer_width - source_x) - (int)SCREEN_WIDTH) >= 0) {
		// there's enough data in layer to draw a complete line, no wraping needed
		left_width = SCREEN_WIDTH;
		right_width = 0; // no wraping flag
	} else {
		// wrapping needed
		left_width = layer_width - source_x;
		right_width = SCREEN_WIDTH - left_width;
		dest_buffer_r = dest_buffer_l + left_width;
		layer_buffer_r = layer_buffer_l - source_x; // move to far left end of layer
	}

	// test if transparency is on or off

	if (transparent) {
		// use version that will draw a transparent bitmap(slighlty slower)
		// first draw left half then right half

		// draw each line of the bitmap
		for (y = 0; y < dest_height; y++) {
			// copy the next row into the destination buffer
			for (x = 0; x < left_width; x++) {
				// test for transparent pixel i.e. 0, if not transparent then draw
				if ((pixel = layer_buffer_l[x])) {
					dest_buffer_l[x] = pixel;
				}
			}

			// move to next line in destination buffer and in layer buffer
			dest_buffer_l += SCREEN_WIDTH;
			layer_buffer_l += layer_width;
		}

		// now right half

		// draw each line of the bitmap
		if (right_width) {
			for (y = 0; y < dest_height; y++) {
				// copy the next row into the destination buffer
				for (x = 0; x < right_width; x++) {
					// test for transparent pixel i.e. 0, if not transparent then draw
					if ((pixel = layer_buffer_r[x])) {
						dest_buffer_r[x] = pixel;
					}
				}

				// move to next line in destination buffer and in layer buffer
				dest_buffer_r += SCREEN_WIDTH;
				layer_buffer_r += layer_width;
			}
		}
	} else {
		// draw each line of the bitmap, note how each pixel doesn't need to be
		// tested for transparency hence a memcpy can be used (very fast!)
		for (y = 0; y < dest_height; y++) {
			// copy the next row into the destination buffer using memcpy for speed
			memcpy((void *)dest_buffer_l, (void *)layer_buffer_l, left_width);

			// move to next line in double buffer and in bitmap buffer
			dest_buffer_l += SCREEN_WIDTH;
			layer_buffer_l += layer_width;
		}

		// now right half if needed
		if (right_width) {
			for (y = 0; y < dest_height; y++) {
				// copy the next row into the destination buffer using memcpy for speed
				memcpy((void *)dest_buffer_r, (void *)layer_buffer_r, right_width);

				// move to next line in double buffer and in bitmap buffer
				dest_buffer_r += SCREEN_WIDTH;
				layer_buffer_r += layer_width;
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////////

void Layer_Build(layer_ptr dest_layer, int dest_x, int dest_y, unsigned char *source_buffer, int source_x, int source_y, int width, int height)
{
	// this function is used to build up the layer out of smaller pieces
	// this allows a layer to be very long, tall etc. also the source data buffer
	// must be constructed such that there are 320 bytes per row

	int y,                       // looping variable
		layer_width;             // the width of the layer

	unsigned char *source_data;   // pointer to start of source bitmap image
	unsigned char *layer_buffer;  // pointer to layer buffer

	// extract width of layer
	layer_width = dest_layer->width;

	// compute starting location in layer buffer
	layer_buffer = dest_layer->buffer + layer_width * dest_y + dest_x; ;

	// compute starting location in source image buffer
	source_data = source_buffer + (source_y << 8) + (source_y << 6) + source_x;

	// scan each line of source image into layer buffer
	for (y = 0; y < height; y++) {
		// copy the next row into the layer buffer using memcpy for speed
		memcpy((void *)layer_buffer, (void *)source_data, width);

		// move to next line in source buffer and in layer buffer
		source_data += SCREEN_WIDTH;
		layer_buffer += layer_width;
	}
}

//////////////////////////////////////////////////////////////////////////////

int Layer_Create(layer_ptr dest_layer, int width, int height)
{
	// this function can be used to allocate the memory needed for a layer
	// the width must be divisible by two.

	if ((dest_layer->buffer = (unsigned char *)malloc(width * height + 2)) == NULL) {
		return(0);
	} else {
		// save the dimensions of layer
		dest_layer->width = width;
		dest_layer->height = height;

		return(1);
	}
}

//////////////////////////////////////////////////////////////////////////////

void Layer_Delete(layer_ptr the_layer)
{
	// this function deletes the memory used by a layer

	if (the_layer->buffer) {
		free(the_layer->buffer);
	}
}

//////////////////////////////////////////////////////////////////////////////

void Print_Char_DB(int xc, int yc, char c, int color, int transparent)
{
	// this function is used to print a character on the double buffer. It uses the
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

	// compute offset of character in double buffer, use shifting to multiply
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

		// move to next line in double buffer and in rom character data area
		offset += SCREEN_WIDTH;
		work_char++;
	}
}

//////////////////////////////////////////////////////////////////////////////

void Print_String_DB(int x, int y, int color, char *string, int transparent)
{
	// this function prints an entire string into the double buffer with fixed
	// spacing between each character by calling the Print_Char() function

	int index,   // loop index
		length;  // length of string

	// compute length of string
	length = strlen(string);

	// print the string a character at a time
	for (index = 0; index < length; index++) {
		Print_Char_DB(x + (index << 3), y, string[index], color, transparent);
	}
}

///////////////////////////////////////////////////////////////////////////////

void Write_Pixel_DB(int x, int y, int color)
{
	// plots the pixel in the desired color to the double buffer
	// to accomplish the multiplications

	// use the fact that 320*y = 256*y + 64*y = y<<8 + y<<6
	RETRO.framebuffer[((y << 8) + (y << 6)) + x] = (unsigned char)color;
}

///////////////////////////////////////////////////////////////////////////////

int Read_Pixel_DB(int x, int y)
{
	// this function read a pixel from the double buffer

	// use the fact that 320*y = 256*y + 64*y = y<<8 + y<<6
	return((int)(RETRO.framebuffer[((y << 8) + (y << 6)) + x]));
}

//////////////////////////////////////////////////////////////////////////////
