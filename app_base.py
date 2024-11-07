# -*- coding: utf-8 -*-
import io
import time
import dearpygui.dearpygui as dpg


def _is_raspberrypi():
    try:
        with io.open("/sys/firmware/devicetree/base/model", "r") as m:
            if "raspberry pi" in m.read().lower():
                return True
    except Exception:
        pass
    return False


class AppBase:
    def __init__(self, title, width=480, height=320):
        self._title = title
        self._width = width
        self._height = height

        self._fonts = {}
        self._widgets = {}

    def on_start(self):
        pass

    def on_layout(self):
        pass

    def on_frame(self):
        pass

    def on_stop(self):
        pass

    def run(self):
        dpg.create_context()
        dpg.create_viewport(title=self._title, width=self._width, height=self._height, resizable=False, decorated=False)
        dpg.setup_dearpygui()

        with dpg.font_registry():
            self.on_start()
        with dpg.window(tag="Main Window"):
            self.on_layout()

        dpg.show_viewport()
        dpg.set_primary_window("Main Window", True)
        if _is_raspberrypi():
            dpg.toggle_viewport_fullscreen()

        try:
            while dpg.is_dearpygui_running():
                self.on_frame()
                dpg.render_dearpygui_frame()
                time.sleep(0.2)
        except KeyboardInterrupt:
            pass

        self.on_stop()
        dpg.destroy_context()

    def register_font(self, name, path, size):
        self._fonts[name] = dpg.add_font(path, size)
