//
// voxel.c
//
#include "lib/retro.h"
#include "lib/retromain.h"
#include "lib/retrofont.h"
#include "black3.h"
#include "black4.h"
#include "black8.h"

#define WORLD_X_SIZE   320  // width of universe
#define WORLD_Y_SIZE   200  // height of universe

// constants used to represent angles for the ray casting a 60 degree field of view
#define ANGLE_0        0
#define ANGLE_1        5
#define ANGLE_2        10
#define ANGLE_4        20
#define ANGLE_5        25
#define ANGLE_6        30
#define ANGLE_15       80
#define ANGLE_30       160
#define ANGLE_45       240
#define ANGLE_60       320
#define ANGLE_90       480
#define ANGLE_135      720
#define ANGLE_180      960
#define ANGLE_225      1200
#define ANGLE_270      1440
#define ANGLE_315      1680
#define ANGLE_360      1920

// there are 1920 degrees in our circle or 360/1920 is the conversion factor
// from real degrees to our degrees
#define ANGULAR_INCREMENT  ((float)0.1875)

// conversion constants from radians to degrees and vicversa
#define DEG_TO_RAD         ((float)3.1415926/(float)180)
#define RAD_TO_DEG         ((float)180/(float)3.1415926)

// G L O B A L S  ////////////////////////////////////////////////////////////

pcx_picture image_pcx;        // general pcx image

int play_x = 1000,              // the current world x of player
play_y = 1000,              // the current world y of player
play_z = 150,
play_ang = ANGLE_90,        // the current viewing angle of player
play_dist = 70,
mountain_scale = 10;        // scaling factor for mountains

float play_dir_x,              // the direction the player is pointing in
play_dir_y,
play_dir_z,
cos_look[ANGLE_360],     // cosine look up table
sin_look[ANGLE_360],     // sin look up table
sphere_cancel[ANGLE_60]; // cancels fish eye distortion

char buffer[80];

float speed = 0;  // speed of player

// F U N C T I O N S ////////////////////////////////////////////////////////

void Line_VDB(int y1, int y2, int x, int color)
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

//////////////////////////////////////////////////////////////////////////////

int Initialize(const char *filename)
{
	// this function builds all the look up tables for the terrain generator and
	// loads in the terrain texture map

	int ang;           // looping variable

	float rad_angle;   // current angle in radians

	// create sin and cos look up first
	for (ang = 0; ang < ANGLE_360; ang++) {
		// compute current angle in radians
		rad_angle = (float)ang * ANGULAR_INCREMENT * DEG_TO_RAD;

		// now compute the sin and cos
		sin_look[ang] = sin(rad_angle);
		cos_look[ang] = cos(rad_angle);
	}

	// create inverse cosine viewing distortion filter
	for (ang = 0; ang < ANGLE_30; ang++) {
		// compute current angle in radians
		rad_angle = (float)ang * ANGULAR_INCREMENT * DEG_TO_RAD;

		// now compute the sin and cos
		sphere_cancel[ang + ANGLE_30] = 1 / cos(rad_angle);
		sphere_cancel[ANGLE_30 - ang] = 1 / cos(rad_angle);
	}

	// intialize the pcx structure
	PCX_Init((pcx_picture_ptr)&image_pcx);

	// load in the textures
	return(PCX_Load(filename, (pcx_picture_ptr)&image_pcx, 1));
}

/////////////////////////////////////////////////////////////////////////////

void Draw_Terrain(int play_x, int play_y, int play_z, int play_ang, int play_dist)
{
	// this function draws the entire terrain based on the location and orientation
	// of the player's viewpoint

	int curr_ang,       // current angle being processed
		xr, yr,          // location of ray in world coords
		x_fine, y_fine,  // the texture coordinates the ray hit
		pixel_color,    // the color of textel
		ray,            // looping variable
		row,            // the current video row begin processed
		row_inv,        // the inverted row to make upward positive
		scale,          // the scale of the current strip

		top,            // top of strip
		bottom;         // bottom of strip

	float ray_length;   // the length of the ray after distortion compensation

	// start the current angle off -30 degrees to the left of the player's
	// current viewing direction
	curr_ang = play_ang - ANGLE_30;

	// test for underflow
	if (curr_ang < 0) {
		curr_ang += ANGLE_360;
	}

	// cast a series of rays for every column of the screen
	for (ray = 1; ray < 320; ray++) {
		// for each column compute the pixels that should be displayed
		// for each screen pixel, process from top to bottom
		for (row = 100; row < 150; row++) {
			// compute length of ray
			row_inv = 200 - row;

			// use the current height and distance to compute length of ray.
			ray_length = sphere_cancel[ray] * ((float)(play_dist * play_z) / (float)(play_z - row_inv));

			// rotate ray into position of sample
			xr = (int)((float)play_x + ray_length * cos_look[curr_ang]);
			yr = (int)((float)play_y - ray_length * sin_look[curr_ang]);

			// compute texture coords
			x_fine = xr % WORLD_X_SIZE;
			y_fine = yr % WORLD_Y_SIZE;

			// using texture index locate texture pixel in textures
			pixel_color = image_pcx.buffer[x_fine + (y_fine * 320)];

			// draw the strip
			scale = (int)mountain_scale * pixel_color / (int)(ray_length + 1);
			top = 50 + row - scale;
			bottom = top + scale;
			Line_VDB(top, bottom, ray, pixel_color);
		}

		// move to next angle
		if (++curr_ang >= ANGLE_360)
			curr_ang = ANGLE_0;
	}
}

// M A I N //////////////////////////////////////////////////////////////////

void DEMO_Render(double deltatime)
{
	// reset velocity
	speed = 0;

	// compute starting time of this frame
	starting_time = Timer_Query();

	// change viewing distance
	if (RETRO_KeyState(SDL_SCANCODE_F)) {
		play_dist += 10;
	}
	if (RETRO_KeyState(SDL_SCANCODE_C)) {
		play_dist -= 10;
	}
	// change viewing height
	if (RETRO_KeyState(SDL_SCANCODE_U)) {
		play_z += 10;
	}
	if (RETRO_KeyState(SDL_SCANCODE_D)) {
		play_z -= 10;
	}
	// change viewing position
	if (RETRO_KeyState(SDL_SCANCODE_RIGHT)) {
		if ((play_ang += ANGLE_5) >= ANGLE_360) {
			play_ang -= ANGLE_360;
		}
	}
	if (RETRO_KeyState(SDL_SCANCODE_LEFT)) {
		if ((play_ang -= ANGLE_5) < 0) {
			play_ang += ANGLE_360;
		}
	}
	// move foward
	if (RETRO_KeyState(SDL_SCANCODE_UP)) {
		speed = 20;
	}
	// move backward
	if (RETRO_KeyState(SDL_SCANCODE_DOWN)) {
		speed = -20;
	}

	// compute trajectory vector for this view angle
	play_dir_x = cos_look[play_ang];
	play_dir_y = -sin_look[play_ang];
	play_dir_z = 0;

	// translate viewpoint
	play_x += speed * play_dir_x;
	play_y += speed * play_dir_y;
	play_z += speed * play_dir_z;

	// draw the terrain
	Draw_Terrain(play_x, play_y, play_z, play_ang, play_dist);

	// draw tactical
	sprintf(buffer, "Height = %d Distance = %d     ", play_z, play_dist);
	Print_String_DB(0, 0, 10, buffer, 0);
	sprintf(buffer, "Pos: X=%d, Y=%d, Z=%d    ", play_x, play_y, play_z);
	Print_String_DB(0, 10, 10, buffer, 0);

	// lock onto 18 frames per second max
	Time_Delay_ms(50 - (Timer_Query() - starting_time));
}

void DEMO_Initialize(void)
{
	Initialize("assets/voxterr5.pcx");

	// set scale of mountains
	mountain_scale = 15;
}
