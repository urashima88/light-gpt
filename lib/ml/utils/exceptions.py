import os
import sys
import logging
from typing import Type

_FATAL_FLAG = "_fatal_exception"

def fatal(
        exc_type: Type[Exception], 
        message: str, 
        logger: logging.Logger, 
        exit_code: int = 1
    ) -> Exception:
    exc = exc_type(message)
    setattr(exc, _FATAL_FLAG, (logger, exit_code))
    return exc

def handle_exception(exc_type, exc_value, exc_traceback):
    if hasattr(exc_value, _FATAL_FLAG):
        logger, exit_code = getattr(exc_value, _FATAL_FLAG)
        tb = exc_traceback
        while tb.tb_next:
            tb = tb.tb_next
        filename = os.path.basename(tb.tb_frame.f_code.co_filename)
        lineno = tb.tb_lineno
        logger.critical(
            exc_value,
            exc_info=(exc_type, exc_value, exc_traceback),
            extra={'real_filename': filename, 'real_lineno': lineno}
        )
        for handler in logger.handlers:
            handler.flush()
        sys.exit(exit_code)
    else:
        sys.__excepthook__(exc_type, exc_value, exc_traceback)