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

#include "CommandBuffer.h"

namespace stapel::backend::vulkan
{
CommandBuffer::CommandBuffer(VkDevice dev, VkCommandPool pool)
    : device_(dev), pool_(pool)
{
    VkCommandBufferAllocateInfo alloc_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = pool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1,
    };

    vkAllocateCommandBuffers(dev, &alloc_info, &buf_);
}

CommandBuffer::~CommandBuffer()
{
    vkFreeCommandBuffers(device_, pool_, 1, &buf_);
}

void CommandBuffer::record(std::function<void(VkCommandBuffer)> f)
{
    VkCommandBufferBeginInfo cmd_buf_begin_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    };

    vkResetCommandBuffer(buf_, 0);
    vkBeginCommandBuffer(buf_, &cmd_buf_begin_info);
    f(buf_);
    vkEndCommandBuffer(buf_);
}
} // namespace stapel::backend::vulkan
