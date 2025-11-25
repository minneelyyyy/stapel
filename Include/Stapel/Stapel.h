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

#pragma once

#include "StapelAPI.h"

#ifdef WIN32
    #include <windows.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

[[noreturn]]
inline void _stapel_fatal_impl(const char* format, ...) {
    char buffer[2048];

    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

#ifdef WIN32
    MessageBoxA(nullptr, buffer, "Fatal Error", MB_ICONERROR | MB_OK);
#else
    fprintf(stderr, "Fatal Error: %s\n", buffer);
#endif

    exit(EXIT_FAILURE);
}

#define INDEX_INVAL (0xFFFFFFFF)

#define STAPEL_FATAL(...) _stapel_fatal_impl(__VA_ARGS__)
