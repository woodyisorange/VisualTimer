
//TODO: Set the icon for the exe file, taskbar, window etc.
//TODO: Custom window? e.g. no title bar?
//TODO: Use UpdateLayeredWindow() to have custom alpha?
//TODO: Mouse input:
//      - Scroll to add/remove time?
//      - Shift+Scroll to set transparency?
//      - Drag to move?

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

#define TAU 6.2831855f

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <string.h>
#include <stdio.h>
#include <math.h>

#define MS_PER_SECOND 1000
#define MS_PER_MINUTE 60000
#define MS_PER_HOUR 3600000

void DisplayError(HWND Window, const char* Title, uint32 ErrorCode)
{
    LPSTR ErrorMessage = NULL;

    FormatMessageA(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
        null, // Source
        ErrorCode,
        0, // LanguageId
        (LPTSTR)&ErrorMessage,
        0, // Buffer Size
        null); // Arguments

    MessageBoxA(
        Window,
        ErrorMessage,
        Title,
        MB_OK | MB_ICONEXCLAMATION);

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
    const char* ProgramName = "Visual Timer";

    uint64 MillisecondsTotal = 90 * 60 * 1000;
    uint64 MillisecondsRemaining = MillisecondsTotal;

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
        DisplayError(null, ProgramName, ErrorCode);
        return ErrorCode;
    }

    HWND Window = CreateWindowExA(
        WS_EX_LAYERED,
        WindowClassName,
        ProgramName,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, // X
        CW_USEDEFAULT, // Y
        256, // Width
        256, // Height
        null, // Parent
        null, // Menu
        Instance,
        null); // User Parameter

    if (Window == null)
    {
        uint32 ErrorCode = GetLastError();
        DisplayError(Window, ProgramName, ErrorCode);
        return ErrorCode;
    }

    float WindowAlpha = 0.333f;
    SetLayeredWindowAttributes(
        Window,
        0, // Key Colour (unused)
        WindowAlpha * 255,
        LWA_ALPHA);

    ShowWindow(Window, SW_NORMAL);

    bool8 IsRunning = true;
    uint32 ExitCode = 0;
    uint64 OldMilliseconds = GetTickCount64();

    while (IsRunning)
    {
        MSG Message;
        while (PeekMessageA(&Message, null, 0, 0, PM_REMOVE))
        {
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

        uint64 NewMilliseconds = GetTickCount();
        uint64 DeltaMilliseconds = NewMilliseconds - OldMilliseconds;
        OldMilliseconds = NewMilliseconds;

        if (MillisecondsRemaining > 0)
        {
            // Cap timer to double digit hours
            uint64 MaxMilliseconds = (MS_PER_HOUR * 100) - MS_PER_SECOND;
            if (MillisecondsRemaining > MaxMilliseconds)
            {
                MillisecondsRemaining = MaxMilliseconds;
            }

            if (DeltaMilliseconds < MillisecondsRemaining)
            {
                MillisecondsRemaining -= DeltaMilliseconds;
            }
            else
            {
                MillisecondsRemaining = 0;
                MessageBoxA(
                    Window,
                    "The time has come.",
                    ProgramName,
                    MB_OK);
            }
        }

        RECT ClientRect;
        GetClientRect(Window, &ClientRect);
        uint32 ClientWidth = ClientRect.right - ClientRect.left;
        uint32 ClientHeight = ClientRect.bottom - ClientRect.top;

        uint32 CenterX = ClientWidth / 2;
        uint32 CenterY = ClientHeight / 2;

        uint32 ClientRadius = (ClientWidth < ClientHeight) ? (ClientWidth/2) : (ClientHeight/2);

        HDC DeviceContext = GetDC(Window);

        COLORREF ClearColour = 0x00FFFFFF;
        COLORREF TimerColour = 0x000000FF;

        HGDIOBJ OriginalBrush = SelectObject(DeviceContext, GetStockObject(DC_BRUSH));
        COLORREF OriginalDcBrushColour = SetDCBrushColor(DeviceContext, ClearColour);
        HGDIOBJ OriginalPen = SelectObject(DeviceContext, GetStockObject(DC_PEN));
        COLORREF OriginalDcPenColour = SetDCPenColor(DeviceContext, ClearColour);

        FillRect(DeviceContext, &ClientRect, GetStockObject(DC_BRUSH));

        uint32 CircleRadius = ClientRadius * 0.9f;
        uint32 InnerCircleRadius = ClientRadius * 0.5f;

        float PercentRemaining = (float)MillisecondsRemaining / (float)MillisecondsTotal;
        float StartAngleRadians = (TAU * -PercentRemaining) - (TAU * 0.5f);

        float CircleStartX = sinf(StartAngleRadians) * CircleRadius + CenterX;
        float CircleStartY = cosf(StartAngleRadians) * CircleRadius + CenterY;

        SetDCBrushColor(DeviceContext, TimerColour);
        SetDCPenColor(DeviceContext, ClearColour);
        Pie(
            DeviceContext,
            CenterX - CircleRadius,
            CenterY - CircleRadius,
            CenterX + CircleRadius,
            CenterY + CircleRadius,
            CircleStartX,
            CircleStartY,
            ClientWidth / 2,
            0);

        SetDCBrushColor(DeviceContext, ClearColour);
        SetDCPenColor(DeviceContext, ClearColour);
        Ellipse(
            DeviceContext,
            CenterX - InnerCircleRadius,
            CenterY - InnerCircleRadius,
            CenterX + InnerCircleRadius,
            CenterY + InnerCircleRadius);

        uint64 SecondsRemaining = (MillisecondsRemaining % MS_PER_MINUTE) / MS_PER_SECOND;
        uint64 MinutesRemaining = (MillisecondsRemaining % MS_PER_HOUR) / MS_PER_MINUTE;
        uint64 HoursRemaining = MillisecondsRemaining / MS_PER_HOUR;

        char Text[16];
        sprintf_s(
            Text,
            sizeof(Text),
            "%02llu:%02llu:%02llu",
            HoursRemaining,
            MinutesRemaining,
            SecondsRemaining);

        uint32 TextLength = strlen(Text);

        SIZE TextSize;
        TextSize.cx = 0;
        TextSize.cy = 0;

        GetTextExtentPoint32A(DeviceContext, Text, TextLength, &TextSize);

        TextOutA(
            DeviceContext,
            CenterX - (TextSize.cx/2),
            CenterY - (TextSize.cy/2),
            Text,
            TextLength);

        SetDCBrushColor(DeviceContext, OriginalDcBrushColour);
        SelectObject(DeviceContext, OriginalBrush);
        SetDCPenColor(DeviceContext, OriginalDcPenColour);
        SelectObject(DeviceContext, OriginalPen);
        ReleaseDC(Window, DeviceContext);

        Sleep(50);
    }

    if (ExitCode != 0)
    {
        DisplayError(Window, ProgramName, ExitCode);
    }
    return ExitCode;
}

