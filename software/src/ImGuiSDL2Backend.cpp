/**
 * @file
 * @author chu
 * @date 2024/11/15
 */
#include <ImGuiSDL2Backend.hpp>

using namespace std;

#if SDL_VERSION_ATLEAST(2,0,4) && !defined(__EMSCRIPTEN__) && !defined(__ANDROID__) && !(defined(__APPLE__) && TARGET_OS_IOS) && !defined(__amigaos4__)
#define SDL_HAS_CAPTURE_AND_GLOBAL_MOUSE    1
#else
#define SDL_HAS_CAPTURE_AND_GLOBAL_MOUSE    0
#endif

#ifdef __EMSCRIPTEN__
EM_JS(void, EmscriptenOpenURL_, (char const* url), {
    url = url ? UTF8ToString(url) : null;
    if (url)
        window.open(url, '_blank');
});
#endif

ImGuiKey ImGuiSDL2Backend::KeyEventToImGuiKey(SDL_Keycode keycode, SDL_Scancode scancode) noexcept
{
    IM_UNUSED(scancode);

    switch (keycode)
    {
        case SDLK_TAB: return ImGuiKey_Tab;
        case SDLK_LEFT: return ImGuiKey_LeftArrow;
        case SDLK_RIGHT: return ImGuiKey_RightArrow;
        case SDLK_UP: return ImGuiKey_UpArrow;
        case SDLK_DOWN: return ImGuiKey_DownArrow;
        case SDLK_PAGEUP: return ImGuiKey_PageUp;
        case SDLK_PAGEDOWN: return ImGuiKey_PageDown;
        case SDLK_HOME: return ImGuiKey_Home;
        case SDLK_END: return ImGuiKey_End;
        case SDLK_INSERT: return ImGuiKey_Insert;
        case SDLK_DELETE: return ImGuiKey_Delete;
        case SDLK_BACKSPACE: return ImGuiKey_Backspace;
        case SDLK_SPACE: return ImGuiKey_Space;
        case SDLK_RETURN: return ImGuiKey_Enter;
        case SDLK_ESCAPE: return ImGuiKey_Escape;
        case SDLK_QUOTE: return ImGuiKey_Apostrophe;
        case SDLK_COMMA: return ImGuiKey_Comma;
        case SDLK_MINUS: return ImGuiKey_Minus;
        case SDLK_PERIOD: return ImGuiKey_Period;
        case SDLK_SLASH: return ImGuiKey_Slash;
        case SDLK_SEMICOLON: return ImGuiKey_Semicolon;
        case SDLK_EQUALS: return ImGuiKey_Equal;
        case SDLK_LEFTBRACKET: return ImGuiKey_LeftBracket;
        case SDLK_BACKSLASH: return ImGuiKey_Backslash;
        case SDLK_RIGHTBRACKET: return ImGuiKey_RightBracket;
        case SDLK_BACKQUOTE: return ImGuiKey_GraveAccent;
        case SDLK_CAPSLOCK: return ImGuiKey_CapsLock;
        case SDLK_SCROLLLOCK: return ImGuiKey_ScrollLock;
        case SDLK_NUMLOCKCLEAR: return ImGuiKey_NumLock;
        case SDLK_PRINTSCREEN: return ImGuiKey_PrintScreen;
        case SDLK_PAUSE: return ImGuiKey_Pause;
        case SDLK_KP_0: return ImGuiKey_Keypad0;
        case SDLK_KP_1: return ImGuiKey_Keypad1;
        case SDLK_KP_2: return ImGuiKey_Keypad2;
        case SDLK_KP_3: return ImGuiKey_Keypad3;
        case SDLK_KP_4: return ImGuiKey_Keypad4;
        case SDLK_KP_5: return ImGuiKey_Keypad5;
        case SDLK_KP_6: return ImGuiKey_Keypad6;
        case SDLK_KP_7: return ImGuiKey_Keypad7;
        case SDLK_KP_8: return ImGuiKey_Keypad8;
        case SDLK_KP_9: return ImGuiKey_Keypad9;
        case SDLK_KP_PERIOD: return ImGuiKey_KeypadDecimal;
        case SDLK_KP_DIVIDE: return ImGuiKey_KeypadDivide;
        case SDLK_KP_MULTIPLY: return ImGuiKey_KeypadMultiply;
        case SDLK_KP_MINUS: return ImGuiKey_KeypadSubtract;
        case SDLK_KP_PLUS: return ImGuiKey_KeypadAdd;
        case SDLK_KP_ENTER: return ImGuiKey_KeypadEnter;
        case SDLK_KP_EQUALS: return ImGuiKey_KeypadEqual;
        case SDLK_LCTRL: return ImGuiKey_LeftCtrl;
        case SDLK_LSHIFT: return ImGuiKey_LeftShift;
        case SDLK_LALT: return ImGuiKey_LeftAlt;
        case SDLK_LGUI: return ImGuiKey_LeftSuper;
        case SDLK_RCTRL: return ImGuiKey_RightCtrl;
        case SDLK_RSHIFT: return ImGuiKey_RightShift;
        case SDLK_RALT: return ImGuiKey_RightAlt;
        case SDLK_RGUI: return ImGuiKey_RightSuper;
        case SDLK_APPLICATION: return ImGuiKey_Menu;
        case SDLK_0: return ImGuiKey_0;
        case SDLK_1: return ImGuiKey_1;
        case SDLK_2: return ImGuiKey_2;
        case SDLK_3: return ImGuiKey_3;
        case SDLK_4: return ImGuiKey_4;
        case SDLK_5: return ImGuiKey_5;
        case SDLK_6: return ImGuiKey_6;
        case SDLK_7: return ImGuiKey_7;
        case SDLK_8: return ImGuiKey_8;
        case SDLK_9: return ImGuiKey_9;
        case SDLK_a: return ImGuiKey_A;
        case SDLK_b: return ImGuiKey_B;
        case SDLK_c: return ImGuiKey_C;
        case SDLK_d: return ImGuiKey_D;
        case SDLK_e: return ImGuiKey_E;
        case SDLK_f: return ImGuiKey_F;
        case SDLK_g: return ImGuiKey_G;
        case SDLK_h: return ImGuiKey_H;
        case SDLK_i: return ImGuiKey_I;
        case SDLK_j: return ImGuiKey_J;
        case SDLK_k: return ImGuiKey_K;
        case SDLK_l: return ImGuiKey_L;
        case SDLK_m: return ImGuiKey_M;
        case SDLK_n: return ImGuiKey_N;
        case SDLK_o: return ImGuiKey_O;
        case SDLK_p: return ImGuiKey_P;
        case SDLK_q: return ImGuiKey_Q;
        case SDLK_r: return ImGuiKey_R;
        case SDLK_s: return ImGuiKey_S;
        case SDLK_t: return ImGuiKey_T;
        case SDLK_u: return ImGuiKey_U;
        case SDLK_v: return ImGuiKey_V;
        case SDLK_w: return ImGuiKey_W;
        case SDLK_x: return ImGuiKey_X;
        case SDLK_y: return ImGuiKey_Y;
        case SDLK_z: return ImGuiKey_Z;
        case SDLK_F1: return ImGuiKey_F1;
        case SDLK_F2: return ImGuiKey_F2;
        case SDLK_F3: return ImGuiKey_F3;
        case SDLK_F4: return ImGuiKey_F4;
        case SDLK_F5: return ImGuiKey_F5;
        case SDLK_F6: return ImGuiKey_F6;
        case SDLK_F7: return ImGuiKey_F7;
        case SDLK_F8: return ImGuiKey_F8;
        case SDLK_F9: return ImGuiKey_F9;
        case SDLK_F10: return ImGuiKey_F10;
        case SDLK_F11: return ImGuiKey_F11;
        case SDLK_F12: return ImGuiKey_F12;
        case SDLK_F13: return ImGuiKey_F13;
        case SDLK_F14: return ImGuiKey_F14;
        case SDLK_F15: return ImGuiKey_F15;
        case SDLK_F16: return ImGuiKey_F16;
        case SDLK_F17: return ImGuiKey_F17;
        case SDLK_F18: return ImGuiKey_F18;
        case SDLK_F19: return ImGuiKey_F19;
        case SDLK_F20: return ImGuiKey_F20;
        case SDLK_F21: return ImGuiKey_F21;
        case SDLK_F22: return ImGuiKey_F22;
        case SDLK_F23: return ImGuiKey_F23;
        case SDLK_F24: return ImGuiKey_F24;
        case SDLK_AC_BACK: return ImGuiKey_AppBack;
        case SDLK_AC_FORWARD: return ImGuiKey_AppForward;
        default: break;
    }
    return ImGuiKey_None;
}

void ImGuiSDL2Backend::Initialize(SDL_Window* window)
{
    ImGuiIO& io = ImGui::GetIO();
    IMGUI_CHECKVERSION();
    IM_ASSERT(io.BackendPlatformUserData == nullptr && "Already initialized a platform backend!");

    // Check and store if we are on a SDL backend that supports global mouse position
    // ("wayland" and "rpi" don't support it, but we chose to use a white-list instead of a black-list)
    bool mouseCanUseGlobalState = false;
#if SDL_HAS_CAPTURE_AND_GLOBAL_MOUSE
    const char* sdlBackend = ::SDL_GetCurrentVideoDriver();
    const char* globalMouseWhitelist[] = { "windows", "cocoa", "x11", "DIVE", "VMAN" };
    for (int n = 0; n < IM_ARRAYSIZE(globalMouseWhitelist); n++)
    {
        if (::strncmp(sdlBackend, globalMouseWhitelist[n], ::strlen(globalMouseWhitelist[n])) == 0)
            mouseCanUseGlobalState = true;
    }
#endif

    // Setup backend capabilities flags
    auto* bd = IM_NEW(Data)();
    io.BackendPlatformUserData = static_cast<void*>(bd);
    io.BackendPlatformName = "imgui_impl_sdl2";
    io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;       // We can honor GetMouseCursor() values (optional)
    io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;        // We can honor io.WantSetMousePos requests (optional, rarely used)

    bd->Window = window;
    bd->WindowID = ::SDL_GetWindowID(window);
    bd->Renderer = nullptr;
    bd->MouseCanUseGlobalState = mouseCanUseGlobalState;

    ImGuiPlatformIO& platform_io = ImGui::GetPlatformIO();
    platform_io.Platform_SetClipboardTextFn = SetClipboardText;
    platform_io.Platform_GetClipboardTextFn = GetClipboardText;
    platform_io.Platform_ClipboardUserData = nullptr;
    platform_io.Platform_SetImeDataFn = SetImeData;
#ifdef __EMSCRIPTEN__
    platform_io.Platform_OpenInShellFn = [](ImGuiContext*, const char* url) { EmscriptenOpenURL_(url); return true; };
#endif

    // Gamepad handling
    bd->GamepadMode = GamepadModes::AutoFirst;
    bd->WantUpdateGamepadsList = true;

    // Load mouse cursors
    bd->MouseCursors[ImGuiMouseCursor_Arrow] = ::SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW);
    bd->MouseCursors[ImGuiMouseCursor_TextInput] = ::SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_IBEAM);
    bd->MouseCursors[ImGuiMouseCursor_ResizeAll] = ::SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZEALL);
    bd->MouseCursors[ImGuiMouseCursor_ResizeNS] = ::SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENS);
    bd->MouseCursors[ImGuiMouseCursor_ResizeEW] = ::SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZEWE);
    bd->MouseCursors[ImGuiMouseCursor_ResizeNESW] = ::SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENESW);
    bd->MouseCursors[ImGuiMouseCursor_ResizeNWSE] = ::SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENWSE);
    bd->MouseCursors[ImGuiMouseCursor_Hand] = ::SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_HAND);
    bd->MouseCursors[ImGuiMouseCursor_NotAllowed] = ::SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_NO);

    // Set platform dependent data in viewport
    // Our mouse update function expect PlatformHandle to be filled for the main viewport
    ImGuiViewport* mainViewport = ImGui::GetMainViewport();
    mainViewport->PlatformHandle = reinterpret_cast<void*>(static_cast<intptr_t>(bd->WindowID));
    mainViewport->PlatformHandleRaw = nullptr;
    SDL_SysWMinfo info;
    SDL_VERSION(&info.version);
    if (::SDL_GetWindowWMInfo(window, &info))
    {
#if defined(SDL_VIDEO_DRIVER_WINDOWS)
        mainViewport->PlatformHandleRaw = static_cast<void*>(info.info.win.window);
#elif defined(__APPLE__) && defined(SDL_VIDEO_DRIVER_COCOA)
        mainViewport->PlatformHandleRaw = static_cast<void*>(info.info.cocoa.window);
#endif
    }

    // From 2.0.5: Set SDL hint to receive mouse click events on window focus, otherwise SDL doesn't emit the event.
    // Without this, when clicking to gain focus, our widgets wouldn't activate even though they showed as hovered.
    // (This is unfortunately a global SDL setting, so enabling it might have a side-effect on your application.
    // It is unlikely to make a difference, but if your app absolutely needs to ignore the initial on-focus click:
    // you can ignore SDL_MOUSEBUTTONDOWN events coming right after a SDL_WINDOWEVENT_FOCUS_GAINED)
#ifdef SDL_HINT_MOUSE_FOCUS_CLICKTHROUGH
    ::SDL_SetHint(SDL_HINT_MOUSE_FOCUS_CLICKTHROUGH, "1");
#endif

    // From 2.0.18: Enable native IME.
    // IMPORTANT: This is used at the time of SDL_CreateWindow() so this will only affects secondary windows, if any.
    // For the main window to be affected, your application needs to call this manually before calling SDL_CreateWindow().
#ifdef SDL_HINT_IME_SHOW_UI
    ::SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");
#endif

    // From 2.0.22: Disable auto-capture, this is preventing drag and drop across multiple windows (see #5710)
#ifdef SDL_HINT_MOUSE_AUTO_CAPTURE
    ::SDL_SetHint(SDL_HINT_MOUSE_AUTO_CAPTURE, "0");
#endif
}

void ImGuiSDL2Backend::Shutdown() noexcept
{
    auto* bd = GetBackendData();
    IM_ASSERT(bd != nullptr && "No platform backend to shutdown, or already shutdown?");
    ImGuiIO& io = ImGui::GetIO();

    if (bd->ClipboardTextData)
        ::SDL_free(bd->ClipboardTextData);
    for (ImGuiMouseCursor cursor_n = 0; cursor_n < ImGuiMouseCursor_COUNT; cursor_n++)
        ::SDL_FreeCursor(bd->MouseCursors[cursor_n]);
    CloseGamepads();

    io.BackendPlatformName = nullptr;
    io.BackendPlatformUserData = nullptr;
    io.BackendFlags &= ~(ImGuiBackendFlags_HasMouseCursors | ImGuiBackendFlags_HasSetMousePos | ImGuiBackendFlags_HasGamepad);
    IM_DELETE(bd);
}

void ImGuiSDL2Backend::SetGamepadMode(GamepadModes mode, struct _SDL_GameController** manualGamepadsArray,
    int manualGamepadsCount) noexcept
{
    auto* bd = GetBackendData();
    CloseGamepads();
    if (mode == GamepadModes::Manual)
    {
        IM_ASSERT(manualGamepadsArray != nullptr && manualGamepadsCount > 0);
        for (int n = 0; n < manualGamepadsCount; n++)
            bd->Gamepads.push_back(manualGamepadsArray[n]);
    }
    else
    {
        IM_ASSERT(manualGamepadsArray == nullptr && manualGamepadsCount <= 0);
        bd->WantUpdateGamepadsList = true;
    }
    bd->GamepadMode = mode;
}

void ImGuiSDL2Backend::NewFrame()
{
    auto* bd = GetBackendData();
    IM_ASSERT(bd != nullptr && "Context or backend not initialized! Did you call ImGui_ImplSDL2_Init()?");
    ImGuiIO& io = ImGui::GetIO();

    // Setup display size (every frame to accommodate for window resizing)
    int w = 0, h = 0;
    int displayW, displayH;
    ::SDL_GetWindowSize(bd->Window, &w, &h);
    if (::SDL_GetWindowFlags(bd->Window) & SDL_WINDOW_MINIMIZED)
        w = h = 0;
    if (bd->Renderer != nullptr)
        ::SDL_GetRendererOutputSize(bd->Renderer, &displayW, &displayH);
    else
        ::SDL_GL_GetDrawableSize(bd->Window, &displayW, &displayH);
    io.DisplaySize = ImVec2(static_cast<float>(w), static_cast<float>(h));
    if (w > 0 && h > 0)
    {
        io.DisplayFramebufferScale = ImVec2(static_cast<float>(displayW) / static_cast<float>(w),
            static_cast<float>(displayH) / static_cast<float>(h));
    }

    // Setup time step (we don't use SDL_GetTicks() because it is using millisecond resolution)
    // (Accept SDL_GetPerformanceCounter() not returning a monotonically increasing value. Happens in VMs and Emscripten, see #6189, #6114, #3644)
    static Uint64 kFrequency = ::SDL_GetPerformanceFrequency();
    Uint64 current_time = ::SDL_GetPerformanceCounter();
    if (current_time <= bd->Time)
        current_time = bd->Time + 1;
    io.DeltaTime = bd->Time > 0 ? static_cast<float>(static_cast<double>(current_time - bd->Time) / static_cast<double>(kFrequency)) :
        static_cast<float>(1.0f / 60.0f);
    bd->Time = current_time;

    if (bd->MouseLastLeaveFrame && bd->MouseLastLeaveFrame >= ImGui::GetFrameCount() && bd->MouseButtonsDown == 0)
    {
        bd->MouseWindowID = 0;
        bd->MouseLastLeaveFrame = 0;
        io.AddMousePosEvent(-FLT_MAX, -FLT_MAX);
    }

    UpdateMouseData();
    UpdateMouseCursor();

    // Update game controllers (if enabled and available)
    UpdateGamepads();
}

bool ImGuiSDL2Backend::ProcessEvent(const SDL_Event* event)
{
    auto* bd = GetBackendData();
    IM_ASSERT(bd != nullptr && "Context or backend not initialized! Did you call ImGui_ImplSDL2_Init()?");
    ImGuiIO& io = ImGui::GetIO();

    switch (event->type)
    {
        case SDL_MOUSEMOTION:
            {
                if (GetViewportForWindowID(event->motion.windowID) == nullptr)
                    return false;
                ImVec2 mousePos { static_cast<float>(event->motion.x), static_cast<float>(event->motion.y) };
                io.AddMouseSourceEvent(event->motion.which == SDL_TOUCH_MOUSEID ? ImGuiMouseSource_TouchScreen : ImGuiMouseSource_Mouse);
                io.AddMousePosEvent(mousePos.x, mousePos.y);
                return true;
            }
        case SDL_MOUSEWHEEL:
            {
                if (GetViewportForWindowID(event->wheel.windowID) == nullptr)
                    return false;
#if SDL_VERSION_ATLEAST(2,0,18) // If this fails to compile on Emscripten: update to latest Emscripten!
                float wheelX = -event->wheel.preciseX;
                float wheelY = event->wheel.preciseY;
#else
                float wheelX = -(float)event->wheel.x;
                float wheelY = (float)event->wheel.y;
#endif
#if defined(__EMSCRIPTEN__) && !SDL_VERSION_ATLEAST(2,31,0)
                wheel_x /= 100.0f;
#endif
                io.AddMouseSourceEvent(event->wheel.which == SDL_TOUCH_MOUSEID ? ImGuiMouseSource_TouchScreen : ImGuiMouseSource_Mouse);
                io.AddMouseWheelEvent(wheelX, wheelY);
                return true;
            }
        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEBUTTONUP:
            {
                if (GetViewportForWindowID(event->button.windowID) == nullptr)
                    return false;
                int mouseButton = -1;
                if (event->button.button == SDL_BUTTON_LEFT) { mouseButton = 0; }
                if (event->button.button == SDL_BUTTON_RIGHT) { mouseButton = 1; }
                if (event->button.button == SDL_BUTTON_MIDDLE) { mouseButton = 2; }
                if (event->button.button == SDL_BUTTON_X1) { mouseButton = 3; }
                if (event->button.button == SDL_BUTTON_X2) { mouseButton = 4; }
                if (mouseButton == -1)
                    break;
                io.AddMouseSourceEvent(event->button.which == SDL_TOUCH_MOUSEID ? ImGuiMouseSource_TouchScreen : ImGuiMouseSource_Mouse);
                io.AddMouseButtonEvent(mouseButton, (event->type == SDL_MOUSEBUTTONDOWN));
                bd->MouseButtonsDown = (event->type == SDL_MOUSEBUTTONDOWN) ? (bd->MouseButtonsDown | (1 << mouseButton)) :
                    (bd->MouseButtonsDown & ~(1 << mouseButton));
                return true;
            }
        case SDL_TEXTINPUT:
            {
                if (GetViewportForWindowID(event->text.windowID) == nullptr)
                    return false;
                io.AddInputCharactersUTF8(event->text.text);
                return true;
            }
        case SDL_KEYDOWN:
        case SDL_KEYUP:
            {
                if (GetViewportForWindowID(event->key.windowID) == nullptr)
                    return false;
                UpdateKeyModifiers(static_cast<SDL_Keymod>(event->key.keysym.mod));
                auto key = KeyEventToImGuiKey(event->key.keysym.sym, event->key.keysym.scancode);
                io.AddKeyEvent(key, (event->type == SDL_KEYDOWN));
                // To support legacy indexing (<1.87 user code). Legacy backend uses SDLK_*** as indices to IsKeyXXX() functions.
                io.SetKeyEventNativeData(key, event->key.keysym.sym, event->key.keysym.scancode, event->key.keysym.scancode);
                return true;
            }
        case SDL_WINDOWEVENT:
            {
                if (GetViewportForWindowID(event->window.windowID) == nullptr)
                    return false;
                // - When capturing mouse, SDL will send a bunch of conflicting LEAVE/ENTER event on every mouse move, but the final ENTER tends to be right.
                // - However we won't get a correct LEAVE event for a captured window.
                // - In some cases, when detaching a window from main viewport SDL may send SDL_WINDOWEVENT_ENTER one frame too late,
                //   causing SDL_WINDOWEVENT_LEAVE on previous frame to interrupt drag operation by clear mouse position. This is why
                //   we delay process the SDL_WINDOWEVENT_LEAVE events by one frame. See issue #5012 for details.
                Uint8 windowEvent = event->window.event;
                if (windowEvent == SDL_WINDOWEVENT_ENTER)
                {
                    bd->MouseWindowID = event->window.windowID;
                    bd->MouseLastLeaveFrame = 0;
                }
                if (windowEvent == SDL_WINDOWEVENT_LEAVE)
                    bd->MouseLastLeaveFrame = ImGui::GetFrameCount() + 1;
                if (windowEvent == SDL_WINDOWEVENT_FOCUS_GAINED)
                    io.AddFocusEvent(true);
                else if (event->window.event == SDL_WINDOWEVENT_FOCUS_LOST)
                    io.AddFocusEvent(false);
                return true;
            }
        case SDL_CONTROLLERDEVICEADDED:
        case SDL_CONTROLLERDEVICEREMOVED:
            {
                bd->WantUpdateGamepadsList = true;
                return true;
            }
        default:
            break;
    }
    return false;
}

ImGuiSDL2Backend::Data* ImGuiSDL2Backend::GetBackendData() noexcept
{
    return ImGui::GetCurrentContext() ? static_cast<Data*>(ImGui::GetIO().BackendPlatformUserData) : nullptr;
}

const char* ImGuiSDL2Backend::GetClipboardText(ImGuiContext*) noexcept
{
    auto* bd = GetBackendData();
    if (!bd)
        return nullptr;
    if (bd->ClipboardTextData)
        ::SDL_free(bd->ClipboardTextData);
    bd->ClipboardTextData = ::SDL_GetClipboardText();
    return bd->ClipboardTextData;
}

void ImGuiSDL2Backend::SetClipboardText(ImGuiContext*, const char* text) noexcept
{
    ::SDL_SetClipboardText(text);
}

void ImGuiSDL2Backend::SetImeData(ImGuiContext*, ImGuiViewport*, ImGuiPlatformImeData* data) noexcept
{
    if (data->WantVisible)
    {
        ::SDL_Rect r = {
            .x = static_cast<int>(data->InputPos.x),
            .y = static_cast<int>(data->InputPos.y),
            .w = 1,
            .h = static_cast<int>(data->InputLineHeight)
        };
        ::SDL_SetTextInputRect(&r);
    }
}

void ImGuiSDL2Backend::UpdateKeyModifiers(SDL_Keymod sdlKeyMods)
{
    ImGuiIO& io = ImGui::GetIO();
    io.AddKeyEvent(ImGuiMod_Ctrl, (sdlKeyMods & KMOD_CTRL) != 0);
    io.AddKeyEvent(ImGuiMod_Shift, (sdlKeyMods & KMOD_SHIFT) != 0);
    io.AddKeyEvent(ImGuiMod_Alt, (sdlKeyMods & KMOD_ALT) != 0);
    io.AddKeyEvent(ImGuiMod_Super, (sdlKeyMods & KMOD_GUI) != 0);
}

ImGuiViewport* ImGuiSDL2Backend::GetViewportForWindowID(Uint32 windowId) noexcept
{
    auto* bd = GetBackendData();
    return (bd && windowId == bd->WindowID) ? ImGui::GetMainViewport() : nullptr;
}

void ImGuiSDL2Backend::UpdateMouseData()
{
    auto* bd = GetBackendData();
    ImGuiIO& io = ImGui::GetIO();

    // We forward mouse input when hovered or captured (via SDL_MOUSEMOTION) or when focused (below)
#if SDL_HAS_CAPTURE_AND_GLOBAL_MOUSE
    // SDL_CaptureMouse() let the OS know e.g. that our imgui drag outside the SDL window boundaries shouldn't e.g. trigger other operations outside
    ::SDL_CaptureMouse((bd->MouseButtonsDown != 0) ? SDL_TRUE : SDL_FALSE);
    SDL_Window* focusedWindow = ::SDL_GetKeyboardFocus();
    const bool isAppFocused = (bd->Window == focusedWindow);
#else
    const bool isAppFocused = (::SDL_GetWindowFlags(bd->Window) & SDL_WINDOW_INPUT_FOCUS) != 0; // SDL 2.0.3 and non-windowed systems: single-viewport only
#endif
    if (isAppFocused)
    {
        // (Optional) Set OS mouse position from Dear ImGui if requested (rarely used, only when io.ConfigNavMoveSetMousePos is enabled by user)
        if (io.WantSetMousePos)
            ::SDL_WarpMouseInWindow(bd->Window, static_cast<int>(io.MousePos.x), static_cast<int>(io.MousePos.y));

        // (Optional) Fallback to provide mouse position when focused (SDL_MOUSEMOTION already provides this when hovered or captured)
        if (bd->MouseCanUseGlobalState && bd->MouseButtonsDown == 0)
        {
            int windowX, windowY, mouseXGlobal, mouseYGlobal;
            SDL_GetGlobalMouseState(&mouseXGlobal, &mouseYGlobal);
            SDL_GetWindowPosition(bd->Window, &windowX, &windowY);
            io.AddMousePosEvent(static_cast<float>(mouseXGlobal - windowX), static_cast<float>(mouseYGlobal - windowY));
        }
    }
}

void ImGuiSDL2Backend::UpdateMouseCursor() noexcept
{
    ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_NoMouseCursorChange)
        return;
    auto* bd = GetBackendData();

    ImGuiMouseCursor imguiCursor = ImGui::GetMouseCursor();
    if (io.MouseDrawCursor || imguiCursor == ImGuiMouseCursor_None)
    {
        // Hide OS mouse cursor if imgui is drawing it or if it wants no cursor
        ::SDL_ShowCursor(SDL_FALSE);
    }
    else
    {
        // Show OS mouse cursor
        SDL_Cursor* expectedCursor = bd->MouseCursors[imguiCursor] ? bd->MouseCursors[imguiCursor] :
            bd->MouseCursors[ImGuiMouseCursor_Arrow];
        if (bd->MouseLastCursor != expectedCursor)
        {
            ::SDL_SetCursor(expectedCursor); // SDL function doesn't have an early out (see #6113)
            bd->MouseLastCursor = expectedCursor;
        }
        ::SDL_ShowCursor(SDL_TRUE);
    }
}

void ImGuiSDL2Backend::CloseGamepads() noexcept
{
    auto* bd = GetBackendData();
    if (bd->GamepadMode != GamepadModes::Manual)
    {
        for (SDL_GameController* gamepad : bd->Gamepads)
            ::SDL_GameControllerClose(gamepad);
    }
    bd->Gamepads.resize(0);
}

void ImGuiSDL2Backend::UpdateGamepadButton(Data* bd, ImGuiIO& io, ImGuiKey key, SDL_GameControllerButton buttonNo)
{
    bool mergedValue = false;
    for (SDL_GameController* gamepad : bd->Gamepads)
        mergedValue |= ::SDL_GameControllerGetButton(gamepad, buttonNo) != 0;
    io.AddKeyEvent(key, mergedValue);
}

namespace
{
    inline float Saturate(float v) noexcept
    {
        return v < 0.0f ? 0.0f : v  > 1.0f ? 1.0f : v;
    }
}

void ImGuiSDL2Backend::UpdateGamepadAnalog(Data* bd, ImGuiIO& io, ImGuiKey key, SDL_GameControllerAxis axisNo, float v0, float v1)
{
    float mergedValue = 0.0f;
    for (SDL_GameController* gamepad : bd->Gamepads)
    {
        float vn = Saturate((static_cast<float>(::SDL_GameControllerGetAxis(gamepad, axisNo)) - v0) / static_cast<float>(v1 - v0));
        if (mergedValue < vn)
            mergedValue = vn;
    }
    io.AddKeyAnalogEvent(key, mergedValue > 0.1f, mergedValue);
}

void ImGuiSDL2Backend::UpdateGamepads()
{
    auto* bd = GetBackendData();
    ImGuiIO& io = ImGui::GetIO();

    // Update list of controller(s) to use
    if (bd->WantUpdateGamepadsList && bd->GamepadMode != GamepadModes::Manual)
    {
        CloseGamepads();
        int joystickCount = ::SDL_NumJoysticks();
        for (int n = 0; n < joystickCount; n++)
        {
            if (::SDL_IsGameController(n))
            {
                if (SDL_GameController* gamepad = ::SDL_GameControllerOpen(n))
                {
                    bd->Gamepads.push_back(gamepad);
                    if (bd->GamepadMode == GamepadModes::AutoFirst)
                        break;
                }
            }
        }
        bd->WantUpdateGamepadsList = false;
    }

    // FIXME: Technically feeding gamepad shouldn't depend on this now that they are regular inputs.
    if ((io.ConfigFlags & ImGuiConfigFlags_NavEnableGamepad) == 0)
        return;
    io.BackendFlags &= ~ImGuiBackendFlags_HasGamepad;
    if (bd->Gamepads.Size == 0)
        return;
    io.BackendFlags |= ImGuiBackendFlags_HasGamepad;

    // Update gamepad inputs
    const int kThumbDeadZone = 8000; // SDL_gamecontroller.h suggests using this value.
    UpdateGamepadButton(bd, io, ImGuiKey_GamepadStart,       SDL_CONTROLLER_BUTTON_START);
    UpdateGamepadButton(bd, io, ImGuiKey_GamepadBack,        SDL_CONTROLLER_BUTTON_BACK);
    UpdateGamepadButton(bd, io, ImGuiKey_GamepadFaceLeft,    SDL_CONTROLLER_BUTTON_X);              // Xbox X, PS Square
    UpdateGamepadButton(bd, io, ImGuiKey_GamepadFaceRight,   SDL_CONTROLLER_BUTTON_B);              // Xbox B, PS Circle
    UpdateGamepadButton(bd, io, ImGuiKey_GamepadFaceUp,      SDL_CONTROLLER_BUTTON_Y);              // Xbox Y, PS Triangle
    UpdateGamepadButton(bd, io, ImGuiKey_GamepadFaceDown,    SDL_CONTROLLER_BUTTON_A);              // Xbox A, PS Cross
    UpdateGamepadButton(bd, io, ImGuiKey_GamepadDpadLeft,    SDL_CONTROLLER_BUTTON_DPAD_LEFT);
    UpdateGamepadButton(bd, io, ImGuiKey_GamepadDpadRight,   SDL_CONTROLLER_BUTTON_DPAD_RIGHT);
    UpdateGamepadButton(bd, io, ImGuiKey_GamepadDpadUp,      SDL_CONTROLLER_BUTTON_DPAD_UP);
    UpdateGamepadButton(bd, io, ImGuiKey_GamepadDpadDown,    SDL_CONTROLLER_BUTTON_DPAD_DOWN);
    UpdateGamepadButton(bd, io, ImGuiKey_GamepadL1,          SDL_CONTROLLER_BUTTON_LEFTSHOULDER);
    UpdateGamepadButton(bd, io, ImGuiKey_GamepadR1,          SDL_CONTROLLER_BUTTON_RIGHTSHOULDER);
    UpdateGamepadAnalog(bd, io, ImGuiKey_GamepadL2,          SDL_CONTROLLER_AXIS_TRIGGERLEFT,  0.0f, 32767);
    UpdateGamepadAnalog(bd, io, ImGuiKey_GamepadR2,          SDL_CONTROLLER_AXIS_TRIGGERRIGHT, 0.0f, 32767);
    UpdateGamepadButton(bd, io, ImGuiKey_GamepadL3,          SDL_CONTROLLER_BUTTON_LEFTSTICK);
    UpdateGamepadButton(bd, io, ImGuiKey_GamepadR3,          SDL_CONTROLLER_BUTTON_RIGHTSTICK);
    UpdateGamepadAnalog(bd, io, ImGuiKey_GamepadLStickLeft,  SDL_CONTROLLER_AXIS_LEFTX,  -kThumbDeadZone, -32768);
    UpdateGamepadAnalog(bd, io, ImGuiKey_GamepadLStickRight, SDL_CONTROLLER_AXIS_LEFTX,  +kThumbDeadZone, +32767);
    UpdateGamepadAnalog(bd, io, ImGuiKey_GamepadLStickUp,    SDL_CONTROLLER_AXIS_LEFTY,  -kThumbDeadZone, -32768);
    UpdateGamepadAnalog(bd, io, ImGuiKey_GamepadLStickDown,  SDL_CONTROLLER_AXIS_LEFTY,  +kThumbDeadZone, +32767);
    UpdateGamepadAnalog(bd, io, ImGuiKey_GamepadRStickLeft,  SDL_CONTROLLER_AXIS_RIGHTX, -kThumbDeadZone, -32768);
    UpdateGamepadAnalog(bd, io, ImGuiKey_GamepadRStickRight, SDL_CONTROLLER_AXIS_RIGHTX, +kThumbDeadZone, +32767);
    UpdateGamepadAnalog(bd, io, ImGuiKey_GamepadRStickUp,    SDL_CONTROLLER_AXIS_RIGHTY, -kThumbDeadZone, -32768);
    UpdateGamepadAnalog(bd, io, ImGuiKey_GamepadRStickDown,  SDL_CONTROLLER_AXIS_RIGHTY, +kThumbDeadZone, +32767);
}
