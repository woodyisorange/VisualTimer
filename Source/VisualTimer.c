
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

typedef float float32;
static_assert(sizeof(float32) == 4, "Bad type size");

typedef double float64;
static_assert(sizeof(float64) == 8, "Bad type size");

#define true 1
#define false 0
#define null 0

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

struct
{
    const char* ProgramName;
    HWND MainWindow;
    uint32 MainWindowWidth;
    uint32 MainWindowHeight;
    float32 MainWindowAlpha;
} Globals;

typedef struct
{
    union
    {
        struct
        {
            uint8 Red;
            uint8 Green;
            uint8 Blue;
            uint8 Alpha;
        };
        uint32 Raw;
    };
} rgba32;

rgba32 MakeColour(uint8 Red, uint8 Green, uint8 Blue, uint8 Alpha)
{
    rgba32 Colour;
    Colour.Red = Red;
    Colour.Green = Green;
    Colour.Blue = Blue;
    Colour.Alpha = Alpha;
    return Colour;
}

void DisplayErrorMessage(const char* ErrorMessage)
{
    MessageBoxA(
        Globals.MainWindow,
        ErrorMessage,
        Globals.ProgramName,
        MB_OK |
        MB_ICONEXCLAMATION);
};

void DisplayErrorCode(uint32 ErrorCode)
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

    DisplayErrorMessage(ErrorMessage);

    LocalFree(ErrorMessage);
};

LRESULT WindowProcedure(
    HWND Window,
    UINT Message,
    WPARAM WordParameter,
    LPARAM LongParameter)
{
	LRESULT Result = 0;

    if (Globals.MainWindow && Window != Globals.MainWindow)
    {
        DisplayErrorMessage("Unexpected Window!");
        PostQuitMessage(0);
        return Result;
    }

    switch (Message)
    {
        case WM_PAINT:
        {
            PAINTSTRUCT Paint;
            HDC DeviceContext = BeginPaint(Window, &Paint);
            {
                RECT WindowRect;
                GetClientRect(Globals.MainWindow, &WindowRect);
                Globals.MainWindowWidth = WindowRect.right - WindowRect.left;
                Globals.MainWindowHeight = WindowRect.bottom - WindowRect.top;

                rgba32 RectColour = MakeColour(255, 0, 0, 0);
                HBRUSH RectBrush = CreateSolidBrush(RectColour.Raw);
                FillRect(DeviceContext, &WindowRect, RectBrush);
            }
            EndPaint(Window, &Paint);
        } break;

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
    ZeroMemory(&Globals, sizeof(Globals));

    Globals.ProgramName = "Visual Timer";

    const char* WindowClassName = "VisualTimerMainWindow";

    WNDCLASSEXA WindowClass = { 0 };
    WindowClass.cbSize = sizeof(WNDCLASSEXA);
    WindowClass.style = CS_HREDRAW | CS_VREDRAW;
    WindowClass.lpfnWndProc = &WindowProcedure;
    WindowClass.hInstance = Instance;
    WindowClass.lpszClassName = WindowClassName;
    WindowClass.hCursor = LoadCursor(null, IDC_ARROW);

    if (RegisterClassExA(&WindowClass) == 0)
    {
        uint32 ErrorCode = GetLastError();
        DisplayErrorCode(ErrorCode);
        return ErrorCode;
    }

    Globals.MainWindowWidth = 512;
    Globals.MainWindowHeight = 512;

    Globals.MainWindow = CreateWindowExA(
        WS_EX_LAYERED,
        WindowClassName,
        Globals.ProgramName,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, // X
        CW_USEDEFAULT, // Y
        Globals.MainWindowWidth,
        Globals.MainWindowHeight,
        null, // Parent
        null, // Menu
        Instance,
        null); // User Parameter

    if (Globals.MainWindow == null)
    {
        uint32 ErrorCode = GetLastError();
        DisplayErrorCode(ErrorCode);
        return ErrorCode;
    }

    Globals.MainWindowAlpha = 0.5f;

    SetLayeredWindowAttributes(
        Globals.MainWindow,
        0, // Key Colour (unused)
        Globals.MainWindowAlpha * 255,
        LWA_ALPHA);

    ShowWindow(Globals.MainWindow, SW_NORMAL);

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
            DisplayErrorCode(ErrorCode);
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
        DisplayErrorCode(ExitCode);
    }
    return ExitCode;
}

