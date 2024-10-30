# -*- coding: utf-8 -*-
import os
import sys
import logging


class MaxLevelFilter(logging.Filter):
    def __init__(self, level):
        super(MaxLevelFilter, self).__init__()
        self.level = level

    def filter(self, record):
        return record.levelno < self.level


def setup_console_logger(log_format = "[%(asctime)s][%(levelname)s][%(module)s:%(funcName)s:%(lineno)d] %(message)s"):
    if os.getenv("DEBUG") == "1":
        verbose = True
    else:
        verbose = False

    logger = logging.getLogger()
    if verbose:
        logger.setLevel(logging.DEBUG)
    else:
        logger.setLevel(logging.INFO)

    # 当且仅当默认 logger 中没有其他 handler 时，才添加 handler
    # 用于防止在 multiprocessing 中重复添加 handler
    if len(logger.handlers) > 0:
        return

    formatter = logging.Formatter(log_format)

    # 使用 stdout 输出 ERROR 以下级别日志
    logger_stdout = logging.StreamHandler(sys.stdout)
    logger_stdout.addFilter(MaxLevelFilter(logging.ERROR))
    logger_stdout.setLevel(logging.DEBUG)
    logger_stdout.setFormatter(formatter)
    logger.addHandler(logger_stdout)

    # 使用 stderr 输出 ERROR 及以上级别日志
    logger_stderr = logging.StreamHandler(sys.stderr)
    logger_stderr.setLevel(logging.ERROR)
    logger_stderr.setFormatter(formatter)
    logger.addHandler(logger_stderr)
