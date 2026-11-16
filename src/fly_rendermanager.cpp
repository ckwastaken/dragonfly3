#include <fcntl.h>
#include <string>
#include <iostream>
#include <sys/utsname.h>
#include "include/cef_app.h"
#include "include/cef_render_process_handler.h"
#include "include/cef_v8.h"
#include "include/cef_base.h"
#include "fly_toast.h"
#include "buildconfigs.h"

inline bool dangerousApisAvailable = true;

class V8Handler : public CefV8Handler {
public:
    bool Execute(const CefString& name,
                 CefRefPtr<CefV8Value> object,
                 const CefV8ValueList& arguments,
                 CefRefPtr<CefV8Value>& retval,
                 CefString& exception) override {
        CefRefPtr<CefV8Context> context = CefV8Context::GetCurrentContext();
        CefRefPtr<CefFrame> frame = context->GetFrame();

        if (!frame) {
            showToast("no frame");
            return true;
        }

        if (name == "version_major") {
            retval = CefV8Value::CreateInt(configs::major);
            return true;
        }

        if (name == "version_minor") {
            retval = CefV8Value::CreateInt(configs::minor);
            return true;
        }

        if (name == "version_patch") {
            retval = CefV8Value::CreateInt(configs::patch);
            return true;
        }

        if (name == "version_kernel") {
            struct utsname buffer;
            if (uname(&buffer) == 0) {
                retval = CefV8Value::CreateString(buffer.release);
                return true;
            } else {
                perror("uname");
                return false;
            }
        }

        if (dangerousApisAvailable == false) {
            showToast("dangerous apis are not availible on this page");
            return true;
        }

        if (name == "termWrite") {
            if (arguments.size() == 1 && arguments[0]->IsString()) {
                CefRefPtr<CefProcessMessage> msg = CefProcessMessage::Create("terminalviewd:write");
                CefRefPtr<CefListValue> args = msg->GetArgumentList();
                args->SetString(1, arguments[0]->GetStringValue());
                frame->SendProcessMessage(PID_BROWSER, msg);
                return true;
            }
            exception = "Expected one TEXT.";
            return true;
        }

        if (name == "termSize") {
            if (arguments.size() == 2 && arguments[0]->IsInt() && arguments[1]->IsInt()) {
                CefRefPtr<CefProcessMessage> msg = CefProcessMessage::Create("terminalviewd:size");
                CefRefPtr<CefListValue> args = msg->GetArgumentList();
                args->SetInt(0, arguments[0]->GetIntValue());
                args->SetInt(1, arguments[1]->GetIntValue());
                frame->SendProcessMessage(PID_BROWSER, msg);
                return true;
            }
            exception = "Expected two INT";
            return true;
        }

        if (name == "termOpen") {
            CefRefPtr<CefProcessMessage> msg = CefProcessMessage::Create("terminalviewd:open");

            CefRefPtr<CefListValue> args = msg->GetArgumentList();
            args->SetString(3, "/bin/bash");
            args->SetInt(1, 80); // cols
            args->SetInt(2, 24); // rows

            frame->SendProcessMessage(PID_BROWSER, msg);

            return true;
        }
        return false;
    }

private:
    IMPLEMENT_REFCOUNTING(V8Handler);
};

class fly_rendermanager : public CefApp, public CefRenderProcessHandler {
    public:
        fly_rendermanager() = default;

        void OnContextCreated(CefRefPtr<CefBrowser> browser,
                              CefRefPtr<CefFrame> frame,
                              CefRefPtr<CefV8Context> context) override {
            // Retrieve the frame's `window` object
            CefRefPtr<CefV8Value> window = context->GetGlobal();

            // Create a V8 handler and function
            CefRefPtr<CefV8Handler> handler = new V8Handler();

            // Optional: attach it to a namespace object (e.g., `window.myApp.add`)
            CefRefPtr<CefV8Value> fly_private_api_obj = CefV8Value::CreateObject(nullptr, nullptr);
            window->SetValue("fly_DANGEROUS_APIS_BE_CAREFUL", fly_private_api_obj, V8_PROPERTY_ATTRIBUTE_NONE);

            //**
            //** THESE APIS ARE CONSIDERED DANGEROUS... USE THEM WITH CAUTION
            //**
            if (dangerousApisAvailable) { // dangerous apis availible
                CefRefPtr<CefV8Value> termWrite = CefV8Value::CreateFunction("termWrite", handler);
                CefRefPtr<CefV8Value> termOpen = CefV8Value::CreateFunction("termOpen", handler);
                CefRefPtr<CefV8Value> termSize = CefV8Value::CreateFunction("termSize", handler);

                fly_private_api_obj->SetValue("termWrite", termWrite, V8_PROPERTY_ATTRIBUTE_NONE);
                fly_private_api_obj->SetValue("termOpen", termOpen, V8_PROPERTY_ATTRIBUTE_NONE);
                fly_private_api_obj->SetValue("termSize", termSize, V8_PROPERTY_ATTRIBUTE_NONE);
            }

            //**
            //** THESE APIS ARE CONSIDERED SAFE... USE THEM WITH (less) CAUTION
            //**
            CefRefPtr<CefV8Value> fly_api_obj = CefV8Value::CreateObject(nullptr, nullptr);
            window->SetValue("fly", fly_api_obj, V8_PROPERTY_ATTRIBUTE_NONE);

            CefRefPtr<CefV8Value> version_major = CefV8Value::CreateFunction("version_major", handler);
            fly_api_obj->SetValue("version_major", version_major, V8_PROPERTY_ATTRIBUTE_NONE);
            CefRefPtr<CefV8Value> version_minor = CefV8Value::CreateFunction("version_minor", handler);
            fly_api_obj->SetValue("version_minor", version_minor, V8_PROPERTY_ATTRIBUTE_NONE);
            CefRefPtr<CefV8Value> version_patch = CefV8Value::CreateFunction("version_patch", handler);
            fly_api_obj->SetValue("version_patch", version_patch, V8_PROPERTY_ATTRIBUTE_NONE);
            CefRefPtr<CefV8Value> version_kernel = CefV8Value::CreateFunction("version_kernel", handler);
            fly_api_obj->SetValue("version_kernel", version_kernel, V8_PROPERTY_ATTRIBUTE_NONE);
        }

    private:
        IMPLEMENT_REFCOUNTING(fly_rendermanager);
        DISALLOW_COPY_AND_ASSIGN(fly_rendermanager);
};
