#pragma once

#ifdef __cplusplus
extern "C" {
#endif

int wayland_main(int argc, char** argv);
const char* twl_wait_until_ready(void);

#ifdef __cplusplus
}
#endif
