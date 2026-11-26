#include "include/cef_app.h"

namespace FTM {
    struct GlobalRefs {
        CefRefPtr<CefClient> client;
        CefBrowserSettings settings;
        CefRefPtr<CefDictionaryValue> extra_info = nullptr;
        CefRefPtr<CefRequestContext> request_context = nullptr;
    };

    void InitalizationLogic(
            CefRefPtr<CefClient> client,
            const CefBrowserSettings & settings
        );

    void New(std::string url);
}
