#include <windows.h>
#include <stdio.h>
#include <stdint.h>
#include <cstring>
#include "../brand/var.h"

static bool running = true;
static int width = 800;
static int height = 600;

static int wOffset = 16;
static int hOffset = 39;

static HANDLE framebufferMap = nullptr;
static uint32_t* framebuffer = nullptr;
static BITMAPINFO bitmapInfo;

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
        case WM_CLOSE:
        case WM_DESTROY:
            running = false;
            PostQuitMessage(0);
            return 0;

        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);

            StretchDIBits(
                hdc,
                0, 0, width, height,
                0, 0, width, height,
                framebuffer,
                &bitmapInfo,
                DIB_RGB_COLORS,
                SRCCOPY
            );

            EndPaint(hwnd, &ps);
            return 0;
        }
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}

bool initFramebuffer()
{
    size_t size = width * height * sizeof(uint32_t);

    framebufferMap = CreateFileMappingA(
        INVALID_HANDLE_VALUE,
        NULL,
        PAGE_READWRITE,
        0,
        (DWORD)size,
        "KatziFramebuffer"
    );

    if (!framebufferMap)
        return false;

    framebuffer = (uint32_t*)MapViewOfFile(
        framebufferMap,
        FILE_MAP_ALL_ACCESS,
        0,
        0,
        size
    );

    return framebuffer != nullptr;
}

bool startRenderer()
{
    STARTUPINFOA si = {};
    PROCESS_INFORMATION pi = {};

    si.cb = sizeof(si);

    char cmd[256];

    snprintf(
        cmd,
        sizeof(cmd),
        "build/tab.exe KatziFramebuffer %d %d",
        width,
        height
    );

    HANDLE hJob = CreateJobObjectA(NULL, NULL);
    if (!hJob)
        return false;

    JOBOBJECT_EXTENDED_LIMIT_INFORMATION info = {};
    info.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;

    if (!SetInformationJobObject(
            hJob,
            JobObjectExtendedLimitInformation,
            &info,
            sizeof(info)))
    {
        CloseHandle(hJob);
        return false;
    }

    BOOL ok = CreateProcessA(
        NULL,
        cmd,
        NULL,
        NULL,
        FALSE,
        0,
        NULL,
        NULL,
        &si,
        &pi
    );

    if (!ok)
    {
        CloseHandle(hJob);
        return false;
    }

    if (!AssignProcessToJobObject(hJob, pi.hProcess))
    {
        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);
        CloseHandle(hJob);
        return false;
    }

    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);

    static HANDLE gJob = nullptr;
    gJob = hJob;

    return true;
}

int gui()
{
    bitmapInfo.bmiHeader.biSize = sizeof(bitmapInfo.bmiHeader);
    bitmapInfo.bmiHeader.biWidth = width;
    bitmapInfo.bmiHeader.biHeight = -height;
    bitmapInfo.bmiHeader.biPlanes = 1;
    bitmapInfo.bmiHeader.biBitCount = 32;
    bitmapInfo.bmiHeader.biCompression = BI_RGB;

    if (!initFramebuffer())
        return 1;

    if (!startRenderer())
        return 1;

    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int)
{
    WNDCLASSA wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "Katzi";

    RegisterClassA(&wc);

    HWND hwnd = CreateWindowExA(
        0,
        wc.lpszClassName,
        "Katzi",
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        width+wOffset,
        height+hOffset,
        nullptr,
        nullptr,
        hInstance,
        nullptr
    );

    if (gui())
        return 1;

    MSG msg;

    while (running)
    {
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        InvalidateRect(hwnd, nullptr, FALSE);
        Sleep(16);
    }

    UnmapViewOfFile(framebuffer);
    CloseHandle(framebufferMap);

    return 0;
}