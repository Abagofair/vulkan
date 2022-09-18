#pragma once

#include <stdlib.h>
#include "external/cglm/vec2-ext.h"

#include <vulkan/vulkan.h>

struct Window
{
    ivec2 dimensions;
    const char* title;
};

struct CreateWindowParams
{
    ivec2 dimensions;
    const char* title;
};

struct SupportedVulkanExtensions
{
    const char** names;
    uint32_t count;
};

struct Window *CreateWindow(struct CreateWindowParams *params);

void DestroyWindow();

void GetVulkanExtensions(struct SupportedVulkanExtensions* supportedVulkanExtensions);

void GetVulkanSurface(VkInstance instance, VkSurfaceKHR *surface);

void GetFramebufferSize(int *width, int *height);