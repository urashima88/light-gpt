import os
from pathlib import Path
import glob

import torch
from torch import Tensor
import numpy as np

from data.base_dataset import BaseDataset
from utils.log import setup_logger

logger = setup_logger(
    os.environ.get("LOG_LEVEL", "INFO"),
    bool(int(os.environ.get("USE_STREAM_HANDLER", 1))),
    bool(int(os.environ.get("USE_FILE_HANDLER", 0)))
)


class FineWebDataset(BaseDataset):
    def __init__(self, pattern: str, seq_len: int):
        '''
        ...

        :param pattern: a string with a glob pattern to search for validation set files.
        :type pattern: str
        :param seq_len: the length of the context the model works with (the number of tokens fed to the model in one forward step).
        :type seq_len: int
        '''
        files = [Path(p) for p in sorted(glob.glob(pattern))]
        if not files:
            logger.error("")
            raise FileNotFoundError(f"No files found for pattern: {pattern}")
        
        tokens = torch.cat([self.load_data_shard(file) for file in files]).contiguous()
        usable = ((tokens.numel() - 1) // seq_len) * seq_len
        if usable <= 0:
            logger.error("")
            raise ValueError(f"Validation split is too short for TRAIN_SEQ_LEN={seq_len}")
        self.tokens = tokens[: usable + 1]

    @staticmethod
    def load_data_shard(file: Path) -> Tensor:
        '''
        Extracts a one-dimensional array of tokens in uint16 format from a file and returns it as a PyTorch tensor.
        '''
        header_bytes = 256 * np.dtype("<i4").itemsize
        token_bytes = np.dtype("<u2").itemsize

        header = np.fromfile(file, dtype="<i4", count=256)
        # fields in header: 
        # 0 - magic number = 20240520 (20.05.2024 - the day the format was created or the dataset was exported), confirming that the file is indeed a FineWeb shard
        # 1 - format version
        # 2 - number of tokens
        if header.size != 256 or int(header[0]) != 20240520 or int(header[1]) != 1:
            logger.error("")
            raise ValueError(f"Unexpected shard header for {file}")
        
        num_tokens = int(header[2])
        expected_size = header_bytes + num_tokens * token_bytes
        if file.stat().st_size != expected_size:
            logger.error("")
            raise ValueError(f"Shard size mismatch for {file}: expected {expected_size} bytes")
        
        tokens_np = np.fromfile(file, dtype="<u2", count=num_tokens, offset=header_bytes)
        if tokens_np.size != num_tokens:
            logger.error("")
            raise ValueError(f"Short read for {file}")
        
        return torch.from_numpy(tokens_np.astype(np.uint16, copy=False))

