import os

from utils.log import setup_logger

logger = setup_logger(
    os.environ.get("LOG_LEVEL", "INFO"),
    bool(int(os.environ.get("USE_STREAM_HANDLER", 1))),
    bool(int(os.environ.get("USE_FILE_HANDLER", 0)))
)

# flash_attention.py

def define_using_fa3():
    ...