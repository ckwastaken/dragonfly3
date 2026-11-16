#include <iostream>
#include <strings.h>
#include "fly_gui.h"
#include "global.h"
#include "include/cef_app.h"
#include "include/wrapper/cef_helpers.h"
#include "include/cef_resource_request_handler.h"
#include "include/cef_version.h"
#include "salt.h"

//std::string ua = "Mozilla/5.0 (X11; Linux; Pixel 8 Pro) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/" + std::to_string(CHROME_VERSION_MAJOR) + ".0.0.0 Safari/537.36";

std::string ua = "Mozilla/5.0 (X11; Linux; " + salt::mask() + ") AppleWebKit/537.36 (KHTML, like Gecko) Chrome/" + std::to_string(CHROME_VERSION_MAJOR) + ".0.0.0 Safari/537.36";

/* NOTE:                                   /\/\/\/\/\/\
                                           | | | | | |
            salt::mask returns a placeholder, frequently replace this value with a random companies other
    flagship, but dont use the same brand twice in a row. this is an important part of the anti fingerprinting
*/

CefResourceRequestHandler::ReturnValue fly::OnBeforeResourceLoad(
    CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefFrame> frame,
    CefRefPtr<CefRequest> request,
    CefRefPtr<CefCallback> callback) {

    // Ensure we are running on the CEF IO thread
    CEF_REQUIRE_IO_THREAD();

    std::string url = request->GetURL().ToString();

    // Example 2: Inject a custom HTTP header
    CefRequest::HeaderMap headers;
    request->GetHeaderMap(headers);
    headers.insert(std::make_pair("User-Agent", ua));

    headers.insert(std::make_pair("DNT", "1"));
    // READ https://developer.mozilla.org/en-US/docs/Web/HTTP/Reference/Headers/DNT

    headers.insert(std::make_pair("Sec-GPC", "1"));
    // READ https://developer.mozilla.org/en-US/docs/Web/HTTP/Reference/Headers/Sec-GPC

    request->SetHeaderMap(headers);

    return RV_CONTINUE;
}

void OverrideUserAgentAndHints(CefRefPtr<CefBrowser> browser) {
   if (!browser) return;

    CefRefPtr<CefDictionaryValue> params = CefDictionaryValue::Create();

    // 1. Set User-Agent and navigator.platform
    params->SetString("userAgent", ua);
    params->SetString("platform", "dragonfly");

    // 2. Helper lambda to build brand dictionaries
    auto createBrand = [](const std::string& brand, const std::string& version) {
        CefRefPtr<CefDictionaryValue> dict = CefDictionaryValue::Create();
        dict->SetString("brand", brand);
        dict->SetString("version", version);
        return dict;
    };

    // 3. Populate brands (significant version)
    CefRefPtr<CefListValue> brands = CefListValue::Create();
    brands->SetDictionary(0, createBrand("Chromium", std::to_string(CHROME_VERSION_MAJOR)));
    brands->SetDictionary(1, createBrand("Not:A-Brand", "24"));
    brands->SetDictionary(2, createBrand("Google Chrome", std::to_string(CHROME_VERSION_MAJOR)));

    // 4. Populate fullVersionList (GREASE + full version)
    CefRefPtr<CefListValue> fullVersionList = CefListValue::Create();
    fullVersionList->SetDictionary(0, createBrand("Chromium", std::to_string(CHROME_VERSION_MAJOR) + ".0.0.0"));
    fullVersionList->SetDictionary(1, createBrand("Not:A-Brand", "24.0.0.0"));
    fullVersionList->SetDictionary(2, createBrand("Google Chrome", std::to_string(CHROME_VERSION_MAJOR) + ".0.0.0"));

    // 5. Build complete userAgentMetadata
    CefRefPtr<CefDictionaryValue> metadata = CefDictionaryValue::Create();
    metadata->SetList("brands", brands);
    metadata->SetList("fullVersionList", fullVersionList);
    metadata->SetString("fullVersion", std::to_string(CHROME_VERSION_MAJOR) + ".0.0.0");
    metadata->SetString("platform", "dragonfly");
    metadata->SetString("platformVersion", "");
    metadata->SetString("architecture", "");
    metadata->SetString("model", salt::mask());
    metadata->SetBool("mobile", false);
    metadata->SetString("bitness", "");

    params->SetDictionary("userAgentMetadata", metadata);

    // 6. Execute the CDP command
    browser->GetHost()->ExecuteDevToolsMethod(
        0, "Emulation.setUserAgentOverride", params);
}

void fly::OnAfterCreated(CefRefPtr<CefBrowser> browser) {
    CEF_REQUIRE_UI_THREAD();
    browser_ = browser;

    CefRefPtr<CefFrame> frame = browser->GetFocusedFrame();
    //frame->ExecuteJavaScript("document.body.requestFullscreen();", frame->GetURL(), 0);

    OverrideUserAgentAndHints(browser);
}

void fly::OnLoadStart(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, TransitionType transition_type) {
    CEF_REQUIRE_UI_THREAD();

    if (frame->IsMain()) {
        std::string j = R"(
            (() => {
                Object.defineProperty(navigator, 'userAgentData', {
                  get: () => undefined,
                  configurable: true
                });

                Object.defineProperty(navigator, 'geolocation', {
                  get: () => undefined,
                  configurable: true
                });

                if (window.location.href.indexOf("chrome://new") == 0) {
                    location.href="https://sprigatito.fanclub.rocks/"
                }
            })();
        )";
        CefString url = frame->GetURL();
        frame->ExecuteJavaScript(j, url, 0);
    }
}
