#include "include/cef_image.h"

CefRefPtr<CefImage> new_clear_icon() {
        // Single 32-bit pixel: R=0, G=0, B=0, Alpha=0 (fully transparent)
        const uint8_t transparent_pixel[4] = {0, 0, 0, 0};

        CefRefPtr<CefImage> image = CefImage::CreateImage();
        image->AddBitmap(
            1.0f,                          // scale_factor
            1,                             // pixel_width
            1,                             // pixel_height
            CEF_COLOR_TYPE_RGBA_8888,      // color_type
            CEF_ALPHA_TYPE_PREMULTIPLIED,  // alpha_type
            transparent_pixel,             // pixel_data
            sizeof(transparent_pixel));    // pixel_data_size

        return image;
}


CefRefPtr<CefImage> new_blank_icon() {
    // Single 32-bit pixel: R=255, G=255, B=255, Alpha=255 (opaque white)
    const uint8_t white_pixel[4] = {255, 255, 255, 255};

    CefRefPtr<CefImage> image = CefImage::CreateImage();
    image->AddBitmap(
        1.0f,                          // scale_factor
        1,                             // pixel_width
        1,                             // pixel_height
        CEF_COLOR_TYPE_RGBA_8888,      // color_type
        CEF_ALPHA_TYPE_PREMULTIPLIED,  // alpha_type
        white_pixel,                   // pixel_data
        sizeof(white_pixel));          // pixel_data_size (4 bytes)

    return image;
}