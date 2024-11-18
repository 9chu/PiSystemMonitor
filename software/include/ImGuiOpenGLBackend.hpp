/**
 * @file
 * @author chu
 * @date 2024/11/17
 */
#pragma once
#include <imgui.h>
#include <Result.hpp>

class ImGuiOpenGLBackend
{
public:
    static void Initialize();
    static void Shutdown() noexcept;

    static void NewFrame() noexcept;
    static void RenderDrawData(ImDrawData* drawData) noexcept;
    static void Clear(int width, int height) noexcept;

private:
    static void CreateFontsTexture() noexcept;
    static void DestroyFontsTexture() noexcept;
    static void CreateDeviceObjects() noexcept;
    static void DestroyDeviceObjects() noexcept;
};
