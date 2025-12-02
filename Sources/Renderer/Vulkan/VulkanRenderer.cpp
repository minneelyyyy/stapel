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
#include <print>
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

    switch (window.backend()) {
#ifdef USE_X11
    case Window::X11:
        exts.push_back(VK_KHR_XLIB_SURFACE_EXTENSION_NAME);
        break;
#endif
#ifdef USE_WAYLAND
    case Window::Wayland:
        exts.push_back(VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME);
        break;
#endif
#ifdef USE_WIN32
    case Window::Windows:
        exts.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
        break;
#endif
    default:
        STAPEL_FATAL("No surface present");
    }

    uint32_t count;
    vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr);

    std::vector<VkExtensionProperties> extensions(count);
    if (vkEnumerateInstanceExtensionProperties(nullptr, &count, extensions.data()) != VK_SUCCESS)
        STAPEL_FATAL("failure to get instance properties");

    VkApplicationInfo appinfo = {
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
        std::println(
            "WARNING: validation layers specified (implicit from debug build), but none selected");

    VkInstanceCreateInfo info = {
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

#if 0
void Renderer::drawBackground(VkCommandBuffer cmd, Image& img)
{
    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, gradient_pipeline_);

    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, gradient_pipeline_layout_, 0, 1,
                            &draw_image_desc_, 0, nullptr);

    vkCmdDispatch(cmd, std::ceil(draw_img_.value().width() / 16.0),
                  std::ceil(draw_img_.value().height() / 16.0), 1);
}
#endif

void Renderer::drawFrame()
{
    if (window_.resized()) {
        device_->rebuildSwapchain();
        window_.resizeHandled();
    }

    Device::Frame& frame = device_->acquireNextFrame();
    CommandBuffer& cmd = frame.getCmdBuffer();
    Image& img = frame.getSwapchainImage();

    cmd.record([&](VkCommandBuffer cmd) {
        draw_img_->transition(cmd, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_UNDEFINED);

        VkClearColorValue color = {
            .float32 = {1.0, 0.0, 0.0, 1.0},
        };

        VkImageSubresourceRange range = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .levelCount = VK_REMAINING_MIP_LEVELS,
            .layerCount = VK_REMAINING_ARRAY_LAYERS,
        };

        vkCmdClearColorImage(cmd, draw_img_->image(), VK_IMAGE_LAYOUT_GENERAL, &color, 1, &range);

        draw_img_->transition(cmd, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
        img.transition(cmd, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

        draw_img_->copy(cmd, img);
        img.transition(cmd, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
    });

    device_->submitCmdBuffer(cmd, frame);
    device_->present(frame);
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

DescriptorAllocator::~DescriptorAllocator()
{
    vkDestroyDescriptorPool(device_, pool_, nullptr);
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

static PhysicalDeviceInfo create_device_info(VkInstance instance, VkSurfaceKHR surface)
{
    auto devs = vulkan::DeviceBuilder(instance, surface)
                    .ext(VK_KHR_SWAPCHAIN_EXTENSION_NAME)
                    .ext(VK_KHR_SPIRV_1_4_EXTENSION_NAME)
                    .ext(VK_KHR_MAINTENANCE1_EXTENSION_NAME)
                    .extendedDynamicState(true)
                    .synchronization2(true)
                    .dynamicRendering(true)
                    .descriptorIndexing(true)
                    .bufferDeviceAddress(true)
                    .swapchainMaintenance1(true) // oops forgor
                    .devices();

    if (devs.size() == 0)
        STAPEL_FATAL("No valid device found!");

    return devs.front();
}

static VmaAllocator create_vma_allocator(VkInstance instance, Device& device)
{
    VmaAllocatorCreateInfo allocator_info = {
        .flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT,
        .physicalDevice = device.physical(),
        .device = device.device(),
        .instance = instance,
    };

    VmaAllocator alloc;
    vmaCreateAllocator(&allocator_info, &alloc);

    return alloc;
}

Renderer::Renderer(Window& window, const char* name, uint32_t version)
    : window_(window),
      instance_(create_instance(window, name, version)),
      surface_(window.createVulkanSurface(instance_)),
      device_(std::make_unique<Device>(window, instance_, surface_,
                                       create_device_info(instance_, surface_))),
      alloc_(create_vma_allocator(instance_, *device_)),
      draw_img_(std::make_unique<Image>(alloc_, device_->device(), window.width(), window.height(),
                                        1, VK_FORMAT_R16G16B16A16_SFLOAT))
{
#if 0
    std::vector<DescriptorAllocator::PoolSizeRatio> sizes = {{VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1}};

    auto descriptor_alloc_ = std::make_unique<DescriptorAllocator>(device_.device(), 10, sizes);

    auto draw_image_desc_layout_ = DescriptorSetLayoutBuilder()
                                       .addBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE)
                                       .build(device_.device(), VK_SHADER_STAGE_COMPUTE_BIT);

    auto draw_image_desc_ = descriptor_alloc_->allocate(draw_image_desc_layout_);

    VkDescriptorImageInfo img_info = {
        .imageView = draw_img_.view(),
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

    auto gradient_pipeline_layout_ = create_compute_pipeline_layout(*device_, draw_image_desc_layout_);

    auto pipeline = create_compute_pipeline(*device_, gradient_pipeline_layout_);
    if (!pipeline.has_value())
        STAPEL_FATAL("failure to create compute shader pipeline");

    gradient_pipeline_ = pipeline.value();

    del_.push([&]() {
        vkDestroyPipelineLayout(device_->device(), gradient_pipeline_layout_, nullptr);
        vkDestroyPipeline(device_->device(), gradient_pipeline_, nullptr);
    });
#endif
}

stapel::Renderer::Backend Renderer::backend() const
{
    return stapel::Renderer::Backend::Vulkan;
}

Renderer::~Renderer()
{
    device_->waitIdle();

    draw_img_.reset();
    vmaDestroyAllocator(alloc_);
    device_.reset();
    vkDestroySurfaceKHR(instance_, surface_, nullptr);
    vkDestroyInstance(instance_, nullptr);
}
} // namespace stapel::backend::vulkan
