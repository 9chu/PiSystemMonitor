#!/usr/bin/env python3
# -*- coding: utf-8 -*-
import io
import os
import math
import datetime
from log import setup_console_logger
from app import App


def main():
    setup_console_logger()
    app = App()
    app.run()


if __name__ == "__main__":
    main()
