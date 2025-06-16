/*
 * UI for MechCommander No-CD Patcher. 
 * Only for Windows 10 and newer.
 */

#include <windows.h>
#include <commctrl.h>
#include <shlobj.h>
#include <stdio.h>
#include <shobjidl.h>  // For IFileDialog
#include <objbase.h>   // For COM functionality
#include "patch.h"     // For patch function

// Windows 10 DPI awareness - needed for the DPI awareness context values
#ifndef _DPI_AWARENESS_CONTEXTS_
#define DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2  ((DPI_AWARENESS_CONTEXT)-4)
#endif

// Control IDs
#define ID_TEXT_PATH 101
#define ID_BUTTON_BROWSE 102
#define ID_BUTTON_PATCH 103
#define ID_STATIC_LABEL 104
#define ID_STATIC_DESCRIPTION 105
#define ID_STATIC_STATUS 106

// Global handles
HWND g_hwndTextPath = NULL;
HWND g_hwndButtonPatch = NULL;
HWND g_hwndStatusText = NULL;
BOOL g_statusTextError = FALSE;  // Track if status text should be red

// Forward declarations for all functions
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
static void InitializeCommonControls(void);
static void EnableDPIAwareness(void);
static int ScaleForDpi(int value, int dpi);
static void BrowseForFolder(HWND hwndOwner);
static void CreateControls(HWND hwnd, HINSTANCE hInstance);

// Function to initialize common controls
static void InitializeCommonControls(void) {
    INITCOMMONCONTROLSEX icc;
    icc.dwSize = sizeof(icc);
    icc.dwICC = ICC_WIN95_CLASSES;
    InitCommonControlsEx(&icc);
}

// Helper function to enable DPI awareness for Windows 10
static void EnableDPIAwareness(void) {
    // Get SetProcessDpiAwarenessContext function
    HMODULE hUser32 = GetModuleHandle(TEXT("user32.dll"));
    if (hUser32) {
        typedef BOOL (*SetProcessDpiAwarenessContextFunc)(DPI_AWARENESS_CONTEXT);
        SetProcessDpiAwarenessContextFunc setProcessDpiAwarenessContext = 
            (SetProcessDpiAwarenessContextFunc)GetProcAddress(hUser32, "SetProcessDpiAwarenessContext");

        if (setProcessDpiAwarenessContext) {
            // Try to set Per-Monitor V2 DPI awareness (Windows 10 1703+)
            if (setProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2)) {
                return;  // Success with V2 awareness
            }
            
            // If V2 failed, try V1 (earlier Windows 10)
            setProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE);
        }
    }
}

// Helper function to scale value based on DPI
static int ScaleForDpi(int value, int dpi) {
    return (value * dpi) / 96; // 96 is the default DPI
}

// Application entry point
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // Set DPI awareness as early as possible
    EnableDPIAwareness();    InitializeCommonControls();
    
    const char CLASS_NAME[] = "MechCommanderNoCDPatch";
    
    WNDCLASS wc = {0};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    
    RegisterClass(&wc);
    HWND hwnd = CreateWindowEx(        0,                          // Optional window styles
        CLASS_NAME,                 // Window class
        "MechCommander No-CD Patcher",// Window title
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,        // Window style - non-resizable
        
        // Size and position
        CW_USEDEFAULT, CW_USEDEFAULT, 450, 200,
        
        NULL,       // Parent window    
        NULL,       // Menu
        hInstance,  // Instance handle
        NULL        // Additional application data
    );
      if (hwnd == NULL) {
        return 0;
    }
      CreateControls(hwnd, hInstance);
    ShowWindow(hwnd, nCmdShow);
    
    MSG msg = {0};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    
    return (int)msg.wParam;
}

// Function to open a folder browser dialog using IFileDialog
static void BrowseForFolder(HWND hwndOwner) {
    HRESULT hr;
    IFileDialog *pfd = NULL;
    IShellItem *psiResult = NULL;
    PWSTR pszFilePath = NULL;
    char szFolder[MAX_PATH] = "";
      hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    
    if (SUCCEEDED(hr)) hr = CoCreateInstance(&CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER, 
                         &IID_IFileDialog, (void**)&pfd);
    
    FILEOPENDIALOGOPTIONS options;
    if (SUCCEEDED(hr)) hr = pfd->lpVtbl->GetOptions(pfd, &options);
    if (SUCCEEDED(hr)) hr = pfd->lpVtbl->SetOptions(pfd, options | FOS_PICKFOLDERS);
    
    if (SUCCEEDED(hr)) hr = pfd->lpVtbl->SetTitle(pfd, L"Select Game Folder");
    if (SUCCEEDED(hr)) hr = pfd->lpVtbl->Show(pfd, hwndOwner);
    if (SUCCEEDED(hr)) hr = pfd->lpVtbl->GetResult(pfd, &psiResult);
    if (SUCCEEDED(hr)) hr = psiResult->lpVtbl->GetDisplayName(psiResult, SIGDN_FILESYSPATH, &pszFilePath);
      if (SUCCEEDED(hr)) {
        WideCharToMultiByte(CP_ACP, 0, pszFilePath, -1, szFolder, MAX_PATH, NULL, NULL);
        SetWindowText(g_hwndTextPath, szFolder);
        EnableWindow(g_hwndButtonPatch, TRUE);
        CoTaskMemFree(pszFilePath);
    }
    if (psiResult) psiResult->lpVtbl->Release(psiResult);
    if (pfd) pfd->lpVtbl->Release(pfd);
    if (SUCCEEDED(hr)) CoUninitialize();
}

// Function to create and position controls based on DPI
static void CreateControls(HWND hwnd, HINSTANCE hInstance) {
    // Get DPI setting for this window using Windows 10 API directly
    int dpi = (int)GetDpiForWindow(hwnd);
    if (dpi == 0) dpi = 96; // Fallback to default DPI if the API fails
    
    char currentPath[MAX_PATH] = "";
    if (g_hwndTextPath) {
        GetWindowText(g_hwndTextPath, currentPath, MAX_PATH);
    }
    
    BOOL patchEnabled = g_hwndButtonPatch ? IsWindowEnabled(g_hwndButtonPatch) : FALSE;
    
    if (g_hwndTextPath) DestroyWindow(g_hwndTextPath);
    if (g_hwndButtonPatch) DestroyWindow(g_hwndButtonPatch);
    if (g_hwndStatusText) DestroyWindow(g_hwndStatusText);
    DestroyWindow(GetDlgItem(hwnd, ID_STATIC_LABEL));
    DestroyWindow(GetDlgItem(hwnd, ID_BUTTON_BROWSE));
    DestroyWindow(GetDlgItem(hwnd, ID_STATIC_DESCRIPTION));
    int xDesc = ScaleForDpi(20, dpi);
    int yDesc = ScaleForDpi(20, dpi);
    int descWidth = ScaleForDpi(400, dpi);
    int descHeight = ScaleForDpi(60, dpi);
    
    // Path label position and size (shifted down to make room for description)
    int x1 = ScaleForDpi(20, dpi);
    int y1 = yDesc + descHeight + ScaleForDpi(20, dpi);
    int labelWidth = ScaleForDpi(150, dpi);
    int labelHeight = ScaleForDpi(20, dpi);
    
    int x2 = ScaleForDpi(20, dpi);
    int y2 = y1 + labelHeight + ScaleForDpi(5, dpi);
    int editWidth = ScaleForDpi(320, dpi);
    int editHeight = ScaleForDpi(25, dpi);
    
    int x3 = ScaleForDpi(350, dpi);
    int y3 = y2;
    int browseWidth = ScaleForDpi(80, dpi);
    int browseHeight = ScaleForDpi(25, dpi);
    
    int x4 = ScaleForDpi(150, dpi);
    int y4 = y2 + editHeight + ScaleForDpi(20, dpi);
    int patchWidth = ScaleForDpi(150, dpi);
    int patchHeight = ScaleForDpi(30, dpi);    CreateWindowEx(
        0, "STATIC", 
        "Patch MechCommander to not require the CD to be inserted.\r\n"
        "Use at your own risk!\r\n"
        "(Original executable will not be modified.)",
        WS_CHILD | WS_VISIBLE | SS_CENTER,
        xDesc, yDesc, descWidth, descHeight,
        hwnd, (HMENU)ID_STATIC_DESCRIPTION, hInstance, NULL);

    CreateWindowEx(
        0, "STATIC", "Game Folder:", 
        WS_CHILD | WS_VISIBLE,
        x1, y1, labelWidth, labelHeight,
        hwnd, (HMENU)ID_STATIC_LABEL, hInstance, NULL);
        
    g_hwndTextPath = CreateWindowEx(
        WS_EX_CLIENTEDGE, "EDIT", currentPath, 
        WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
        x2, y2, editWidth, editHeight,
        hwnd, (HMENU)ID_TEXT_PATH, hInstance, NULL);
        
    CreateWindowEx(
        0, "BUTTON", "Browse...", 
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        x3, y3, browseWidth, browseHeight,
        hwnd, (HMENU)ID_BUTTON_BROWSE, hInstance, NULL);    g_hwndButtonPatch = CreateWindowEx(
        0, "BUTTON", "Patch", 
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | (patchEnabled ? 0 : WS_DISABLED),
        x4, y4, patchWidth, patchHeight,
        hwnd, (HMENU)ID_BUTTON_PATCH, hInstance, NULL);
    
    // Add status text area below controls
    int x5 = ScaleForDpi(20, dpi);
    int y5 = y4 + patchHeight + ScaleForDpi(20, dpi);
    int statusWidth = ScaleForDpi(400, dpi);
    int statusHeight = ScaleForDpi(25, dpi);
    
    // Store status text if it exists
    char statusText[512] = "";
    if (g_hwndStatusText) {
        GetWindowText(g_hwndStatusText, statusText, sizeof(statusText));
    }
    
    g_hwndStatusText = CreateWindowEx(
        0, "STATIC", statusText,
        WS_CHILD | WS_VISIBLE | SS_LEFT,
        x5, y5, statusWidth, statusHeight,
        hwnd, (HMENU)ID_STATIC_STATUS, hInstance, NULL);
    
    int minClientWidth = x3 + browseWidth + ScaleForDpi(20, dpi);  // Browse button right edge + padding
    int minClientHeight = y5 + statusHeight + ScaleForDpi(20, dpi); // Status text bottom edge + padding
    
    RECT windowRect = {0, 0, minClientWidth, minClientHeight};
    AdjustWindowRect(&windowRect, 
                     WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, 
                     FALSE);
    
    SetWindowPos(hwnd, NULL, 0, 0, 
                 windowRect.right - windowRect.left,
                 windowRect.bottom - windowRect.top,
                 SWP_NOMOVE | SWP_NOZORDER);
    InvalidateRect(hwnd, NULL, TRUE);
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    // Handler for DPI change
    case WM_DPICHANGED: {
        // lParam contains the suggested new rect for the window
        RECT* prcNewWindow = (RECT*)lParam;
        SetWindowPos(hwnd, NULL, 
                     prcNewWindow->left, prcNewWindow->top,
                     prcNewWindow->right - prcNewWindow->left, 
                     prcNewWindow->bottom - prcNewWindow->top,
                     SWP_NOZORDER | SWP_NOACTIVATE);
          // Create controls with new DPI
        CreateControls(hwnd, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE));
        return 0;
    }
        
    case WM_COMMAND: {
        // Get the ID of the control that sent the message
        int controlId = LOWORD(wParam);
          switch (controlId) {
            case ID_BUTTON_BROWSE:
                BrowseForFolder(hwnd);
                return 0;
                  case ID_BUTTON_PATCH:
                {
                    char path[MAX_PATH];
                    GetWindowText(g_hwndTextPath, path, MAX_PATH);
                      // Call patch function
                    const char *result = patch(path);
                    
                    // Update status text
                    if (result == NULL) {
                        // Success
                        SetWindowText(g_hwndStatusText, "Game patched successfully!");
                        g_statusTextError = FALSE;  // Not an error
                    } else {
                        // Error
                        SetWindowText(g_hwndStatusText, result);
                        g_statusTextError = TRUE;   // Show as error
                    }
                    // Force redraw to update text color
                    InvalidateRect(g_hwndStatusText, NULL, TRUE);
                }
                return 0;
        }
        break;
    }
      case WM_PAINT: {
        PAINTSTRUCT ps;
        BeginPaint(hwnd, &ps);
        EndPaint(hwnd, &ps);
        return 0;
    }
    
    case WM_CTLCOLORSTATIC: {
        HWND ctrlHwnd = (HWND)lParam;
        HDC hdc = (HDC)wParam;
        
        // Check if this is our status text and should be red
        if (ctrlHwnd == g_hwndStatusText && g_statusTextError) {
            SetTextColor(hdc, RGB(255, 0, 0));  // Red text
            SetBkMode(hdc, TRANSPARENT);         // Transparent background
            return (LRESULT)GetStockObject(NULL_BRUSH);
        }
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    
    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    
    return 0;
}
