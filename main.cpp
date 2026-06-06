#include <windows.h>
#include <shellapi.h>
#include <commctrl.h>
#include <cstdio>

#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "kernel32.lib")
#pragma comment(lib, "comctl32.lib")

// ===== 开启 Win7+ 视觉样式（关键）=====
#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

HWND hProgress = NULL;
HWND hLabel = NULL;
HWND hCancel = NULL;
bool g_cancelled = false;

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_COMMAND:
            if (LOWORD(wParam) == 1001) {
                g_cancelled = true;
                DestroyWindow(hwnd);
            }
            break;
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

bool CreateCountdownWindow() {
    const char CLASS_NAME[] = "CountdownWindow";

    INITCOMMONCONTROLSEX icex;
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC = ICC_PROGRESS_CLASS;
    InitCommonControlsEx(&icex);

    WNDCLASS wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(
        0, CLASS_NAME, "打开文件",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,
        CW_USEDEFAULT, CW_USEDEFAULT, 350, 150,
        NULL, NULL, GetModuleHandle(NULL), NULL
    );

    if (!hwnd) return false;

    hLabel = CreateWindow("STATIC", "将在 5 秒后打开 Eaglercraft_IR_1.8.8.html...",
        WS_CHILD | WS_VISIBLE | SS_CENTER,
        20, 20, 300, 20, hwnd, NULL, NULL, NULL);

    // Win7+ 风格进度条
    hProgress = CreateWindowEx(
        0, PROGRESS_CLASS, NULL,
        WS_CHILD | WS_VISIBLE | PBS_SMOOTH,
        20, 50, 300, 20, hwnd, NULL, NULL, NULL
    );

    SendMessage(hProgress, PBM_SETRANGE, 0, MAKELPARAM(0, 100));
    SendMessage(hProgress, PBM_SETPOS, 0, 0);

    hCancel = CreateWindow("BUTTON", "取消",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        140, 80, 70, 25, hwnd, (HMENU)1001, NULL, NULL);

    ShowWindow(hwnd, SW_SHOWNORMAL);
    UpdateWindow(hwnd);

    for (int i = 5; i > 0; i--) {
        char label[128];
        sprintf(label, "将在 %d 秒后打开 Eaglercraft_IR_1.8.8.html...", i);
        SetWindowText(hLabel, label);
        SendMessage(hProgress, PBM_SETPOS, (5 - i) * 20, 0);

        MSG msg;
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        if (g_cancelled) return false;
        Sleep(1000);
    }

    SendMessage(hProgress, PBM_SETPOS, 100, 0);
    DestroyWindow(hwnd);
    return true;
}

void OpenFile() {
    const char* filePath = "Eaglercraft_IR_1.8.8.html";

    if (!CreateCountdownWindow()) {
        MessageBoxA(NULL, "已取消打开文件", "提示", MB_OK | MB_ICONINFORMATION);
        return;
    }

    if (GetFileAttributesA(filePath) == INVALID_FILE_ATTRIBUTES) {
        char errMsg[128];
        sprintf(errMsg, "找不到文件: %s\\n请确保文件在当前目录下！", filePath);
        MessageBoxA(NULL, errMsg, "错误", MB_ICONERROR | MB_OK);
        return;
    }

    char cmd[512];
    sprintf(cmd, "start \"\" \"%s\"", filePath);
    system(cmd);
}

int main() {
    OpenFile();
    return 0;
}
