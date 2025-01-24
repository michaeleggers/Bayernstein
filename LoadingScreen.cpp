#include "LoadingScreen.h"

#include <chrono>

#include "hkd_interface.h"
#include "r_gl.h"

std::string LoadingScreen::m_LoadingHint;

LoadingScreen::LoadingScreen()
    : m_Image({}),
      m_IsRunning(false)
{
    m_LoadingHint = "Loading game...";
}

void LoadingScreen::Start(CFont* font)
{
    if ( m_IsRunning ) return;

    m_IsRunning    = true;
    m_RenderThread = std::thread(&LoadingScreen::show, this, font);
}

void LoadingScreen::Stop()
{
    m_IsRunning = false;
    m_RenderThread.join();
    // TODO: ^^ maybe check for joinable() first ???
}

void LoadingScreen::SetHint(const std::string& text)
{
    m_LoadingHint = text;
}

void LoadingScreen::show(CFont* font)
{
    int  counter    = 0;
    int  counterMax = 100;
    bool increment  = true;
    auto renderer   = new GLRender();
    if ( !renderer->Init(GetRenderer()->GetWindow()) ) // init new renderer instance with existing window
    {
        SDL_Log("Could not initialize renderer.\n");
        return;
    }
    // These must be called after the new renderer (opengl context) is created,
    // otherwise they are associated with the main thread in the TextureManager singleton:
    renderer->RegisterFont(font);
    m_Image = CreateSprite("splash_screen.png", glm::vec2(0.0f), glm::vec2(2730.0f, 2048.0f));

    while ( m_IsRunning )
    {
        float       barWidth         = (float)counter / (float)counterMax;
        glm::vec2   windowDimensions = renderer->GetWindowDimensions();
        const float charWidth        = font->m_Size * 0.602f;
        const float maxChars         = windowDimensions.x / charWidth;
        float       textWidthRel     = (float)m_LoadingHint.length() / maxChars;
        renderer->RenderBegin();
        renderer->Begin2D();

        renderer->SetShapeColor(glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
        renderer->DrawBox(0, 0, windowDimensions.x, windowDimensions.y);
        renderer->DrawSprite(&m_Image, glm::vec2(0.0f, -0.15f), glm::vec2(.704f));
        renderer->SetShapeColor(glm::vec4(1.0f, 0.2f, 0.2f, 1.0f));
        renderer->DrawBox(0.5f - barWidth / 2.0f, 0.89, barWidth, 0.01, COORD_MODE_REL);
        renderer->SetFont(font, glm::vec4(1.0f));
        renderer->R_DrawText(m_LoadingHint, 0.5f - textWidthRel / 2.0f, 0.94, COORD_MODE_REL);
        renderer->End2D();
        renderer->RenderEnd();

        counter = increment ? counter + 1 : counter - 1;
        if ( counter == counterMax || counter == 0 )
        {
            increment = !increment;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(8));
    }
    renderer->Shutdown(false);
}
