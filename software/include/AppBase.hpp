/**
 * @file
 * @author chu
 * @date 2024/11/14
 */
#pragma once
#include <SDL.h>
#include "Result.hpp"

struct AppBaseConfig
{
    std::string Title;
    int InitialWidth = 1280;
    int InitialHeight = 720;
    double TargetFPS = 10;
    bool Resizable = false;
    bool Borderless = true;
    bool FullScreen = false;
};

class AppBase
{
public:
    AppBase() noexcept = default;
    virtual ~AppBase() noexcept;

public:
    Result<void> Initialize(const AppBaseConfig& config) noexcept;
    void Run() noexcept;

protected:
    virtual void OnStart() noexcept = 0;
    virtual void OnFrame(double delta) noexcept = 0;
    virtual void OnStop() noexcept = 0;
    virtual void OnExitRequest(bool& doExit) noexcept;

private:
    ::SDL_Window* m_pMainWindow = nullptr;
    ::SDL_GLContext m_pGLContext = nullptr;
    bool m_bExit = false;
    double m_dTargetFps = 10;
};
