//
// zdemo.c
//
#include "lib/retro.h"
#include "lib/retromain.h"
#include "lib/retrofont.h"
#include "black3.h"
#include "black4.h"
#include "black8.h"
#include "black11.h"
#include "black15.h"

void DEMO_Render(double deltatime)
{
	int x = 160,  // local position of object
		y = 100,
		z = 100;

	// initialize the z buffer with a impossibly distant value
	Fill_Z_Buffer(16000);

	// draw two interesecting triangles
	Draw_Tri_3D_Z(x, y, z, x - 30, y + 40, z, x + 40, y + 50, z, 10);
	Draw_Tri_3D_Z(x + 10, y + 10, z - 5, x - 50, y + 60, z + 5, x + 30, y + 20, z - 2, 1);
}

void DEMO_Initialize(void)
{
	// Init palette
	RETRO_SetColor(0, 0, 0, 0);
	for (int i = 1; i < RETRO_COLORS; i++) {
		RETRO_SetColor(i, RANDOM(RETRO_COLORS), RANDOM(RETRO_COLORS), RANDOM(RETRO_COLORS));
	}

	// create a 200 line z buffer (128k)
	Create_Z_Buffer(200);
}

void DEMO_Deinitialize(void)
{
	Delete_Z_Buffer();
}
