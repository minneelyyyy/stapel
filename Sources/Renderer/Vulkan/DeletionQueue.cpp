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

#include "DeletionQueue.h"

namespace stapel::backend
{
DeletionQueue::DeletionQueue()
{}

DeletionQueue::~DeletionQueue()
{
    DeleteAll();
}

void DeletionQueue::Push(std::function<void()>&& f)
{
    fns_.push_back(f);
}

void DeletionQueue::DeleteAll()
{
    for (auto it = fns_.rbegin(); it != fns_.rend(); it++) {
        (*it)();
    }

    fns_.clear();
}
}; // namespace stapel::backend
