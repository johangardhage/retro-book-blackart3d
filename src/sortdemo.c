//
// sortdemo.c
//
#include "lib/retro.h"
#include "lib/retromain.h"
#include "lib/retrofont.h"
#include "black3.h"
#include "black4.h"
#include "black8.h"
#include "black11.h"
#include "black15.h"

object test_objects[16];                // objects in universe

void DEMO_Render(double deltatime)
{
	int index;

	float x, y, z;

	char buffer[80];

	// compute starting time of this frame
	starting_time = Timer_Query();

	// move the light source
	if (RETRO_KeyState(SDL_SCANCODE_T)) {
		x = light_source.x;
		y = cos(.2) * light_source.y - sin(.2) * light_source.z;
		z = sin(.2) * light_source.y + cos(.2) * light_source.z;

		light_source.y = y;
		light_source.z = z;
	}
	if (RETRO_KeyState(SDL_SCANCODE_G)) {
		y = light_source.y;
		x = cos(.2) * light_source.x + sin(.2) * light_source.z;
		z = -sin(.2) * light_source.x + cos(.2) * light_source.z;

		light_source.x = x;
		light_source.z = z;
	}
	if (RETRO_KeyState(SDL_SCANCODE_B)) {
		z = light_source.z;
		x = cos(.2) * light_source.x - sin(.2) * light_source.y;
		y = sin(.2) * light_source.x + cos(.2) * light_source.y;

		light_source.x = x;
		light_source.y = y;
	}
	if (RETRO_KeyState(SDL_SCANCODE_Y)) {
		x = light_source.x;
		y = cos(-.2) * light_source.y - sin(-.2) * light_source.z;
		z = sin(-.2) * light_source.y + cos(-.2) * light_source.z;

		light_source.y = y;
		light_source.z = z;
	}
	if (RETRO_KeyState(SDL_SCANCODE_H)) {
		y = light_source.y;
		x = cos(-.2) * light_source.x + sin(-.2) * light_source.z;
		z = -sin(-.2) * light_source.x + cos(-.2) * light_source.z;

		light_source.x = x;
		light_source.z = z;
	}
	if (RETRO_KeyState(SDL_SCANCODE_N)) {
		z = light_source.z;
		x = cos(-.2) * light_source.x - sin(-.2) * light_source.y;
		y = sin(-.2) * light_source.x + cos(-.2) * light_source.y;

		light_source.x = x;
		light_source.y = y;
	}
	// move the viewpoint
	if (RETRO_KeyState(SDL_SCANCODE_UP)) {
		view_point.y += 5;
	}
	if (RETRO_KeyState(SDL_SCANCODE_DOWN)) {
		view_point.y -= 5;
	}
	if (RETRO_KeyState(SDL_SCANCODE_RIGHT)) {
		view_point.x += 5;
	}
	if (RETRO_KeyState(SDL_SCANCODE_LEFT)) {
		view_point.x -= 5;
	}
	if (RETRO_KeyState(SDL_SCANCODE_PAGEUP)) {
		view_point.z += 5;
	}
	if (RETRO_KeyState(SDL_SCANCODE_PAGEDOWN)) {
		view_point.z -= 5;
	}
	// change the viewangles
	if (RETRO_KeyState(SDL_SCANCODE_Z)) {
		if ((view_angle.ang_x += 5) > 360) {
			view_angle.ang_x = 0;
		}
	}
	if (RETRO_KeyState(SDL_SCANCODE_A)) {
		if ((view_angle.ang_x -= 5) < 0) {
			view_angle.ang_x = 360;
		}
	}
	if (RETRO_KeyState(SDL_SCANCODE_X)) {
		if ((view_angle.ang_y += 5) > 360) {
			view_angle.ang_y = 0;
		}
	}
	if (RETRO_KeyState(SDL_SCANCODE_S)) {
		if ((view_angle.ang_y -= 5) < 0) {
			view_angle.ang_y = 360;
		}
	}
	if (RETRO_KeyState(SDL_SCANCODE_C)) {
		if ((view_angle.ang_z += 5) > 360) {
			view_angle.ang_z = 0;
		}
	}
	if (RETRO_KeyState(SDL_SCANCODE_D)) {
		if ((view_angle.ang_z -= 5) < 0) {
			view_angle.ang_z = 360;
		}
	}
	// change the world pos
	if (RETRO_KeyState(SDL_SCANCODE_O)) {
		test_objects[0].world_pos.x -= 2;
	}
	if (RETRO_KeyState(SDL_SCANCODE_P)) {
		test_objects[0].world_pos.x += 2;
	}
	if (RETRO_KeyState(SDL_SCANCODE_LEFTBRACKET)) {
		Scale_Object(&test_objects[0], 1.1);
	}
	if (RETRO_KeyState(SDL_SCANCODE_RIGHTBRACKET)) {
		Scale_Object(&test_objects[0], .9);
	}

	// rotate one of the objects
	Rotate_Object(&test_objects[0], 3, 6, 9);

	// now that user has possible moved viewpoint, create the global world to camera transformation matrix
	Create_World_To_Camera();

	// reset polygon list
	sprintf(buffer, "                           ");
	Print_String_DB(0, 0, 10, buffer, 0);
	Generate_Poly_List(NULL, RESET_POLY_LIST);

	for (index = 0; index < 16; index++) {
		// test if object is visible
		if (!Remove_Object(&test_objects[index], OBJECT_CULL_XYZ_MODE)) {
			// convert object local coordinates to world coordinate
			Local_To_World_Object(&test_objects[index]);

			// remove the backfaces and shade object
			Remove_Backfaces_And_Shade(&test_objects[index]);

			// convert world coordinates to camera coordinate
			World_To_Camera_Object(&test_objects[index]);

			// clip the objects polygons against viewing volume
			Clip_Object_3D(&test_objects[index], CLIP_Z_MODE);

			// generate the final polygon list
			Generate_Poly_List(&test_objects[index], ADD_TO_POLY_LIST);
		} else {
			sprintf(buffer, "%d, ", index);
			Print_String_DB(0 + index * 26, 0, 10, buffer, 0);
		}
	}

	// sort the polygons
	Sort_Poly_List();

	// draw the polygon list
	Draw_Poly_List();

	// lock onto 18 frames per second max
	Time_Delay_ms(18 - (Timer_Query() - starting_time));
}

void DEMO_Initialize(void)
{
	int index;

	// build all look up tables
	Build_Look_Up_Tables();

	// load in the test object
	for (index = 0; index < 16; index++) {
		if (!PLG_Load_Object(&test_objects[index], "assets/cube.plg", 1)) {
			printf("\nCouldn't load file");
		}
	}

	// set the position of the object
	for (index = 0; index < 16; index++) {
		test_objects[index].world_pos.x = -150 + (index % 4) * 100;
		test_objects[index].world_pos.y = 0;
		test_objects[index].world_pos.z = 200 + 300 * (index >> 2);
	}

	// read the 3D color palette off disk
	Load_Palette_Disk("assets/standard.pal", (RGB_palette_ptr)&color_palette_3d);
	Write_Palette(0, 255, (RGB_palette_ptr)&color_palette_3d);
}
