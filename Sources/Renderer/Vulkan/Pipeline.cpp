/* Copyright 2025 minneelyyyy <abigail@minneelyyyy.dev>
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
**/

#include "Pipeline.h"

#include <Stapel/Stapel.h>

#include <fstream>
#include <optional>
#include <vector>

namespace stapel::backend::vulkan
{
static std::optional<VkShaderModule> load_shader_module(VkDevice device,
                                                        const std::string& file_path)
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
    if (vkCreateShaderModule(device, &create_info, nullptr, &mod) != VK_SUCCESS)
        return {};

    return mod;
}

Pipeline::Pipeline(VkDevice device, VkPipeline pipeline)
    : device_(device), pipeline_(pipeline)
{}

Pipeline::~Pipeline()
{
    vkDestroyPipeline(device_, pipeline_, nullptr);
}

static VkPipeline create_compute_pipeline(VkDevice dev, VkPipelineLayout layout,
                                          const std::string& path, const std::string& entry)
{
    auto mod = load_shader_module(dev, path);
    if (!mod.has_value())
        STAPEL_FATAL("failed to generate shader");

    VkPipelineShaderStageCreateInfo stage_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = VK_SHADER_STAGE_COMPUTE_BIT,
        .module = mod.value(),
        .pName = entry.c_str(),
    };

    VkComputePipelineCreateInfo compute_pipeline_create_info = {
        .sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
        .stage = stage_info,
        .layout = layout,
    };

    VkPipeline pipeline;
    vkCreateComputePipelines(dev, VK_NULL_HANDLE, 1, &compute_pipeline_create_info, nullptr,
                             &pipeline);

    vkDestroyShaderModule(dev, mod.value(), nullptr);

    return pipeline;
}

ComputePipeline::ComputePipeline(VkDevice dev, VkPipelineLayout layout, const std::string& path,
                                 const std::string& entry)
    : Pipeline(dev, create_compute_pipeline(dev, layout, path, entry))
{}

ComputePipeline::~ComputePipeline()
{
    vkDestroyPipeline(device_, pipeline_, nullptr);
}
} // namespace stapel::backend::vulkan