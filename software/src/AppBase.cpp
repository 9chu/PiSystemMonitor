/**
 * @file
 * @author chu
 * @date 2024/11/14
 */
#include <AppBase.hpp>

#include <imgui.h>
#include <implot.h>
#include <spdlog/spdlog.h>
#include <SDLError.hpp>
#include <ImGuiSDL2Backend.hpp>
#include <ImGuiOpenGLBackend.hpp>

using namespace std;

AppBase::~AppBase() noexcept
{
    ImGuiOpenGLBackend::Shutdown();
    ImGuiSDL2Backend::Shutdown();
    ImPlot::DestroyContext();
    ImGui::DestroyContext();

    ::SDL_GL_DeleteContext(m_pGLContext);
    ::SDL_DestroyWindow(m_pMainWindow);
}

Result<void> AppBase::Initialize(const AppBaseConfig& config) noexcept
{
    if (auto ret = ::SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER); ret != 0)
    {
        spdlog::error("SDL_Init failed: {}", ::SDL_GetError());
        return MakeSDLError(ret);
    }

#ifdef SDL_HINT_IME_SHOW_UI
    ::SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");
#endif

    ::SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    ::SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    ::SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    ::SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    ::SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
    auto windowFlags = SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI;
    if (config.Resizable)
        windowFlags |= SDL_WINDOW_RESIZABLE;
    if (config.Borderless)
        windowFlags |= SDL_WINDOW_BORDERLESS;
    if (config.FullScreen)
        windowFlags |= SDL_WINDOW_FULLSCREEN;
    SDL_Window* window = ::SDL_CreateWindow(config.Title.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, config.InitialWidth,
        config.InitialHeight, static_cast<SDL_WindowFlags>(windowFlags));
    if (window == nullptr)
    {
        spdlog::error("SDL_CreateWindow failed: {}", ::SDL_GetError());
        return MakeSDLError(-1);
    }

    SDL_GLContext glContext = ::SDL_GL_CreateContext(window);
    if (glContext == nullptr)
    {
        spdlog::error("SDL_GL_CreateContext failed: {}", ::SDL_GetError());
        ::SDL_DestroyWindow(window);
        return MakeSDLError(-1);
    }
    ::SDL_GL_MakeCurrent(window, glContext);
    ::SDL_GL_SetSwapInterval(1); // Enable vsync

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    ImGui::StyleColorsDark();
    ImPlot::StyleColorsDark();

    ImGuiSDL2Backend::Initialize(window);
    ImGuiOpenGLBackend::Initialize();

    m_pMainWindow = window;
    m_pGLContext = glContext;
    m_dTargetFps = config.TargetFPS;
    return {};
}

void AppBase::Run() noexcept
{
    OnStart();

    static Uint64 kFrequency = ::SDL_GetPerformanceFrequency();
    auto lastTick = ::SDL_GetPerformanceCounter();
    m_bExit = false;
    while (!m_bExit)
    {
        auto currentTick = ::SDL_GetPerformanceCounter();
        auto deltaTime = static_cast<double>(currentTick - lastTick) / static_cast<double>(kFrequency);
        lastTick = currentTick;

        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        ::SDL_Event event;
        while (::SDL_PollEvent(&event))
        {
            ImGuiSDL2Backend::ProcessEvent(&event);
            if (event.type == SDL_QUIT || (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE &&
                event.window.windowID == ::SDL_GetWindowID(m_pMainWindow)))
            {
                bool doExit = true;
                OnExitRequest(doExit);
                if (doExit)
                    m_bExit = true;
            }
        }
        if (::SDL_GetWindowFlags(m_pMainWindow) & SDL_WINDOW_MINIMIZED)
        {
            ::SDL_Delay(100);
            continue;
        }

        ImGuiOpenGLBackend::NewFrame();
        ImGuiSDL2Backend::NewFrame();
        ImGui::NewFrame();

        OnFrame(deltaTime);

        ImGuiIO& io = ImGui::GetIO();
        ImGui::Render();
        ImGuiOpenGLBackend::Clear(static_cast<int>(io.DisplaySize.x), static_cast<int>(io.DisplaySize.y));
        ImGuiOpenGLBackend::RenderDrawData(ImGui::GetDrawData());
        ::SDL_GL_SwapWindow(m_pMainWindow);

        auto currentTickEndFrame = ::SDL_GetPerformanceCounter();
        auto frameTime = static_cast<double>(currentTickEndFrame - currentTick) / static_cast<double>(kFrequency);
        if (frameTime < 1.0 / m_dTargetFps)
        {
            auto sleepTimeMs = static_cast<int>(1000.0 * (1.0 / m_dTargetFps - frameTime));
            ::SDL_Delay(sleepTimeMs);
        }
    }

    OnStop();
}

void AppBase::OnExitRequest(bool& doExit) noexcept
{
}
