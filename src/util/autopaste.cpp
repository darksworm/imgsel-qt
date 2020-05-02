#include <vector>

#ifdef WIN32
#include <windows.h>

void sendPaste() {
    std::vector<INPUT> inputs;
    std::vector<WORD> scanCodes {
        0x1d, // left ctrl
        0x2f  // v
    };

    for (auto scanCode : scanCodes) {
        INPUT ip;

        ip.type = INPUT_KEYBOARD;
        ip.ki.time = 0;
        ip.ki.wVk = 0; //We're doing scan codes instead
        ip.ki.dwExtraInfo = 0;

        //This let's you do a hardware scan instead of a virtual keypress
        ip.ki.dwFlags = KEYEVENTF_SCANCODE;
        ip.ki.wScan = scanCode;  

        inputs.emplace_back(ip);
    }

    // KeyDown events
    for (auto input : inputs) {
        SendInput(1, &input, sizeof(INPUT));
    }

    // KeyUp events
    for (auto input : inputs) {
        input.ki.dwFlags = KEYEVENTF_SCANCODE | KEYEVENTF_KEYUP;
        SendInput(1, &input, sizeof(INPUT));
    }
}
#elif defined WITH_X11
#include <X11/Xlib.h>
#include <X11/keysym.h>

XKeyEvent createKeyEvent(Display *display, Window &win,
                         Window &winRoot, bool press,
                         int keycode, int modifiers) {
    XKeyEvent event;

    event.display = display;
    event.window = win;
    event.root = winRoot;
    event.subwindow = None;
    event.time = CurrentTime;
    event.x = 1;
    event.y = 1;
    event.x_root = 1;
    event.y_root = 1;
    event.same_screen = True;
    event.keycode = XKeysymToKeycode(display, keycode);
    event.state = modifiers;
    if (press) {
        event.type = KeyPress;
    } else {
        event.type = KeyRelease;
    }

    return event;
}

void sendPaste() {
    // Obtain the X11 display.
    Display *display = XOpenDisplay(0);
    if (display == NULL) {
        return;
    }

    // Get the root window for the current display.
    Window winRoot = XDefaultRootWindow(display);

    // Find the window which has the current keyboard focus.
    Window winFocus;
    int revert;
    XGetInputFocus(display, &winFocus, &revert);

    // Send a fake key press event to the window.
    XKeyEvent event = createKeyEvent(display, winFocus, winRoot, true, XK_v, ControlMask);
    XSendEvent(event.display, event.window, True, KeyPressMask, (XEvent *) &event);

    // Send a fake key press event to the window.
    event = createKeyEvent(display, winFocus, winRoot, false, XK_v, ControlMask);
    XSendEvent(event.display, event.window, True, KeyPressMask, (XEvent *) &event);

    XCloseDisplay(display);
}
#else 
void sendPaste() {
    // sorry bud, no paste for you ;(
}
#endif
