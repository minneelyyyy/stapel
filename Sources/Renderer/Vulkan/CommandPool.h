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

#include "CommandBuffer.h"

namespace stapel::backend::vulkan
{
class CommandPool
{
public:
    CommandPool(VkDevice dev, uint32_t idx);
    ~CommandPool();

    CommandPool(const CommandPool&) = delete;
    CommandPool& operator=(const CommandPool&) = delete;
    CommandPool(CommandPool&&) = delete;
    CommandPool& operator=(CommandPool&&) = delete;

    CommandBuffer createBuffer();

public:
    friend class CommandBuffer;
    friend class Device;

private:
    VkDevice device_;
    VkCommandPool pool_;
};
} // namespace stapel::backend::vulkan