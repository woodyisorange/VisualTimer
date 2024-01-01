
//TODO: Arrow key input
//TODO: Use UpdateLayeredWindow() to have custom alpha?
//TODO: Mouse input: Drag to move
//TODO: Once drag-to-move, remove title bar
//TODO: Understand resource files and rc.exe so that:
//TODO: Set the icon for the exe file, taskbar, window etc.
//TODO: Help text?
//TODO: Type in remaining time?
//TODO: (later) Smoother custom drawing?
//TODO: (later) OpenGl to replace GDI?

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

#define TICK_TIMER_ID 1

#define MAX(A, B) (((A) > (B)) ? (A) : (B))
#define MIN(A, B) (((A) < (B)) ? (A) : (B))
#define CLAMP(A, Min, Max) MAX(MIN(A, Max), Min)

struct globals
{
    uint64 MillisecondsTotal;
    uint64 MillisecondsRemaining;
    float32 WindowAlpha;
} Globals;

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
}

LRESULT WindowProcedure(
    HWND Window,
    UINT Message,
    WPARAM WordParameter,
    LPARAM LongParameter)
{
	LRESULT Result = 0;

    switch (Message)
    {
        case WM_MOUSEWHEEL:
        {
            bool8 IsShifted = GET_KEYSTATE_WPARAM(WordParameter) & MK_SHIFT;
            int32 Delta = GET_WHEEL_DELTA_WPARAM(WordParameter) / WHEEL_DELTA;

            if (IsShifted)
            {
                Globals.WindowAlpha += 0.1f * Delta;
                Globals.WindowAlpha = CLAMP(Globals.WindowAlpha, 0.1f, 1.0f);
                SetLayeredWindowAttributes(
                    Window,
                    0, // Key Colour (unused)
                    (uint8)(Globals.WindowAlpha * 255.0f),
                    LWA_ALPHA);
            }
            else
            {
                Delta *= MS_PER_MINUTE;

                // Add to wherever the timer has got to
                Globals.MillisecondsTotal = Globals.MillisecondsRemaining;

                if (Delta < 0 && (Globals.MillisecondsTotal < -Delta))
                {
                    Globals.MillisecondsTotal = 0;
                }
                else
                {
                    Globals.MillisecondsTotal += Delta;

                    // Round off to the nearest minute + a second from which the next tick will be
                    // subtracted, displaying the round minute
                    Globals.MillisecondsTotal -= Globals.MillisecondsTotal % MS_PER_MINUTE;
                    Globals.MillisecondsTotal += MS_PER_SECOND;
                }

                // Restart the clock
                Globals.MillisecondsRemaining = Globals.MillisecondsTotal;
            }
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

    //
    // Setup + Create Window
    //

    const char* ProgramName = "Visual Timer";

    memset(&Globals, 0, sizeof(Globals));

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
        WS_EX_LAYERED | WS_EX_TOPMOST,
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

    Globals.WindowAlpha += 0.4f;
    SetLayeredWindowAttributes(
        Window,
        0, // Key Colour (unused)
        (uint8)(Globals.WindowAlpha * 255.0f),
        LWA_ALPHA);

    ShowWindow(Window, SW_NORMAL);

    HBITMAP BackBuffer = null;
    uint32 BackBufferWidth = 0;
    uint32 BackBufferHeight = 0;

    //
    // Main Loop
    //

    bool8 IsRunning = true;
    int32 ExitCode = 0;
    uint64 OldMilliseconds = GetTickCount64();

    // Use a timer to tick our main loop by triggering a WM_TIMER message
    SetTimer(Window, TICK_TIMER_ID, MS_PER_SECOND/10, (TIMERPROC)null);

    while (IsRunning)
    {
        //
        // Pump Win32 Events
        //
        {
            MSG Message;
            while (PeekMessageA(&Message, null, 0, 0, PM_REMOVE))
            {
                if (Message.message == WM_QUIT)
                {
                    IsRunning = false;
                    ExitCode = (int32)Message.wParam;
                }
                else
                {
                    TranslateMessage(&Message);
                    DispatchMessage(&Message);
                }
            }
        }

        //
        // Tick timer
        //
        {
            uint64 NewMilliseconds = GetTickCount();
            uint64 DeltaMilliseconds = NewMilliseconds - OldMilliseconds;
            OldMilliseconds = NewMilliseconds;

            if (Globals.MillisecondsRemaining > 0)
            {
                // Cap timer to double digit hours
                uint64 MaxMilliseconds = (MS_PER_HOUR * 100) - MS_PER_SECOND;
                if (Globals.MillisecondsRemaining > MaxMilliseconds)
                {
                    Globals.MillisecondsRemaining = MaxMilliseconds;
                }

                if (DeltaMilliseconds < Globals.MillisecondsRemaining)
                {
                    Globals.MillisecondsRemaining -= DeltaMilliseconds;
                }
                else
                {
                    Globals.MillisecondsRemaining = 0;
                    MessageBoxA(
                        Window,
                        "The time has come.",
                        ProgramName,
                        MB_OK);
                }
            }
        }

        //
        // Redraw Window
        //
        {
            COLORREF ClearColour = 0x00FFFFFF;
            COLORREF TimerColour = 0x000000FF;

            RECT ClientRect;
            GetClientRect(Window, &ClientRect);
            uint32 ClientWidth = ClientRect.right - ClientRect.left;
            uint32 ClientHeight = ClientRect.bottom - ClientRect.top;

            uint32 CenterX = ClientWidth / 2;
            uint32 CenterY = ClientHeight / 2;

            uint32 ClientRadius = MIN(ClientWidth, ClientHeight) / 2;

            HDC WindowDeviceContext = GetDC(Window);

            if (ClientWidth != BackBufferWidth || ClientHeight != BackBufferHeight)
            {
                if (BackBuffer)
                {
                    DeleteObject(BackBuffer);
                }

                BackBufferWidth = ClientWidth;
                BackBufferHeight = ClientHeight;
                BackBuffer = CreateCompatibleBitmap(
                    WindowDeviceContext,
                    BackBufferWidth,
                    BackBufferHeight);

                if (!BackBuffer)
                {
                    uint32 ErrorCode = GetLastError();
                    DisplayError(Window, ProgramName, ErrorCode);
                    return ErrorCode;
                }
            }

            HDC BackBufferDeviceContext = CreateCompatibleDC(WindowDeviceContext);

            HBITMAP OriginalBitmap = SelectObject(BackBufferDeviceContext, BackBuffer);

            HGDIOBJ OriginalBrush = SelectObject(
                BackBufferDeviceContext,
                GetStockObject(DC_BRUSH));

            COLORREF OriginalDcBrushColour = SetDCBrushColor(
                BackBufferDeviceContext,
                ClearColour);

            HGDIOBJ OriginalPen = SelectObject(
                BackBufferDeviceContext,
                GetStockObject(DC_PEN));

            COLORREF OriginalDcPenColour = SetDCPenColor(
                BackBufferDeviceContext,
                ClearColour);

            FillRect(BackBufferDeviceContext, &ClientRect, GetStockObject(DC_BRUSH));

            int32 CircleRadius = (int32)((float32)ClientRadius * 0.9f);
            int32 InnerCircleRadius = (int32)((float32)ClientRadius * 0.5f);

            float32 PercentRemaining = 1.0f;
            if (Globals.MillisecondsTotal > 0)
            {
                PercentRemaining =
                    (float32)Globals.MillisecondsRemaining / (float32)Globals.MillisecondsTotal;
            }
            float32 StartAngleRadians = (TAU * -PercentRemaining) - (TAU * 0.5f);

            float32 CircleStartX = sinf(StartAngleRadians) * CircleRadius + CenterX;
            float32 CircleStartY = cosf(StartAngleRadians) * CircleRadius + CenterY;

            SetDCBrushColor(BackBufferDeviceContext, TimerColour);
            SetDCPenColor(BackBufferDeviceContext, ClearColour);
            Pie(
                BackBufferDeviceContext,
                CenterX - CircleRadius,
                CenterY - CircleRadius,
                CenterX + CircleRadius,
                CenterY + CircleRadius,
                (int32)CircleStartX,
                (int32)CircleStartY,
                ClientWidth / 2,
                0);

            SetDCBrushColor(BackBufferDeviceContext, ClearColour);
            SetDCPenColor(BackBufferDeviceContext, ClearColour);
            Ellipse(
                BackBufferDeviceContext,
                CenterX - InnerCircleRadius,
                CenterY - InnerCircleRadius,
                CenterX + InnerCircleRadius,
                CenterY + InnerCircleRadius);

            uint64 SecondsRemaining =
                (Globals.MillisecondsRemaining % MS_PER_MINUTE) / MS_PER_SECOND;

            uint64 MinutesRemaining =
                (Globals.MillisecondsRemaining % MS_PER_HOUR) / MS_PER_MINUTE;

            uint64 HoursRemaining = Globals.MillisecondsRemaining / MS_PER_HOUR;

            char Text[16];
            sprintf_s(
                Text,
                sizeof(Text),
                "%02llu:%02llu:%02llu",
                HoursRemaining,
                MinutesRemaining,
                SecondsRemaining);

            int32 TextLength = (int32)strlen(Text);

            SIZE TextSize;
            TextSize.cx = 0;
            TextSize.cy = 0;

            GetTextExtentPoint32A(BackBufferDeviceContext, Text, TextLength, &TextSize);

            TextOutA(
                BackBufferDeviceContext,
                CenterX - (TextSize.cx/2),
                CenterY - (TextSize.cy/2),
                Text,
                TextLength);

            BitBlt(
                WindowDeviceContext,
                0,
                0,
                BackBufferWidth,
                BackBufferHeight,
                BackBufferDeviceContext,
                0,
                0,
                SRCCOPY);

            SetDCBrushColor(BackBufferDeviceContext, OriginalDcBrushColour);
            SelectObject(BackBufferDeviceContext, OriginalBrush);

            SetDCPenColor(BackBufferDeviceContext, OriginalDcPenColour);
            SelectObject(BackBufferDeviceContext, OriginalPen);

            OriginalBitmap = SelectObject(BackBufferDeviceContext, OriginalBitmap);

            DeleteDC(BackBufferDeviceContext);

            ReleaseDC(Window, WindowDeviceContext);
        }

        WaitMessage();
    }

    if (ExitCode != 0)
    {
        DisplayError(Window, ProgramName, ExitCode);
    }
    return ExitCode;
}

