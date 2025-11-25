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

#include <iostream>

#include <dlfcn.h>

#define STAPEL_LIBRARY_SO "bin/libStapelEngine.so"
#define STAPEL_ENTRY_FN "stapel_engine_entry"

int main(int argc, char **argv)
{
    void* handle = dlopen(STAPEL_LIBRARY_SO, RTLD_NOW);
    if (!handle) {
        std::cout
            << "Error: Could not locate dynamic library `" << STAPEL_LIBRARY_SO << "': "
            << dlerror() << "\n";

        return 1;
    }

    using EngineEntry = int(*)(int, char**);
    EngineEntry entry = reinterpret_cast<EngineEntry>(dlsym(handle, STAPEL_ENTRY_FN));
    if (!entry) {
        std::cout
            << "Error: Failed to load `" << STAPEL_ENTRY_FN
            <<"' from " << STAPEL_LIBRARY_SO << ": "
            << dlerror() << "\n";

        dlclose(handle);

        return 1;
    }

    int r = entry(argc, argv);
    if (r != 0) {
        std::cout << "Error: Engine entry point returned with error code " << r << ".\n";
        return r;
    }

    dlclose(handle);

    return 0;
}