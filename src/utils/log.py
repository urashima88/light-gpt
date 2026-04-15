import logging
from logging import Logger
import re
import os

class Level:
    def __init__(self, type, color):
        self.type = type
        self.color = color

# Used ANSI color codes
LEVELS = {
    "DEBUG": Level(logging.DEBUG, '\033[36m'),       # Cyan
    "INFO": Level(logging.INFO, '\033[32m'),         # Green
    "WARNING": Level(logging.WARNING, '\033[33m'),   # Yellow
    "ERROR": Level(logging.ERROR, '\033[31m'),       # Red
    "CRITICAL": Level(logging.CRITICAL, '\033[35m'), # Magenta
}

class DummyWandb:
    """Useful if we wish to not use wandb but have all the same signatures"""
    def __init__(self):
        pass
    def log(self, *args, **kwargs):
        pass
    def finish(self):
        pass

class CustomFormatter(logging.Formatter):
    """Custom formatter that adds colors to log messages."""
    RESET = '\033[0m'
    BOLD = '\033[1m'
    def format(self, record):
        if hasattr(record, 'real_filename'):
            record.filename = record.real_filename
        if hasattr(record, 'real_lineno'):
            record.lineno = record.real_lineno
        # Add color to the level name
        levelname = record.levelname
        if levelname in LEVELS:
            record.levelname = f"{LEVELS[levelname].color}{self.BOLD}{levelname}{self.RESET}"
        # Format the message
        message = super().format(record)
        # Add color to specific parts of the message
        if levelname == 'INFO':
            # Highlight numbers and percentages
            message = re.sub(r'(\d+\.?\d*\s*(?:GB|MB|%|docs))', rf'{self.BOLD}\1{self.RESET}', message)
            message = re.sub(r'(Shard \d+)', rf'{LEVELS["INFO"].color}{self.BOLD}\1{self.RESET}', message)
        return message
    
class RankFilter(logging.Filter):
    def __init__(self, master_only=True):
        super().__init__()
        self.master_only = master_only

    def filter(self, record):
        if not self.master_only:
            return True
        rank = int(os.environ.get('RANK', 0))
        return rank == 0

def setup_logger(
        log_level: str, 
        use_stream_handler: bool,
        use_file_handler: bool,
        logs_dir: str = "../logs"
    ) -> Logger:
    log_level = log_level.upper()
    if log_level in LEVELS:
        level = LEVELS[log_level].type
    else:
        print(f"The selected logging level {log_level} is incorrect, switching to INFO level.")
        level = logging.INFO

    os.makedirs(logs_dir, exist_ok=True)

    handlers = []
    if use_stream_handler:
        stream_handler = logging.StreamHandler()
        stream_handler.setFormatter(CustomFormatter('%(asctime)s - %(filename)s:%(lineno)d - %(levelname)s - %(message)s'))
        stream_handler.addFilter(RankFilter(master_only=True))
        handlers.append(stream_handler)
    if use_file_handler:
        file_handler = logging.FileHandler(f"{logs_dir}/{__file__}.log", mode="w")
        file_handler.setFormatter(CustomFormatter('%(asctime)s - %(filename)s:%(lineno)d - %(levelname)s - %(message)s'))
        file_handler.addFilter(RankFilter(master_only=True))
        handlers.append(file_handler)

    logging.basicConfig(
        level=level,
        handlers=handlers
    )
    
    return logging.getLogger()