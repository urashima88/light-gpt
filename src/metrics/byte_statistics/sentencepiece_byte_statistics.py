from typing import Tuple
from logging import Logger

import torch
from torch import Tensor
import numpy as np

from metrics.byte_statistics.byte_statistics_calculator import ByteStatisticsCalculator
from tokenizers.sentencepiece_tokenizer import SentencePieceTokenizer

class SentencePieceByteStatistics(ByteStatisticsCalculator):
    def __init__(self, tokenizer: SentencePieceTokenizer, model_vocab_size: int, logger: Logger):
        self.tokenizer = tokenizer
        self.model_vocab_size = model_vocab_size
        self.logger = logger

    def build_luts(self, device: torch.device) -> Tuple[Tensor, Tensor, Tensor]:
        sp_vocab_size = int(self.tokenizer.get_vocab_size())
        table_size = max(sp_vocab_size, self.model_vocab_size)

        base_bytes_np = np.zeros((table_size,), dtype=np.int16)
        has_leading_space_np = np.zeros((table_size,), dtype=np.bool_)
        is_boundary_token_np = np.ones((table_size,), dtype=np.bool_)
        
        for token_id in range(sp_vocab_size):
            if self.tokenizer.sp.is_control(token_id) or self.tokenizer.sp.is_unknown(token_id) or self.tokenizer.sp.is_unused(token_id):
                continue 
        
            is_boundary_token_np[token_id] = False

            if self.tokenizer.sp.is_byte(token_id):
                base_bytes_np[token_id] = 1
                continue

            piece = self.tokenizer.sp.id_to_piece(token_id)
            if piece.startswith("▁"):
                has_leading_space_np[token_id] = True
                piece = piece[1:]

            base_bytes_np[token_id] = len(piece.encode("utf-8"))
        
        base_bytes = torch.tensor(base_bytes_np, dtype=torch.int16, device=device)
        has_leading_space = torch.tensor(has_leading_space_np, dtype=torch.bool, device=device)
        is_boundary = torch.tensor(is_boundary_token_np, dtype=torch.bool, device=device)

        return base_bytes, has_leading_space, is_boundary