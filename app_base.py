# -*- coding: utf-8 -*-
import os
import ctypes
import logging
import imgui
import OpenGL.GL as gl
from sdl2 import *
from imgui.integrations.opengl import FixedPipelineRenderer


MOUSE_WHEEL_OFFSET_SCALE = 0.5

def _is_raspberrypi() -> bool:
    try:
        with io.open('/sys/firmware/devicetree/base/model', 'r') as m:
            if 'raspberry pi' in m.read().lower():
                return True
    except Exception:
        pass
    return False


class ApplicationBase:
    def __init__(self, title="PiSystemMonitor", width=480, height=320):
        self.running = False
        self.fonts = {}

        # 初始化 SDL
        if SDL_Init(SDL_INIT_EVERYTHING) < 0:
            logging.error(f"SDL could not initialize, error: {SDL_GetError().decode('utf-8')}")
            raise RuntimeError("SDL could not initialize")

        # 配置 OpenGL
        SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8)
        SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8)
        SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8)
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1)
        SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1)
        if _is_raspberrypi():
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES)
        else:
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY)
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2)
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0)

        # 其他选项
        SDL_SetHint(SDL_HINT_MAC_CTRL_CLICK_EMULATE_RIGHT_CLICK, b"1")
        SDL_SetHint(SDL_HINT_VIDEO_HIGHDPI_DISABLED, b"1")

        # 创建窗口
        self.window = SDL_CreateWindow(
            title.encode("utf-8"),
            SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
            width, height,
            SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN)
        if self.window is None:
            logging.error(f"SDL_CreateWindow fail, error: {SDL_GetError().decode('utf-8')}")
            raise RuntimeError("Could not create window")

        # 创建 OpenGL
        self.gl_context = SDL_GL_CreateContext(self.window)
        if self.gl_context is None:
            logging.error(f"SDL_GL_CreateContext fail, error: {SDL_GetError().decode('utf-8')}")
            raise RuntimeError("Could not create OpenGL context")
        SDL_GL_SetSwapInterval(1)
        SDL_GL_MakeCurrent(self.window, self.gl_context)

        # 初始化 imgui
        imgui.create_context()
        self.imgui_io = imgui.get_io()
        self._refresh_display_size()
        self._map_keys()

        # 创建 Renderer
        self.imgui_renderer = FixedPipelineRenderer()

    def _refresh_display_size(self):
        width_ptr = ctypes.pointer(ctypes.c_int(0))
        height_ptr = ctypes.pointer(ctypes.c_int(0))
        SDL_GetWindowSize(self.window, width_ptr, height_ptr)
        self.imgui_io.display_size = width_ptr[0], height_ptr[0]
        self.imgui_io.display_fb_scale = 1, 1

    def _map_keys(self):
        key_map = self.imgui_io.key_map
        key_map[imgui.KEY_TAB] = SDLK_TAB
        key_map[imgui.KEY_LEFT_ARROW] = SDL_SCANCODE_LEFT
        key_map[imgui.KEY_RIGHT_ARROW] = SDL_SCANCODE_RIGHT
        key_map[imgui.KEY_UP_ARROW] = SDL_SCANCODE_UP
        key_map[imgui.KEY_DOWN_ARROW] = SDL_SCANCODE_DOWN
        key_map[imgui.KEY_PAGE_UP] = SDL_SCANCODE_PAGEUP
        key_map[imgui.KEY_PAGE_DOWN] = SDL_SCANCODE_PAGEDOWN
        key_map[imgui.KEY_HOME] = SDL_SCANCODE_HOME
        key_map[imgui.KEY_END] = SDL_SCANCODE_END
        key_map[imgui.KEY_DELETE] = SDLK_DELETE
        key_map[imgui.KEY_BACKSPACE] = SDLK_BACKSPACE
        key_map[imgui.KEY_ENTER] = SDLK_RETURN
        key_map[imgui.KEY_ESCAPE] = SDLK_ESCAPE
        key_map[imgui.KEY_A] = SDLK_a
        key_map[imgui.KEY_C] = SDLK_c
        key_map[imgui.KEY_V] = SDLK_v
        key_map[imgui.KEY_X] = SDLK_x
        key_map[imgui.KEY_Y] = SDLK_y
        key_map[imgui.KEY_Z] = SDLK_z

    def _handle_event(self, event: SDL_Event):
        if event.type == SDL_MOUSEWHEEL:
            mouse_wheel_offset = event.wheel.y * MOUSE_WHEEL_OFFSET_SCALE
            self.imgui_io.mouse_wheel = mouse_wheel_offset
        elif event.type == SDL_MOUSEBUTTONDOWN:
            if event.button.button == SDL_BUTTON_LEFT:
                self.imgui_io.mouse_down[0] = True
            elif event.button.button == SDL_BUTTON_RIGHT:
                self.imgui_io.mouse_down[1] = True
            elif event.button.button == SDL_BUTTON_MIDDLE:
                self.imgui_io.mouse_down[2] = True
        elif event.type == SDL_MOUSEBUTTONUP:
            if event.button.button == SDL_BUTTON_LEFT:
                self.imgui_io.mouse_down[0] = False
            elif event.button.button == SDL_BUTTON_RIGHT:
                self.imgui_io.mouse_down[1] = False
            elif event.button.button == SDL_BUTTON_MIDDLE:
                self.imgui_io.mouse_down[2] = False
        elif event.type == SDL_MOUSEMOTION:
            self.imgui_io.mouse_pos = event.motion.x, event.motion.y
        elif event.type == SDL_KEYUP or event.type == SDL_KEYDOWN:
            key = event.key.keysym.sym & ~SDLK_SCANCODE_MASK
            if key < SDL_NUM_SCANCODES:
                self.imgui_io.keys_down[key] = event.type == SDL_KEYDOWN

            modifier = SDL_GetModState()
            self.imgui_io.key_shift = ((modifier & KMOD_SHIFT) != 0)
            self.imgui_io.key_ctrl = ((modifier & KMOD_CTRL) != 0)
            self.imgui_io.key_alt = ((modifier & KMOD_ALT) != 0)
        elif event.type == SDL_TEXTINPUT:
            for char in event.text.text.decode('utf-8'):
                self.imgui_io.add_input_character(ord(char))

    def register_font(self, name: str, filename: str, size: float):
        font = self.imgui_io.fonts.add_font_from_file_ttf(os.path.join("res", filename), size)
        self.imgui_renderer.refresh_font_texture()
        self.fonts[name] = font

    def get_font(self, name: str):
        return self.fonts[name]

    def run(self):
        self.on_start()
        self.running = True

        last_tick = SDL_GetTicks64() / 1000.0
        event = SDL_Event()
        try:
            while self.running:
                while SDL_PollEvent(ctypes.byref(event)) != 0:
                    if event.type == SDL_QUIT:
                        self.running = False
                        break
                    self._handle_event(event)

                current_tick = SDL_GetTicks64() / 1000.0
                delta_time = max(0.001, current_tick - last_tick)
                last_tick = current_tick

                self._refresh_display_size()
                self.imgui_io.delta_time = delta_time

                # 渲染 imgui
                imgui.new_frame()
                self.on_frame(delta_time)
                imgui.render()
                draw_data = imgui.get_draw_data()

                # 绘图
                gl.glClearColor(1., 1., 1., 1)
                gl.glClear(gl.GL_COLOR_BUFFER_BIT)
                self.imgui_renderer.render(draw_data)
                SDL_GL_SwapWindow(self.window)
        except KeyboardInterrupt:
            self.running = False

        self.on_stop()

    def close(self):
        self.imgui_renderer.shutdown()
        SDL_GL_DeleteContext(self.gl_context)
        SDL_DestroyWindow(self.window)
        SDL_Quit()

    def on_start(self):
        pass

    def on_frame(self, delta_time: float):
        imgui.show_test_window()

    def on_stop(self):
        pass
