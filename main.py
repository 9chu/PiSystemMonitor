#!python3
# -*- coding: utf-8 -*-
import log
from app import Application


def main():
    log.setup_console_logger()

    inst = Application()
    inst.run()
    inst.close()


if __name__ == "__main__":
    main()
