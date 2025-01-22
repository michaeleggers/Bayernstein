#include "LoadingScreen.h"

#include "hkd_interface.h"

LoadingScreen::LoadingScreen()
    : m_Image(CreateSprite("splash_screen.png", glm::vec2(0.0f), glm::vec2(2730.0f, 2048.0f)))
{
}

bool LoadingScreen::show(double elapsedTime, CFont* font)
{
    double duration = 2500.0;
    if (elapsedTime > duration) {
        return false;
    }
    auto renderer = GetRenderer();
    renderer->Begin2D();
    glm::vec2 windowDimensions = renderer->GetWindowDimensions();
    renderer->SetShapeColor(glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
    renderer->DrawBox(0, 0, windowDimensions.x, windowDimensions.y);
    renderer->DrawSprite(&m_Image, glm::vec2(0.0f, -0.15f), glm::vec2(.704f));
    renderer->SetShapeColor(glm::vec4(1.0f, 0.2f, 0.2f, 1.0f));
    renderer->DrawBox(0.0f, 0.89, 1.0f * elapsedTime / duration, 0.01, COORD_MODE_REL);
    renderer->SetFont(font, glm::vec4(1.0f));
    renderer->R_DrawText("There's nothing to load, but it looks cool :P", 0.28f, 0.94, COORD_MODE_REL);
    renderer->End2D();
    return true;
}
