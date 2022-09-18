//
// Created by Anders on 23/08/2022.
//

#include <assert.h>

#include <SDL.h>
#include <SDL_vulkan.h>

#include "Window.h"

static SDL_Window* sdlWindow;

struct Window* CreateWindow(struct CreateWindowParams* params)
{
    assert(params != NULL);

    if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
    {
        printf("SDL_Init failed: %s\n", SDL_GetError());
        abort();
    }

    sdlWindow = SDL_CreateWindow(
        params->title,
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        params->dimensions[0],
        params->dimensions[1],
        SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE
    );

    if (sdlWindow == NULL)
    {
        printf("Could not create SDL_Window\n");
        printf("SDL_Window failed: %s\n", SDL_GetError());
        abort();
    }

    struct Window* window = malloc(sizeof *window);
    int success = memcpy_s(window->dimensions,
             sizeof(ivec2),
             params->dimensions,
             sizeof(ivec2));

    if (success != 0)
    {
        //log
        abort();
    }

    return window;
}

void DestroyWindow()
{
	SDL_DestroyWindow(sdlWindow);

	printf("Destroyed SDL window\n");
}

void GetVulkanExtensions(struct SupportedVulkanExtensions* supportedVulkanExtensions)
{
    assert(supportedVulkanExtensions != NULL);

    SDL_bool succeeded = SDL_Vulkan_GetInstanceExtensions(sdlWindow, &supportedVulkanExtensions->count, NULL);
    if (succeeded == SDL_FALSE)
    {
        printf("Error: %s\n", SDL_GetError());
        abort();
    }

    supportedVulkanExtensions->names = malloc(sizeof(char) * 255 * (supportedVulkanExtensions->count) + 1);
    succeeded = SDL_Vulkan_GetInstanceExtensions(sdlWindow, &supportedVulkanExtensions->count, supportedVulkanExtensions->names);
    if (succeeded == SDL_FALSE)
    {
        printf("Error: %s\n", SDL_GetError());
        abort();
    }

    /*if (_enableValidationLayers)
    {
        extensions[(*extensionCount)] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
    }

    ++(*extensionCount);*/
}

void GetVulkanSurface(VkInstance instance, VkSurfaceKHR *surface)
{
    SDL_bool success = SDL_Vulkan_CreateSurface(sdlWindow, instance, surface);
    if (success != SDL_TRUE)
    {
        abort();
    }

    printf("Get a VkSurfaceKHR from SDL\n");
}

void GetFramebufferSize(int *width, int *height)
{
    SDL_Vulkan_GetDrawableSize(sdlWindow, width, height);
}