#pragma once

class IWindow {
    public:
        IWindow();
        ~IWindow();

        virtual void createWindow(int height, int width);
    private:
};
