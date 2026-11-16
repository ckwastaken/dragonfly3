#include <iostream>

#include "fly_toast.h"
#include "include/cef_display_handler.h"
// Created by mizhan on 11/10/26.

class fly_displayhandler : public CefDisplayHandler {
    public:
        fly_displayhandler() = default;

        // CefDisplayHandler method: intercept console logs
        bool OnConsoleMessage(CefRefPtr<CefBrowser> browser,
                              cef_log_severity_t level,
                              const CefString& message,
                              const CefString& source,
                              int line) override {

            CefString m = message.ToString();
            if (m.ToString().starts_with(":::toast:::")) {
                showToast(m.ToString().erase(0, 11), 2);
            }

            if (level >= LOGSEVERITY_ERROR) {
                showToast(m, 2);
            }

            return false;
        }
    private:
        IMPLEMENT_REFCOUNTING(fly_displayhandler);
        DISALLOW_COPY_AND_ASSIGN(fly_displayhandler);
};