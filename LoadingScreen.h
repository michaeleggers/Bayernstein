#ifndef LOADINGSCREEN_H
#define LOADINGSCREEN_H

#include <atomic>
#include <thread>

#include "r_common.h"
#include "r_font.h"

class LoadingScreen
{
  public:
    LoadingScreen();
    void Start(CFont* font);
    void Stop();
    static void SetHint(const std::string& text);

    ~LoadingScreen()
    {
        Stop(); // Ensure the thread is stopped when the object is destroyed
    }

  private:
    void show(CFont* font);

    std::atomic<bool> m_IsRunning;
    std::thread       m_RenderThread;

    Sprite m_Image;
    static std::string m_LoadingHint;
};

#endif