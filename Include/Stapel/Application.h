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

#include "Window.h"
#include "Renderer.h"

#include <string>

namespace stapel
{
    struct ApplicationInfo
    {
        std::string name;
        int v_major, v_minor, v_patch;
    };

    class IApplication
    {
    public:
        /// Called in the engine before anything else happens
        virtual void PreEngineInitHook(int, char **) {};

        /// Used to control window behavior
        virtual void WindowCreateSpecHook(WindowSpecification& spec)
        {
            spec.width = 1200;
            spec.height = 900;
            spec.title = "Stapel Game";
        }

        virtual void RendererCreateSpecHook(RendererSpecification& spec) {}

        /// Returns the application info struct for this application
        virtual const ApplicationInfo& GetApplicationInfo() = 0;
    };
}

#ifdef _WIN32
#   define EXPORT __declspec(dllexport)
#else
#   define EXPORT
#endif

// Defines a getter function for the engine to use to grab
// an instance of your application.
#define CREATE_APP_INSTANCE(_game_class) \
extern "C" EXPORT \
stapel::IApplication* CreateApplicationInstance() \
{ \
    auto* app = new _game_class(); \
    return app; \
}
