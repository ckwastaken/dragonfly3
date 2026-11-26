#include <filesystem>
#include <string>

#include "include/cef_app.h"
#include "include/cef_browser.h"
#include "include/cef_version_info.h"
#include "include/wrapper/cef_helpers.h"

#include "fly_gui.h"
#include "fly_task_manager.h"

#include <thread>

#include "salt.h"
#include <unistd.h>
CefRefPtr<CefBrowser> g_browser;

void load(const std::string& url) {
    if (g_browser && g_browser->GetMainFrame()) {
        g_browser->GetMainFrame()->LoadURL(url);
    }
}

class CefAppHandler : public CefApp, public CefBrowserProcessHandler, public CefRenderProcessHandler {
public:
    CefAppHandler() = default;

    CefRefPtr<CefBrowserProcessHandler> GetBrowserProcessHandler() override {
        return this;
    }

    CefRefPtr<CefRenderProcessHandler> GetRenderProcessHandler() override {
        return new fly_rendermanager();
    }

    void OnBeforeCommandLineProcessing(
        const CefString& process_type,
        CefRefPtr<CefCommandLine> command_line) override {
        command_line->AppendSwitchWithValue("enable-features", "OverlayScrollbar");
        command_line->AppendSwitchWithValue("disable-features", "GtkUi");
        command_line->AppendSwitch("force-dark-mode");
        command_line->AppendSwitch("enable-do-not-track");
        command_line->AppendSwitch("force-devtools-available");
        //command_line->AppendSwitch("kiosk");
        command_line->AppendSwitchWithValue("force-device-scale-factor","1");
        command_line->AppendSwitchWithValue("ozone-platform", "wayland");
        command_line->AppendSwitchWithValue("apps-gallery-url", configs::galleryat);
    }

    void OnContextInitialized() override {
        CEF_REQUIRE_UI_THREAD();

        CefRefPtr<fly> client(new fly());

        CefBrowserSettings browser_settings;

        const std::string font = "Nimbus Sans";
        CefString(&browser_settings.standard_font_family) = font;
        CefString(&browser_settings.sans_serif_font_family) = font;
        CefString(&browser_settings.serif_font_family) = font;

        FTM::InitalizationLogic(client, browser_settings);
        FTM::New("https://google.com");
    }

private:
    IMPLEMENT_REFCOUNTING(CefAppHandler);
};

int startux(int argc, char* argv[]) {

    //setenv("GTK_THEME","",1);

    const std::string cache_dir = "/tmp/cache/cef-v" + std::to_string(cef_version_info(0));
    std::filesystem::create_directories(cache_dir);

    CefMainArgs main_args(argc, argv);
    CefRefPtr<CefAppHandler> app(new CefAppHandler());

    int exit_code = CefExecuteProcess(main_args, app, nullptr);
    if (exit_code >= 0) {
        return exit_code;
    }

    CefSettings settings;
    CefString(&settings.chrome_policy_id) = std::filesystem::current_path().string();
    CefString(&settings.root_cache_path) = cache_dir;
    settings.no_sandbox = false; // true is bad for security but useful during testing

    if (!CefInitialize(main_args, settings, app, nullptr)) {
        return 1;
    }

    CefRunMessageLoop();
    CefShutdown();
    return 0;

}
