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

#include "util.h"

#include <Stapel/Stapel.h>

namespace stapel::backend::vulkan::util
{
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
} // namespace stapel::backend::vulkan::util