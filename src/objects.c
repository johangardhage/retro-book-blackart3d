//
// objects.c
//
#include "lib/retro.h"
#include "lib/retromain.h"
#include "lib/retrofont.h"
#include "black3.h"
#include "black4.h"
#include "black8.h"
#include "black11.h"

object test_object[4];   // the test objects
char buffer[80]; // used to print strings

void DEMO_Render(double deltatime)
{
	// compute starting time of this frame
	starting_time = Timer_Query();

	// test if user is moving viewpoint in positive X
	if (RETRO_KeyState(SDL_SCANCODE_RIGHT)) {
		view_point.x += 5;
	}
	// test if user is moving viewpoint in negative X
	if (RETRO_KeyState(SDL_SCANCODE_LEFT)) {
		view_point.x -= 5;
	}
	// test if user is moving viewpoint in positive Y
	if (RETRO_KeyState(SDL_SCANCODE_UP)) {
		view_point.y += 5;
	}
	// test if user is moving viewpoint in negative Y
	if (RETRO_KeyState(SDL_SCANCODE_DOWN)) {
		view_point.y -= 5;
	}
	// test if user is moving viewpoint in positive Z
	if (RETRO_KeyState(SDL_SCANCODE_PAGEUP)) {
		view_point.z += 5;
	}
	// test if user is moving viewpoint in negative Z
	if (RETRO_KeyState(SDL_SCANCODE_PAGEDOWN)) {
		view_point.z -= 5;
	}
	// this section takes care of view angle rotation
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

	// create the global world to camera transformation matrix
	Create_World_To_Camera();

	// blank object removal message area
	sprintf(buffer, "Objects Removed                       ");
	Print_String_DB(0, 180, 10, buffer, 0);

	// process each object
	for (int index = 0; index < 4; index++) {
		// test if this object should be processed
		if (!Remove_Object(&test_object[index], OBJECT_CULL_XYZ_MODE)) {
			// convert to world coordinates
			Local_To_World_Object((object_ptr)&test_object[index]);

			// shade and remove backfaces, ignore the backface part for now
			// notice that backface shadin and backface removal is done in world coordinates
			Remove_Backfaces_And_Shade((object_ptr)&test_object[index]);

			// convert to camera coordinates
			World_To_Camera_Object((object_ptr)&test_object[index]);

			// draw the object
			Draw_Object_Solid((object_ptr)&test_object[index]);
		} else {
			sprintf(buffer, "%d, ", index);
			Print_String_DB(128 + index * 16, 180, 10, buffer, 0);
		}
	}

	// print out viewpoint
	sprintf(buffer, "Viewpoint is at (%d,%d,%d)     ", (int)view_point.x, (int)view_point.y, (int)view_point.z);
	Print_String_DB(0, 0, 10, buffer, 0);
	sprintf(buffer, "Viewangle is at (%d,%d,%d)     ", (int)view_angle.ang_x, (int)view_angle.ang_y, (int)view_angle.ang_z);
	Print_String_DB(0, 10, 10, buffer, 0);

	// lock onto 18 frames per second max
	Time_Delay_ms(18 - (Timer_Query() - starting_time));
}

void DEMO_Initialize(void)
{
	// load in the objects from the command line
	for (int index = 0; index < 4; index++) {
		if (!PLG_Load_Object((object_ptr)&test_object[index], "assets/cube.plg", 1)) {
			printf("\nCouldn't find file %s", "assets/cube.plg");
			return;
		}
	}

	// position the objects 300 units in front of user
	for (int index = 0; index < 4; index++) {
		Position_Object((object_ptr)&test_object[index], -150 + index * 100, 0, 300);
	}

	// set the viewpoint
	view_point.x = 0;
	view_point.y = 0;
	view_point.z = 0;

	// create the sin/cos lookup tables used for the rotation function
	Build_Look_Up_Tables();
	// read the 3d color palette off disk
	Load_Palette_Disk("assets/standard.pal", (RGB_palette_ptr)&color_palette_3d);
	Write_Palette(0, 255, (RGB_palette_ptr)&color_palette_3d);

	// set viewing distance
	viewing_distance = 250;
}
