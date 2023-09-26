//
// wiredemo.c
//
#include "lib/retro.h"
#include "lib/retromain.h"
#include "lib/retrofont.h"
#include "black3.h"
#include "black4.h"
#include "black8.h"
#include "black11.h"

object test_object;   // the test object

void DEMO_Render(double deltatime)
{
	// compute starting time of this frame
	starting_time = Timer_Query();

	// test if user is moving object to right
	if (RETRO_KeyState(SDL_SCANCODE_RIGHT)) {
		test_object.world_pos.x += 5;
	}
	// test if user is moving object to left
	if (RETRO_KeyState(SDL_SCANCODE_LEFT)) {
		test_object.world_pos.x -= 5;
	}
	// test if user is moving object up
	if (RETRO_KeyState(SDL_SCANCODE_UP)) {
		test_object.world_pos.y -= 5;
	}
	// test if user is moving object down
	if (RETRO_KeyState(SDL_SCANCODE_DOWN)) {
		test_object.world_pos.y += 5;
	}
	// test if user is moving object farther
	if (RETRO_KeyState(SDL_SCANCODE_PAGEUP)) {
		test_object.world_pos.z += 15;
	}
	// test if user is moving object nearer
	if (RETRO_KeyState(SDL_SCANCODE_PAGEDOWN)) {
		test_object.world_pos.z -= 15;
	}

	// rotate the object on all three axes
	Rotate_Object((object_ptr)&test_object, 2, 4, 6);

	// convert the local coordinates into camera coordinates for projection
	// note the viewer is at (0,0,0) with angles 0,0,0 so the transformaton
	// is simply to add the world position to each local vertex
	for (int index = 0; index < test_object.num_vertices; index++) {
		test_object.vertices_camera[index].x = test_object.vertices_local[index].x + test_object.world_pos.x;
		test_object.vertices_camera[index].y = test_object.vertices_local[index].y + test_object.world_pos.y;
		test_object.vertices_camera[index].z = test_object.vertices_local[index].z + test_object.world_pos.z;
	}

	// draw the object
	Draw_Object_Wire((object_ptr)&test_object);

	// lock onto 18 frames per second max
	Time_Delay_ms(18 - (Timer_Query() - starting_time));
}

void DEMO_Initialize(void)
{
	// Init palette
	RETRO_SetColor(0, 0, 0, 0);
	for (int i = 1; i < RETRO_COLORS; i++) {
		RETRO_SetColor(i, RANDOM(RETRO_COLORS), RANDOM(RETRO_COLORS), RANDOM(RETRO_COLORS));
	}

	// load in the object from the command line
	if (!PLG_Load_Object(&test_object, "assets/cubew.plg", 1)) {
		printf("\nCouldn't find file %s", "assets/cubew.plg");
		return;
	}

	// position the object
	test_object.world_pos.x = 0;
	test_object.world_pos.y = 0;
	test_object.world_pos.z = 300;

	// create the sin/cos lookup tables used for the rotation function
	Build_Look_Up_Tables();

	// set viewing distance
	viewing_distance = 250;
}
