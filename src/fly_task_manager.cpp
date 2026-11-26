//
// Created by may on 11/26/26.
//

#include "fly_task_manager.h"

#include <filesystem>

#include "buildconfigs.h"
#include "include/cef_app.h"
#include "include/cef_browser.h"
#include "include/wrapper/cef_helpers.h"

namespace FTM {
    const std::string url = "file:" + std::filesystem::current_path().string() + "/../test_files/";

    CefRefPtr<CefClient> G_client;
    CefBrowserSettings G_settings;
    CefRefPtr<CefDictionaryValue> G_extra_info = nullptr;
    CefRefPtr<CefRequestContext> G_request_context = nullptr;

    void InitalizationLogic(
        CefRefPtr<CefClient> client,
        const CefBrowserSettings & settings
    ) {
        G_client = client;
        G_settings = settings;
    }

    void New(std::string url) {

        CefWindowInfo window_info;
        //window_info.runtime_style = CEF_RUNTIME_STYLE_ALLOY;
        window_info.bounds = CefRect(0, 0, 414, 848);
        CefString(&window_info.window_name) = "device - " +
            configs::make + " " + configs::model
        + " (" + std::to_string(configs::major) + "."
        + std::to_string(configs::minor) + "."
        + std::to_string(configs::patch) + ")";

        CefBrowserHost::CreateBrowser(
            window_info, G_client, url, G_settings, nullptr, nullptr);
    }
}
