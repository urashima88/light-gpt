import logging
import re
import os

def print0(s="", **kwargs) -> None:
    ddp_rank = int(os.environ.get('RANK', 0))
    if ddp_rank == 0:
        print(s, **kwargs)

class DummyWandb:
    """Useful if we wish to not use wandb but have all the same signatures"""
    def __init__(self):
        pass
    def log(self, *args, **kwargs):
        pass
    def finish(self):
        pass

class ColoredFormatter(logging.Formatter):
    """Custom formatter that adds colors to log messages."""
    # ANSI color codes
    COLORS = {
        'DEBUG': '\033[36m',    # Cyan
        'INFO': '\033[32m',     # Green
        'WARNING': '\033[33m',  # Yellow
        'ERROR': '\033[31m',    # Red
        'CRITICAL': '\033[35m', # Magenta
    }
    RESET = '\033[0m'
    BOLD = '\033[1m'
    def format(self, record):
        # Add color to the level name
        levelname = record.levelname
        if levelname in self.COLORS:
            record.levelname = f"{self.COLORS[levelname]}{self.BOLD}{levelname}{self.RESET}"
        # Format the message
        message = super().format(record)
        # Add color to specific parts of the message
        if levelname == 'INFO':
            # Highlight numbers and percentages
            message = re.sub(r'(\d+\.?\d*\s*(?:GB|MB|%|docs))', rf'{self.BOLD}\1{self.RESET}', message)
            message = re.sub(r'(Shard \d+)', rf'{self.COLORS["INFO"]}{self.BOLD}\1{self.RESET}', message)
        return message
    

def setup_logger(
        log_level: str, 
        use_stream_handler: bool,
        use_file_handler: bool,
        logs_dir: str = "logs"
    ):
    match (log_level.upper()):
        case "DEBUG":
            level = logging.DEBUG
        case "INFO":
            level = logging.INFO
        case "WARNING":
            level = logging.WARNING
        case "ERROR":
            level = logging.ERROR
        case "CRITICAL":
            level = logging.CRITICAL
        case _:
            print0(f"The selected logging level {log_level} is incorrect, switching to INFO level.")
            level = logging.INFO

    os.makedirs(logs_dir, exist_ok=True)

    handlers = []
    if use_stream_handler:
        stream_handler = logging.StreamHandler()
        stream_handler.setFormatter(ColoredFormatter('%(asctime)s - %(filename)s - %(levelname)s - %(message)s'))
        handlers.append(stream_handler)
    if use_file_handler:
        file_handler = logging.FileHandler(f"{logs_dir}/{__file__}.log", mode="w")
        stream_handler.setFormatter(ColoredFormatter('%(asctime)s - %(filename)s - %(levelname)s - %(message)s'))
        handlers.append(file_handler)

    logging.basicConfig(
        level=level,
        handlers=handlers
    )
    
    return logging.getLogger()