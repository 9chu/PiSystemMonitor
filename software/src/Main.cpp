/**
 * @file
 * @author chu
 * @date 2024/11/17
 */
#include <App.hpp>

#include <spdlog/spdlog.h>
#include <SDL2/SDL.h>

SDLMAIN_DECLSPEC int main(int argc, char* argv[])
{
    App app;
    if (auto result = app.Initialize(); !result)
    {
        spdlog::error("Failed to initialize the application: {}({})", result.GetError().message(), result.GetError().value());
        return 1;
    }

    app.Run();
    return 0;
}
