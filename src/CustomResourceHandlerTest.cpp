#include <cstring>
#include <string>

#include "include/cef_app.h"
#include "include/cef_browser.h"
#include "include/wrapper/cef_helpers.h"

class CustomResourceHandler : public CefResourceHandler {
 public:
  CustomResourceHandler(const std::string& data, const std::string& mime_type)
      : data_(data), mime_type_(mime_type), offset_(0) {}

  // 1. Handle the request immediately
  bool Open(CefRefPtr<CefRequest> request,
            bool& handle_request,
            CefRefPtr<CefCallback> callback) override {
    handle_request = true; // Process immediately without waiting for callback
    return true;
  }

  // 2. Set HTTP status code, MIME type, and custom response headers
  void GetResponseHeaders(CefRefPtr<CefResponse> response,
                          int64_t& response_length,
                          CefString& redirectUrl) override {

    response->SetMimeType(mime_type_);
    response->SetStatus(200);
    response->SetStatusText("OK");

    // Optional: Add custom headers (e.g., CORS)
    CefResponse::HeaderMap headers;
    headers.insert(std::make_pair("Access-Control-Allow-Origin", "*"));

    headers.insert(std::make_pair("Redirect", data_));

    response->SetHeaderMap(headers);

    response_length = static_cast<int64_t>(data_.size());
  }

  // 3. Copy chunks of data into CEF's output buffer
  bool Read(void* data_out,
            int bytes_to_read,
            int& bytes_read,
            CefRefPtr<CefResourceReadCallback> callback) override {
    bytes_read = 0;

    if (offset_ < data_.size()) {
      size_t transfer_size = std::min(static_cast<size_t>(bytes_to_read),
                                      data_.size() - offset_);
      std::memcpy(data_out, data_.data() + offset_, transfer_size);
      offset_ += transfer_size;
      bytes_read = static_cast<int>(transfer_size);
      return true; // Return true while data remains
    }

    return false; // Return false when finished reading
  }

  void Cancel() override {
  }

 private:
  std::string data_;
  std::string mime_type_;
  size_t offset_;

  IMPLEMENT_REFCOUNTING(CustomResourceHandler);
};
