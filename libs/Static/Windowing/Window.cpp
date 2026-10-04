#include "MacWindow.cpp"
#include "WindowsWindow.cpp"
#include "LinuxWindow.cpp"

#if defined(_WIN32)
    using Window = WindowsWindow;
#elif defined(__linux__)
    #error "Support for linux build is not present yet."
    using Window = LinuxWindow;
#elif defined(__APPLE__)
    #error "Support for linux build is not present yet."
    using Window = MacWindow;
#else
    #error "Unsupported Operating System! This project only supports Windows, Linux, and macOS."
#endif
