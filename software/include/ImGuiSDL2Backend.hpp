/**
 * @file
 * @author chu
 * @date 2024/11/15
 */
#pragma once
#include <SDL.h>
#include <SDL_syswm.h>
#include <imgui.h>
#ifdef __APPLE__
#include <TargetConditionals.h>
#endif
#ifdef __EMSCRIPTEN__
#include <emscripten/em_js.h>
#endif

class ImGuiSDL2Backend
{
public:
    enum class GamepadModes
    {
        AutoFirst = 0,
        AutoAll,
        Manual,
    };

    static ImGuiKey KeyEventToImGuiKey(SDL_Keycode keycode, SDL_Scancode scancode) noexcept;

    static void Initialize(SDL_Window* window);
    static void Shutdown() noexcept;

    static void SetGamepadMode(GamepadModes mode, struct _SDL_GameController** manualGamepadsArray, int manualGamepadsCount) noexcept;
    static void NewFrame();
    static bool ProcessEvent(const SDL_Event* event);

private:
    struct Data
    {
        SDL_Window* Window = nullptr;
        Uint32 WindowID = 0;
        SDL_Renderer* Renderer = nullptr;
        Uint64 Time = 0;
        char* ClipboardTextData = nullptr;

        // Mouse handling
        Uint32 MouseWindowID = 0;
        int MouseButtonsDown = 0;
        SDL_Cursor* MouseCursors[ImGuiMouseCursor_COUNT] =
            { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };
        SDL_Cursor* MouseLastCursor = nullptr;
        int MouseLastLeaveFrame = 0;
        bool MouseCanUseGlobalState = 0;

        // Gamepad handling
        ImVector<SDL_GameController*> Gamepads;
        GamepadModes GamepadMode = GamepadModes::AutoFirst;
        bool WantUpdateGamepadsList = false;
    };

    static Data* GetBackendData() noexcept;
    static const char* GetClipboardText(ImGuiContext*) noexcept;
    static void SetClipboardText(ImGuiContext*, const char* text) noexcept;
    static void SetImeData(ImGuiContext*, ImGuiViewport*, ImGuiPlatformImeData* data) noexcept;
    static void UpdateKeyModifiers(SDL_Keymod sdlKeyMods);
    static ImGuiViewport* GetViewportForWindowID(Uint32 windowId) noexcept;
    static void UpdateMouseData();
    static void UpdateMouseCursor() noexcept;
    static void CloseGamepads() noexcept;
    static void UpdateGamepadButton(Data* bd, ImGuiIO& io, ImGuiKey key, SDL_GameControllerButton buttonNo);
    static void UpdateGamepadAnalog(Data* bd, ImGuiIO& io, ImGuiKey key, SDL_GameControllerAxis axisNo, float v0, float v1);
    static void UpdateGamepads();
};
