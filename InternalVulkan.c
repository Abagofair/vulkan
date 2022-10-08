//
// Created by Anders on 28/08/2022.
//

#pragma once

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <vulkan/vulkan.h>

#include "File.h"
#include "InternalVulkan.h"
#include "Utilities.h"
#include "Window.h"
#include "Timer.h"
#include "AssetManager.h"
#include "external/cglm/mat4.h"
#include "external/cglm/affine.h"
#include "external/cglm/clipspace/view_rh_zo.h"
#include "external/cglm/clipspace/persp_rh_zo.h"

#define STB_IMAGE_IMPLEMENTATION
#include "external/stb/stb_image.h"

#define ENGINE_NAME "DUNNO"

#define MAX_FRAMES_IN_FLIGHT 2

struct UniformBufferObject {
	mat4 model;
	mat4 view;
	mat4 proj;
};

struct Vertex {
	vec3 pos;
	vec3 color;
	vec2 texCoord;
};

static VkVertexInputBindingDescription GetVertexBindingDescription()
{
	VkVertexInputBindingDescription bindingDescription = { .binding = 0,
							       .stride = sizeof(struct Vertex),
							       .inputRate = VK_VERTEX_INPUT_RATE_VERTEX };

	return bindingDescription;
}

static const VkVertexInputAttributeDescription *GetAttributeDescriptions()
{
	VkVertexInputAttributeDescription *attributeDescriptions =
		malloc(3 * sizeof(VkVertexInputAttributeDescription));

	attributeDescriptions[0].binding = 0;
	attributeDescriptions[0].location = 0;
	attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
	attributeDescriptions[0].offset = offsetof(struct Vertex, pos);

	attributeDescriptions[1].binding = 0;
	attributeDescriptions[1].location = 1;
	attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
	attributeDescriptions[1].offset = offsetof(struct Vertex, color);

	attributeDescriptions[2].binding = 0;
	attributeDescriptions[2].location = 2;
	attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
	attributeDescriptions[2].offset = offsetof(struct Vertex, texCoord);

	return attributeDescriptions;
}

static const struct Vertex vertices[] = {
	{ .pos = { -0.5f, -0.5f, 0.0f }, .color = { 1.0f, 0.0f, 0.0f }, .texCoord = { 0.0f, 1.0f } },
	{ .pos = { 0.5f, -0.5f, 0.0f }, .color = { 0.0f, 1.0f, 0.0f }, .texCoord = { 0.0f, 0.0f } },
	{ .pos = { 0.5f, 0.5f, 0.0f }, .color = { 0.0f, 0.0f, 1.0f }, .texCoord = { 1.0f, 0.0f } },
	{ .pos = { -0.5f, 0.5f, 0.0f }, .color = { 1.0f, 1.0f, 1.0f }, .texCoord = { 1.0f, 1.0f } },

	{ .pos = { -0.5f, -0.5f, -0.5f }, .color = { 1.0f, 0.0f, 0.0f }, .texCoord = { 0.0f, 1.0f } },
	{ .pos = { 0.5f, -0.5f, -0.5f }, .color = { 0.0f, 1.0f, 0.0f }, .texCoord = { 0.0f, 0.0f } },
	{ .pos = { 0.5f, 0.5f, -0.5f }, .color = { 0.0f, 0.0f, 1.0f }, .texCoord = { 1.0f, 0.0f } },
	{ .pos = { -0.5f, 0.5f, -0.5f }, .color = { 1.0f, 1.0f, 1.0f }, .texCoord = { 1.0f, 1.0f } }
};

static const uint16_t indices[] = { 0, 1, 2, 2, 3, 0, 4, 5, 6, 6, 7, 4 };

static VkInstance vulkanInstance;
static VkPhysicalDevice vulkanPhysicalDevice; // implicitly destroyed when destroying VkInstance
static VkDevice vulkanDevice;
static VkQueue vulkanGraphicsQueue; // implicitly cleaned up
static VkQueue vulkanPresentQueue;
static VkSurfaceKHR vulkanSurface;

static VkSwapchainKHR vulkanSwapChain;
static uint32_t swapChainImageCount = 0;
static VkImage *swapChainImages = NULL;
static VkFormat swapChainImageFormat;
static VkExtent2D swapChainExtent;

static VkImage depthImage;
static VkDeviceMemory depthImageMemory;
static VkImageView depthImageView;

static VkRenderPass vulkanRenderPass;
static VkDescriptorSetLayout descriptorSetLayout;
static VkPipelineLayout vulkanPipelineLayout;
static VkPipeline vulkanGraphicsPipeline;

static VkImageView *swapChainImageViews = NULL;
static VkFramebuffer *swapChainFramebuffers = NULL;

static VkCommandPool vulkanCommandPool;
static VkCommandBuffer *vulkanCommandBuffers;

static VkSemaphore *imageAvailableSemaphore;
static VkSemaphore *renderFinishedSemaphore;
static VkFence *inFlightFence;

static VkBuffer vertexBuffer;
static VkDeviceMemory vertexBufferMemory;
static VkBuffer indexBuffer;
static VkDeviceMemory indexBufferMemory;

static VkBuffer *uniformBuffers;
static VkDeviceMemory *uniformBuffersMemory;

static uint32_t mipLevels;
static VkImage textureImage;
static VkDeviceMemory textureImageMemory;
static VkImageView textureImageView;
static VkSampler textureSampler;

static VkDescriptorPool descriptorPool;
static VkDescriptorSet *descriptorSets;

static bool framebufferResized = false;

// Validation and debugging primitives
static VkDebugUtilsMessengerEXT debugMessenger;

const bool enableValidationLayers = true;
static uint32_t enabledValidationLayerCount = 1;
static const char *enabledValidationLayers[] = { "VK_LAYER_KHRONOS_validation" };

static uint32_t deviceExtensionsCount = 1;
static const char *deviceExtensions[] = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

static uint32_t currentFrame = 0;

struct QueueFamilyIndices {
	bool isSet;
	uint32_t graphicsFamily;
	uint32_t presentFamily;
};

struct SwapChainSupportDetails {
	VkSurfaceCapabilitiesKHR capabilities;
	uint32_t formatsCount;
	VkSurfaceFormatKHR *formats;
	uint32_t presentModesCount;
	VkPresentModeKHR *presentModes;
};

static VkFormat FindSupportedFormat(VkFormat *candidates, uint32_t candidateCount, VkImageTiling tiling,
				    VkFormatFeatureFlags features)
{
	assert(candidates != NULL);

	for (uint32_t i = 0; i < candidateCount; ++i)
	{
		VkFormat format = candidates[i];

		VkFormatProperties props;
		vkGetPhysicalDeviceFormatProperties(vulkanPhysicalDevice, format, &props);

		if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
		{
			return format;
		}
		else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
		{
			return format;
		}
	}

	printf("A supported format was not found\n");
	abort();
}

static VkFormat FindDepthFormat()
{
	VkFormat formats[] = {
		VK_FORMAT_D32_SFLOAT,
		VK_FORMAT_D32_SFLOAT_S8_UINT,
		VK_FORMAT_D24_UNORM_S8_UINT
	};

	return FindSupportedFormat(formats, 3, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}

static VkSurfaceFormatKHR ChooseSwapSurfaceFormat(struct SwapChainSupportDetails *swapChainSupportDetails)
{
	for (uint32_t i = 0; i < swapChainSupportDetails->formatsCount; ++i)
	{
		VkSurfaceFormatKHR format = swapChainSupportDetails->formats[i];
		if (format.format == VK_FORMAT_B8G8R8A8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
		{
			return format;
		}
	}

	return swapChainSupportDetails->formats[0];
}

static VkPresentModeKHR ChooseSwapPresentMode(struct SwapChainSupportDetails *swapChainSupportDetails)
{
	for (uint32_t i = 0; i < swapChainSupportDetails->presentModesCount; ++i)
	{
		VkPresentModeKHR presentMode = swapChainSupportDetails->presentModes[i];
		if (presentMode == VK_PRESENT_MODE_MAILBOX_KHR)
		{
			return presentMode;
		}
	}

	return VK_PRESENT_MODE_FIFO_KHR;
}

static VkExtent2D ChooseSwapExtent(struct SwapChainSupportDetails *swapChainSupportDetails)
{
	if (swapChainSupportDetails->capabilities.currentExtent.width != UINT32_MAX)
	{
		return swapChainSupportDetails->capabilities.currentExtent;
	}

	int width, height;
	GetFramebufferSize(&width, &height);

	VkExtent2D actualExtent = { .width = width, .height = height };

	actualExtent.width = ClampU32(actualExtent.width, swapChainSupportDetails->capabilities.minImageExtent.width,
				      swapChainSupportDetails->capabilities.maxImageExtent.width);

	actualExtent.height = ClampU32(actualExtent.height, swapChainSupportDetails->capabilities.minImageExtent.height,
				       swapChainSupportDetails->capabilities.maxImageExtent.height);

	return actualExtent;
}

static struct SwapChainSupportDetails *QuerySwapChainSupport(VkPhysicalDevice device)
{
	struct SwapChainSupportDetails *details = malloc(sizeof(struct SwapChainSupportDetails));

	details->formats = NULL;
	details->presentModes = NULL;

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, vulkanSurface, &details->capabilities);

	uint32_t formatCount = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, vulkanSurface, &formatCount, NULL);
	if (formatCount != 0)
	{
		details->formats = malloc(formatCount * sizeof(VkSurfaceFormatKHR));
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, vulkanSurface, &formatCount, details->formats);
		details->formatsCount = formatCount;
	}

	uint32_t presentModeCount = 0;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, vulkanSurface, &presentModeCount, NULL);
	if (presentModeCount != 0)
	{
		details->presentModes = malloc(presentModeCount * sizeof(VkPresentModeKHR));
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, vulkanSurface, &presentModeCount,
							  details->presentModes);
		details->presentModesCount = presentModeCount;
	}

	return details;
}

static void DestroySwapChainSupportDetails(struct SwapChainSupportDetails *supportDetails)
{
	assert(supportDetails != NULL);

	free(supportDetails->formats);
	free(supportDetails->presentModes);
	free(supportDetails);
}

static bool CheckDeviceExtensionSupport(VkPhysicalDevice device)
{
	uint32_t extensionCount = 0;
	vkEnumerateDeviceExtensionProperties(device, NULL, &extensionCount, NULL);

	VkExtensionProperties *availableExtensions = malloc(extensionCount * sizeof(VkExtensionProperties));
	vkEnumerateDeviceExtensionProperties(device, NULL, &extensionCount, availableExtensions);

	uint32_t matchedExtensions = 0;
	for (uint32_t i = 0; i < extensionCount; ++i)
	{
		VkExtensionProperties availableExtension = availableExtensions[i];
		for (uint32_t j = 0; j < deviceExtensionsCount; ++j)
		{
			const char *requiredExtensionName = deviceExtensions[j];
			if (strcmp(requiredExtensionName, availableExtension.extensionName) == 0)
			{
				++matchedExtensions;
			}
		}
	}

	free(availableExtensions);

	return deviceExtensionsCount == matchedExtensions;
}

static struct QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device)
{
	assert(device != NULL);

	struct QueueFamilyIndices familyIndices = { .graphicsFamily = -1, .presentFamily = -1, .isSet = false };

	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, NULL);

	VkQueueFamilyProperties *queueFamilies = malloc(queueFamilyCount * sizeof(struct VkQueueFamilyProperties));
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies);

	for (uint32_t i = 0; i < queueFamilyCount; ++i)
	{
		VkQueueFamilyProperties queueFamily = queueFamilies[i];
		if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			familyIndices.graphicsFamily = i;
		}

		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, vulkanSurface, &presentSupport);

		if (presentSupport)
		{
			familyIndices.presentFamily = i;
			familyIndices.isSet = true;
		}
	}

	free(queueFamilies);

	return familyIndices;
}

static void CreateSwapChain()
{
	struct SwapChainSupportDetails *swapChainSupportDetails = QuerySwapChainSupport(vulkanPhysicalDevice);

	VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapChainSupportDetails);
	VkPresentModeKHR presentMode = ChooseSwapPresentMode(swapChainSupportDetails);
	VkExtent2D extent = ChooseSwapExtent(swapChainSupportDetails);

	/*
     * However, simply sticking to this minimum means that we may sometimes have
     * to wait on the driver to complete internal operations before we can acquire
     * another image to render to. Therefore, it is recommended to request at
     * least one more image than the minimum
     */
	uint32_t imageCount = swapChainSupportDetails->capabilities.minImageCount + 1;

	if (swapChainSupportDetails->capabilities.maxImageCount > 0 &&
	    imageCount > swapChainSupportDetails->capabilities.maxImageCount)
	{
		imageCount = swapChainSupportDetails->capabilities.maxImageCount;
	}

	struct QueueFamilyIndices indices = FindQueueFamilies(vulkanPhysicalDevice);

	VkSwapchainCreateInfoKHR createInfo = { .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
						.pNext = NULL,
						.flags = 0,
						.surface = vulkanSurface,
						.minImageCount = imageCount,
						.imageFormat = surfaceFormat.format,
						.imageColorSpace = surfaceFormat.colorSpace,
						.imageExtent = extent,
						.imageArrayLayers = 1,
						.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
						//.imageSharingMode = ,
						//.queueFamilyIndexCount = ,
						//.pQueueFamilyIndices = ,
						.preTransform = swapChainSupportDetails->capabilities.currentTransform,
						.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
						.presentMode = presentMode,
						.clipped = VK_TRUE,
						.oldSwapchain = NULL };

	uint32_t queueFamilyIndices[2] = { indices.graphicsFamily, indices.presentFamily };
	if (indices.graphicsFamily != indices.presentFamily)
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	else
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount = 0; // Optional
		createInfo.pQueueFamilyIndices = NULL; // Optional
	}

	VkResult result = vkCreateSwapchainKHR(vulkanDevice, &createInfo, NULL, &vulkanSwapChain);
	if (result != VK_SUCCESS)
	{
		printf("Could not create swap chain\n");
		abort();
	}

	swapChainImageFormat = surfaceFormat.format;
	swapChainExtent = extent;

	vkGetSwapchainImagesKHR(vulkanDevice, vulkanSwapChain, &imageCount, NULL);
	swapChainImages = malloc(imageCount * sizeof(VkImage));
	vkGetSwapchainImagesKHR(vulkanDevice, vulkanSwapChain, &imageCount, swapChainImages);
	swapChainImageCount = imageCount;

	DestroySwapChainSupportDetails(swapChainSupportDetails);

	printf("Created a swap chain\n");
}

static bool IsDeviceSuitable(VkPhysicalDevice device)
{
	VkPhysicalDeviceProperties deviceProperties;
	vkGetPhysicalDeviceProperties(device, &deviceProperties);

	VkPhysicalDeviceFeatures deviceFeatures;
	vkGetPhysicalDeviceFeatures(device,
				    &deviceFeatures); // query for optional stuff like texture compression,
	// 64-bit floats and multi viewport rendering

	struct QueueFamilyIndices indices = FindQueueFamilies(device);

	bool hasRequiredDeviceExtensions = CheckDeviceExtensionSupport(device);

	bool swapChainAdequate = false;
	if (hasRequiredDeviceExtensions)
	{
		struct SwapChainSupportDetails *swapChainSupportDetails = QuerySwapChainSupport(device);
		swapChainAdequate = swapChainSupportDetails->formatsCount > 0 &&
				    swapChainSupportDetails->presentModesCount > 0;

		DestroySwapChainSupportDetails(swapChainSupportDetails);
	}

	return indices.isSet && deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU &&
	       deviceFeatures.geometryShader && hasRequiredDeviceExtensions && swapChainAdequate &&
	       deviceFeatures.samplerAnisotropy;
}

static void PickPhysicalDevice()
{
	assert(vulkanInstance != NULL);

	vulkanPhysicalDevice = NULL;
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(vulkanInstance, &deviceCount, NULL);

	if (deviceCount == 0)
	{
		abort();
	}

	VkPhysicalDevice *devices = malloc(deviceCount * sizeof(VkPhysicalDevice));
	vkEnumeratePhysicalDevices(vulkanInstance, &deviceCount, devices);

	for (uint32_t i = 0; i < deviceCount; ++i)
	{
		VkPhysicalDevice device = devices[i];
		if (IsDeviceSuitable(device))
		{
			vulkanPhysicalDevice = device;
		}
	}

	free(devices);

	if (vulkanPhysicalDevice == NULL)
	{
		abort();
	}

	printf("Picked a compatible physical device\n");
}

static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
						    VkDebugUtilsMessageTypeFlagsEXT messageType,
						    const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
						    void *pUserData)
{
	printf("validation layer: %s\n", pCallbackData->pMessage);
	return VK_FALSE;
}

static void CreateInstanceCreateInfo(VkApplicationInfo *applicationInfo)
{
	VkDebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfoExtMessenger = {
		.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
		.pNext = NULL,
		.flags = 0,
		.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
				   VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
				   VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
		.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
			       VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
			       VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
		.pfnUserCallback = DebugCallback,
		.pUserData = NULL
	};

	struct SupportedVulkanExtensions supportedVulkanExtensions;
	GetVulkanExtensions(&supportedVulkanExtensions);

	if (enableValidationLayers)
	{
		supportedVulkanExtensions.names[supportedVulkanExtensions.count++] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
	}

	VkInstanceCreateInfo createInfo = { .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
					    .pNext = &debugUtilsMessengerCreateInfoExtMessenger,
					    .flags = 0,
					    .pApplicationInfo = applicationInfo,
					    .enabledLayerCount = enabledValidationLayerCount,
					    .ppEnabledLayerNames = enabledValidationLayers,
					    .enabledExtensionCount = supportedVulkanExtensions.count,
					    .ppEnabledExtensionNames = supportedVulkanExtensions.names };

	VkResult result = vkCreateInstance(&createInfo, NULL, &vulkanInstance);

	if (result != VK_SUCCESS)
	{
		printf("vkCreateInstance was not a success\n");
		abort();
	}

	free(supportedVulkanExtensions.names);

	printf("Created VkInstance\n");
}

static VkResult CreateDebugUtilsMessengerEXT(VkInstance vkInstance,
					     const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
					     const VkAllocationCallbacks *pAllocator,
					     VkDebugUtilsMessengerEXT *pDebugMessenger)
{
	PFN_vkCreateDebugUtilsMessengerEXT func =
		(PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(vkInstance, "vkCreateDebugUtilsMessengerEXT");
	if (func != NULL)
	{
		return func(vkInstance, pCreateInfo, pAllocator, pDebugMessenger);
	}
	else
	{
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

static bool CheckValidationLayerSupport()
{
	uint32_t validationLayerPropertyCount = 1;
	vkEnumerateInstanceLayerProperties(&validationLayerPropertyCount, NULL);

	VkLayerProperties *layerProperties = malloc(sizeof(VkLayerProperties) * validationLayerPropertyCount);
	vkEnumerateInstanceLayerProperties(&validationLayerPropertyCount, &layerProperties[0]);

	for (int i = 0; i < enabledValidationLayerCount; ++i)
	{
		bool layerFound = false;
		const char *layerName = enabledValidationLayers[i];
		for (int j = 0; j < validationLayerPropertyCount; ++j)
		{
			VkLayerProperties layerProperty = layerProperties[j];
			if (strcmp(layerName, layerProperty.layerName) != 0)
			{
				layerFound = true;
				break;
			}
		}

		if (!layerFound)
		{
			return false;
		}
	}

	free(layerProperties);

	return true;
}

static void DestroyDebugUtilsMessengerEXT(VkInstance vkInstance, VkDebugUtilsMessengerEXT debugMessengerExt,
					  const VkAllocationCallbacks *pAllocator)
{
	PFN_vkDestroyDebugUtilsMessengerEXT func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
		vulkanInstance, "vkDestroyDebugUtilsMessengerEXT");
	if (func != NULL)
	{
		func(vkInstance, debugMessengerExt, pAllocator);
	}
}

static void SetupDebugMessenger()
{
	VkDebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfoExtMessenger = {
		.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
		.pNext = NULL,
		.flags = 0,
		.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
				   VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
		.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
			       VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
			       VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
		.pfnUserCallback = DebugCallback,
		.pUserData = NULL
	};

	if (CreateDebugUtilsMessengerEXT(vulkanInstance, &debugUtilsMessengerCreateInfoExtMessenger, NULL,
					 &debugMessenger) != VK_SUCCESS)
	{
		printf("CreateDebugUtilsMessengerEXT setup error\n");
		abort();
	}
}

static void CreateLogicalDevice()
{
	struct QueueFamilyIndices indices = FindQueueFamilies(vulkanPhysicalDevice);

	if (!indices.isSet)
	{
		abort();
	}

	float queuePriority = 1.0f;

	struct VkDeviceQueueCreateInfo graphicsQueueCreateInfo = { .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
								   .pNext = NULL,
								   .flags = 0,
								   .queueFamilyIndex = indices.graphicsFamily,
								   .queueCount = 1,
								   .pQueuePriorities = &queuePriority };

	struct VkDeviceQueueCreateInfo presentQueueCreateInfo = { .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
								  .pNext = NULL,
								  .flags = 0,
								  .queueFamilyIndex = indices.presentFamily,
								  .queueCount = 1,
								  .pQueuePriorities = &queuePriority };

	struct VkPhysicalDeviceFeatures deviceFeatures = { .samplerAnisotropy = VK_TRUE };

	struct VkDeviceQueueCreateInfo queueCreateInfos[2] = { graphicsQueueCreateInfo, presentQueueCreateInfo };

	struct VkDeviceCreateInfo createInfo = { .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
						 .pNext = NULL,
						 .flags = 0,
						 .queueCreateInfoCount = 2,
						 .pQueueCreateInfos = queueCreateInfos,
						 .enabledLayerCount = 0,
						 .ppEnabledLayerNames = NULL,
						 .enabledExtensionCount = deviceExtensionsCount,
						 .ppEnabledExtensionNames = deviceExtensions,
						 .pEnabledFeatures = &deviceFeatures };

	if (enableValidationLayers)
	{
		createInfo.enabledLayerCount = enabledValidationLayerCount;
		createInfo.ppEnabledLayerNames = enabledValidationLayers;
	}
	else
	{
		createInfo.enabledLayerCount = 0;
	}

	VkResult result = vkCreateDevice(vulkanPhysicalDevice, &createInfo, NULL, &vulkanDevice);
	if (result != VK_SUCCESS)
	{
		printf("Could not create logical device\n");
		abort();
	}

	vkGetDeviceQueue(vulkanDevice, indices.graphicsFamily, 0, &vulkanGraphicsQueue);
	vkGetDeviceQueue(vulkanDevice, indices.presentFamily, 0, &vulkanPresentQueue);

	printf("Created a logical device and created graphics queue and present queue\n");
}

static void CreateSurface()
{
	GetVulkanSurface(vulkanInstance, &vulkanSurface);
}

static void CreateImageViews()
{
	swapChainImageViews = malloc(swapChainImageCount * sizeof(VkImageView));

	for (uint32_t i = 0; i < swapChainImageCount; ++i)
	{
		VkComponentMapping componentMapping = { .r = VK_COMPONENT_SWIZZLE_IDENTITY,
							.g = VK_COMPONENT_SWIZZLE_IDENTITY,
							.b = VK_COMPONENT_SWIZZLE_IDENTITY,
							.a = VK_COMPONENT_SWIZZLE_IDENTITY };

		VkImageSubresourceRange imageSubresourceRange = { .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
								  .baseMipLevel = 0,
								  .levelCount = 1,
								  .baseArrayLayer = 0,
								  .layerCount = 1 };

		VkImage image = swapChainImages[i];
		VkImageViewCreateInfo createInfo = { .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
						     .pNext = NULL,
						     .flags = 0,
						     .image = image,
						     .viewType = VK_IMAGE_VIEW_TYPE_2D,
						     .format = swapChainImageFormat,
						     .components = componentMapping,
						     .subresourceRange = imageSubresourceRange };

		VkResult result = vkCreateImageView(vulkanDevice, &createInfo, NULL, &swapChainImageViews[i]);
		if (result != VK_SUCCESS)
		{
			printf("Could not create imageview\n");
			abort();
		}
	}

	printf("Created a set of image views\n");
}

static VkShaderModule CreateShaderModule(const char *byteCode, uint64_t size)
{
	VkShaderModuleCreateInfo createInfo = { .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
						.pNext = NULL,
						.codeSize = size,
						.flags = 0,
						.pCode = (const uint32_t *)byteCode };

	VkShaderModule shaderModule;
	VkResult result = vkCreateShaderModule(vulkanDevice, &createInfo, NULL, &shaderModule);
	if (result != VK_SUCCESS)
	{
		printf("Could not create shadermodule\n");
		abort();
	}

	free(byteCode);

	return shaderModule;
}

void CreateGraphicsPipeline()
{
	uint64_t vertShaderFileSize = 0;
	const char *vertShaderCode = ReadBytes("shaders/quad.glsl.vert.spv", &vertShaderFileSize);
	if (vertShaderCode == NULL)
	{
		abort();
	}

	//WriteBytes("shaders/quad.vert.write.spv", vertShaderCode, vertShaderFileSize);

	uint64_t fragShaderFileSize = 0;
	const char *fragShaderCode = ReadBytes("shaders/quad.glsl.frag.spv", &fragShaderFileSize);
	if (fragShaderCode == NULL)
	{
		abort();
	}

	//WriteBytes("shaders/quad.glsl.vert.frag.write.spv", fragShaderCode, fragShaderFileSize);

	VkShaderModule vertShaderModule = CreateShaderModule(vertShaderCode, vertShaderFileSize);
	VkShaderModule fragShaderModule = CreateShaderModule(fragShaderCode, fragShaderFileSize);

	VkPipelineShaderStageCreateInfo vertShaderStageInfo = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.stage = VK_SHADER_STAGE_VERTEX_BIT,
		.module = vertShaderModule,
		.pName = "main",
		.pSpecializationInfo = NULL
	};

	VkPipelineShaderStageCreateInfo fragShaderStageInfo = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.stage = VK_SHADER_STAGE_FRAGMENT_BIT,
		.module = fragShaderModule,
		.pName = "main",
		.pSpecializationInfo = NULL
	};

	VkPipelineShaderStageCreateInfo shaderStages[2] = { vertShaderStageInfo, fragShaderStageInfo };

	// for dynamic viewport and scissor setup
	VkDynamicState dynamicStates[2] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };

	VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.dynamicStateCount = 2,
		.pDynamicStates = dynamicStates
	};

	struct VkVertexInputBindingDescription bindingDescription = GetVertexBindingDescription();

	const struct VkVertexInputAttributeDescription *attributeDescription = GetAttributeDescriptions();

	VkPipelineVertexInputStateCreateInfo vertexInputInfo = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.vertexBindingDescriptionCount = 1,
		.pVertexBindingDescriptions = &bindingDescription,
		.vertexAttributeDescriptionCount = 3,
		.pVertexAttributeDescriptions = attributeDescription
	};

	VkPipelineInputAssemblyStateCreateInfo inputAssembly = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
		.primitiveRestartEnable = VK_FALSE
	};

	VkViewport viewport = { .x = 0,
				.y = 0,
				.width = (float)swapChainExtent.width,
				.height = (float)swapChainExtent.height,
				.minDepth = 0.0f,
				.maxDepth = 1.0f };

	VkRect2D scissor = { .offset = { .x = 0, .y = 0 }, .extent = swapChainExtent };

	VkPipelineViewportStateCreateInfo viewportState = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.viewportCount = 1,
		.pViewports = NULL,
		.scissorCount = 1,
		.pScissors = NULL
	};

	VkPipelineRasterizationStateCreateInfo rasterizer = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.depthClampEnable = VK_FALSE,
		.rasterizerDiscardEnable = VK_FALSE,
		.polygonMode = VK_POLYGON_MODE_FILL,
		.lineWidth = 1.0f,
		.cullMode = VK_CULL_MODE_BACK_BIT,
		.frontFace = VK_FRONT_FACE_CLOCKWISE,
		.depthBiasEnable = VK_FALSE,
		.depthBiasConstantFactor = 0.0f,
		.depthBiasClamp = 0.0f,
		.depthBiasSlopeFactor = 0.0f
	};

	VkPipelineMultisampleStateCreateInfo multisampling = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
		.sampleShadingEnable = VK_FALSE,
		.minSampleShading = 1.0f,
		.pSampleMask = NULL,
		.alphaToCoverageEnable = VK_FALSE,
		.alphaToOneEnable = VK_FALSE
	};

	VkPipelineColorBlendAttachmentState colorBlendAttachment = { .blendEnable = VK_FALSE,
								     /*.srcColorBlendFactor = VK_BLEND_FACTOR_ONE,
								     .dstColorBlendFactor = VK_BLEND_FACTOR_ZERO,
								     .colorBlendOp = VK_BLEND_OP_ADD,
								     .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
								     .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
								     .alphaBlendOp = VK_BLEND_OP_ADD,*/
								     .colorWriteMask = VK_COLOR_COMPONENT_R_BIT |
										       VK_COLOR_COMPONENT_G_BIT |
										       VK_COLOR_COMPONENT_B_BIT |
										       VK_COLOR_COMPONENT_A_BIT };

	VkPipelineColorBlendStateCreateInfo colorBlending = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.logicOpEnable = VK_FALSE,
		.logicOp = VK_LOGIC_OP_COPY,
		.attachmentCount = 1,
		.pAttachments = &colorBlendAttachment,
		.blendConstants = { 0.0f, 0.0f, 0.0f, 0.0f }
	};

	VkPipelineLayoutCreateInfo pipelineLayoutInfo = { .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
							  .pNext = NULL,
							  .flags = 0,
							  .setLayoutCount = 1,
							  .pSetLayouts = &descriptorSetLayout,
							  .pushConstantRangeCount = 0,
							  .pPushConstantRanges = NULL };

	VkResult result = vkCreatePipelineLayout(vulkanDevice, &pipelineLayoutInfo, NULL, &vulkanPipelineLayout);
	if (result != VK_SUCCESS)
	{
		printf("Could not create pipeline layout\n");
		abort();
	}

	VkPipelineDepthStencilStateCreateInfo depthStencilStateCreateInfo = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.depthTestEnable = VK_TRUE,
		.depthWriteEnable = VK_TRUE,
		.depthCompareOp = VK_COMPARE_OP_LESS,
		.depthBoundsTestEnable = VK_FALSE,
		.minDepthBounds = 0.0f,
		.maxDepthBounds = 1.0f,
		.stencilTestEnable = VK_FALSE
	};

	VkGraphicsPipelineCreateInfo pipelineCreateInfo = { .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
							    .pNext = NULL,
							    .flags = 0,
							    .stageCount = 2,
							    .pStages = shaderStages,
							    .pVertexInputState = &vertexInputInfo,
							    .pInputAssemblyState = &inputAssembly,
							    .pViewportState = &viewportState,
							    .pRasterizationState = &rasterizer,
							    .pMultisampleState = &multisampling,
							    .pDepthStencilState = &depthStencilStateCreateInfo,
							    .pColorBlendState = &colorBlending,
							    .pDynamicState = &dynamicStateCreateInfo,
							    .layout = vulkanPipelineLayout,
							    .renderPass = vulkanRenderPass,
							    .subpass = 0,
							    .basePipelineHandle = NULL,
							    .basePipelineIndex = -1 };

	VkResult pipelineResult =
		vkCreateGraphicsPipelines(vulkanDevice, NULL, 1, &pipelineCreateInfo, NULL, &vulkanGraphicsPipeline);
	if (pipelineResult != VK_SUCCESS)
	{
		printf("Could not create graphics pipeline\n");
		abort();
	}

	vkDestroyShaderModule(vulkanDevice, vertShaderModule, NULL);
	vkDestroyShaderModule(vulkanDevice, fragShaderModule, NULL);

	free(attributeDescription);

	printf("Created a graphics pipeline\n");
}

static void CreateRenderPass()
{
	VkAttachmentDescription colorAttachment = { .flags = 0,
						    .format = swapChainImageFormat,
						    .samples = VK_SAMPLE_COUNT_1_BIT,
						    .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
						    .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
						    .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
						    .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
						    .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
						    .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR };

	VkAttachmentReference colorAttachmentRef = { .attachment = 0,
						     .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };

	VkAttachmentDescription depthAttachment = {
		.flags = 0,
		.format = FindDepthFormat(),
		.samples = VK_SAMPLE_COUNT_1_BIT,
		.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
		.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
		.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
		.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
		.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
		.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
	};

	VkAttachmentReference depthAttachmentRef = {
		.attachment = 1,
		.layout	= VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
	};

	VkSubpassDescription subpassDescription = { .flags = 0,
						    .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
						    .inputAttachmentCount = 0,
						    .pInputAttachments = NULL,
						    .colorAttachmentCount = 1,
						    .pColorAttachments = &colorAttachmentRef,
						    .pResolveAttachments = NULL,
						    .pDepthStencilAttachment = &depthAttachmentRef,
						    .preserveAttachmentCount = 0,
						    .pPreserveAttachments = NULL };

	VkSubpassDependency subpassDependency = { .srcSubpass = VK_SUBPASS_EXTERNAL,
						  .dstSubpass = 0,
						  .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
						  .srcAccessMask = 0,
						  .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
						  .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT };

	VkAttachmentDescription attachments[] = { colorAttachment, depthAttachment };

	VkRenderPassCreateInfo renderPassCreateInfo = { .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
							.pNext = NULL,
							.flags = 0,
							.attachmentCount = 2,
							.pAttachments = attachments,
							.subpassCount = 1,
							.pSubpasses = &subpassDescription,
							.dependencyCount = 1,
							.pDependencies = &subpassDependency };

	VkResult result = vkCreateRenderPass(vulkanDevice, &renderPassCreateInfo, NULL, &vulkanRenderPass);
	if (result != VK_SUCCESS)
	{
		printf("Could not create render pass\n");
		abort();
	}

	printf("Created a render pass\n");
}

static void CreateFramebuffers()
{
	swapChainFramebuffers = malloc(swapChainImageCount * sizeof(VkFramebuffer));
	if (swapChainFramebuffers == NULL)
	{
		abort();
	}

	for (uint32_t i = 0; i < swapChainImageCount; ++i)
	{
		VkImageView attachments[] = {
			swapChainImageViews[i],
			depthImageView
		};

		VkFramebufferCreateInfo framebufferCreateInfo = { .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
								  .pNext = NULL,
								  .flags = 0,
								  .renderPass = vulkanRenderPass,
								  .attachmentCount = 2,
								  .pAttachments = attachments,
								  .width = swapChainExtent.width,
								  .height = swapChainExtent.height,
								  .layers = 1 };

		VkResult result =
			vkCreateFramebuffer(vulkanDevice, &framebufferCreateInfo, NULL, &swapChainFramebuffers[i]);
		if (result != VK_SUCCESS)
		{
			printf("Could not create VkFramebuffer\n");
			abort();
		}
	}

	printf("Created framebuffers\n");
}

static void CreateCommandPool()
{
	struct QueueFamilyIndices queueFamilyIndices = FindQueueFamilies(vulkanPhysicalDevice);

	VkCommandPoolCreateInfo commandPoolCreateInfo = { .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
							  .pNext = NULL,
							  .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
							  .queueFamilyIndex = queueFamilyIndices.graphicsFamily };

	VkResult result = vkCreateCommandPool(vulkanDevice, &commandPoolCreateInfo, NULL, &vulkanCommandPool);
	if (result != VK_SUCCESS)
	{
		printf("Could not create command pool\n");
		abort();
	}

	printf("Created a command pool\n");
}

static void CreateCommandBuffers()
{
	vulkanCommandBuffers = malloc(MAX_FRAMES_IN_FLIGHT * sizeof(VkCommandBuffer));
	VkCommandBufferAllocateInfo allocateInfo = { .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
						     .pNext = NULL,
						     .commandPool = vulkanCommandPool,
						     .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
						     .commandBufferCount = MAX_FRAMES_IN_FLIGHT };

	VkResult result = vkAllocateCommandBuffers(vulkanDevice, &allocateInfo, vulkanCommandBuffers);
	if (result != VK_SUCCESS)
	{
		printf("Could not create command buffer\n");
		abort();
	}

	printf("Created a command buffer\n");
}

static void CreateSyncObjects()
{
	imageAvailableSemaphore = malloc(MAX_FRAMES_IN_FLIGHT * sizeof(VkSemaphore));
	renderFinishedSemaphore = malloc(MAX_FRAMES_IN_FLIGHT * sizeof(VkSemaphore));
	inFlightFence = malloc(MAX_FRAMES_IN_FLIGHT * sizeof(VkFence));

	VkSemaphoreCreateInfo createInfo = { .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
					     .pNext = NULL,
					     .flags = 0 };

	VkFenceCreateInfo fenceCreateInfo = { .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
					      .pNext = NULL,
					      .flags = VK_FENCE_CREATE_SIGNALED_BIT };

	for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
	{
		VkResult semaphoreCreateResult =
			vkCreateSemaphore(vulkanDevice, &createInfo, NULL, &imageAvailableSemaphore[i]);
		if (semaphoreCreateResult != VK_SUCCESS)
		{
			printf("Could not semaphore\n");
			abort();
		}

		semaphoreCreateResult = vkCreateSemaphore(vulkanDevice, &createInfo, NULL, &renderFinishedSemaphore[i]);
		if (semaphoreCreateResult != VK_SUCCESS)
		{
			printf("Could not semaphore\n");
			abort();
		}

		VkResult fenceCreateResult = vkCreateFence(vulkanDevice, &fenceCreateInfo, NULL, &inFlightFence[i]);

		if (fenceCreateResult != VK_SUCCESS)
		{
			printf("Could not create fence\n");
			abort();
		}
	}

	printf("Created sync objects\n");
}

static void RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex)
{
	VkCommandBufferBeginInfo beginInfo = { .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
					       .pNext = NULL,
					       .flags = 0,
					       .pInheritanceInfo = NULL };

	VkResult result = vkBeginCommandBuffer(commandBuffer, &beginInfo);
	if (result != VK_SUCCESS)
	{
		printf("Could not begin command buffer\n");
		abort();
	}

	VkClearValue clearValues[] = {
		{ .color = { 0.9f, 0.25f, 0.6f, 1.0f } },
		{ 1.0f, 0.0f }
	};

	VkRenderPassBeginInfo renderPassBeginInfo = { .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
						      .pNext = NULL,
						      .renderPass = vulkanRenderPass,
						      .framebuffer = swapChainFramebuffers[imageIndex],
						      .renderArea = { .extent = swapChainExtent,
								      .offset = { .x = 0, .y = 0 } },
						      .clearValueCount = 2,
						      .pClearValues = clearValues };

	vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vulkanGraphicsPipeline);

	VkBuffer vertexBuffers[] = { vertexBuffer };
	VkDeviceSize offsets[] = { 0 };
	vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
	vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT16);
	/*
	 * if dynamic viewport and scissor state then we need to set these before vkCmdDraw*/
	VkViewport viewport = { .x = 0.0f,
				.y = 0.0f,
				.width = (float)swapChainExtent.width,
				.height = (float)swapChainExtent.height,
				.minDepth = 0.0f,
				.maxDepth = 1.0f };
	vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

	VkRect2D scissor = { .offset = { .x = 0, .y = 0 }, .extent = swapChainExtent };
	vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vulkanPipelineLayout, 0, 1,
				&descriptorSets[currentFrame], 0, NULL);

	vkCmdDrawIndexed(commandBuffer, 12, 1, 0, 0, 0);
	vkCmdEndRenderPass(commandBuffer);
	VkResult endCommandBufferResult = vkEndCommandBuffer(commandBuffer);
	if (endCommandBufferResult != VK_SUCCESS)
	{
		printf("Failed to endcommandbuffer\n");
		abort();
	}
}

static void CleanupSwapChain()
{
	vkDestroyImageView(vulkanDevice, depthImageView, NULL);
	vkDestroyImage(vulkanDevice, depthImage, NULL);
	vkFreeMemory(vulkanDevice, depthImageMemory, NULL);

	for (uint32_t i = 0; i < swapChainImageCount; ++i)
	{
		VkFramebuffer vkFramebuffer = swapChainFramebuffers[i];
		vkDestroyFramebuffer(vulkanDevice, vkFramebuffer, NULL);

		VkImageView swapChainImageView = swapChainImageViews[i];
		vkDestroyImageView(vulkanDevice, swapChainImageView, NULL);
	}

	vkDestroySwapchainKHR(vulkanDevice, vulkanSwapChain, NULL);
}

static void UpdateUniformBuffer(uint32_t currentImage)
{
	double currentTime = Timer_Now();

	struct UniformBufferObject ubo;
	glm_mat4_identity(ubo.model);
	glm_mat4_identity(ubo.view);
	glm_mat4_identity(ubo.proj);

	vec3 axis = { 0.0f, 0.0f, 1.0f };
	glm_rotate(ubo.model, (float)currentTime * glm_rad(90.0f), axis);

	vec3 eye = { 0.0f, 2.0f, 2.0f };
	vec3 center = { 0.0f, 0.0f, 0.0f };
	vec3 up = { 0.0f, 1.0f, 0.0f };
	glm_lookat_rh_zo(eye, center, up, ubo.view);

	glm_perspective_rh_zo(glm_rad(45.0f), (float)swapChainExtent.width / (float)swapChainExtent.height, 0.1f,
			      100.0f, ubo.proj);

	void *data;
	vkMapMemory(vulkanDevice, uniformBuffersMemory[currentImage], 0, sizeof(ubo), 0, &data);
	memcpy(data, &ubo, sizeof(ubo));
	vkUnmapMemory(vulkanDevice, uniformBuffersMemory[currentImage]);
}

void DrawFrame()
{
	vkWaitForFences(vulkanDevice, 1, &inFlightFence[currentFrame], VK_TRUE, UINT64_MAX);

	uint32_t imageIndex;
	VkResult acquireResult = vkAcquireNextImageKHR(vulkanDevice, vulkanSwapChain, UINT64_MAX,
						       imageAvailableSemaphore[currentFrame], NULL, &imageIndex);

	if (acquireResult == VK_ERROR_OUT_OF_DATE_KHR || acquireResult == VK_SUBOPTIMAL_KHR)
	{
		RecreateSwapChain();
		return;
	}
	else if (acquireResult != VK_SUCCESS)
	{
		printf("Could not queue presentqueue\n");
		abort();
	}

	vkResetFences(vulkanDevice, 1, &inFlightFence[currentFrame]);

	vkResetCommandBuffer(vulkanCommandBuffers[currentFrame], 0);
	RecordCommandBuffer(vulkanCommandBuffers[currentFrame], imageIndex);

	UpdateUniformBuffer(currentFrame);

	VkSemaphore waitSemaphores[] = { imageAvailableSemaphore[currentFrame] };
	VkSemaphore signalSemaphores[] = { renderFinishedSemaphore[currentFrame] };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

	VkSubmitInfo submitInfo = { .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
				    .pNext = NULL,
				    .waitSemaphoreCount = 1,
				    .pWaitSemaphores = waitSemaphores,
				    .pWaitDstStageMask = waitStages,
				    .commandBufferCount = 1,
				    .pCommandBuffers = &vulkanCommandBuffers[currentFrame],
				    .signalSemaphoreCount = 1,
				    .pSignalSemaphores = signalSemaphores };

	VkResult result = vkQueueSubmit(vulkanGraphicsQueue, 1, &submitInfo, inFlightFence[currentFrame]);
	if (result != VK_SUCCESS)
	{
		printf("Could not submit command buffer to queue\n");
		abort();
	}

	VkSwapchainKHR swapChains[] = { vulkanSwapChain };

	VkPresentInfoKHR presentInfoKhr = { .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
					    .pNext = NULL,
					    .waitSemaphoreCount = 1,
					    .pWaitSemaphores = signalSemaphores,
					    .swapchainCount = 1,
					    .pSwapchains = swapChains,
					    .pImageIndices = &imageIndex,
					    .pResults = NULL };

	VkResult presentResult = vkQueuePresentKHR(vulkanPresentQueue, &presentInfoKhr);

	if (presentResult == VK_ERROR_OUT_OF_DATE_KHR || presentResult == VK_SUBOPTIMAL_KHR || framebufferResized)
	{
		framebufferResized = false;
		RecreateSwapChain();
	}
	else if (presentResult != VK_SUCCESS)
	{
		printf("Could not queue presentqueue\n");
		abort();
	}

	currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

static VkCommandBuffer BeginSingleTimeCommands()
{
	VkCommandBufferAllocateInfo allocateInfo = { .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
						     .pNext = NULL,
						     .commandPool = vulkanCommandPool,
						     .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
						     .commandBufferCount = 1 };

	VkCommandBuffer commandBuffer;
	VkResult allocateCommandBufferResult = vkAllocateCommandBuffers(vulkanDevice, &allocateInfo, &commandBuffer);
	if (allocateCommandBufferResult != VK_SUCCESS)
	{
		printf("Could not allocate single time commandbuffer\n");
		abort();
	}

	VkCommandBufferBeginInfo beginInfo = { .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
					       .pNext = NULL,
					       .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT };

	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	return commandBuffer;
}

static void EndSingleTimeCommands(VkCommandBuffer commandBuffer)
{
	vkEndCommandBuffer(commandBuffer);

	VkSubmitInfo submitInfo = { .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
				    .pNext = NULL,
				    .waitSemaphoreCount = 0,
				    .pWaitSemaphores = NULL,
				    .pWaitDstStageMask = NULL,
				    .commandBufferCount = 1,
				    .pCommandBuffers = &commandBuffer,
				    .signalSemaphoreCount = 0,
				    .pSignalSemaphores = NULL };

	vkQueueSubmit(vulkanGraphicsQueue, 1, &submitInfo, NULL);
	vkQueueWaitIdle(vulkanGraphicsQueue);

	vkFreeCommandBuffers(vulkanDevice, vulkanCommandPool, 1, &commandBuffer);
}

static uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
	VkPhysicalDeviceMemoryProperties memoryProperties;
	vkGetPhysicalDeviceMemoryProperties(vulkanPhysicalDevice, &memoryProperties);

	for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; ++i)
	{
		if ((typeFilter & (1 << i)) &&
		    (memoryProperties.memoryTypes[i].propertyFlags & properties) == properties)
		{
			return i;
		}
	}

	printf("Could not find a suitable memory type!\n");
	abort();
}

static void CreateBuffer(VkDeviceSize deviceSize, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags propertyFlags,
			 VkBuffer *buffer, VkDeviceMemory *deviceMemory)
{
	VkBufferCreateInfo bufferCreateInfo = { .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
						.pNext = NULL,
						.flags = 0,
						.size = deviceSize,
						.usage = usageFlags,
						.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
						.queueFamilyIndexCount = 0,
						.pQueueFamilyIndices = NULL };

	VkResult result = vkCreateBuffer(vulkanDevice, &bufferCreateInfo, NULL, buffer);
	if (result != VK_SUCCESS)
	{
		printf("Could not create VertexBuffer\n");
		abort();
	}

	printf("Created vertex buffer\n");

	VkMemoryRequirements memoryRequirements;
	vkGetBufferMemoryRequirements(vulkanDevice, *buffer, &memoryRequirements);

	VkMemoryAllocateInfo allocateInfo = { .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
					      .pNext = NULL,
					      .allocationSize = memoryRequirements.size,
					      .memoryTypeIndex = FindMemoryType(memoryRequirements.memoryTypeBits,
										propertyFlags) };

	VkResult allocateMemoryResult = vkAllocateMemory(vulkanDevice, &allocateInfo, NULL, deviceMemory);
	if (allocateMemoryResult != VK_SUCCESS)
	{
		printf("Could not allocate memory\n");
		abort();
	}

	vkBindBufferMemory(vulkanDevice, *buffer, *deviceMemory, 0);
}

static void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
	VkCommandBuffer commandBuffer = BeginSingleTimeCommands();

	VkBufferCopy copyRegion = { .srcOffset = 0, .dstOffset = 0, .size = size };
	vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

	EndSingleTimeCommands(commandBuffer);
}

static void CreateDescriptorSetLayout()
{
	VkDescriptorSetLayoutBinding uboLayoutBinding = { .binding = 0,
							  .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
							  .descriptorCount = 1,
							  .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
							  .pImmutableSamplers = NULL };

	VkDescriptorSetLayoutBinding samplerLayoutBinding = { .binding = 1,
							      .descriptorCount = 1,
							      .descriptorType =
								      VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
							      .pImmutableSamplers = NULL,
							      .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT };

	VkDescriptorSetLayoutBinding *bindings = malloc(2 * sizeof(VkDescriptorSetLayoutBinding));
	bindings[0] = uboLayoutBinding;
	bindings[1] = samplerLayoutBinding;

	VkDescriptorSetLayoutCreateInfo layoutCreateInfo = {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.bindingCount = 2,
		.pBindings = bindings
	};

	VkResult layoutCreateResult =
		vkCreateDescriptorSetLayout(vulkanDevice, &layoutCreateInfo, NULL, &descriptorSetLayout);
	if (layoutCreateResult != VK_SUCCESS)
	{
		printf("Could not create descriptor set layout\n");
		abort();
	}

	printf("Created descriptor set layout\n");

	free(bindings);
}

static void CreateIndexBuffer()
{
	VkDeviceSize bufferSize = sizeof(indices[0]) * 12;

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &stagingBuffer,
		     &stagingBufferMemory);

	void *data;
	vkMapMemory(vulkanDevice, stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, indices, (size_t)bufferSize);
	vkUnmapMemory(vulkanDevice, stagingBufferMemory);

	CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		     VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &indexBuffer, &indexBufferMemory);

	CopyBuffer(stagingBuffer, indexBuffer, bufferSize);

	vkDestroyBuffer(vulkanDevice, stagingBuffer, NULL);
	vkFreeMemory(vulkanDevice, stagingBufferMemory, NULL);
}

static void CreateVertexBuffer()
{
	VkDeviceSize bufferSize = sizeof(vertices[0]) * 8;

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &stagingBuffer,
		     &stagingBufferMemory);

	void *data;
	vkMapMemory(vulkanDevice, stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, vertices, (size_t)bufferSize);
	vkUnmapMemory(vulkanDevice, stagingBufferMemory);

	CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		     VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &vertexBuffer, &vertexBufferMemory);

	CopyBuffer(stagingBuffer, vertexBuffer, bufferSize);

	vkDestroyBuffer(vulkanDevice, stagingBuffer, NULL);
	vkFreeMemory(vulkanDevice, stagingBufferMemory, NULL);
}

static void CreateUniformBuffers()
{
	VkDeviceSize bufferSize = sizeof(struct UniformBufferObject);

	uniformBuffers = malloc(MAX_FRAMES_IN_FLIGHT * sizeof(VkBuffer));
	uniformBuffersMemory = malloc(MAX_FRAMES_IN_FLIGHT * sizeof(VkDeviceMemory));

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		CreateBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			     &uniformBuffers[i], &uniformBuffersMemory[i]);
	}
}

static void CreateDescriptorPool()
{
	VkDescriptorPoolSize *poolSizes = malloc(2 * sizeof(VkDescriptorPoolSize));
	poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSizes[0].descriptorCount = MAX_FRAMES_IN_FLIGHT;
	poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSizes[1].descriptorCount = MAX_FRAMES_IN_FLIGHT;

	VkDescriptorPoolCreateInfo poolInfo = {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.maxSets = MAX_FRAMES_IN_FLIGHT,
		.poolSizeCount = 2,
		.pPoolSizes = poolSizes,
	};

	VkResult result = vkCreateDescriptorPool(vulkanDevice, &poolInfo, NULL, &descriptorPool);
	if (result != VK_SUCCESS)
	{
		printf("Could not create descriptor pool\n");
		abort();
	}

	printf("Created descriptor pool\n");

	free(poolSizes);
}

static void CreateDescriptorSets()
{
	VkDescriptorSetLayout *layouts = malloc(MAX_FRAMES_IN_FLIGHT * sizeof(VkDescriptorSetLayout));
	layouts[0] = descriptorSetLayout;
	layouts[1] = descriptorSetLayout;
	VkDescriptorSetAllocateInfo allocateInfo = { .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
						     .pNext = NULL,
						     .descriptorPool = descriptorPool,
						     .descriptorSetCount = MAX_FRAMES_IN_FLIGHT,
						     .pSetLayouts = layouts };

	descriptorSets = malloc(MAX_FRAMES_IN_FLIGHT * sizeof(VkDescriptorSet));
	VkResult result = vkAllocateDescriptorSets(vulkanDevice, &allocateInfo, descriptorSets);
	if (result != VK_SUCCESS)
	{
		printf("Could not allocate descriptor sets\n");
		abort();
	}

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		VkDescriptorBufferInfo bufferInfo = { .buffer = uniformBuffers[i],
						      .offset = 0,
						      .range = sizeof(struct UniformBufferObject) };

		VkDescriptorImageInfo imageInfo = { .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
						    .imageView = textureImageView,
						    .sampler = textureSampler };

		VkWriteDescriptorSet *descriptorSetWrites = malloc(2 * sizeof(VkWriteDescriptorSet));
		descriptorSetWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorSetWrites[0].pNext = NULL;
		descriptorSetWrites[0].pTexelBufferView = NULL;
		descriptorSetWrites[0].dstSet = descriptorSets[i];
		descriptorSetWrites[0].dstBinding = 0;
		descriptorSetWrites[0].dstArrayElement = 0;
		descriptorSetWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorSetWrites[0].descriptorCount = 1;
		descriptorSetWrites[0].pBufferInfo = &bufferInfo;

		descriptorSetWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorSetWrites[1].pNext = NULL;
		descriptorSetWrites[1].pTexelBufferView = NULL;
		descriptorSetWrites[1].dstSet = descriptorSets[i];
		descriptorSetWrites[1].dstBinding = 1;
		descriptorSetWrites[1].dstArrayElement = 0;
		descriptorSetWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorSetWrites[1].descriptorCount = 1;
		descriptorSetWrites[1].pImageInfo = &imageInfo;

		/*VkWriteDescriptorSet descriptorWrite = { .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
							 .dstSet = descriptorSets[i],
							 .dstBinding = 0,
							 .dstArrayElement = 0,
							 .descriptorCount = 1,
							 .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
							 .pImageInfo = NULL,
							 .pBufferInfo = &bufferInfo,
							 .pTexelBufferView = NULL };*/

		vkUpdateDescriptorSets(vulkanDevice, 2, descriptorSetWrites, 0, NULL);

		free(descriptorSetWrites);
	}

	free(layouts);
}

static void GenerateMipmaps(VkImage image, VkFormat imageFormat, uint32_t texWidth, uint32_t texHeight, uint32_t mipLevel)
{
	VkFormatProperties formatProperties;
	vkGetPhysicalDeviceFormatProperties(vulkanPhysicalDevice, imageFormat, &formatProperties);

	if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
		printf("texture image format does not support linear blitting!\n");
		abort();
	}

	VkCommandBuffer commandBuffer = BeginSingleTimeCommands();

	VkImageMemoryBarrier imageMemoryBarrier = {
		.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
		.pNext = NULL,
		.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.subresourceRange = {
			.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
			.baseArrayLayer = 0,
			.layerCount = 1,
			.levelCount = 1
		},
		.image = image
	};

	int32_t mipWidth = (int32_t)texWidth;
	int32_t mipHeight = (int32_t)texHeight;

	for (uint32_t i = 1; i < mipLevel; ++i)
	{
		imageMemoryBarrier.subresourceRange.baseMipLevel = i - 1;
		imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

		vkCmdPipelineBarrier(commandBuffer,
				     VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
				     0, NULL,
				     0, NULL,
				     1, &imageMemoryBarrier);

		VkImageBlit imageBlit = {
			.srcSubresource = {
				.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
				.mipLevel = i - 1,
				.baseArrayLayer = 0,
				.layerCount = 1
			},
			.srcOffsets = {
				{ 0, 0, 0 },
				{ mipWidth, mipHeight, 1 }
			},
			.dstSubresource = {
				.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
				.mipLevel = i,
				.baseArrayLayer = 0,
				.layerCount = 1
			},
			.dstOffsets = {
				{ 0, 0, 0 },
				{ mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 }
			}
		};

		vkCmdBlitImage(commandBuffer,
			       image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			       image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			       1, &imageBlit,
			       VK_FILTER_LINEAR);

		imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		vkCmdPipelineBarrier(commandBuffer,
				     VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
				     0, NULL,
				     0, NULL,
				     1, &imageMemoryBarrier);

		if (mipWidth > 1)
		{
			mipWidth /= 2;
		}

		if (mipHeight > 1)
		{
			mipHeight /= 2;
		}
	}

	imageMemoryBarrier.subresourceRange.baseMipLevel = mipLevels - 1;
	imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

	vkCmdPipelineBarrier(commandBuffer,
			     VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
			     0, NULL,
			     0, NULL,
			     1, &imageMemoryBarrier);

	EndSingleTimeCommands(commandBuffer);
}

static void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
{
	VkCommandBuffer commandBuffer = BeginSingleTimeCommands();

	VkBufferImageCopy region = { .bufferOffset = 0,
				     .bufferRowLength = 0,
				     .bufferImageHeight = 0,
				     .imageSubresource = { .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
							   .mipLevel = 0,
							   .baseArrayLayer = 0,
							   .layerCount = 1 },
				     .imageOffset = { 0, 0, 0 },
				     .imageExtent = { width, height, 1 } };

	vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

	EndSingleTimeCommands(commandBuffer);
}

static void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevel)
{
	VkCommandBuffer commandBuffer = BeginSingleTimeCommands();

	VkImageMemoryBarrier memoryBarrier = { .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
					       .pNext = NULL,
					       .srcAccessMask = 0, //TODO
					       .dstAccessMask = 0, //TODO
					       .oldLayout = oldLayout,
					       .newLayout = newLayout,
					       .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
					       .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
					       .image = image,
					       .subresourceRange = { .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
								     .baseMipLevel = 0,
								     .levelCount = mipLevel,
								     .baseArrayLayer = 0,
								     .layerCount = 1 } };

	VkPipelineStageFlags sourceStage;
	VkPipelineStageFlags destinationStage;

	if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
	{
		memoryBarrier.srcAccessMask = 0;
		memoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
		 newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
	{
		memoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		memoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	else
	{
		printf("unsupported layout transition!\n");
		abort();
	}

	vkCmdPipelineBarrier(commandBuffer, sourceStage, destinationStage, 0, 0, NULL, 0, NULL, 1, &memoryBarrier);

	EndSingleTimeCommands(commandBuffer);
}

static void CreateImage(uint32_t width, uint32_t height, uint32_t mipLevel, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage,
			VkMemoryPropertyFlags properties, VkImage *image, VkDeviceMemory *imageMemory)
{
	VkImageCreateInfo imageCreateInfo = { .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
					      .pNext = NULL,
					      .flags = 0,
					      .imageType = VK_IMAGE_TYPE_2D,
					      .format = format,
					      .extent = { .width = width, .height = height, .depth = 1 },
					      .mipLevels = mipLevel,
					      .arrayLayers = 1,
					      .samples = VK_SAMPLE_COUNT_1_BIT,
					      .tiling = tiling,
					      .usage = usage,
					      .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
					      .queueFamilyIndexCount = 0,
					      .pQueueFamilyIndices = NULL,
					      .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED };

	VkResult imageCreateResult = vkCreateImage(vulkanDevice, &imageCreateInfo, NULL, image);
	if (imageCreateResult != VK_SUCCESS)
	{
		printf("Could not create VkImage for texture\n");
		abort();
	}

	VkMemoryRequirements memoryRequirements;
	vkGetImageMemoryRequirements(vulkanDevice, *image, &memoryRequirements);

	VkMemoryAllocateInfo allocateInfo = { .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
					      .pNext = NULL,
					      .allocationSize = memoryRequirements.size,
					      .memoryTypeIndex =
						      FindMemoryType(memoryRequirements.memoryTypeBits, properties) };

	VkResult allocateTextureMemoryResult = vkAllocateMemory(vulkanDevice, &allocateInfo, NULL, imageMemory);
	if (allocateTextureMemoryResult != VK_SUCCESS)
	{
		printf("Could not allocate memory for textureImageMemory\n");
		abort();
	}

	vkBindImageMemory(vulkanDevice, *image, *imageMemory, 0);
}

static void CreateTextureImage()
{
	struct AssetTexture *texture = GetTexture("texture1");
	if (texture == NULL)
	{
		printf("Could not find image\n");
		abort();
	}

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	CreateBuffer(texture->bufferSize,
		     VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		     &stagingBuffer,
		     &stagingBufferMemory);

	void *data;
	vkMapMemory(vulkanDevice, stagingBufferMemory, 0, texture->bufferSize, 0, &data);
	memcpy(data, texture->buffer, texture->bufferSize);
	vkUnmapMemory(vulkanDevice, stagingBufferMemory);

	CreateImage(texture->width,
		    texture->height,
		    texture->mipmapCount,
		    VK_FORMAT_R8G8B8A8_SRGB,
		    VK_IMAGE_TILING_OPTIMAL,
		    VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
		    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		    &textureImage,
		    &textureImageMemory);

	TransitionImageLayout(textureImage,
			      VK_FORMAT_R8G8B8A8_SRGB,
			      VK_IMAGE_LAYOUT_UNDEFINED,
			      VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			      texture->mipmapCount);

	VkCommandBuffer commandBuffer = BeginSingleTimeCommands();

	uint32_t regionCount = texture->mipmapCount + 1;
	VkBufferImageCopy *regions = malloc(regionCount * sizeof(VkBufferImageCopy));
	int w = texture->width;
	int h = texture->height;
	int offset = 0;
	for (uint32_t mipLevel = 0; mipLevel < regionCount; ++mipLevel)
	{
		VkBufferImageCopy region = {
			.bufferOffset = offset,
			.bufferRowLength = 0,
			.bufferImageHeight = 0,
			.imageSubresource = {
				.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
				.mipLevel = mipLevel,
				.baseArrayLayer = 0,
				.layerCount = 1
			},
			.imageOffset = { 0, 0, 0 },
			.imageExtent = { w, h, 1 }
		};

		regions[mipLevel] = region;

		offset += w * h * texture->channels;

		w /= 2;
		h /= 2;
	}

	vkCmdCopyBufferToImage(commandBuffer,
			       stagingBuffer,
			       textureImage,
			       VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			       regionCount,
			       regions);

	EndSingleTimeCommands(commandBuffer);

	mipLevels = texture->mipmapCount;

	//CopyBufferToImage(stagingBuffer, textureImage, texture->width, texture->height);

	//GenerateMipmaps(textureImage, VK_FORMAT_R8G8B8A8_SRGB, texWidth, texHeight, mipLevels);

	TransitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			      VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, mipLevels);

	vkDestroyBuffer(vulkanDevice, stagingBuffer, NULL);
	vkFreeMemory(vulkanDevice, stagingBufferMemory, NULL);

	free(regions);
}

static void CreateTextureImageView()
{
	VkImageViewCreateInfo viewInfo = { .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
					   .pNext = NULL,
					   .flags = 0,
					   .image = textureImage,
					   .viewType = VK_IMAGE_VIEW_TYPE_2D,
					   .format = VK_FORMAT_R8G8B8A8_SRGB,
					   .components = { .r = VK_COMPONENT_SWIZZLE_IDENTITY,
							   .g = VK_COMPONENT_SWIZZLE_IDENTITY,
							   .b = VK_COMPONENT_SWIZZLE_IDENTITY,
							   .a = VK_COMPONENT_SWIZZLE_IDENTITY },
					   .subresourceRange = { .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
								 .baseMipLevel = 0,
								 .levelCount = mipLevels,
								 .baseArrayLayer = 0,
								 .layerCount = 1 } };

	VkResult createImageViewResult = vkCreateImageView(vulkanDevice, &viewInfo, NULL, &textureImageView);
	if (createImageViewResult != VK_SUCCESS)
	{
		printf("Could not create texture image view\n");
		abort();
	}
}

static void CreateTextureSampler()
{
	VkPhysicalDeviceProperties properties;
	vkGetPhysicalDeviceProperties(vulkanPhysicalDevice, &properties);

	VkSamplerCreateInfo samplerInfo = { .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
					    .pNext = NULL,
					    .magFilter = VK_FILTER_LINEAR,
					    .minFilter = VK_FILTER_LINEAR,
					    .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
					    .addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
					    .addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
					    .addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
					    .mipLodBias = 0.0f,
					    .anisotropyEnable = VK_TRUE,
					    .maxAnisotropy = properties.limits.maxSamplerAnisotropy,
					    .compareEnable = VK_FALSE,
					    .compareOp = VK_COMPARE_OP_ALWAYS,
					    .minLod = 0.0f,
					    .maxLod = (float)mipLevels,
					    .borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
					    .unnormalizedCoordinates = VK_FALSE };

	VkResult createSamplerResult = vkCreateSampler(vulkanDevice, &samplerInfo, NULL, &textureSampler);
	if (createSamplerResult != VK_SUCCESS)
	{
		printf("Failed to create texture sampler\n");
		abort();
	}
}

static bool HasStencilComponent(VkFormat format)
{
	return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}

static void CreateDepthResources()
{
	VkFormat depthFormat = FindDepthFormat();

	CreateImage(swapChainExtent.width, swapChainExtent.height, 1, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
		    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &depthImage, &depthImageMemory);

	VkComponentMapping componentMapping = { .r = VK_COMPONENT_SWIZZLE_IDENTITY,
						.g = VK_COMPONENT_SWIZZLE_IDENTITY,
						.b = VK_COMPONENT_SWIZZLE_IDENTITY,
						.a = VK_COMPONENT_SWIZZLE_IDENTITY };

	VkImageSubresourceRange imageSubresourceRange = { .aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
							  .baseMipLevel = 0,
							  .levelCount = 1,
							  .baseArrayLayer = 0,
							  .layerCount = 1 };

	VkImage image = depthImage;
	VkImageViewCreateInfo createInfo = { .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
					     .pNext = NULL,
					     .flags = 0,
					     .image = image,
					     .viewType = VK_IMAGE_VIEW_TYPE_2D,
					     .format = depthFormat,
					     .components = componentMapping,
					     .subresourceRange = imageSubresourceRange };

	VkResult result = vkCreateImageView(vulkanDevice, &createInfo, NULL, &depthImageView);
	if (result != VK_SUCCESS)
	{
		printf("Could not create depth imageview\n");
		abort();
	}
}

void RecreateSwapChain()
{
	vkDeviceWaitIdle(vulkanDevice);

	CleanupSwapChain();

	CreateSwapChain();
	CreateImageViews();
	CreateDepthResources();
	CreateFramebuffers();
}

void CreateVulkanInstance(struct Window *window)
{
	assert(window != NULL);

	VkApplicationInfo applicationInfo = { .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
					      .pNext = NULL,
					      .pApplicationName = window->title,
					      .applicationVersion = 0,
					      .pEngineName = ENGINE_NAME,
					      .engineVersion = 0,
					      .apiVersion = VK_API_VERSION_1_0 };

	if (!CheckValidationLayerSupport())
	{
		printf("validation layers requested, but not available!\n");
		abort();
	}

	CreateInstanceCreateInfo(&applicationInfo);
	SetupDebugMessenger();
	CreateSurface();
	PickPhysicalDevice();
	CreateLogicalDevice();
	CreateSwapChain();
	CreateImageViews();
	CreateRenderPass();
	CreateDescriptorSetLayout();
	CreateGraphicsPipeline();
	CreateDepthResources();
	CreateFramebuffers();
	CreateCommandPool();
	CreateTextureImage();
	CreateTextureImageView();
	CreateTextureSampler();
	CreateVertexBuffer();
	CreateIndexBuffer();
	CreateUniformBuffers();
	CreateDescriptorPool();
	CreateDescriptorSets();
	CreateCommandBuffers();
	CreateSyncObjects();

	printf("------\n");
	printf("Created a vulkan instance and every necessary object required for drawing\n");
	printf("------\n");
}

void DestroyVulkan()
{
	vkDeviceWaitIdle(vulkanDevice);

	CleanupSwapChain();

	vkDestroySampler(vulkanDevice, textureSampler, NULL);
	vkDestroyImageView(vulkanDevice, textureImageView, NULL);
	vkDestroyImage(vulkanDevice, textureImage, NULL);
	vkFreeMemory(vulkanDevice, textureImageMemory, NULL);

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		vkDestroyBuffer(vulkanDevice, uniformBuffers[i], NULL);
		vkFreeMemory(vulkanDevice, uniformBuffersMemory[i], NULL);
	}

	free(uniformBuffers);
	free(uniformBuffersMemory);

	vkDestroyDescriptorPool(vulkanDevice, descriptorPool, NULL);
	vkDestroyDescriptorSetLayout(vulkanDevice, descriptorSetLayout, NULL);

	vkDestroyBuffer(vulkanDevice, vertexBuffer, NULL);
	vkFreeMemory(vulkanDevice, vertexBufferMemory, NULL);

	vkDestroyBuffer(vulkanDevice, indexBuffer, NULL);
	vkFreeMemory(vulkanDevice, indexBufferMemory, NULL);

	for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
	{
		vkWaitForFences(vulkanDevice, 1, &inFlightFence[i], VK_TRUE, UINT64_MAX);
		vkDestroyFence(vulkanDevice, inFlightFence[i], NULL);
		vkDestroySemaphore(vulkanDevice, imageAvailableSemaphore[i], NULL);
		vkDestroySemaphore(vulkanDevice, renderFinishedSemaphore[i], NULL);
	}

	vkDestroyCommandPool(vulkanDevice, vulkanCommandPool, NULL);

	vkDestroyPipeline(vulkanDevice, vulkanGraphicsPipeline, NULL);
	vkDestroyRenderPass(vulkanDevice, vulkanRenderPass, NULL);
	vkDestroyPipelineLayout(vulkanDevice, vulkanPipelineLayout, NULL);

	vkDestroyDevice(vulkanDevice, NULL);

	if (enableValidationLayers)
	{
		DestroyDebugUtilsMessengerEXT(vulkanInstance, debugMessenger, NULL);
	}

	vkDestroySurfaceKHR(vulkanInstance, vulkanSurface, NULL);
	vkDestroyInstance(vulkanInstance, NULL);

	free(swapChainImageViews);
	free(swapChainImages);
	free(swapChainFramebuffers);

	free(vulkanCommandBuffers);
	free(inFlightFence);
	free(imageAvailableSemaphore);
	free(renderFinishedSemaphore);

	printf("Finished destroying vulkan objects and malloced support objects\n");
}