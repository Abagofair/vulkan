#include <SDL.h>
#include "Window.h"
#include "InternalVulkan.h"
#include "File.h"
#include "Timer.h"
#include "AssetManager.h"

int main(int argc, char *argv[])
{
	//mat4 rot;
	//glm_mat4_identity(rot);
	//glm_mat4_print(rot, stdout);

	Timer_Start();

	struct CreateWindowParams params = { .dimensions = { 1920, 1080 }, .title = "Hello, World!" };

	struct Window *window = CreateWindow(&params);

	FILE *f = fopen("test.ass", "rb");
	LoadTextures(f);

	CreateVulkanInstance(window);

	SDL_Event e;
	int quit = 0;

	while (!quit)
	{
		while (SDL_PollEvent(&e) != 0)
		{
			if (e.type == SDL_QUIT)
			{
				quit = 1;
				break;
			}
			if (e.window.event == SDL_WINDOWEVENT_MINIMIZED)
			{
				RecreateSwapChain();
				break;
			}
		}

		DrawFrame();
	}

	DestroyVulkan();
	DestroyWindow();
	DestroyTextures();

	printf("Exiting....\n");

	SDL_Quit();

	return 0;
}