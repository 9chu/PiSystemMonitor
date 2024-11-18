/**
 * @file
 * @author chu
 * @date 2024/11/17
 */
#include <ImGuiOpenGLBackend.hpp>

#if defined(_WIN32) && !defined(APIENTRY)
#define APIENTRY __stdcall                  // It is customary to use APIENTRY for OpenGL function pointer declarations on all platforms.  Additionally, the Windows OpenGL header needs APIENTRY.
#endif
#if defined(_WIN32) && !defined(WINGDIAPI)
#define WINGDIAPI __declspec(dllimport)     // Some Windows OpenGL headers need this
#endif
#if defined(__APPLE__)
#define GL_SILENCE_DEPRECATION
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

using namespace std;

namespace
{
    struct ImGUIOpenGLData
    {
        GLuint FontTexture = 0;
    };

    ImGUIOpenGLData* GetBackendData() noexcept
    {
        return ImGui::GetCurrentContext() ? static_cast<ImGUIOpenGLData*>(ImGui::GetIO().BackendRendererUserData) : nullptr;
    }

    void SetupRenderState(ImDrawData* drawData, int fbWidth, int fbHeight) noexcept
    {
        // Setup render state: alpha-blending enabled, no face culling, no depth testing, scissor enabled, vertex/texcoord/color pointers, polygon fill.
        ::glEnable(GL_BLEND);
        ::glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        //glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA); // In order to composite our output buffer we need to preserve alpha
        ::glDisable(GL_CULL_FACE);
        ::glDisable(GL_DEPTH_TEST);
        ::glDisable(GL_STENCIL_TEST);
        ::glDisable(GL_LIGHTING);
        ::glDisable(GL_COLOR_MATERIAL);
        ::glEnable(GL_SCISSOR_TEST);
        ::glEnableClientState(GL_VERTEX_ARRAY);
        ::glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        ::glEnableClientState(GL_COLOR_ARRAY);
        ::glDisableClientState(GL_NORMAL_ARRAY);
        ::glEnable(GL_TEXTURE_2D);
        ::glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        ::glShadeModel(GL_SMOOTH);
        ::glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

        // If you are using this code with non-legacy OpenGL header/contexts (which you should not, prefer using imgui_impl_opengl3.cpp!!),
        // you may need to backup/reset/restore other state, e.g. for current shader using the commented lines below.
        // (DO NOT MODIFY THIS FILE! Add the code in your calling function)
        //   GLint last_program;
        //   glGetIntegerv(GL_CURRENT_PROGRAM, &last_program);
        //   glUseProgram(0);
        //   ImGui_ImplOpenGL2_RenderDrawData(...);
        //   glUseProgram(last_program)
        // There are potentially many more states you could need to clear/setup that we can't access from default headers.
        // e.g. glBindBuffer(GL_ARRAY_BUFFER, 0), glDisable(GL_TEXTURE_CUBE_MAP).

        // Setup viewport, orthographic projection matrix
        // Our visible imgui space lies from draw_data->DisplayPos (top left) to draw_data->DisplayPos+data_data->DisplaySize (bottom right). DisplayPos is (0,0) for single viewport apps.
        ::glViewport(0, 0, static_cast<GLsizei>(fbWidth), static_cast<GLsizei>(fbHeight));
        ::glMatrixMode(GL_PROJECTION);
        ::glPushMatrix();
        ::glLoadIdentity();
        ::glOrtho(drawData->DisplayPos.x, drawData->DisplayPos.x + drawData->DisplaySize.x,
            drawData->DisplayPos.y + drawData->DisplaySize.y, drawData->DisplayPos.y, -1.0f, +1.0f);
        ::glMatrixMode(GL_MODELVIEW);
        ::glPushMatrix();
        ::glLoadIdentity();
    }

}

void ImGuiOpenGLBackend::Initialize()
{
    ImGuiIO& io = ImGui::GetIO();
    IMGUI_CHECKVERSION();
    IM_ASSERT(io.BackendRendererUserData == nullptr && "Already initialized a renderer backend!");

    // Setup backend capabilities flags
    auto* bd = IM_NEW(ImGUIOpenGLData)();
    io.BackendRendererUserData = (void*)bd;
    io.BackendRendererName = "imgui_impl_opengl2";
}

void ImGuiOpenGLBackend::Shutdown() noexcept
{
    auto* bd = GetBackendData();
    IM_ASSERT(bd != nullptr && "No renderer backend to shutdown, or already shutdown?");
    ImGuiIO& io = ImGui::GetIO();

    DestroyDeviceObjects();
    io.BackendRendererName = nullptr;
    io.BackendRendererUserData = nullptr;
    IM_DELETE(bd);
}

void ImGuiOpenGLBackend::NewFrame() noexcept
{
    auto* bd = GetBackendData();
    IM_ASSERT(bd != nullptr && "Context or backend not initialized! Did you call ImGui_ImplOpenGL2_Init()?");

    if (!bd->FontTexture)
        CreateDeviceObjects();
    if (!bd->FontTexture)
        CreateFontsTexture();
}

void ImGuiOpenGLBackend::RenderDrawData(ImDrawData* drawData) noexcept
{
    // Avoid rendering when minimized, scale coordinates for retina displays (screen coordinates != framebuffer coordinates)
    int fbWidth = static_cast<int>(drawData->DisplaySize.x * drawData->FramebufferScale.x);
    int fbHeight = static_cast<int>(drawData->DisplaySize.y * drawData->FramebufferScale.y);
    if (fbWidth == 0 || fbHeight == 0)
        return;

    // Backup GL state
    GLint lastTexture; ::glGetIntegerv(GL_TEXTURE_BINDING_2D, &lastTexture);
    GLint lastPolygonMode[2]; ::glGetIntegerv(GL_POLYGON_MODE, lastPolygonMode);
    GLint lastViewport[4]; ::glGetIntegerv(GL_VIEWPORT, lastViewport);
    GLint lastScissorBox[4]; ::glGetIntegerv(GL_SCISSOR_BOX, lastScissorBox);
    GLint lastShadeModel; ::glGetIntegerv(GL_SHADE_MODEL, &lastShadeModel);
    GLint lastTexEnvMode; ::glGetTexEnviv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, &lastTexEnvMode);
    ::glPushAttrib(GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT | GL_TRANSFORM_BIT);

    // Setup desired GL state
    SetupRenderState(drawData, fbWidth, fbHeight);

    // Will project scissor/clipping rectangles into framebuffer space
    ImVec2 clip_off = drawData->DisplayPos;         // (0,0) unless using multi-viewports
    ImVec2 clip_scale = drawData->FramebufferScale; // (1,1) unless using retina display which are often (2,2)

    // Render command lists
    for (int n = 0; n < drawData->CmdListsCount; n++)
    {
        const ImDrawList* drawList = drawData->CmdLists[n];
        const ImDrawVert* vertexBuffer = drawList->VtxBuffer.Data;
        const ImDrawIdx* indexBuffer = drawList->IdxBuffer.Data;
        ::glVertexPointer(2, GL_FLOAT, sizeof(ImDrawVert),
            static_cast<const GLvoid*>(reinterpret_cast<const char*>(vertexBuffer) + offsetof(ImDrawVert, pos)));
        ::glTexCoordPointer(2, GL_FLOAT, sizeof(ImDrawVert),
            static_cast<const GLvoid*>(reinterpret_cast<const char*>(vertexBuffer) + offsetof(ImDrawVert, uv)));
        ::glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(ImDrawVert),
            static_cast<const GLvoid*>(reinterpret_cast<const char*>(vertexBuffer) + offsetof(ImDrawVert, col)));

        for (int i = 0; i < drawList->CmdBuffer.Size; i++)
        {
            const ImDrawCmd* pcmd = &drawList->CmdBuffer[i];
            if (pcmd->UserCallback)
            {
                // User callback, registered via ImDrawList::AddCallback()
                // (ImDrawCallback_ResetRenderState is a special callback value used by the user to request the renderer to reset render state.)
                if (pcmd->UserCallback == ImDrawCallback_ResetRenderState)
                    SetupRenderState(drawData, fbWidth, fbHeight);
                else
                    pcmd->UserCallback(drawList, pcmd);
            }
            else
            {
                // Project scissor/clipping rectangles into framebuffer space
                ImVec2 clipMin((pcmd->ClipRect.x - clip_off.x) * clip_scale.x, (pcmd->ClipRect.y - clip_off.y) * clip_scale.y);
                ImVec2 clipMax((pcmd->ClipRect.z - clip_off.x) * clip_scale.x, (pcmd->ClipRect.w - clip_off.y) * clip_scale.y);
                if (clipMax.x <= clipMin.x || clipMax.y <= clipMin.y)
                    continue;

                // Apply scissor/clipping rectangle (Y is inverted in OpenGL)
                ::glScissor(static_cast<int>(clipMin.x), static_cast<int>(static_cast<float>(fbHeight) - clipMax.y),
                    static_cast<int>(clipMax.x - clipMin.x), static_cast<int>(clipMax.y - clipMin.y));

                // Bind texture, Draw
                ::glBindTexture(GL_TEXTURE_2D, static_cast<GLuint>(static_cast<intptr_t>(pcmd->GetTexID())));
                ::glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(pcmd->ElemCount), sizeof(ImDrawIdx) == 2 ?
                    GL_UNSIGNED_SHORT : GL_UNSIGNED_INT, indexBuffer + pcmd->IdxOffset);
            }
        }
    }

    // Restore modified GL state
    ::glDisableClientState(GL_COLOR_ARRAY);
    ::glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    ::glDisableClientState(GL_VERTEX_ARRAY);
    ::glBindTexture(GL_TEXTURE_2D, static_cast<GLuint>(lastTexture));
    ::glMatrixMode(GL_MODELVIEW);
    ::glPopMatrix();
    ::glMatrixMode(GL_PROJECTION);
    ::glPopMatrix();
    ::glPopAttrib();
    ::glPolygonMode(GL_FRONT, static_cast<GLenum>(lastPolygonMode[0])); ::glPolygonMode(GL_BACK, static_cast<GLenum>(lastPolygonMode[1]));
    ::glViewport(lastViewport[0], lastViewport[1], static_cast<GLsizei>(lastViewport[2]), static_cast<GLsizei>(lastViewport[3]));
    ::glScissor(lastScissorBox[0], lastScissorBox[1], static_cast<GLsizei>(lastScissorBox[2]), static_cast<GLsizei>(lastScissorBox[3]));
    ::glShadeModel(lastShadeModel);
    ::glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, lastTexEnvMode);
}

void ImGuiOpenGLBackend::Clear(int width, int height) noexcept
{
    static ImVec4 kClearColor = ImVec4(0.f, 0.f, 0.f, 1.f);

    ::glViewport(0, 0, width, height);
    ::glClearColor(kClearColor.x * kClearColor.w, kClearColor.y * kClearColor.w, kClearColor.z * kClearColor.w, kClearColor.w);
    ::glClear(GL_COLOR_BUFFER_BIT);
}

void ImGuiOpenGLBackend::CreateFontsTexture() noexcept
{
    // Build texture atlas
    ImGuiIO& io = ImGui::GetIO();
    auto* bd = GetBackendData();
    unsigned char* pixels;
    int width = 0, height = 0;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);   // Load as RGBA 32-bit (75% of the memory is wasted, but default font is so small) because it is more likely to be compatible with user's existing shaders. If your ImTextureId represent a higher-level concept than just a GL texture id, consider calling GetTexDataAsAlpha8() instead to save on GPU memory.

    // Upload texture to graphics system
    // (Bilinear sampling is required by default. Set 'io.Fonts->Flags |= ImFontAtlasFlags_NoBakedLines' or 'style.AntiAliasedLinesUseTex = false' to allow point/nearest sampling)
    GLint lastTexture;
    ::glGetIntegerv(GL_TEXTURE_BINDING_2D, &lastTexture);
    ::glGenTextures(1, &bd->FontTexture);
    ::glBindTexture(GL_TEXTURE_2D, bd->FontTexture);
    ::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    ::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    ::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    ::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    ::glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    ::glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

    // Store our identifier
    io.Fonts->SetTexID(static_cast<ImTextureID>(static_cast<intptr_t>(bd->FontTexture)));

    // Restore state
    ::glBindTexture(GL_TEXTURE_2D, lastTexture);
}

void ImGuiOpenGLBackend::DestroyFontsTexture() noexcept
{
    ImGuiIO& io = ImGui::GetIO();
    auto* bd = GetBackendData();
    if (bd->FontTexture)
    {
        ::glDeleteTextures(1, &bd->FontTexture);
        io.Fonts->SetTexID(0);
        bd->FontTexture = 0;
    }
}

void ImGuiOpenGLBackend::CreateDeviceObjects() noexcept
{
    CreateFontsTexture();
}

void ImGuiOpenGLBackend::DestroyDeviceObjects() noexcept
{
    DestroyFontsTexture();
}
