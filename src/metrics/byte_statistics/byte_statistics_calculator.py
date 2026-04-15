from abc import ABC, abstractmethod
from typing import Tuple

import torch
from torch import Tensor

class ByteStatisticsCalculator(ABC):
    '''
    An abstract interface for constructing LUTs used in byte counting.
    '''
    @abstractmethod
    def build_luts(self, device: torch.device) -> Tuple[Tensor, Tensor, Tensor]:
        pass