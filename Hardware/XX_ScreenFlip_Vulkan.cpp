#include "WIN32_COMPAT.H"
#include "MFC_stub.h"

#include <vulkan/vulkan.h>
//#include <vulkan/vulkan_xlib.h>

#include "XX_ScreenFlip_Vulkan.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include <SDL_ttf.h> //RERUN
#include "WIN3D.H"
#include <cassert>
#include <cstring>
#include <fstream>
#include <vector>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <iostream>
#include <memory>    // for std::unique_ptr and std::make_unique

extern SWord TOPLINE_Y;
extern SWord TOPLINE_Y2;
extern SWord TOPLINE_Y3;
extern SWord TOPLINE_YY;


// Vulkan objects for sampling the software framebuffer (file scope)
VkImageView      fbImageView = VK_NULL_HANDLE;
VkDescriptorPool fbDescPool  = VK_NULL_HANDLE;
VkDescriptorSet  fbDescSet   = VK_NULL_HANDLE;

// Mark whether the staging memory is HOST_COHERENT (no flush needed).
bool stagingMemoryCoherent = false;

// Helpers
#define VK_CHECK(x)                                                     \
    do {                                                                \
        VkResult err = x;                                               \
        if (err) {                                                      \
            std::cerr << "Detected Vulkan error: " << err << std::endl; \
            abort();                                                    \
        }                                                               \
    } while (0)


uint32_t direct_draw::FindMemory(uint32_t mask, VkMemoryPropertyFlags flags)
{
    VkPhysicalDeviceMemoryProperties mp;
    vkGetPhysicalDeviceMemoryProperties(vkPhys, &mp);

    for (uint32_t i = 0; i < mp.memoryTypeCount; i++)
        if ((mask & (1 << i)) && (mp.memoryTypes[i].propertyFlags & flags) == flags)
            return i;

    assert(false);
    return 0;
}

uint32_t* direct_draw::readSpirvRaw(const char* path, size_t* outBytes)
{
    FILE* f = std::fopen(path, "rb");
    if (!f) throw std::runtime_error(std::string("open failed: ") + path);

    std::fseek(f, 0, SEEK_END);
    long sz = std::ftell(f);
    if (sz <= 0) { std::fclose(f); throw std::runtime_error("empty or tell failed"); }
    std::fseek(f, 0, SEEK_SET);

    size_t byteSize = static_cast<size_t>(sz);
    if (byteSize % 4 != 0) { std::fclose(f); throw std::runtime_error("size % 4 != 0"); }

    uint32_t* buf = static_cast<uint32_t*>(std::malloc(byteSize));
    if (!buf) { std::fclose(f); throw std::runtime_error("malloc failed"); }

    size_t n = std::fread(buf, 1, byteSize, f);
    std::fclose(f);
    if (n != byteSize) { std::free(buf); throw std::runtime_error("partial read"); }

    if (buf[0] != 0x07230203u) { std::free(buf); throw std::runtime_error("bad magic"); }

    *outBytes = byteSize;
    return buf;
}

VkShaderModule direct_draw::loadShader(VkDevice device, const char* path)
{
    size_t byteSize = 0;
    uint32_t* code = readSpirvRaw(path, &byteSize);

    VkShaderModuleCreateInfo ci{};
    ci.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    ci.codeSize = byteSize;
    ci.pCode = code;

    VkShaderModule mod = VK_NULL_HANDLE;
    VkResult res = vkCreateShaderModule(device, &ci, nullptr, &mod);
    std::free(code);

    if (res != VK_SUCCESS) throw std::runtime_error(std::string("vkCreateShaderModule failed: ") + path);
    return mod;
}

// Vulkan_Init (called once after SDL window creation)
//bool direct_draw::Vulkan_Init(SDL_Window* theWin)
bool direct_draw::Vulkan_Init(HWND hWnd)
{
    auto* backend = backend_from_hwnd(hWnd);
    SDL_Window* theWin = backend->window;
    // --- Enumerate all available instance extensions ---
    uint32_t availExtCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &availExtCount, nullptr);
    std::vector<VkExtensionProperties> availExts(availExtCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &availExtCount, availExts.data());

    printf("Available Vulkan instance extensions:\n");
    for (const auto& e : availExts) {
        printf("  %s (spec version %u)\n", e.extensionName, e.specVersion);
    }

    // --- Enumerate all available validation layers ---
    uint32_t availLayerCount = 0;
    vkEnumerateInstanceLayerProperties(&availLayerCount, nullptr);
    std::vector<VkLayerProperties> availLayers(availLayerCount);
    vkEnumerateInstanceLayerProperties(&availLayerCount, availLayers.data());

    printf("Available Vulkan validation layers:\n");
    for (const auto& l : availLayers) {
        printf("  %s (spec version %u)\n", l.layerName, l.specVersion);
    }

    // --- Query required extensions from SDL ---
    std::cerr << "[VK] theWin=" << (void*)theWin << " hWnd=" << (void*)hWnd
              << " backend=" << (void*)backend << std::endl;
    uint32_t ext_count = 0;
    if (!SDL_Vulkan_GetInstanceExtensions(theWin, &ext_count, nullptr)) {
        std::cerr << "[VK] count query failed: " << SDL_GetError() << std::endl;
    }

    std::vector<const char*> extensions(ext_count);
    if (!SDL_Vulkan_GetInstanceExtensions(theWin, &ext_count, extensions.data())) {
        std::cerr << "Failed to get Vulkan instance extensions: " << SDL_GetError() << std::endl;
        return false;
    }

    // --- Validation layers (only if available) ---
    std::vector<const char*> layers;
#ifndef NDEBUG
    for (const auto& l : availLayers) {
        if (strcmp(l.layerName, "VK_LAYER_KHRONOS_validation") == 0) {
            layers.push_back("VK_LAYER_KHRONOS_validation");
            break;
        }
    }
#endif

    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Mig Alley Vulkan";
    appInfo.applicationVersion = VK_MAKE_VERSION(1,0,0);
    appInfo.pEngineName = "DirectDrawFacade";
    appInfo.engineVersion = VK_MAKE_VERSION(1,0,0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo ici{};
    ici.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    ici.pApplicationInfo = &appInfo;
    ici.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    ici.ppEnabledExtensionNames = extensions.data();
    ici.enabledLayerCount = static_cast<uint32_t>(layers.size());
    ici.ppEnabledLayerNames = layers.data();

    // --- Handle portability extension ---
    for (auto ext : extensions) {
        if (strcmp(ext, VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME) == 0) {
            ici.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
            break;
        }
    }
    
    VkResult res = vkCreateInstance(&ici, nullptr, &vkInstance);
    if (res != VK_SUCCESS) {
        printf("vkCreateInstance failed with code %d\n", res);
        throw std::runtime_error("Failed to create Vulkan instance");
    }

    /* Surface from SDL */
    // --- Create surface from SDL window ---
    if (!SDL_Vulkan_CreateSurface(theWin, vkInstance, &vkSurface)) {
        throw std::runtime_error("SDL_Vulkan_CreateSurface failed");
    }

    // STEP 1 FINISHED HERE.

    // STEP 2. Select a suitable physical device and graphics queue family.
    //         Stores results in vkPhys and vkQueueFamily.
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(vkInstance, &deviceCount, nullptr);
    if (deviceCount == 0) {
        throw std::runtime_error("No Vulkan physical devices found");
    }
    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(vkInstance, &deviceCount, devices.data());

    // Choose the first device that supports graphics + present
    for (const auto& dev : devices) {
        VkPhysicalDeviceProperties props;
        vkGetPhysicalDeviceProperties(dev, &props);

        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(dev, &queueFamilyCount, nullptr);
        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(dev, &queueFamilyCount, queueFamilies.data());

        int graphicsIndex = -1;
        int presentIndex = -1;

        for (uint32_t i = 0; i < queueFamilyCount; ++i) {
            if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                graphicsIndex = i;
            }
            VkBool32 presentSupport = VK_FALSE;
            vkGetPhysicalDeviceSurfaceSupportKHR(dev, i, vkSurface, &presentSupport);
            if (presentSupport) {
                presentIndex = i;
            }
        }

        if (graphicsIndex != -1 && presentIndex != -1) {
            vkPhys = dev;
            vkQueueFamily = graphicsIndex;
            // If graphics != present, you may need to handle separate queues.
            break;
        }
    }

    // STEP 2 FINISHED HERE.

    // STEP 3. Create logical device and retrieve graphics queue.
    //         Stores results in vkDevice and vkQueue.

    // Queue priority (1.0 = highest)
    float prio = 1.0f;
    VkDeviceQueueCreateInfo qci{};
    qci.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    qci.queueFamilyIndex = vkQueueFamily;
    qci.queueCount = 1;
    qci.pQueuePriorities = &prio;

    // Device features (enable what you need)
    VkPhysicalDeviceFeatures deviceFeatures{};
    // For now, leave defaults (all VK_FALSE)

    // Required extensions: swapchain
    const char* deviceExtensions[] = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

    VkDeviceCreateInfo dci{};
    dci.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    dci.queueCreateInfoCount = 1;
    dci.pQueueCreateInfos = &qci;
    dci.pEnabledFeatures = &deviceFeatures;
    dci.enabledExtensionCount = 1;
    dci.ppEnabledExtensionNames = deviceExtensions;

#ifndef NDEBUG
    // Validation layers (deprecated on device, but harmless)
    const char* validationLayers[] = { "VK_LAYER_KHRONOS_validation" };
    dci.enabledLayerCount = 1;
    dci.ppEnabledLayerNames = validationLayers;
#else
    dci.enabledLayerCount = 0;
#endif

    if (vkCreateDevice(vkPhys, &dci, nullptr, &vkDevice) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create logical device");
    }
    vkGetDeviceQueue(vkDevice, vkQueueFamily, 0, &vkQueue);

    // STEP 3 FINISHED HERE.

    // STEP 4. Create command pool, allocate command buffer, and set up sync primitives.
    //         
    VkCommandPoolCreateInfo cp{};
    cp.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    cp.queueFamilyIndex = vkQueueFamily;
    cp.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    // allows resetting individual command buffers
    if (vkCreateCommandPool(vkDevice, &cp, nullptr, &cmdPool) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create command pool");
    }

    /* command buffers */
    VkCommandBufferAllocateInfo ca{};
    ca.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    ca.commandPool = cmdPool;
    ca.commandBufferCount = 1;
    ca.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    if (vkAllocateCommandBuffers(vkDevice, &ca, &cmd) != VK_SUCCESS) {
        throw std::runtime_error("Failed to allocate command buffer");
    }

    /* Sync */
    VkSemaphoreCreateInfo si{};
    si.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    if (vkCreateSemaphore(vkDevice, &si, nullptr, &semAcquire) != VK_SUCCESS ||
        vkCreateSemaphore(vkDevice, &si, nullptr, &semPresent) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create semaphores");
    }

    /* Fence */
    VkFenceCreateInfo fi{};
    fi.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fi.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    // start signaled so first frame doesn’t block
    if (vkCreateFence(vkDevice, &fi, nullptr, &fence) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create fence");
    }

    // STEP 4 FINISHED HERE.

    // STEP 5. Create or resize the swapchain for the given window size.
    // Clean up old resources
    for (auto fb : framebuffers) {
        if (fb != VK_NULL_HANDLE) vkDestroyFramebuffer(vkDevice, fb, nullptr);
    }
    framebuffers.clear();

    for (auto view : swapImageViews) {
        if (view != VK_NULL_HANDLE) vkDestroyImageView(vkDevice, view, nullptr);
    }
    swapImageViews.clear();

    if (vkSwapchain != VK_NULL_HANDLE) {
        vkDestroySwapchainKHR(vkDevice, vkSwapchain, nullptr);
        vkSwapchain = VK_NULL_HANDLE;
    }

    /* Swapchain */
    SDL_GetWindowSize(theWin, &g_winWidth, &g_winHeight);
    VkSurfaceCapabilitiesKHR caps;
    VK_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vkPhys, vkSurface, &caps));

    uint32_t formatCount = 0;
    VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(vkPhys, vkSurface, &formatCount, nullptr));
    std::vector<VkSurfaceFormatKHR> formats(formatCount);
    VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(vkPhys, vkSurface, &formatCount, formats.data()));

    uint32_t presentModeCount = 0;
    VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(vkPhys, vkSurface, &presentModeCount, nullptr));
    std::vector<VkPresentModeKHR> presentModes(presentModeCount);
    VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(vkPhys, vkSurface, &presentModeCount, presentModes.data()));
    
    // Choose format (prefer SRGB BGRA8)
    VkSurfaceFormatKHR chosenFormat = formats[0];
    for (const auto& f : formats) {
        if (f.format == VK_FORMAT_B8G8R8A8_SRGB &&
            f.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            chosenFormat = f;
            break;
        }
    }

    // Choose present mode (prefer MAILBOX, fallback FIFO)
    VkPresentModeKHR chosenPresentMode = VK_PRESENT_MODE_FIFO_KHR;
    for (auto pm : presentModes) {
        if (pm == VK_PRESENT_MODE_MAILBOX_KHR) {
            chosenPresentMode = pm;
            break;
        }
    }

    // Choose extent
    VkExtent2D extent{};
    if (caps.currentExtent.width != UINT32_MAX) {
        extent = caps.currentExtent;
    } else {
        extent.width  = std::clamp<uint32_t>(g_winWidth,  caps.minImageExtent.width,  caps.maxImageExtent.width);
        extent.height = std::clamp<uint32_t>(g_winHeight, caps.minImageExtent.height, caps.maxImageExtent.height);
    }

    // Image count
    uint32_t imageCount = caps.minImageCount + 1;
    if (caps.maxImageCount > 0 && imageCount > caps.maxImageCount) {
        imageCount = caps.maxImageCount;
    }
    
    vkSwapFormat = chosenFormat.format;
    vkExtent     = extent;

    const char* fmtName = "UNKNOWN";
    if (vkSwapFormat == VK_FORMAT_B8G8R8A8_UNORM) fmtName = "B8G8R8A8_UNORM";
    else if (vkSwapFormat == VK_FORMAT_B8G8R8A8_SRGB) fmtName = "B8G8R8A8_SRGB";
    else if (vkSwapFormat == VK_FORMAT_R8G8B8A8_UNORM) fmtName = "R8G8B8A8_UNORM";
    else if (vkSwapFormat == VK_FORMAT_R5G6B5_UNORM_PACK16) fmtName = "R5G6B5_UNORM_PACK16";
    printf("Chosen swap format = %s (%d), extent = %u x %u\n", fmtName, (int)vkSwapFormat, vkExtent.width, vkExtent.height);

    VkSwapchainCreateInfoKHR sci{};
    sci.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    sci.surface = vkSurface;
    sci.minImageCount = imageCount;
    sci.imageFormat = vkSwapFormat;
    sci.imageColorSpace = chosenFormat.colorSpace;
    sci.imageExtent = vkExtent;
    sci.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    sci.imageArrayLayers = 1;
    sci.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;   // REQUIRED
    sci.queueFamilyIndexCount = 0;
    sci.pQueueFamilyIndices   = nullptr;
    sci.preTransform = caps.currentTransform;
    sci.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR; // REQUIRED
    sci.presentMode = chosenPresentMode;
    sci.clipped = VK_TRUE; // REQUIRED
    sci.oldSwapchain = VK_NULL_HANDLE;

    VK_CHECK(vkCreateSwapchainKHR(vkDevice, &sci, nullptr, &vkSwapchain));

    // Retrieve images
    swapCount = imageCount;
    VK_CHECK(vkGetSwapchainImagesKHR(vkDevice, vkSwapchain, &swapCount, nullptr));
    swapImages = new VkImage[swapCount];
    swapViews  = new VkImageView[swapCount];
    VK_CHECK(vkGetSwapchainImagesKHR(vkDevice, vkSwapchain, &swapCount, swapImages));

    for (uint32_t i = 0; i < swapCount; i++)
    {
        VkImageViewCreateInfo iv{};
        iv.sType    = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        iv.image = swapImages[i];
        iv.viewType = VK_IMAGE_VIEW_TYPE_2D;
        iv.format = vkSwapFormat;
        iv.components = {
            VK_COMPONENT_SWIZZLE_R,
            VK_COMPONENT_SWIZZLE_G,
            VK_COMPONENT_SWIZZLE_B,
            VK_COMPONENT_SWIZZLE_A
         };
        iv.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        iv.subresourceRange.baseMipLevel   = 0;
        iv.subresourceRange.levelCount     = 1;
        iv.subresourceRange.baseArrayLayer = 0;
        iv.subresourceRange.layerCount     = 1;
        VK_CHECK(vkCreateImageView(vkDevice, &iv, nullptr, &swapViews[i]));
    }
    // STEP 5 FINISHED HERE.

    // STEP 6. Create a staging buffer for uploading the software framebuffer.
    // Create or resize the RGB565 back image, Stores results in fbImage and fbMemory.
    {
        // Clean up old image if it exists
        if (fbImage != VK_NULL_HANDLE) {
            vkDestroyImage(vkDevice, fbImage, nullptr);
            fbImage = VK_NULL_HANDLE;
        }
        if (fbMemory != VK_NULL_HANDLE) {
            vkFreeMemory(vkDevice, fbMemory, nullptr);
            fbMemory = VK_NULL_HANDLE;
        }

        // First create image
        VkImageCreateInfo imageInfo{};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        // use vkExtent (actual swapchain drawable size) so image/staging sizes match the swapchain
        imageInfo.extent.width  = vkExtent.width;
        imageInfo.extent.height = vkExtent.height;
        imageInfo.extent.depth  = 1;
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        // Match the swapchain/staging format so buffer->image copy and sampling are coherent.
        imageInfo.format = vkSwapFormat;
        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        if (vkCreateImage(vkDevice, &imageInfo, nullptr, &fbImage) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create back image (RGB565)");
        }

        // Second allocate memory
        VkMemoryRequirements memReq;
        vkGetImageMemoryRequirements(vkDevice, fbImage, &memReq);

        VkPhysicalDeviceMemoryProperties memProps;
        vkGetPhysicalDeviceMemoryProperties(vkPhys, &memProps);

        uint32_t memTypeIndex = UINT32_MAX;
        for (uint32_t i = 0; i < memProps.memoryTypeCount; ++i) {
            if ((memReq.memoryTypeBits & (1 << i)) &&
                (memProps.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)) {
                memTypeIndex = i;
                break;
            }
        }
        if (memTypeIndex == UINT32_MAX) {
            throw std::runtime_error("Failed to find suitable memory type for back image");
        }

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memReq.size;
        allocInfo.memoryTypeIndex = memTypeIndex;

        if (vkAllocateMemory(vkDevice, &allocInfo, nullptr, &fbMemory) != VK_SUCCESS) {
            throw std::runtime_error("Failed to allocate back image memory");
        }

        // Bind memory
        vkBindImageMemory(vkDevice, fbImage, fbMemory, 0);

        // create image view for sampling the software framebuffer
        VkImageViewCreateInfo iv{};
        iv.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        iv.image = fbImage;
        iv.viewType = VK_IMAGE_VIEW_TYPE_2D;
        iv.format = imageInfo.format;

        // Use identity mapping — swapchain image format and staging buffer already match (BGRA memory for B8G8R8A8)
        iv.components = {
            VK_COMPONENT_SWIZZLE_R,
            VK_COMPONENT_SWIZZLE_G,
            VK_COMPONENT_SWIZZLE_B,
            VK_COMPONENT_SWIZZLE_A
        };
        iv.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        iv.subresourceRange.baseMipLevel = 0;
        iv.subresourceRange.levelCount = 1;
        iv.subresourceRange.baseArrayLayer = 0;
        iv.subresourceRange.layerCount = 1;
        VK_CHECK(vkCreateImageView(vkDevice, &iv, nullptr, &fbImageView));
        printf("fbImageView components swizzle r=%d g=%d b=%d a=%d\n",
               (int)iv.components.r, (int)iv.components.g, (int)iv.components.b, (int)iv.components.a);
    }

    // Create or resize the staging buffer used as the legacy back surface.
    // Stores results in stagingBuf, stagingMem, ctx.stagingPtr.
    
    // Clean up old buffer if it exists
    if (stagingBuf != VK_NULL_HANDLE) {
        vkDestroyBuffer(vkDevice, stagingBuf, nullptr);
        stagingBuf = VK_NULL_HANDLE;
    }
    if (stagingMem != VK_NULL_HANDLE) {
        vkFreeMemory(vkDevice, stagingMem, nullptr);
        stagingMem = VK_NULL_HANDLE;
    }
    stagingPtr = nullptr;

    /* Staging buffer creation */
    // Choose bytes-per-pixel to match the swapchain image format
    uint32_t bytesPerPixel = 4; // default to 4bpp
    if (vkSwapFormat == VK_FORMAT_R5G6B5_UNORM_PACK16) {
        bytesPerPixel = 2;
    } else if (vkSwapFormat == VK_FORMAT_B8G8R8A8_UNORM ||
               vkSwapFormat == VK_FORMAT_B8G8R8A8_SRGB ||
               vkSwapFormat == VK_FORMAT_R8G8B8A8_UNORM ||
               vkSwapFormat == VK_FORMAT_R8G8B8A8_SRGB) {
        bytesPerPixel = 4;
    }
    // allocate staging sized to the drawable (vkExtent) not logical window size
    VkDeviceSize size = static_cast<VkDeviceSize>(vkExtent.width) * vkExtent.height * bytesPerPixel;
    VkBufferCreateInfo bc{};
    bc.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bc.size = size;
    bc.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;  // staging → image
    bc.sharingMode = VK_SHARING_MODE_EXCLUSIVE; // REQUIRED

    if (vkCreateBuffer(vkDevice, &bc, nullptr, &stagingBuf) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create staging buffer");
    }

    // Now allocate memory for the staging buffer and map it.
    VkMemoryRequirements mr;
    vkGetBufferMemoryRequirements(vkDevice, stagingBuf, &mr);
    
    VkPhysicalDeviceMemoryProperties memProps;
    vkGetPhysicalDeviceMemoryProperties(vkPhys, &memProps);

    uint32_t memTypeIndex = UINT32_MAX;
    for (uint32_t i = 0; i < memProps.memoryTypeCount; ++i) {
        if ((mr.memoryTypeBits & (1 << i)) &&
            (memProps.memoryTypes[i].propertyFlags &
             (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT))) {
            memTypeIndex = i;
            break;
        }
    }
    if (memTypeIndex == UINT32_MAX) {
        throw std::runtime_error("Failed to find suitable memory type for staging buffer");
    }

    // remember whether the chosen memory type is coherent (avoids flush)
    stagingMemoryCoherent = ( (memProps.memoryTypes[memTypeIndex].propertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) != 0 );

    VkMemoryAllocateInfo ma{};
    ma.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    ma.allocationSize = mr.size;
    ma.memoryTypeIndex = memTypeIndex;
    
    if (vkAllocateMemory(vkDevice, &ma, nullptr, &stagingMem) != VK_SUCCESS) {
        throw std::runtime_error("Failed to allocate staging buffer memory");
    }

    vkBindBufferMemory(vkDevice, stagingBuf, stagingMem, 0);

    // Map memory
    if (vkMapMemory(vkDevice, stagingMem, 0, size, 0, &stagingPtr) != VK_SUCCESS) {
        throw std::runtime_error("Failed to map staging buffer memory");
    }

    return true;
}

SDL_bool direct_draw::Vulkan_CreateSDLSurface(SDL_Window *theWin)
{
    return SDL_Vulkan_CreateSurface(theWin, vkInstance, &vkSurface);
}


// Helper: Safely free and null an SDL_Surface pointer
static void SafeFreeSurface(SDL_Surface*& surf) {
    if (surf) { SDL_FreeSurface(surf); surf = nullptr; }
}

// Helper: Blit a text surface into the staging buffer (convert to BGRA8888 if needed)
static void blitSurfaceToStaging(SDL_Surface* src, int dstX, int dstY, uint8_t* mapped, float scaleX, float scaleY, VkExtent2D vkExtent) {
    if (!src) return;
    int sx = static_cast<int>(dstX * scaleX + 0.5f);
    int sy = static_cast<int>(dstY * scaleY + 0.5f);
    const uint32_t convTarget = SDL_PIXELFORMAT_BGRA8888;
    const int convBpp = 4;
    SDL_Surface* conv = (src->format->format == convTarget) ? src : SDL_ConvertSurfaceFormat(src, convTarget, 0);
    if (!conv) return;
    const uint8_t* srcBase = reinterpret_cast<const uint8_t*>(conv->pixels);
    int srcPitch = conv->pitch;
    int copyW = std::min(conv->w, static_cast<int>(vkExtent.width) - sx);
    int copyH = std::min(conv->h, static_cast<int>(vkExtent.height) - sy);
    if (copyW <= 0 || copyH <= 0) { if (conv != src) SDL_FreeSurface(conv); return; }
    size_t dstRowBytes = static_cast<size_t>(vkExtent.width) * convBpp;
    for (int y = 0; y < copyH; ++y) {
        const uint8_t* srcRow = srcBase + y * srcPitch;
        uint8_t* dstRow = mapped + (size_t)(sy + y) * dstRowBytes + (size_t)sx * convBpp;
        std::memcpy(dstRow, srcRow, static_cast<size_t>(copyW) * convBpp);
    }
    if (conv != src) SDL_FreeSurface(conv);
}

void direct_draw::XX_ScreenFlip_Vulkan(SDL_Surface* ddsBack)
{
    vkWaitForFences(vkDevice, 1, &fence, VK_TRUE, UINT64_MAX);
    vkResetFences(vkDevice, 1, &fence);

    if (stagingPtr) {
        uint8_t* mapped = reinterpret_cast<uint8_t*>(stagingPtr);
        const float scaleX = (g_winWidth > 0) ? float(vkExtent.width) / float(g_winWidth) : 1.0f;
        const float scaleY = (g_winHeight > 0) ? float(vkExtent.height) / float(g_winHeight) : 1.0f;
        const int dstBpp = 4;
        const size_t dstRowBytes = static_cast<size_t>(vkExtent.width) * dstBpp;

        // --- Fast RGB565 → BGRA8888 conversion for framebuffer ---
        // TODO for better way: 
        // Create an offscreen 16‑bit VkImage (not presentable) and sample from it in 
        // a shader that writes to the presentable 32‑bit swapchain image.
        const uint16_t* srcBase = reinterpret_cast<const uint16_t*>(ddsBack->pixels);
        int srcPitch = ddsBack->pitch / 2; // pitch in pixels for 16bpp
        for (uint32_t y = 0; y < vkExtent.height; ++y) {
            const uint16_t* srcRow = srcBase + y * srcPitch;
            uint8_t* dstRow = mapped + y * dstRowBytes;
            for (uint32_t x = 0; x < vkExtent.width; ++x) {
                uint16_t pixel = srcRow[x];
                uint8_t r = ((pixel >> 11) & 0x1F) << 3;
                uint8_t g = ((pixel >> 5) & 0x3F) << 2;
                uint8_t b = (pixel & 0x1F) << 3;
                dstRow[x * 4 + 0] = b;
                dstRow[x * 4 + 1] = g;
                dstRow[x * 4 + 2] = r;
                dstRow[x * 4 + 3] = 255; // Opaque alpha
            }
        }

        // --- Flush if non-coherent ---
        if (!stagingMemoryCoherent) {
            VkMappedMemoryRange range{ VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE };
            range.memory = stagingMem;
            range.offset = 0;
            range.size = VK_WHOLE_SIZE;
            vkFlushMappedMemoryRanges(vkDevice, 1, &range);
        }
    } else {
        std::cerr << "Warning: stagingPtr is null, skipping frame upload\n";
    }

    // --- Vulkan present logic (unchanged) ---
    uint32_t img;
    vkAcquireNextImageKHR(vkDevice, vkSwapchain, UINT64_MAX, semAcquire, VK_NULL_HANDLE, &img);
    vkResetCommandBuffer(cmd, 0);
    VkCommandBufferBeginInfo bi{};
    bi.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    VK_CHECK(vkBeginCommandBuffer(cmd, &bi));


    // Copy staging buffer -> fbImage and make it shader-readable for the fragment shader.
    {
        VkImageMemoryBarrier toTransfer{ VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
        toTransfer.srcAccessMask = 0;
        toTransfer.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        toTransfer.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        toTransfer.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        toTransfer.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        toTransfer.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        toTransfer.image = fbImage;
        toTransfer.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        toTransfer.subresourceRange.baseMipLevel = 0;
        toTransfer.subresourceRange.levelCount = 1;
        toTransfer.subresourceRange.baseArrayLayer = 0;
        toTransfer.subresourceRange.layerCount = 1;

        vkCmdPipelineBarrier(
            cmd,
            VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            0,
            0, nullptr,
            0, nullptr,
            1, &toTransfer);

        VkBufferImageCopy copyRegion{};
        copyRegion.bufferOffset = 0;
        copyRegion.bufferRowLength = 0;
        copyRegion.bufferImageHeight = 0;
        copyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        copyRegion.imageSubresource.mipLevel = 0;
        copyRegion.imageSubresource.baseArrayLayer = 0;
        copyRegion.imageSubresource.layerCount = 1;
        copyRegion.imageOffset = { 0, 0, 0 };
        copyRegion.imageExtent = { vkExtent.width, vkExtent.height, 1 };

        vkCmdCopyBufferToImage(cmd, stagingBuf, fbImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);

        VkImageMemoryBarrier toShaderRead = toTransfer;
        toShaderRead.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        toShaderRead.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        toShaderRead.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        toShaderRead.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        vkCmdPipelineBarrier(
            cmd,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
            0,
            0, nullptr,
            0, nullptr,
            1, &toShaderRead);
    }

     // Render with the graphics pipeline (this actually runs the fragment shader).
     // Begin render pass on the acquired framebuffer and draw a full-screen triangle.
     {
         VkRenderPassBeginInfo rpbi{ VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
         rpbi.renderPass = renderPass;
         rpbi.framebuffer = framebuffers[img];
         rpbi.renderArea.offset = { 0, 0 };
         rpbi.renderArea.extent = vkExtent;
         VkClearValue clear{};
         clear.color = { {0.0f, 0.0f, 0.0f, 1.0f} };
         rpbi.clearValueCount = 1;
         rpbi.pClearValues = &clear;

         vkCmdBeginRenderPass(cmd, &rpbi, VK_SUBPASS_CONTENTS_INLINE);
         vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
         // bind the combined-image-sampler descriptor set (binding=0 in shader)
         vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &fbDescSet, 0, nullptr);
         // No vertex buffers: vertex shader should produce a full-screen triangle using gl_VertexIndex.
         vkCmdDraw(cmd, 3, 1, 0, 0);
         vkCmdEndRenderPass(cmd);
     }

    vkEndCommandBuffer(cmd);

    VkPipelineStageFlags wait = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    VkSubmitInfo si{ VK_STRUCTURE_TYPE_SUBMIT_INFO };
    si.waitSemaphoreCount = 1;
    si.pWaitSemaphores = &semAcquire;
    si.pWaitDstStageMask = &wait;
    si.commandBufferCount = 1;
    si.pCommandBuffers = &cmd;
    si.signalSemaphoreCount = 1;
    si.pSignalSemaphores = &semPresent;

    vkQueueSubmit(vkQueue, 1, &si, fence);

    VkPresentInfoKHR pi{ VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };
    pi.swapchainCount = 1;
    pi.pSwapchains = &vkSwapchain;
    pi.pImageIndices = &img;
    pi.waitSemaphoreCount = 1;
    pi.pWaitSemaphores = &semPresent;

    vkQueuePresentKHR(vkQueue, &pi);
}

bool direct_draw::CreateRenderPass()
{
    VkAttachmentDescription color{};
    color.format         = vkSwapFormat;
    color.samples        = VK_SAMPLE_COUNT_1_BIT;
    color.loadOp         = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
    color.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    // IMPORTANT: We start from PRESENT (not UNDEFINED) and end at PRESENT
    color.initialLayout  = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    color.finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorRef{};
    colorRef.attachment = 0;
    colorRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorRef;

    	// This dependency performs PRESENT -> COLOR transition on begin,
	// and COLOR -> PRESENT visibility on end
	VkSubpassDependency deps[2]{};

	// EXTERNAL -> subpass: make the presented image ready for color attachment writes
	deps[0].srcSubpass    = VK_SUBPASS_EXTERNAL;
	deps[0].dstSubpass    = 0;
	deps[0].srcStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	deps[0].srcAccessMask = 0;
	deps[0].dstStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	deps[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	// subpass -> EXTERNAL: make color writes visible for presentation
	deps[1].srcSubpass    = 0;
	deps[1].dstSubpass    = VK_SUBPASS_EXTERNAL;
	deps[1].srcStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	deps[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	deps[1].dstStageMask  = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	deps[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;

    VkRenderPassCreateInfo rpci{};
    rpci.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    rpci.attachmentCount = 1;
    rpci.pAttachments    = &color;
    rpci.subpassCount    = 1;
    rpci.pSubpasses      = &subpass;
    rpci.dependencyCount = 2;
    rpci.pDependencies   = deps;

    VK_CHECK(vkCreateRenderPass(vkDevice, &rpci, nullptr, &renderPass));

    return true;
}

bool direct_draw::CreateFramebuffers()
{
    framebuffers.resize(swapCount);

    for (uint32_t i = 0; i < swapCount; ++i) {
        VkImageView attachments[] = { swapViews[i] };

        VkFramebufferCreateInfo fci{};
        fci.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        fci.renderPass = renderPass;
        fci.attachmentCount = 1;
        fci.pAttachments = attachments;
        fci.width  = vkExtent.width;
        fci.height = vkExtent.height;
        fci.layers = 1;

        if (vkCreateFramebuffer(vkDevice, &fci, nullptr, &framebuffers[i]) != VK_SUCCESS)
            return false;
    }
    return true;
}


void direct_draw::createDescriptorSetLayoutAndSampler()
{
    // --- 1. Descriptor set layout: one combined image sampler at binding 0 ---
    VkDescriptorSetLayoutBinding samplerLayoutBinding{};
    samplerLayoutBinding.binding            = 0; // matches "layout(set=0,binding=0)" in shader
    samplerLayoutBinding.descriptorType     = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerLayoutBinding.descriptorCount    = 1;
    samplerLayoutBinding.stageFlags         = VK_SHADER_STAGE_FRAGMENT_BIT;
    samplerLayoutBinding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 1;
    layoutInfo.pBindings    = &samplerLayoutBinding;

    VkResult res = vkCreateDescriptorSetLayout(vkDevice, &layoutInfo, nullptr, &descLayout);
    if (res != VK_SUCCESS) {
        throw std::runtime_error("Failed to create descriptor set layout");
    }

    // --- 2. Sampler creation ---
    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType        = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter    = VK_FILTER_NEAREST; // or VK_FILTER_LINEAR if you want interpolation
    samplerInfo.minFilter    = VK_FILTER_NEAREST;
    samplerInfo.mipmapMode   = VK_SAMPLER_MIPMAP_MODE_NEAREST;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.mipLodBias   = 0.0f;
    samplerInfo.anisotropyEnable = VK_FALSE;
    samplerInfo.maxAnisotropy    = 1.0f;
    samplerInfo.compareEnable    = VK_FALSE;
    samplerInfo.compareOp        = VK_COMPARE_OP_ALWAYS;
    samplerInfo.minLod           = 0.0f;
    samplerInfo.maxLod           = 0.0f;
    samplerInfo.borderColor      = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;

    res = vkCreateSampler(vkDevice, &samplerInfo, nullptr, &fbSampler);
    if (res != VK_SUCCESS) {
        throw std::runtime_error("Failed to create sampler");
    }

    // descriptor pool + allocate one combined image sampler descriptor set
    VkDescriptorPoolSize poolSize{};
    poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSize.descriptorCount = 1;

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.maxSets = 1;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = &poolSize;
    res = vkCreateDescriptorPool(vkDevice, &poolInfo, nullptr, &fbDescPool);
    if (res != VK_SUCCESS) {
        throw std::runtime_error("Failed to create descriptor pool");
    }

    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = fbDescPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &descLayout;
    res = vkAllocateDescriptorSets(vkDevice, &allocInfo, &fbDescSet);
    if (res != VK_SUCCESS) {
        throw std::runtime_error("Failed to allocate descriptor set");
    }

    // update descriptor set with fbImageView + fbSampler
    VkDescriptorImageInfo di{};
    di.sampler = fbSampler;
    di.imageView = fbImageView;
    di.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VkWriteDescriptorSet w{};
    w.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    w.dstSet = fbDescSet;
    w.dstBinding = 0;
    w.dstArrayElement = 0;
    w.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    w.descriptorCount = 1;
    w.pImageInfo = &di;
    vkUpdateDescriptorSets(vkDevice, 1, &w, 0, nullptr);
}

void direct_draw::createPipelineLayout()
{
    // --- 1. Pipeline layout create info ---
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount         = 1;                         // one descriptor set layout
    pipelineLayoutInfo.pSetLayouts            = &descLayout; // pointer to your set layout
    pipelineLayoutInfo.pushConstantRangeCount = 0;                         // no push constants for now
    pipelineLayoutInfo.pPushConstantRanges    = nullptr;

    // --- 2. Create pipeline layout ---
    VkResult res = vkCreatePipelineLayout(vkDevice, &pipelineLayoutInfo, nullptr, &pipelineLayout);
    if (res != VK_SUCCESS) {
        throw std::runtime_error("Failed to create pipeline layout");
    }
}

// Vulkan_CreatePipeline (called once after Vulkan_Init)
bool direct_draw::Vulkan_CreatePipeline()
{
    // --- 1. Load shader modules (SPIR-V compiled earlier) ---
    VkShaderModule vertShaderModule = loadShader(vkDevice, "shader_mig_window.vert.spv");
    VkShaderModule fragShaderModule = loadShader(vkDevice, "shader_mig_window.frag.spv");

    VkPipelineShaderStageCreateInfo vertStage{};
    vertStage.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertStage.stage  = VK_SHADER_STAGE_VERTEX_BIT;
    vertStage.module = vertShaderModule;
    vertStage.pName  = "main";

    VkPipelineShaderStageCreateInfo fragStage{};
    fragStage.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragStage.stage  = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragStage.module = fragShaderModule;
    fragStage.pName  = "main";

    VkPipelineShaderStageCreateInfo stages[] = { vertStage, fragStage };

    // --- 2. Vertex input (none, using gl_VertexIndex) ---

    // --- Fixed function states ---
    VkPipelineVertexInputStateCreateInfo vertexInput{};
    vertexInput.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType    = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    // --- 3. Viewport and scissor ---
    VkViewport viewport{};
    viewport.x        = 0.0f;
    viewport.y        = 0.0f;
    viewport.width  = (float)vkExtent.width;
    viewport.height = (float)vkExtent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = vkExtent;

    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    // We are here
    // --- 4. Rasterizer ---
    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType                   = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable        = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode             = VK_POLYGON_MODE_FILL;
    rasterizer.cullMode                = VK_CULL_MODE_NONE;
    rasterizer.frontFace               = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer.depthBiasEnable         = VK_FALSE;
    rasterizer.lineWidth               = 1.0f;

    // --- 5. Multisampling ---
    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType                = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable  = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    // --- 6. Color blend ---
    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask =
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
        VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;

    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType           = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable   = VK_FALSE;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments    = &colorBlendAttachment;

    // --- 7. Pipeline create info ---
    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount          = 2;
    pipelineInfo.pStages             = stages;
    pipelineInfo.pVertexInputState   = &vertexInput;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState      = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState   = &multisampling;
    pipelineInfo.pColorBlendState    = &colorBlending;
    pipelineInfo.layout              = pipelineLayout; // created earlier
    pipelineInfo.renderPass          = renderPass;     // created earlier
    pipelineInfo.subpass             = 0;
    pipelineInfo.basePipelineHandle  = VK_NULL_HANDLE;

    VkResult res = vkCreateGraphicsPipelines(vkDevice,
                                             VK_NULL_HANDLE,
                                             1,
                                             &pipelineInfo,
                                             nullptr,
                                             &pipeline);
    if (res != VK_SUCCESS) {
        throw std::runtime_error("Failed to create graphics pipeline");
    }

    // --- 8. Cleanup shader modules (no longer needed after pipeline creation) ---
    vkDestroyShaderModule(vkDevice, vertShaderModule, nullptr);
    vkDestroyShaderModule(vkDevice, fragShaderModule, nullptr);

    return true;
}

void direct_draw::Vulkan_Shutdown()
{
    // Best-effort tear-down of Vulkan resources in reverse order of creation.
    // Many variables (vkDevice, vkInstance, vkSurface, etc.) are globals used in this file.
    if (vkDevice != VK_NULL_HANDLE) {
        vkDeviceWaitIdle(vkDevice);
    }

    // Destroy sampler/descriptor resources
    if (vkDevice != VK_NULL_HANDLE) {
        if (fbSampler != VK_NULL_HANDLE) {
            vkDestroySampler(vkDevice, fbSampler, nullptr);
            fbSampler = VK_NULL_HANDLE;
        }
        if (fbDescPool != VK_NULL_HANDLE) {
            vkDestroyDescriptorPool(vkDevice, fbDescPool, nullptr);
            fbDescPool = VK_NULL_HANDLE;
        }
        if (descLayout != VK_NULL_HANDLE) {
            vkDestroyDescriptorSetLayout(vkDevice, descLayout, nullptr);
            descLayout = VK_NULL_HANDLE;
        }
    }

    // Image + image view + memory for the sampled back image
    if (vkDevice != VK_NULL_HANDLE) {
        if (fbImageView != VK_NULL_HANDLE) {
            vkDestroyImageView(vkDevice, fbImageView, nullptr);
            fbImageView = VK_NULL_HANDLE;
        }
        if (fbImage != VK_NULL_HANDLE) {
            vkDestroyImage(vkDevice, fbImage, nullptr);
            fbImage = VK_NULL_HANDLE;
        }
        if (fbMemory != VK_NULL_HANDLE) {
            vkFreeMemory(vkDevice, fbMemory, nullptr);
            fbMemory = VK_NULL_HANDLE;
        }
    }

    // Staging buffer: unmap then free
    if (vkDevice != VK_NULL_HANDLE) {
        if (stagingPtr && stagingMem != VK_NULL_HANDLE) {
            vkUnmapMemory(vkDevice, stagingMem);
            stagingPtr = nullptr;
        }
        if (stagingBuf != VK_NULL_HANDLE) {
            vkDestroyBuffer(vkDevice, stagingBuf, nullptr);
            stagingBuf = VK_NULL_HANDLE;
        }
        if (stagingMem != VK_NULL_HANDLE) {
            vkFreeMemory(vkDevice, stagingMem, nullptr);
            stagingMem = VK_NULL_HANDLE;
        }
    } else {
        // If no device handle but stagingPtr set, clear host pointer to avoid stale pointer
        stagingPtr = nullptr;
    }

    // Pipeline / render pass / pipeline layout
    if (vkDevice != VK_NULL_HANDLE) {
        if (pipeline != VK_NULL_HANDLE) {
            vkDestroyPipeline(vkDevice, pipeline, nullptr);
            pipeline = VK_NULL_HANDLE;
        }
        if (pipelineLayout != VK_NULL_HANDLE) {
            vkDestroyPipelineLayout(vkDevice, pipelineLayout, nullptr);
            pipelineLayout = VK_NULL_HANDLE;
        }
        if (renderPass != VK_NULL_HANDLE) {
            vkDestroyRenderPass(vkDevice, renderPass, nullptr);
            renderPass = VK_NULL_HANDLE;
        }
    }

    // Framebuffers
    if (vkDevice != VK_NULL_HANDLE) {
        for (auto fb : framebuffers) {
            if (fb != VK_NULL_HANDLE) {
                vkDestroyFramebuffer(vkDevice, fb, nullptr);
            }
        }
        framebuffers.clear();
    }

    // Swapchain image views and arrays
    if (vkDevice != VK_NULL_HANDLE) {
        if (swapViews) {
            for (uint32_t i = 0; i < swapCount; ++i) {
                if (swapViews[i] != VK_NULL_HANDLE) {
                    vkDestroyImageView(vkDevice, swapViews[i], nullptr);
                }
            }
            delete[] swapViews;
            swapViews = nullptr;
        }
        if (!swapImageViews.empty()) { // handle whichever array name is used elsewhere
            for (VkImageView v : swapImageViews) {
                if (v != VK_NULL_HANDLE) vkDestroyImageView(vkDevice, v, nullptr);
            }
            swapImageViews.clear();
        }
    }

    // Swapchain itself
    if (vkDevice != VK_NULL_HANDLE && vkSwapchain != VK_NULL_HANDLE) {
        vkDestroySwapchainKHR(vkDevice, vkSwapchain, nullptr);
        vkSwapchain = VK_NULL_HANDLE;
    }

    // Command pool, semaphores, fence
    if (vkDevice != VK_NULL_HANDLE) {
        if (cmdPool != VK_NULL_HANDLE) {
            vkDestroyCommandPool(vkDevice, cmdPool, nullptr);
            cmdPool = VK_NULL_HANDLE;
        }
        if (semAcquire != VK_NULL_HANDLE) {
            vkDestroySemaphore(vkDevice, semAcquire, nullptr);
            semAcquire = VK_NULL_HANDLE;
        }
        if (semPresent != VK_NULL_HANDLE) {
            vkDestroySemaphore(vkDevice, semPresent, nullptr);
            semPresent = VK_NULL_HANDLE;
        }
        if (fence != VK_NULL_HANDLE) {
            vkDestroyFence(vkDevice, fence, nullptr);
            fence = VK_NULL_HANDLE;
        }
    }

    // Destroy device
    if (vkDevice != VK_NULL_HANDLE) {
        vkDestroyDevice(vkDevice, nullptr);
        vkDevice = VK_NULL_HANDLE;
    }

    // Destroy surface and instance
    if (vkInstance != VK_NULL_HANDLE) {
        if (vkSurface != VK_NULL_HANDLE) {
            vkDestroySurfaceKHR(vkInstance, vkSurface, nullptr);
            vkSurface = VK_NULL_HANDLE;
        }
        vkDestroyInstance(vkInstance, nullptr);
        vkInstance = VK_NULL_HANDLE;
    }

    // free swapImages array (host-side array allocated with new[])
    if (swapImages) {
        delete[] swapImages;
        swapImages = nullptr;
    }

    // reset a couple of flags / pointers
    stagingMemoryCoherent = false;
    fbDescSet = VK_NULL_HANDLE;

}