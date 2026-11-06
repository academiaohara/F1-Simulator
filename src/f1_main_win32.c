#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef RGB

#include <stdio.h>
#include <string.h>

#include "f1_ui.h"

static HWND window_handle;
static BITMAPINFO bitmap_info;
static int client_w = 1280, client_h = 720, scale = 2, offset_x, offset_y;
static F1Ui ui;
static F1UiScreen screen = F1_UI_HUB;
static int previous_speed = 4;
static char save_path[MAX_PATH] = "partida.sav";

static LRESULT CALLBACK window_proc(HWND window, UINT message, WPARAM wparam, LPARAM lparam) {
    (void)window;
    if (message == WM_SIZE) { client_w = LOWORD(lparam); client_h = HIWORD(lparam); return 0; }
    if (message == WM_KEYDOWN) {
        if (wparam >= '1' && wparam <= '4') screen = (F1UiScreen)(wparam - '1');
        if (wparam == 'R') f1_ui_set_replay(&ui, F1_EVENT_OVERTAKE, "LEC ADELANTA A NOR POR EL INTERIOR", 150);
        if (wparam == VK_F5) f1_ui_save(&ui, save_path);
        if (wparam == VK_F9) f1_ui_load(&ui, save_path);
        if (screen == F1_UI_HUB && wparam == VK_OEM_4) f1_ui_cycle_team(&ui, -1);
        if (screen == F1_UI_HUB && wparam == VK_OEM_6) f1_ui_cycle_team(&ui, 1);
        if (screen == F1_UI_DEVELOPMENT && wparam == 'N')
            f1_ui_start_project(&ui, F1_COMPONENT_FLOOR);
        if (screen == F1_UI_DEVELOPMENT && wparam == VK_RETURN)
            for (int slot = 0; slot < F1_CAR_PROJECTS; ++slot)
                if (ui.car.project[slot].complete) { f1_car_install_project(&ui.car, slot); break; }
        if (screen == F1_UI_RACE && wparam == VK_SPACE) {
            if (ui.race.speed == 0) f1_ui_set_speed(&ui, previous_speed);
            else { previous_speed = ui.race.speed; f1_ui_set_speed(&ui, 0); }
        }
        if (screen == F1_UI_RACE && wparam == VK_RIGHT)
            f1_ui_set_speed(&ui, ui.race.speed < 1 ? 1 : ui.race.speed * 2);
        if (screen == F1_UI_RACE && wparam == VK_LEFT)
            f1_ui_set_speed(&ui, ui.race.speed <= 1 ? 0 : ui.race.speed / 2);
        if (screen == F1_UI_RACE && wparam == 'P')
            f1_race_request_pit(&ui.race, ui.selected_car, ui.race.car[ui.selected_car].requested_compound);
        if (screen == F1_UI_RACE && wparam == VK_TAB)
            f1_ui_select_other_car(&ui);
        if (screen == F1_UI_RACE && wparam == 'S')
            ui.race.car[ui.selected_car].requested_compound = F1_TYRE_SOFT;
        if (screen == F1_UI_RACE && wparam == 'M')
            ui.race.car[ui.selected_car].requested_compound = F1_TYRE_MEDIUM;
        if (screen == F1_UI_RACE && wparam == 'H')
            ui.race.car[ui.selected_car].requested_compound = F1_TYRE_HARD;
        if (screen == F1_UI_RACE && wparam == 'I')
            ui.race.car[ui.selected_car].requested_compound = F1_TYRE_INTERMEDIATE;
        if (screen == F1_UI_RACE && wparam == 'W')
            ui.race.car[ui.selected_car].requested_compound = F1_TYRE_WET;
        if (screen == F1_UI_RACE && wparam == 'Z')
            f1_ui_set_selected_strategy(&ui, F1_PACE_CONSERVE, ui.race.car[ui.selected_car].ers_mode);
        if (screen == F1_UI_RACE && wparam == 'X')
            f1_ui_set_selected_strategy(&ui, F1_PACE_STANDARD, ui.race.car[ui.selected_car].ers_mode);
        if (screen == F1_UI_RACE && wparam == 'C')
            f1_ui_set_selected_strategy(&ui, F1_PACE_PUSH, ui.race.car[ui.selected_car].ers_mode);
        if (screen == F1_UI_RACE && wparam == 'V')
            f1_ui_set_selected_strategy(&ui, ui.race.car[ui.selected_car].pace, F1_ERS_HARVEST);
        if (screen == F1_UI_RACE && wparam == 'B')
            f1_ui_set_selected_strategy(&ui, ui.race.car[ui.selected_car].pace, F1_ERS_BALANCED);
        if (screen == F1_UI_RACE && wparam == 'N')
            f1_ui_set_selected_strategy(&ui, ui.race.car[ui.selected_car].pace, F1_ERS_DEPLOY);
        if (wparam == VK_ESCAPE) PostMessage(window_handle, WM_CLOSE, 0, 0);
        return 0;
    }
    if (message == WM_CLOSE) { DestroyWindow(window_handle); return 0; }
    if (message == WM_DESTROY) { PostQuitMessage(0); return 0; }
    if (message == WM_ERASEBKGND) return 1;
    return DefWindowProcA(window, message, wparam, lparam);
}

static void present(void) {
    HDC dc = GetDC(window_handle);
    int sy = client_h / SH;
    int dw, dh;
    scale = client_w / SW;
    if (sy < scale) scale = sy;
    if (scale < 1) scale = 1;
    dw = SW * scale; dh = SH * scale;
    offset_x = (client_w - dw) / 2; offset_y = (client_h - dh) / 2;
    if (offset_x < 0) offset_x = 0;
    if (offset_y < 0) offset_y = 0;
    PatBlt(dc, 0, 0, client_w, client_h, BLACKNESS);
    SetStretchBltMode(dc, COLORONCOLOR);
    StretchDIBits(dc, offset_x, offset_y, dw, dh, 0, 0, SW, SH, fb,
                  &bitmap_info, DIB_RGB_COLORS, SRCCOPY);
    ReleaseDC(window_handle, dc);
}

static int screenshot_mode(const char *arguments) {
    char path[MAX_PATH] = "f1_preview.png";
    char view[32] = "hub";
    sscanf(arguments, "%259s %31s", path, view);
    if (strcmp(view, "car") == 0) screen = F1_UI_CAR;
    else if (strcmp(view, "development") == 0) screen = F1_UI_DEVELOPMENT;
    else if (strcmp(view, "race") == 0) screen = F1_UI_RACE;
    gfx_init(); f1_ui_init(&ui);
    if (screen == F1_UI_RACE) f1_ui_set_replay(&ui, F1_EVENT_OVERTAKE, "LEC ADELANTA A NOR", 120);
    f1_ui_render(&ui, screen, screen == F1_UI_CAR ? 270 : -1, screen == F1_UI_CAR ? 130 : -1);
    png_save(path, 2);
    return 0;
}

int WINAPI WinMain(HINSTANCE instance, HINSTANCE previous, LPSTR arguments, int show) {
    WNDCLASSA wc;
    RECT bounds = {0, 0, 1280, 720};
    MSG message;
    LARGE_INTEGER frequency, last;
    (void)previous;
    {
        char *slash;
        GetModuleFileNameA(NULL, save_path, MAX_PATH);
        slash = strrchr(save_path, '\\');
        if (slash) strcpy(slash + 1, "partida.sav");
    }
    if (arguments && strstr(arguments, "--shot") == arguments)
        return screenshot_mode(arguments + 6);
    memset(&wc, 0, sizeof wc);
    wc.lpfnWndProc = window_proc;
    wc.hInstance = instance;
    wc.lpszClassName = "F1PixelManager";
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    RegisterClassA(&wc);
    AdjustWindowRect(&bounds, WS_OVERLAPPEDWINDOW, FALSE);
    window_handle = CreateWindowA(wc.lpszClassName, "F1 Pixel Manager",
        WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT,
        bounds.right - bounds.left, bounds.bottom - bounds.top,
        NULL, NULL, instance, NULL);
    if (!window_handle) return 1;
    ShowWindow(window_handle, show);
    memset(&bitmap_info, 0, sizeof bitmap_info);
    bitmap_info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bitmap_info.bmiHeader.biWidth = SW;
    bitmap_info.bmiHeader.biHeight = -SH;
    bitmap_info.bmiHeader.biPlanes = 1;
    bitmap_info.bmiHeader.biBitCount = 32;
    bitmap_info.bmiHeader.biCompression = BI_RGB;
    gfx_init();
    f1_ui_init(&ui);
    f1_ui_set_speed(&ui, 4);
    QueryPerformanceFrequency(&frequency);
    QueryPerformanceCounter(&last);
    for (;;) {
        while (PeekMessageA(&message, NULL, 0, 0, PM_REMOVE)) {
            if (message.message == WM_QUIT) return 0;
            TranslateMessage(&message);
            DispatchMessageA(&message);
        }
        {
            LARGE_INTEGER now;
            POINT cursor;
            float delta;
            QueryPerformanceCounter(&now);
            delta = (float)(now.QuadPart - last.QuadPart) / (float)frequency.QuadPart;
            last = now;
            if (delta > 0.1f) delta = 0.1f;
            if (screen == F1_UI_RACE) f1_ui_tick(&ui, delta);
            GetCursorPos(&cursor);
            ScreenToClient(window_handle, &cursor);
            f1_ui_render(&ui, screen, (cursor.x - offset_x) / scale, (cursor.y - offset_y) / scale);
        }
        present();
        Sleep(16);
    }
}
