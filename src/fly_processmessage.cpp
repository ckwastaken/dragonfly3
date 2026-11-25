#include <cstring>
#include <iostream>
#include <pty.h>
#include <string>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <thread>
#include <unistd.h>
#include "fly_gui.h"
#include "fly_toast.h"
#include "global.h"
#include "include/base/cef_callback.h"
#include "include/cef_values.h"
#include "include/internal/cef_ptr.h"
#include "include/internal/cef_string.h"
#include "include/wrapper/cef_closure_task.h"

#include "include/cef_path_util.h"
#include "include/internal/cef_types.h"
#include <cstdlib>
#include "twlconnector.h"

#include <string>

void AddCefExeDirToPath() {
    CefString exe_dir;

    // Retrieve the directory containing the current executable
    if (CefGetPath(PK_DIR_EXE, exe_dir)) {
        std::string dir_str = exe_dir.ToString();
        const char* current_path = std::getenv("PATH");
        std::string new_path = current_path ? current_path : "";

#if defined(_WIN32)
        if (!new_path.empty()) new_path += ";";
        new_path += dir_str;
        _putenv_s("PATH", new_path.c_str());
#else
        if (!new_path.empty()) new_path += ":";
        new_path += dir_str;
        setenv("PATH", new_path.c_str(), 1);
#endif
    }
}

namespace {
    int g_master_fd = -1;
    CefRefPtr<CefFrame> g_frame;

    // Safely escapes output so backticks, backslashes, and ${} don't break JS
    std::string EscapeForJsTemplate(const std::string& input) {
        std::string out;
        out.reserve(input.size() * 1.2);
        for (char c : input) {
            switch (c) {
                case '\\': out += "\\\\"; break;
                case '`':  out += "\\`";  break;
                case '$':  out += "\\$";  break;
                default:   out += c;      break;
            }
        }
        return out;
    }

    void SendBack(const std::string& js_code) {
        if (g_frame && g_frame->IsValid()) {
            g_frame->ExecuteJavaScript(js_code, g_frame->GetURL(), 0);
        }
    }
} // namespace

bool process_message(std::string message, CefRefPtr<CefListValue> args, CefRefPtr<CefFrame> frame_f) {
    g_frame = frame_f;

    if (message == "terminalviewd:open") {
        setenv("TERM", "xterm-256color", 1);

        int col = args->GetInt(1);
        int row = args->GetInt(2);
        std::string shell = args->GetString(3);
        if (shell.empty()) shell = "/bin/sh";

        winsize ws = {
            .ws_row = static_cast<unsigned short>(row),
            .ws_col = static_cast<unsigned short>(col),
            .ws_xpixel = 0,
            .ws_ypixel = 0
        };

        AddCefExeDirToPath();

        int master_fd = -1;
        pid_t pid = forkpty(&master_fd, nullptr, nullptr, &ws);

        if (pid < 0) {
            perror("forkpty");
            return false;
        }

        if (pid == 0) {
            execlp(shell.c_str(), shell.c_str(), nullptr);
            perror("execlp");
            _exit(1);
        }

        g_master_fd = master_fd;

        // Read PTY output on a background thread so CEF doesn't freeze
        std::thread([master_fd, pid]() {
            char buf[256];
            ssize_t n;

            while ((n = read(master_fd, buf, sizeof(buf))) > 0) {
                std::string chunk(buf, n);
                std::string js_code = "term.write(`" + EscapeForJsTemplate(chunk) + "`);";

                CefThreadId target_thread = CefCurrentlyOn(TID_RENDERER) ? TID_RENDERER : TID_UI;
                CefPostTask(target_thread, base::BindOnce(&SendBack, js_code));
            }
            close(master_fd);
            waitpid(pid, nullptr, 0);
            if (g_master_fd == master_fd) {
                g_master_fd = -1;
                CefThreadId target_thread = CefCurrentlyOn(TID_RENDERER) ? TID_RENDERER : TID_UI;
                CefPostTask(target_thread, base::BindOnce(&SendBack, "history.go(-1);"));
            }
        }).detach();

        return true;
    }

    if (message == "terminalviewd:write") {
        if (g_master_fd < 0) return false;
        std::string data = args->GetString(1);
        if (!data.empty()) {
            write(g_master_fd, data.data(), data.size());
        }
        return true;
    }

    if (message == "terminalviewd:resize") {
        if (g_master_fd < 0) return false;
        int col = args->GetInt(0);
        int row = args->GetInt(1);

        winsize ws{};
        ws.ws_row = static_cast<unsigned short>(row);
        ws.ws_col = static_cast<unsigned short>(col);

        if (ioctl(g_master_fd, TIOCSWINSZ, &ws) == -1) {
            std::cerr << "ioctl(TIOCSWINSZ) failed: " << std::strerror(errno) << '\n';
            return false;
        }
        return true;
    }

    return false;
}
