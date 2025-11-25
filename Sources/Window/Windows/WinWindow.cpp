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

#include "WinWindow.h"

#ifdef VULKAN_ENABLED
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_win32.h>
#endif

#include <stdexcept>

static int width = 800, height = 600;

void OnSize(HWND hwnd, UINT flag, int width, int height)
{
	// Handle resizing
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_SIZE:
	{
		width = LOWORD(lParam);  // Macro to get the low-order word.
		height = HIWORD(lParam); // Macro to get the high-order word.

		// Respond to the message:
		OnSize(hwnd, (UINT)wParam, width, height);
	}
	break;
	default:
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}

	return 0;
}

#define CLASS_NAME "Stapel Window"

namespace stapel::backend
{
	Win32Window::Win32Window(const WindowSpecification& spec)
	{
		instance_ = GetModuleHandle(nullptr);

		WNDCLASS wc {};
		wc.lpfnWndProc = WindowProc;
		wc.hInstance = instance_;
		wc.lpszClassName = CLASS_NAME;

		RegisterClass(&wc);

		width = spec.width;
		height = spec.height;

		hwnd_ = CreateWindowEx(
			0,
			CLASS_NAME,
			spec.title,
			WS_OVERLAPPEDWINDOW,

			// Size and position
			CW_USEDEFAULT, CW_USEDEFAULT, width, height,

			NULL,
			NULL,
			instance_,
			NULL
		);

		if (!hwnd_)
			throw std::runtime_error("failed to create window");

		ShowWindow(hwnd_, SW_SHOW);
		UpdateWindow(hwnd_);
	}

	Win32Window::~Win32Window()
	{
		if (hwnd_)
			DestroyWindow(hwnd_);

		UnregisterClass(CLASS_NAME, instance_);
	}

	uint32_t Win32Window::Width() const
	{
		return static_cast<uint32_t>(width);
	}

	uint32_t Win32Window::Height() const
	{
		return static_cast<uint32_t>(height);
	}

#ifdef VULKAN_ENABLED
	VkSurfaceKHR Win32Window::CreateVulkanSurface(VkInstance instance)
	{
		VkWin32SurfaceCreateInfoKHR info {};
		info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
		info.hinstance = instance_;
		info.hwnd = hwnd_;

		VkSurfaceKHR surface;
		if (vkCreateWin32SurfaceKHR(instance, &info, nullptr, &surface) != VK_SUCCESS)
			return surface;

		return surface;
	}
#endif
}
