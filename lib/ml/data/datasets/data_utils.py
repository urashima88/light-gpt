from logging import Logger

from data.fineweb import FineWebDataset
from utils.exceptions import fatal

def get_dataset(
        dataset_name: str,
        logger: Logger,
        **kwargs
    ):
    dataset_name = dataset_name.lower()
    match(dataset_name):
        case "fineweb":
            dataset = FineWebDataset(logger, **kwargs)
        case _:
            raise fatal(ValueError, f"dataset_name was not set", logger)
        
    return dataset