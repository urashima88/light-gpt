import os

from data.fineweb import FineWebDataset
from utils.log import setup_logger
from utils.exceptions import fatal

logger = setup_logger(
    os.environ.get("LOG_LEVEL", "INFO"),
    bool(int(os.environ.get("USE_STREAM_HANDLER", 1))),
    bool(int(os.environ.get("USE_FILE_HANDLER", 0)))
)

def get_dataset(
        dataset_name: str,
        **kwargs
    ):
    dataset_name = dataset_name.lower()
    match(dataset_name):
        case "fineweb":
            dataset = FineWebDataset(**kwargs)
        case _:
            raise fatal(ValueError, f"dataset_name was not set", logger)
        
    return dataset