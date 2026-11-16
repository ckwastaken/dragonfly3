#include "keybinds.h"

#include "fly_gui.h"
#include "include/cef_browser.h"
#include "include/internal/cef_types_wrappers.h"
#include "include/views/cef_browser_view.h"
#include "include/views/cef_window.h"

namespace keybinds {
    bool onpress(CefRefPtr<CefBrowser> browser, CefKeyEvent e) {
        if (e.type != KEYEVENT_KEYUP) return false;

        bool ctrl  = (e.modifiers & EVENTFLAG_CONTROL_DOWN) != 0;
        bool shift = (e.modifiers & EVENTFLAG_SHIFT_DOWN) != 0;
        //bool alt   = (e.modifiers & EVENTFLAG_ALT_DOWN) != 0;

        if (ctrl && shift && e.windows_key_code == '8') {
            CefWindowInfo window_info;
            CefBrowserSettings settings;

#if defined(OS_WIN)
            window_info.SetAsPopup(browser->GetHost()->GetWindowHandle(), "DevTools");
#endif

            browser->GetHost()->ShowDevTools(window_info, new fly(), settings, CefPoint());
            return true;
        }
        return false;
    };
}
