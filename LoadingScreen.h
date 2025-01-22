#ifndef LOADINGSCREEN_H
#define LOADINGSCREEN_H

#include "r_common.h"
#include "r_font.h"

class LoadingScreen
{
  public:
    LoadingScreen();
    bool show(double elapsedTime, CFont* font);

  private:
    Sprite m_Image;
};

#endif