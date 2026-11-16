#include <iostream>
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

namespace fs = std::filesystem;

void info() {
    std::cout << "flycompact: The DRAGONFLY multi-call binary!\n"
                "(copyright) ckwastaken 2026\n\n"
                "included in multi-call binary provides:\n"
                "gui: Starts the user interface\n"
                "clean: Cleans cache.. (/tmp/cache)\n"
                "help: Shows this menu\n"
                "\nstring start with '-' assumed to be the gui command\n\n";
    exit(1);
}

int main(int argc, char* argv[]) {
    if (argc < 2) info();

    std::string applet = argv[1];

    if (applet == "gui") {
        std::thread inputThread(readInputWorker);
        inputThread.detach();

        while (true) {
            /*
            {
                std::lock_guard<std::mutex> lock(inputMutex);
                while (!inputQueue.empty()) {
                    std::cout << "\n[User typed]: " << inputQueue.front() << "\n";
                    inputQueue.pop();
                }
            }
            */
            int e = startux(argc, argv);
            exit(e);
        }
    }

    if (applet.starts_with('-')) {
        exit(startux(argc, argv));
    }

    if (applet == "clean") {
        std::uintmax_t deleted_count = fs::remove_all("/tmp/cache");
        std::cout << "Cleaned " << deleted_count << " items\n";
        exit(0);
    }

    if (applet == "toast") {
        showToast("testing!", 2);
        exit(0);
    }

    info();
}