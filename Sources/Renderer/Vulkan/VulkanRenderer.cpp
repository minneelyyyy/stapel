/* Copyright 2025 minneelyyyy <abigail@minneelyyyy.dev>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
**/

#include "VulkanRenderer.h"
#include "Renderer/Vulkan/Device.h"

#include <memory>
#include <version.h>

#include <vulkan/vulkan.h>

#pragma clang diagnostic push
// VMA is particularly annoying with this flag enabled
#pragma clang diagnostic ignored "-Wnullability-completeness"

// only define VMA_IMPLEMENTATION here.
#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"

#pragma clang diagnostic pop

#ifdef USE_X11
#include <X11/Xlib.h>
#include <vulkan/vulkan_xlib.h>
#endif

#ifdef USE_WAYLAND
#include <vulkan/vulkan_wayland.h>
#endif

#ifdef USE_WIN32
#include <vulkan/vulkan_win32.h>
#endif

#include <algorithm>
#include <cmath>
#include <cstring>
#include <fstream>
#include <iostream>
#include <vector>

namespace stapel::backend::vulkan
{
const std::vector<const char*> validation_layers = {"VK_LAYER_KHRONOS_validation"};

#ifdef NDEBUG
bool enable_validation_layers = false;
#else
bool enable_validation_layers = true;
#endif

std::vector<const char*> enabled_validation_layers()
{
    std::vector<const char*> layers;
    layers.reserve(validation_layers.size());

    if (!enable_validation_layers)
        return layers;

    uint32_t layer_count;
    vkEnumerateInstanceLayerProperties(&layer_count, nullptr);

    std::vector<VkLayerProperties> available_layers(layer_count);
    vkEnumerateInstanceLayerProperties(&layer_count, available_layers.data());

    for (const char* layer : validation_layers) {
        for (const auto prop : available_layers) {
            if (!std::strcmp(layer, prop.layerName)) {
                layers.push_back(layer);
                break;
            }
        }
    }

    return layers;
}

VkInstance create_instance(const Window& window, const char* name, uint32_t version)
{
    std::vector<const char*> exts;
    exts.push_back(VK_KHR_SURFACE_EXTENSION_NAME);

#ifdef USE_X11
    if (window.backend() == Window::X11) {
        exts.push_back(VK_KHR_XLIB_SURFACE_EXTENSION_NAME);
    }
#endif

#ifdef USE_WAYLAND
    if (window.backend() == Window::Wayland) {
        exts.push_back(VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME);
    }
#endif

#ifdef USE_WIN32
    if (window.backend() == Window::Windows) {
        exts.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
    }
#endif

    if (exts.size() == 0)
        STAPEL_FATAL("No surface extension");

    uint32_t count;
    vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr);

    std::vector<VkExtensionProperties> extensions(count);
    if (vkEnumerateInstanceExtensionProperties(nullptr, &count, extensions.data()) != VK_SUCCESS)
        STAPEL_FATAL("failure to get instance properties");

    VkApplicationInfo appinfo{
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = name,
        .applicationVersion = version,
        .pEngineName = "Stapel",
        .engineVersion =
            VK_MAKE_VERSION(STAPEL_VERSION_MAJOR, STAPEL_VERSION_MINOR, STAPEL_VERSION_PATCH),
        .apiVersion = VK_API_VERSION_1_4,
    };

    auto layers = enabled_validation_layers();

    if (enable_validation_layers && layers.size() == 0)
        std::cout
            << "WARNING: validation layers specified (implicit from debug build), but none selected"
            << std::endl;

    VkInstanceCreateInfo info{
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo = &appinfo,
        .enabledLayerCount = static_cast<uint32_t>(layers.size()),
        .ppEnabledLayerNames = layers.data(),
        .enabledExtensionCount = static_cast<uint32_t>(exts.size()),
        .ppEnabledExtensionNames = exts.data(),
    };

    VkInstance instance;
    if (vkCreateInstance(&info, nullptr, &instance) != VK_SUCCESS)
        STAPEL_FATAL("Could not instantiate Vulkan");

    return instance;
}

VkSurfaceFormatKHR select_best_surface_format(VkSurfaceKHR surface, VkPhysicalDevice device)
{
    uint32_t count = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &count, nullptr);

    std::vector<VkSurfaceFormatKHR> formats(count);
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &count, formats.data());

    for (const auto format : formats) {
        if (format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR &&
            format.format == VK_FORMAT_B8G8R8_SRGB)
            return format;
    }

    return formats[0];
}

std::optional<VkPresentModeKHR>
select_best_present_mode(VkPhysicalDevice device, VkSurfaceKHR surface,
                         std::span<VkPresentModeKHR> preferred_modes)
{
    uint32_t count = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &count, nullptr);

    // no modes available, hopefully this does not happen.
    if (count == 0)
        return {};

    std::vector<VkPresentModeKHR> modes(count);
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &count, modes.data());

    for (auto preferred_mode : preferred_modes) {
        for (auto mode : modes) {
            if (mode == preferred_mode)
                return mode;
        }
    }

    // no preferred mode found, use the first one available
    return modes.front();
}

VkExtent2D select_best_extent(Window& window, VkSurfaceCapabilitiesKHR capabilities)
{
    if (capabilities.currentExtent.width != static_cast<uint32_t>(0xffffffff)) {
        return capabilities.currentExtent;
    }

    VkExtent2D extent = {
        .width = std::clamp<uint32_t>(window.width(), capabilities.minImageExtent.width,
                                      capabilities.maxImageExtent.width),
        .height = std::clamp<uint32_t>(window.height(), capabilities.minImageExtent.height,
                                       capabilities.maxImageExtent.height),
    };

    return extent;
}

Renderer::Swapchain Renderer::createSwapchain(Window& window, vulkan::Device& device,
                                              VkSurfaceKHR surface)
{
    VkSurfaceFormatKHR surface_format = select_best_surface_format(surface, device.GetPhys());

    VkPresentModeKHR mode_pref[] = {
        VK_PRESENT_MODE_FIFO_KHR,
        VK_PRESENT_MODE_MAILBOX_KHR,
        VK_PRESENT_MODE_IMMEDIATE_KHR,
    };

    VkPresentModeKHR present_mode =
        select_best_present_mode(device.GetPhys(), surface, mode_pref).value();

    VkSurfaceCapabilitiesKHR capabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device.GetPhys(), surface, &capabilities);
    VkExtent2D extent = select_best_extent(window, capabilities);

    uint32_t min_img_count = std::max<>(3u, capabilities.minImageCount);

    if (capabilities.maxImageCount > 0 && min_img_count > capabilities.maxImageCount) {
        min_img_count = capabilities.maxImageCount;
    }

    VkFormat format = surface_format.format;

    uint32_t index = device.GetFamilyIndex();

    VkSwapchainCreateInfoKHR info = {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .flags = 0,
        .surface = surface,
        .minImageCount = min_img_count,
        .imageFormat = format,
        .imageColorSpace = surface_format.colorSpace,
        .imageExtent = extent,
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
        .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 1,
        .pQueueFamilyIndices = &index,
        .preTransform = capabilities.currentTransform,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = present_mode,
        .clipped = VK_TRUE,
        .oldSwapchain = VK_NULL_HANDLE,
    };

    VkSwapchainKHR chain;
    if (vkCreateSwapchainKHR(device.device(), &info, nullptr, &chain) != VK_SUCCESS)
        STAPEL_FATAL("Failed to create swapchain");

    uint32_t count;
    vkGetSwapchainImagesKHR(device.device(), chain, &count, nullptr);

    std::vector<VkImage> vk_images(count);
    vkGetSwapchainImagesKHR(device.device(), chain, &count, vk_images.data());

    std::vector<Image> images;
    images.reserve(count);

    for (VkImage image : vk_images) {
        Image img = Image::wrap(device.device(), extent.width, extent.height, 1, format, image,
                                VK_IMAGE_LAYOUT_UNDEFINED);
        images.push_back(std::move(img));
    }

    return Swapchain{
        .chain = chain,
        .images = std::move(images),
        .extent = {extent.width, extent.height, 1},
        .format = format,
    };
}

VkCommandPool create_command_pool(VkDevice device, uint32_t queue_family_index)
{
    VkCommandPoolCreateInfo cmd_pool_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = queue_family_index,
    };

    VkCommandPool pool;
    if (vkCreateCommandPool(device, &cmd_pool_info, nullptr, &pool) != VK_SUCCESS)
        STAPEL_FATAL("Failed to create command pool");

    return pool;
}

VkCommandBuffer create_command_buffer(VkDevice device, VkCommandPool pool)
{
    VkCommandBufferAllocateInfo alloc_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = pool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1,
    };

    VkCommandBuffer buffer;
    if (vkAllocateCommandBuffers(device, &alloc_info, &buffer) != VK_SUCCESS)
        STAPEL_FATAL("Failed to allocate command buffer");

    return buffer;
}

VkSemaphore create_binary_semaphore(VkDevice device)
{
    VkSemaphoreCreateInfo info = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
    };

    VkSemaphore semaphore;
    if (vkCreateSemaphore(device, &info, nullptr, &semaphore) != VK_SUCCESS)
        STAPEL_FATAL("Failed to create semaphore");

    return semaphore;
}

VkFence create_fence(VkDevice device, bool signaled)
{
    VkFenceCreateInfo info = {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
    };

    if (signaled)
        info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    VkFence fence;
    if (vkCreateFence(device, &info, nullptr, &fence) != VK_SUCCESS)
        STAPEL_FATAL("Failed to create fence");

    return fence;
}

void Renderer::drawBackground(VkCommandBuffer cmd, Image& img)
{
    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, gradient_pipeline_);

    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, gradient_pipeline_layout_, 0, 1,
                            &draw_image_desc_, 0, nullptr);

    vkCmdDispatch(cmd, std::ceil(draw_img_.value().width() / 16.0),
                  std::ceil(draw_img_.value().height() / 16.0), 1);
}

void Renderer::drawFrame()
{
    FrameData& frame = getFrame();

    vkWaitForFences(device_->device(), 1, &frame.render_fence, true, UINT64_MAX);
    vkResetFences(device_->device(), 1, &frame.render_fence);

    frame.del.clear();

    uint32_t image_idx;
    vkAcquireNextImageKHR(device_->device(), swapchain_.chain, UINT64_MAX,
                          frame.swapchain_semaphore, nullptr, &image_idx);

    Image& swapchain_img = swapchain_.images[image_idx];

    VkCommandBuffer cmd = frame.buffer;
    vkResetCommandBuffer(cmd, 0);

    VkCommandBufferBeginInfo cmd_buf_begin_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    };

    Image& draw_img = draw_img_.value();

    if (VK_SUCCESS != vkBeginCommandBuffer(cmd, &cmd_buf_begin_info))
        STAPEL_FATAL("failure to begin command buffer");

    draw_img.transition(cmd, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_UNDEFINED);

    drawBackground(cmd, draw_img);

    draw_img.transition(cmd, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
    swapchain_img.transition(cmd, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    draw_img.copy(cmd, swapchain_img);
    swapchain_img.transition(cmd, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

    if (VK_SUCCESS != vkEndCommandBuffer(cmd))
        STAPEL_FATAL("failure at end of command buffer");

    VkCommandBufferSubmitInfo cmd_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
        .commandBuffer = cmd,
    };

    VkSemaphoreSubmitInfo wait_info = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
        .semaphore = frame.swapchain_semaphore,
        .stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR,
    };

    VkSemaphoreSubmitInfo signal_info = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
        .semaphore = frame.render_semaphore,
        .stageMask = VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT,
    };

    VkSubmitInfo2 submit_info = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
        .waitSemaphoreInfoCount = 1,
        .pWaitSemaphoreInfos = &wait_info,
        .commandBufferInfoCount = 1,
        .pCommandBufferInfos = &cmd_info,
        .signalSemaphoreInfoCount = 1,
        .pSignalSemaphoreInfos = &signal_info,
    };

    if (VK_SUCCESS != vkQueueSubmit2(device_->GetQueue(), 1, &submit_info, frame.render_fence))
        STAPEL_FATAL("failure to submit command queue");

    VkPresentInfoKHR present_info = {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &frame.render_semaphore,
        .swapchainCount = 1,
        .pSwapchains = &swapchain_.chain,
        .pImageIndices = &image_idx,
    };

    switch (vkQueuePresentKHR(device_->GetQueue(), &present_info)) {
    case VK_SUCCESS:
    case VK_SUBOPTIMAL_KHR:
        break;
    default:
        STAPEL_FATAL("queue present error");
    }

    frame_idx_++;
}

class DescriptorSetLayoutBuilder
{
public:
    DescriptorSetLayoutBuilder()
    {}

    DescriptorSetLayoutBuilder& addBinding(uint32_t binding, VkDescriptorType type)
    {
        VkDescriptorSetLayoutBinding set_layout_binding = {
            .binding = binding,
            .descriptorType = type,
            .descriptorCount = 1,
        };

        bindings_.push_back(set_layout_binding);

        return *this;
    }

    VkDescriptorSetLayout build(VkDevice device, VkShaderStageFlags shader_stages,
                                void* next_ptr = nullptr,
                                VkDescriptorSetLayoutCreateFlags flags = 0)
    {
        for (auto& binding : bindings_) {
            binding.stageFlags |= shader_stages;
        }

        VkDescriptorSetLayoutCreateInfo desc_set_layout_create_info = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            .pNext = next_ptr,
            .flags = flags,
            .bindingCount = static_cast<uint32_t>(bindings_.size()),
            .pBindings = bindings_.data(),
        };

        VkDescriptorSetLayout layout;
        if (VK_SUCCESS !=
            vkCreateDescriptorSetLayout(device, &desc_set_layout_create_info, nullptr, &layout))
            STAPEL_FATAL("failed to create descriptor set layout");

        return layout;
    }

private:
    std::vector<VkDescriptorSetLayoutBinding> bindings_;
};

DescriptorAllocator::DescriptorAllocator(VkDevice device, uint32_t max_sets,
                                         std::span<PoolSizeRatio> pool_ratios)
    : device_(device)
{
    std::vector<VkDescriptorPoolSize> pool_sizes;
    pool_sizes.reserve(pool_ratios.size());

    std::transform(pool_ratios.begin(), pool_ratios.end(), std::back_inserter(pool_sizes),
                   [&](const PoolSizeRatio& ratio) {
                       return VkDescriptorPoolSize{
                           .type = ratio.type,
                           .descriptorCount = uint32_t(ratio.ratio * max_sets),
                       };
                   });

    VkDescriptorPoolCreateInfo pool_info = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .maxSets = max_sets,
        .poolSizeCount = static_cast<uint32_t>(pool_sizes.size()),
        .pPoolSizes = pool_sizes.data(),
    };

    vkCreateDescriptorPool(device, &pool_info, nullptr, &pool_);
}

void DescriptorAllocator::clearDescriptors()
{
    vkResetDescriptorPool(device_, pool_, 0);
}

void DescriptorAllocator::destroyPool()
{
    vkDestroyDescriptorPool(device_, pool_, nullptr);
}

VkDescriptorSet DescriptorAllocator::allocate(VkDescriptorSetLayout layout)
{
    VkDescriptorSetAllocateInfo alloc_info = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .descriptorPool = pool_,
        .descriptorSetCount = 1,
        .pSetLayouts = &layout,
    };

    VkDescriptorSet ds;
    if (VK_SUCCESS != vkAllocateDescriptorSets(device_, &alloc_info, &ds))
        STAPEL_FATAL("failure to allocate descriptor set");

    return ds;
}

std::optional<VkShaderModule> load_shader_module(VkDevice device, const char* file_path)
{
    std::ifstream file(file_path, std::ios::ate | std::ios::binary);

    if (!file.is_open())
        return {};

    const auto file_size = static_cast<size_t>(file.tellg());

    std::vector<uint32_t> buffer(file_size / sizeof(uint32_t));

    file.seekg(0);

    file.read(reinterpret_cast<char*>(buffer.data()), buffer.size() * sizeof(uint32_t));

    file.close();

    VkShaderModuleCreateInfo create_info = {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = static_cast<uint32_t>(buffer.size() * sizeof(uint32_t)),
        .pCode = buffer.data(),
    };

    VkShaderModule mod;
    if (vkCreateShaderModule(device, &create_info, nullptr, &mod) != VK_SUCCESS) {
        return {};
    }

    return mod;
}

VkPipelineLayout create_compute_pipeline_layout(Device& dev, VkDescriptorSetLayout layout)
{
    VkPipelineLayoutCreateInfo compute_layout = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount = 1,
        .pSetLayouts = &layout,
    };

    VkPipelineLayout pipeline_layout;
    if (VK_SUCCESS !=
        vkCreatePipelineLayout(dev.device(), &compute_layout, nullptr, &pipeline_layout))
        STAPEL_FATAL("failure to create compute pipeline");

    return pipeline_layout;
}

std::optional<VkPipeline> create_compute_pipeline(Device& dev, VkPipelineLayout layout)
{
    auto mod = load_shader_module(dev.device(), "shaders/gradient.comp.spv");
    if (!mod.has_value())
        STAPEL_FATAL("failed to generate shader");

    VkPipelineShaderStageCreateInfo stage_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = VK_SHADER_STAGE_COMPUTE_BIT,
        .module = mod.value(),
        .pName = "main",
    };

    VkComputePipelineCreateInfo compute_pipeline_create_info = {
        .sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
        .stage = stage_info,
        .layout = layout,
    };

    VkPipeline pipeline;
    vkCreateComputePipelines(dev.device(), VK_NULL_HANDLE, 1, &compute_pipeline_create_info,
                             nullptr, &pipeline);

    return pipeline;
}

Renderer::Renderer(std::shared_ptr<Window> window, const char* name, uint32_t version)
    : window_(window)
{
    instance_ = create_instance(*window, name, version);
    surface_ = window->createVulkanSurface(instance_);

    auto devs = vulkan::DeviceBuilder(instance_, surface_)
                    .ext(VK_KHR_SWAPCHAIN_EXTENSION_NAME)
                    .ext(VK_KHR_SPIRV_1_4_EXTENSION_NAME)
                    .extendedDynamicState(true)
                    .synchronization2(true)
                    .dynamicRendering(true)
                    .descriptorIndexing(true)
                    .bufferDeviceAddress(true)
                    .devices();

    if (devs.size() == 0)
        STAPEL_FATAL("No valid device found!");

    device_ = std::make_unique<vulkan::Device>(instance_, surface_, devs[0]);

    swapchain_ = createSwapchain(*window, *device_, surface_);

    uint32_t size;
    vkGetSwapchainImagesKHR(device_->device(), swapchain_.chain, &size, nullptr);

    frames_.reserve(size);

    for (uint32_t i = 0; i < size; i++) {
        FrameData frame;

        frame.pool = create_command_pool(device_->device(), device_->GetFamilyIndex());
        frame.buffer = create_command_buffer(device_->device(), frame.pool);
        frame.render_fence = create_fence(device_->device(), true);
        frame.render_semaphore = create_binary_semaphore(device_->device());
        frame.swapchain_semaphore = create_binary_semaphore(device_->device());

        frames_.push_back(frame);
    }

    VmaAllocatorCreateInfo allocator_info = {
        .flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT,
        .physicalDevice = device_->GetPhys(),
        .device = device_->device(),
        .instance = instance_,
    };

    vmaCreateAllocator(&allocator_info, &alloc_);

    draw_img_ = Image(alloc_, device_->device(), swapchain_.extent.width, swapchain_.extent.width,
                      swapchain_.extent.depth, swapchain_.format);

    del_.push([&]() { vmaDestroyAllocator(alloc_); });

    std::vector<DescriptorAllocator::PoolSizeRatio> sizes = {{VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1}};

    descriptor_alloc_ = std::make_unique<DescriptorAllocator>(device_->device(), 10, sizes);

    draw_image_desc_layout_ = DescriptorSetLayoutBuilder()
                                  .addBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE)
                                  .build(device_->device(), VK_SHADER_STAGE_COMPUTE_BIT);

    draw_image_desc_ = descriptor_alloc_->allocate(draw_image_desc_layout_);

    VkDescriptorImageInfo img_info = {
        .imageView = draw_img_->view(),
        .imageLayout = VK_IMAGE_LAYOUT_GENERAL,
    };

    VkWriteDescriptorSet draw_image_write = {
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = draw_image_desc_,
        .dstBinding = 0,
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
        .pImageInfo = &img_info,
    };

    vkUpdateDescriptorSets(device_->device(), 1, &draw_image_write, 0, nullptr);

    del_.push([&]() {
        descriptor_alloc_.reset();
        vkDestroyDescriptorSetLayout(device_->device(), draw_image_desc_layout_, nullptr);
    });

    gradient_pipeline_layout_ = create_compute_pipeline_layout(*device_, draw_image_desc_layout_);

    auto pipeline = create_compute_pipeline(*device_, gradient_pipeline_layout_);
    if (!pipeline.has_value())
        STAPEL_FATAL("failure to create compute shader pipeline");

    gradient_pipeline_ = pipeline.value();

    del_.push([&]() {
        vkDestroyPipelineLayout(device_->device(), gradient_pipeline_layout_, nullptr);
        vkDestroyPipeline(device_->device(), gradient_pipeline_, nullptr);
    });
}

stapel::Renderer::Backend Renderer::backend() const
{
    return stapel::Renderer::Backend::Vulkan;
}

void Renderer::destroySwapchain(VkDevice device)
{
    vkDestroySwapchainKHR(device, swapchain_.chain, nullptr);
    swapchain_.images.clear();
}

Renderer::~Renderer()
{
    vkDeviceWaitIdle(device_->device());

    for (auto& frame : frames_) {
        vkDestroyCommandPool(device_->device(), frame.pool, nullptr);

        vkDestroyFence(device_->device(), frame.render_fence, nullptr);
        vkDestroySemaphore(device_->device(), frame.render_semaphore, nullptr);
        vkDestroySemaphore(device_->device(), frame.swapchain_semaphore, nullptr);

        frame.del.clear();
    }

    draw_img_.reset();
    del_.clear();

    destroySwapchain(device_->device());

    device_.reset();

    vkDestroySurfaceKHR(instance_, surface_, nullptr);
    vkDestroyInstance(instance_, nullptr);
}
} // namespace stapel::backend::vulkan
