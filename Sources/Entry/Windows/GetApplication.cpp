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
 */

#include <Stapel/Application.h>

#define WINDOWS_LEAN_AND_MEAN
#include <windows.h>

stapel::IApplication* GetApplication()
{
    HMODULE mod = LoadLibraryEx("libgame.dll", nullptr, 0x0);
    if (!mod)
        return nullptr;

    using GetApplicationInstance = stapel::IApplication* (*)();
    GetApplicationInstance CreateApplicationInstance =
        reinterpret_cast<GetApplicationInstance>(GetProcAddress(mod, "CreateApplicationInstance"));

    if (!CreateApplicationInstance)
    {
        FreeLibrary(mod);
        return nullptr;
    }

    auto* app = CreateApplicationInstance();
    return app;
}
