
//TODO: Set the icon for the exe file, taskbar, window etc.

typedef signed char int8;
static_assert(sizeof(int8) == 1, "Bad type size");

typedef signed short int int16;
static_assert(sizeof(int16) == 2, "Bad type size");

typedef signed int int32;
static_assert(sizeof(int32) == 4, "Bad type size");

typedef signed long long int int64;
static_assert(sizeof(int64) == 8, "Bad type size");

typedef unsigned char uint8;
static_assert(sizeof(uint8) == 1, "Bad type size");

typedef unsigned short int uint16;
static_assert(sizeof(uint16) == 2, "Bad type size");

typedef unsigned int uint32;
static_assert(sizeof(uint32) == 4, "Bad type size");

typedef unsigned long long int uint64;
static_assert(sizeof(uint64) == 8, "Bad type size");

typedef _Bool bool8;
static_assert(sizeof(bool8) == 1, "Bad type size");

#define true 1
#define false 0
#define null 0

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

static const char* GlobalProgramName = "Visual Timer";

void DisplayError(uint32 ErrorCode)
{
    LPSTR ErrorMessage = NULL;

    FormatMessageA(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM,
        null, // Source
        ErrorCode,
        0, // LanguageId
        (LPTSTR)&ErrorMessage,
        0, // Buffer Size
        null); // Arguments

    MessageBoxA(
        null, // Window
        ErrorMessage,
        GlobalProgramName,
        MB_OK |
        MB_ICONEXCLAMATION);

    LocalFree(ErrorMessage);
};

LRESULT WindowProcedure(
    HWND Window,
    UINT Message,
    WPARAM WordParameter,
    LPARAM LongParameter)
{
	LRESULT Result = 0;

    switch (Message)
    {
        case WM_CLOSE:
        case WM_DESTROY:
        {
			PostQuitMessage(0);
        } break;

		default:
		{
			Result = DefWindowProc(
                Window,
                Message,
                WordParameter,
                LongParameter);
		} break;
    }

    return Result;
}

int32 WinMain(
    HINSTANCE Instance,
    HINSTANCE PreviousInstance,
    LPSTR CommandLine,
    int32 ShowCommand)
{
    const char* WindowClassName = "VisualTimerMainWindow";

    WNDCLASSEXA WindowClass = { 0 };
    WindowClass.cbSize = sizeof(WNDCLASSEXA);
    WindowClass.lpfnWndProc = &WindowProcedure;
    WindowClass.hInstance = Instance;
    WindowClass.lpszClassName = WindowClassName;
    WindowClass.hCursor = LoadCursor(null, IDC_ARROW);

    if (RegisterClassExA(&WindowClass) == 0)
    {
        uint32 ErrorCode = GetLastError();
        DisplayError(ErrorCode);
        return ErrorCode;
    }

    uint32 WindowWidth = 512;
    uint32 WindowHeight = 512;

    HWND Window = CreateWindowExA(
        0, // Extendend Style
        WindowClassName,
        GlobalProgramName,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, // X
        CW_USEDEFAULT, // Y
        WindowWidth,
        WindowHeight,
        null, // Parent
        null, // Menu
        Instance,
        null); // User Parameter

    if (Window == null)
    {
        uint32 ErrorCode = GetLastError();
        DisplayError(ErrorCode);
        return ErrorCode;
    }

    ShowWindow(Window, SW_NORMAL);

    bool8 IsRunning = true;
    uint32 ExitCode = 0;

    while (IsRunning)
    {
        MSG Message;
        int8 MessageResult = GetMessage(
            &Message,
            null, // Window
            0, // Min Message Filter
            0); // Max Message Filter

        if (MessageResult == -1)
        {
            uint32 ErrorCode = GetLastError();
            DisplayError(ErrorCode);
            return ErrorCode;
        }

        if (Message.message == WM_QUIT)
        {
            IsRunning = false;
            ExitCode = Message.wParam;
        }
        else
        {
            TranslateMessage(&Message);
            DispatchMessage(&Message);
        }
    }

    if (ExitCode != 0)
    {
        DisplayError(ExitCode);
    }
    return ExitCode;
}
