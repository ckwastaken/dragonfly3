#include <iostream>

void info() {
    std::cout << "flycompact: The DRAGONFLY multi-call binary!\n"
                "(copyright) ckwastaken 2026\n\n"
                "included in multi-call binary provides:\n"
                "gui: Starts flygui + compositor.. do not run inside of itself\n"
                "clean: Cleans cache.. (/tmp/cache)\n"
                "license: Shows the license\n"
                "help: Shows this menu\n";
    exit(1);
}

#include <ostream>
#include <string>
#include "fly_gui.h"
#include <filesystem>
#include "fly_toast.h"
#include <iostream>
#include <string>
#include <thread>
#include <mutex>
#include <queue>
#include <chrono>
#include <atomic>
#include <future>

#include "twlconnector.h"

std::mutex inputMutex;
std::queue<std::string> inputQueue;

void readInputWorker() {
    std::string line;
    while (std::getline(std::cin, line)) {
        std::lock_guard<std::mutex> lock(inputMutex);
        inputQueue.push(line);
        load(line);
    }
}

int main(int argc, char* argv[]) {
    const char* isfly = std::getenv("IS_FLY");

    if (argc < 2) info();

    std::string applet = argv[1];

    if (applet == "gui") {
        if (isfly && std::string(isfly) == "yes") {
            std::cout << "\033[41mWARNING!!!\033[0m\n";
            std::cout << "\033[41mdo NOT run flygui inside of itself!\033[0m\n";
            exit(1);
        }

        std::thread inputThread(readInputWorker);
        inputThread.detach();

        while (true) {


            std::thread twl_thread([argc, argv]() {
                wayland_main(0, nullptr);
            });

            const char* wayland_display = twl_wait_until_ready();
            if (!wayland_display) {
                twl_thread.join();
                return 1;
            }

            // Point flycompact / CEF to TinyWL's Wayland socket
            setenv("WAYLAND_DISPLAY", wayland_display, 1);
            setenv("XDG_CURRENT_DESKTOP", "MyCompositor", 1);
            setenv("XDG_SESSION_TYPE", "wayland", 1);
            setenv("IS_FLY", "yes", 1);

            startux(0, nullptr);

            exit(0);

        }
    }

    if (applet.starts_with("--")) {
        exit(startux(argc, argv));
    }

    if (applet == "clean") {
        namespace fs = std::filesystem;

        std::uintmax_t deleted_count = fs::remove_all("/tmp/cache");
        std::cout << "Cleaned " << deleted_count << " items\n";
        exit(0);
    }

    if (applet == "license") {
        std::cout << R"(
Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

Don't break compatibility with packages designed for this software.

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

The Software shall be used for Good, not Evil.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
)";
        exit(0);
    }

    if (applet == "toast") {
        showToast("testing!", 2);
        exit(0);
    }

    info();
}