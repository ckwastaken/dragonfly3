#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/Xft/Xft.h>
#include <cmath>
#include <iostream>
#include <string>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <chrono>

struct ToastRequest {
    std::string message;
    int durationMs;
};

class ToastManager {
public:
    static void show(const std::string& message, int durationSeconds = 3) {
        static ToastManager instance;
        {
            std::lock_guard<std::mutex> lock(instance.mtx);
            instance.queue.push({message, durationSeconds * 1000});
        }
        instance.cv.notify_one();
    }

private:
    std::mutex mtx;
    std::condition_variable cv;
    std::queue<ToastRequest> queue;
    bool running = true;

    ToastManager() {
        XInitThreads();
        std::thread([this]() {
            workerLoop();
        }).detach();
    }

    // Damped sine wave: overshoots target slightly and bounces smoothly into rest
    static double easeOutSineBounce(double t) {
        if (t <= 0.0) return 0.0;
        if (t >= 1.0) return 1.0;

        // Exponential decay * cosine wave creates a natural springy sine bounce
        const double decay = 6.5;       // Controls how fast the bounce settles
        const double frequency = 2.5;   // Number of half-oscillations (1 overshoot + settle)
        return 1.0 - std::exp(-decay * t) * std::cos(frequency * M_PI * t);
    }

    // Slight upward anticipation bounce before sliding off-screen
    static double easeInBack(double t) {
        const double c1 = 1.70158;
        const double c3 = c1 + 1.0;
        return c3 * t * t * t - c1 * t * t;
    }

    void workerLoop() {
        Display* display = XOpenDisplay(nullptr);
        if (!display) return;

        int screen = DefaultScreen(display);
        Window root = RootWindow(display, screen);
        int screenWidth = DisplayWidth(display, screen);
        int screenHeight = DisplayHeight(display, screen);

        XftFont* font = XftFontOpenName(display, screen, "Nimbus Sans:size=13");
        if (!font) {
            font = XftFontOpenName(display, screen, "sans-13");
        }

        while (running) {
            ToastRequest req;
            {
                std::unique_lock<std::mutex> lock(mtx);
                cv.wait(lock, [this]() { return !queue.empty() || !running; });
                if (!running) break;
                req = std::move(queue.front());
                queue.pop();
            }

            displaySingleToast(display, screen, root, screenWidth, screenHeight, font, req);
        }

        if (font) XftFontClose(display, font);
        XCloseDisplay(display);
    }

    void processExposeEvents(Display* display, Window toast, XftDraw* draw,
                             XftColor* textColor, XftFont* font,
                             int padX, int padY, const std::string& message) {
        while (XPending(display)) {
            XEvent ev;
            XNextEvent(display, &ev);
            if (ev.type == Expose) {
                XftDrawStringUtf8(draw, textColor, font,
                                  padX, padY + font->ascent,
                                  reinterpret_cast<const FcChar8*>(message.c_str()),
                                  message.length());
            }
        }
    }

    void animateSlide(Display* display, Window toast, XftDraw* draw,
                      XftColor* textColor, XftFont* font,
                      int posX, int startY, int endY,
                      int padX, int padY, const std::string& message,
                      int animDurationMs, bool isSlideIn) {
        auto animStart = std::chrono::steady_clock::now();

        while (true) {
            auto now = std::chrono::steady_clock::now();
            double elapsedMs = std::chrono::duration<double, std::milli>(now - animStart).count();
            double t = elapsedMs / animDurationMs;
            if (t > 1.0) t = 1.0;

            double eased = isSlideIn ? easeOutSineBounce(t) : easeInBack(t);
            int currentY = static_cast<int>(std::round(startY + (endY - startY) * eased));

            XMoveWindow(display, toast, posX, currentY);
            processExposeEvents(display, toast, draw, textColor, font, padX, padY, message);
            XFlush(display);

            if (t >= 1.0) break;
            std::this_thread::sleep_for(std::chrono::milliseconds(16)); // ~60 FPS
        }
    }

    void displaySingleToast(Display* display, int screen, Window root,
                            int screenWidth, int screenHeight,
                            XftFont* font, const ToastRequest& req) {
        XGlyphInfo extents;
        XftTextExtentsUtf8(display, font,
                           reinterpret_cast<const FcChar8*>(req.message.c_str()),
                           req.message.length(), &extents);

        int textWidth = extents.xOff;
        int textHeight = font->ascent + font->descent;

        int padX = 28;
        int padY = 14;
        int width = textWidth + (padX * 2);
        int height = textHeight + (padY * 2);

        int posX = (screenWidth - width) / 2;
        int hiddenY = screenHeight + 10;
        int targetY = screenHeight - height - 80;

        XSetWindowAttributes attrs;
        attrs.override_redirect = True;
        attrs.background_pixel = 0xd0d0d0;
        attrs.border_pixel = 0xd0d0d0;

        Window toast = XCreateWindow(
            display, root,
            posX, hiddenY, width, height, 1,
            CopyFromParent, InputOutput, CopyFromParent,
            CWOverrideRedirect | CWBackPixel ,
            &attrs
        );

        Atom windowType = XInternAtom(display, "_NET_WM_WINDOW_TYPE", False);
        Atom notificationType = XInternAtom(display, "_NET_WM_WINDOW_TYPE_NOTIFICATION", False);
        XChangeProperty(display, toast, windowType, XA_ATOM, 32, PropModeReplace,
                        reinterpret_cast<unsigned char*>(&notificationType), 1);

        XSelectInput(display, toast, ExposureMask);
        XMapRaised(display, toast);
        XFlush(display);

        Visual* visual = DefaultVisual(display, screen);
        Colormap colormap = DefaultColormap(display, screen);
        XftDraw* draw = XftDrawCreate(display, toast, visual, colormap);

        XftColor textColor;
        XRenderColor renderColor = {0, 0, 0, 0xEEEE};
        XftColorAllocValue(display, visual, colormap, &renderColor, &textColor);

        // 1. Slide in with a springy sine bounce (420ms gives the bounce room to breathe)
        animateSlide(display, toast, draw, &textColor, font,
                     posX, hiddenY, targetY, padX, padY, req.message,
                     675, true);

        // 2. Hold at resting position
        auto holdEnd = std::chrono::steady_clock::now() + std::chrono::milliseconds(req.durationMs);
        int x11Fd = ConnectionNumber(display);

        while (true) {
            processExposeEvents(display, toast, draw, &textColor, font, padX, padY, req.message);
            XFlush(display);

            auto now = std::chrono::steady_clock::now();
            if (now >= holdEnd) break;

            auto remainingMs = std::chrono::duration_cast<std::chrono::milliseconds>(holdEnd - now).count();
            fd_set inFds;
            FD_ZERO(&inFds);
            FD_SET(x11Fd, &inFds);

            struct timeval tv;
            tv.tv_sec = remainingMs / 1000;
            tv.tv_usec = (remainingMs % 1000) * 1000;

            select(x11Fd + 1, &inFds, nullptr, nullptr, &tv);
        }

        // 3. Little upward hop + smooth slide off-screen
        animateSlide(display, toast, draw, &textColor, font,
                     posX, targetY, hiddenY, padX, padY, req.message,
                     573, false);

        XftColorFree(display, visual, colormap, &textColor);
        XftDrawDestroy(draw);
        XDestroyWindow(display, toast);
        XFlush(display);
    }
};

void showToast(const std::string& message, int durationSeconds = 3) {
    ToastManager::show(message, durationSeconds);
}