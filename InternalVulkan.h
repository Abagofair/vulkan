#pragma once

struct Window;

void RecreateSwapChain();
void CreateVulkanInstance(struct Window* window);
void DrawFrame();
void DestroyVulkan();