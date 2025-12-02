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

#pragma once

#include <vulkan/vulkan.h>

#include <string>

namespace stapel::backend::vulkan
{
class Pipeline
{
public:
    Pipeline(VkDevice dev, VkPipeline pipeline);
    ~Pipeline();

    Pipeline(const Pipeline& other) = delete;
    Pipeline operator=(const Pipeline& other) = delete;
    Pipeline(Pipeline&& other) = delete;
    Pipeline operator=(Pipeline&& other) = delete;

protected:
    VkDevice device_;
    VkPipeline pipeline_;
};

class ComputePipeline : public Pipeline
{
public:
    ComputePipeline(VkDevice dev, VkPipelineLayout layout, const std::string& shader_path,
                    const std::string& entry);
    ~ComputePipeline();
};
} // namespace stapel::backend::vulkan