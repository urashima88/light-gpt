from abc import ABC, abstractmethod

import torch
import torch.nn as nn

class BaseModel(ABC, nn.Module):
    
    @abstractmethod
    def forward(self, input_ids: torch.Tensor, **kwargs) -> torch.Tensor:
        pass

    @abstractmethod
    def init_weights(self):
        pass
