//
// gourdemo.c
//
#include "lib/retro.h"
#include "lib/retromain.h"
#include "lib/retrofont.h"
#include "black3.h"
#include "black4.h"
#include "black11.h"

void DEMO_Render(double deltatime)
{
	static int x = 140, y = 60;     // position of triangle
	static int intensity_1 = 15;
	static int intensity_2 = 50;
	static int intensity_3 = 5;

	char buffer[80]; // output string buffer

	if (RETRO_KeyState(SDL_SCANCODE_7)) { // increase vertex 1 intensity
		if (++intensity_1 > 63)
			intensity_1 = 63;
	}
	if (RETRO_KeyState(SDL_SCANCODE_4)) { // decrease vertex 1 intensity
		if (--intensity_1 < 0) {
			intensity_1 = 0;
		}
	}
	if (RETRO_KeyState(SDL_SCANCODE_8)) { // increase vertex 2 intensity
		if (++intensity_2 > 63) {
			intensity_2 = 63;
		}
	}
	if (RETRO_KeyState(SDL_SCANCODE_5)) { // decrease vertex 2 intensity
		if (--intensity_2 < 0) {
			intensity_2 = 0;
		}
	}
	if (RETRO_KeyState(SDL_SCANCODE_9)) { // increase vertex 31 intensity
		if (++intensity_3 > 63) {
			intensity_3 = 63;
		}
	}
	if (RETRO_KeyState(SDL_SCANCODE_6)) { // decrease vertex 3 intensity
		if (--intensity_3 < 0) {
			intensity_3 = 0;
		}
	}

	// draw gouraud shaded triangle
	Draw_Triangle_2D_Gouraud(x, y, x - 50, y + 60, x + 30, y + 80, RETRO.framebuffer, intensity_1, intensity_2, intensity_3);

	// label vertices
	Print_String(x, y - 10, 9, "1", 1);
	Print_String(x - 60, y + 60, 9, "2", 1);
	Print_String(x + 40, y + 80, 9, "3", 1);

	// print out vertex intensities
	sprintf(buffer, "Vertex 1 = %d  ", intensity_1);
	Print_String_DB(0, 0, 12, buffer, 0);

	sprintf(buffer, "Vertex 2 = %d  ", intensity_2);
	Print_String_DB(0, 10, 12, buffer, 0);

	sprintf(buffer, "Vertex 3 = %d  ", intensity_3);
	Print_String_DB(0, 20, 12, buffer, 0);
}

void DEMO_Initialize(void)
{
	// set palette
	for (int i = 1; i < 15; i++) {
		RETRO_SetColor(i, RANDOM(255), RANDOM(255), RANDOM(255));
	}

	// alter the palette and introduce some greys
	Make_Grey_Palette();
}
