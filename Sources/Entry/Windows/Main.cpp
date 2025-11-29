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
**/

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <Stapel/Stapel.h>
#include <iostream>

#define STAPEL_LIBRARY_DLL "bin\\StapelEngine.dll"
#define STAPEL_ENTRY_SYMBOL "stapel_engine_entry"

static void AlertLastError(const char* message)
{
    LPVOID lpMsgBuf;
    char msgBuf[2048] = "";
    DWORD dw = GetLastError();

    if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL,
                      dw, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&lpMsgBuf, 0, NULL) == 0) {
        MessageBox(NULL, TEXT("FormatMessage failed"), TEXT("Error"), MB_OK);
        ExitProcess(dw);
    }

    ::snprintf(msgBuf, sizeof(msgBuf), "%s: %s", message, (char*)lpMsgBuf);
    MessageBox(NULL, (LPCTSTR)msgBuf, TEXT("Error"), MB_OK);

    LocalFree(lpMsgBuf);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pCmdLine, int nCmdShow)
{
    HMODULE mod = LoadLibraryExA(STAPEL_LIBRARY_DLL, nullptr, 0x0);
    if (!mod) {
        AlertLastError("Failed to load " STAPEL_LIBRARY_DLL);
        return -1;
    }

    using Entry = int (*)(int, char**);
    Entry fnEntry = reinterpret_cast<Entry>(GetProcAddress(mod, STAPEL_ENTRY_SYMBOL));

    if (!fnEntry) {
        AlertLastError("Failed to load symbol " STAPEL_ENTRY_SYMBOL " from " STAPEL_LIBRARY_DLL ".");
        FreeLibrary(mod);
        return -1;
    }

    // The engine just uses GetModuleHandle in Window creation so it doesn't need to be passed around
    // during the initialization phase.
    int r = fnEntry(__argc, __argv);

    if (r != 0) {
        MessageBox(NULL, (LPCTSTR) "Engine entry return indicated failure.", TEXT("Error"), MB_OK);
        return r;
    }

    FreeLibrary(mod);
    return 0;
}