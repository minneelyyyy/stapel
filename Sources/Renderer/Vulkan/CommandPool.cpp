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

#include "CommandPool.h"

namespace stapel::backend::vulkan
{
CommandPool::CommandPool(VkDevice dev, uint32_t idx)
    : device_(dev)
{
    VkCommandPoolCreateInfo cmd_pool_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = idx,
    };

    vkCreateCommandPool(dev, &cmd_pool_info, nullptr, &pool_);
}

CommandPool::~CommandPool()
{
    vkDestroyCommandPool(device_, pool_, nullptr);
}

CommandBuffer CommandPool::createBuffer()
{
    return CommandBuffer(device_, pool_);
}
} // namespace stapel::backend::vulkan