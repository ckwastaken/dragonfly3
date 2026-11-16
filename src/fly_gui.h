#pragma once
#include "include/cef_app.h"
#include "include/cef_browser.h"
#include "include/cef_client.h"
#include "include/cef_life_span_handler.h"
#include "include/cef_load_handler.h"
#include "include/cef_request_handler.h"          // 1. Add this header
#include "include/cef_resource_request_handler.h"
#include "include/wrapper/cef_helpers.h"
#include "include/cef_version_info.h"
#include <filesystem>

#include "fly_resourcemanager.cpp"
#include "fly_processmessage.h"
#include "fly_displayhandler.cpp"
#include "fly_rendermanager.cpp"
#include "keybinds.h"
#include "include/cef_render_process_handler.h"

void load(const std::string& url);
int startux(int argc, char* argv[]);

// 2. Add public CefRequestHandler to inheritance list
class fly : public CefClient,
            public CefLifeSpanHandler,
            public CefLoadHandler,
            public CefRequestHandler,
            public CefResourceRequestHandler,
            public CefDisplayHandler,
            public CefRenderProcessHandler,
            public CefKeyboardHandler {
public:
    fly() = default;

    CefRefPtr<CefResourceRequestHandler> GetResourceRequestHandler(
    CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefFrame> frame,
    CefRefPtr<CefRequest> request,
    bool is_navigation,
    bool is_download,
    const CefString& request_initiator,
    bool& disable_default_handling) override {
        if (frame->GetURL().ToString().starts_with("devtools:")) return nullptr;
        return new fly_resourcemanager();
    }

    bool OnProcessMessageReceived(CefRefPtr<CefBrowser> browser,
                              CefRefPtr<CefFrame> frame,
                              CefProcessId source_process,
                              CefRefPtr<CefProcessMessage> message) override {

        std::string message_name = message->GetName();
        CefRefPtr<CefListValue> args = message->GetArgumentList();

        return process_message(message_name, args, frame);
    }

    CefRefPtr<CefDisplayHandler> GetDisplayHandler() override {
        return new fly_displayhandler();
    }

    CefRefPtr<CefKeyboardHandler> GetKeyboardHandler() override {
        return this;
    }

    bool OnPreKeyEvent(CefRefPtr<CefBrowser> browser,
                     const CefKeyEvent& event,
                     CefEventHandle os_event,
                     bool* is_keyboard_shortcut) override {
        return keybinds::onpress(browser, event);
    }

    CefRefPtr<CefLoadHandler> GetLoadHandler() override {
        return this;
    }

    // 3. Register CefRequestHandler with CefClient
    CefRefPtr<CefRequestHandler> GetRequestHandler() override {
        return this;
    }

    // Lifecycle & Load methods
    void OnAfterCreated(CefRefPtr<CefBrowser> browser) override;


    // This is a problem area someone will need to fix later (probably jerry)

    CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() override {
        return this;
    }

    bool DoClose(CefRefPtr<CefBrowser> browser) override {
        return false;
    }

    void OnBeforeClose(CefRefPtr<CefBrowser> browser) override {
        CefQuitMessageLoop();
    }

    // Problem area end

    void OnLoadStart(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, TransitionType transition_type) override;

    // Resource Request method
    ReturnValue OnBeforeResourceLoad(
        CefRefPtr<CefBrowser> browser,
        CefRefPtr<CefFrame> frame,
        CefRefPtr<CefRequest> request,
        CefRefPtr<CefCallback> callback) override;

private:
    CefRefPtr<CefBrowser> browser_;

    IMPLEMENT_REFCOUNTING(fly);
};