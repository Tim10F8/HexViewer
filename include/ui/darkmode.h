#ifndef DARKMODE_H
#define DARKMODE_H

#ifdef _WIN32
#include <windows.h>
#include <dwmapi.h>
#pragma comment(lib, "dwmapi.lib")
#endif

#ifdef __APPLE__
#include <AppKit/AppKit.h>
#endif

#ifdef __linux__
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <cstring>
#endif

#ifdef _WIN32

inline void ApplyDarkTitleBar(HWND hWnd, bool dark)
{
    if (!hWnd)
        return;

    BOOL value = dark ? TRUE : FALSE;

    DwmSetWindowAttribute(hWnd, 19, &value, sizeof(value));

    DwmSetWindowAttribute(hWnd, 20, &value, sizeof(value));
}

inline void ApplyDarkMenu(HMENU hMenu, bool dark)
{
    if (!hMenu)
        return;

    MENUINFO mi;
    mi.cbSize = sizeof(mi);
    mi.fMask = MIM_BACKGROUND | MIM_APPLYTOSUBMENUS;

    if (dark)
    {
        mi.hbrBack = CreateSolidBrush(RGB(32, 32, 32));
    }
    else
    {
        mi.hbrBack = (HBRUSH)(COLOR_MENU + 1);
    }

    SetMenuInfo(hMenu, &mi);
}

#endif

#ifdef __APPLE__

inline void ApplyDarkTitleBar(NSWindow* window, bool dark)
{
    if (!window) return;
    NSAppearanceName name = dark ? NSAppearanceNameDarkAqua : NSAppearanceNameAqua;
    [window setAppearance:[NSAppearance appearanceNamed:name]];
    [window setTitleVisibility:NSWindowTitleVisible];
    [window setTitlebarAppearsTransparent:NO];
}

inline void ApplyDarkMenu(NSMenu* menu, bool dark)
{
    if (!menu) return;
    NSAppearanceName name = dark ? NSAppearanceNameDarkAqua : NSAppearanceNameAqua;
    NSAppearance* appearance = [NSAppearance appearanceNamed:name];
    [NSApp setAppearance:appearance];
    if ([menu respondsToSelector:@selector(setAppearance:)]) {
        [menu setAppearance:appearance];
    }
}

#endif

#ifdef __linux__

inline void ApplyDarkTitleBar(Display* dpy, Window win, bool dark)
{
    Atom atom = XInternAtom(dpy, "_GTK_THEME_VARIANT", False);
    if (atom == None) return;

    if (dark) {
        const char* value = "dark";
        XChangeProperty(
            dpy,
            win,
            atom,
            XA_STRING,
            8,
            PropModeReplace,
            (unsigned char*)value,
            (int)strlen(value) + 1
        );
    }
    else {
        XDeleteProperty(dpy, win, atom);
    }
}

inline void ApplyDarkMenu(void*, bool) {}

#endif

#endif
