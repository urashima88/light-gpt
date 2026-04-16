
import torch

from models.base_model import BaseModel

class GPT(BaseModel):
    def __init__(self, cfg):
        super().__init__() 
        self.cfg = cfg

    def forward(self, input_ids: torch.Tensor, **kwargs) -> torch.Tensor:
        ...

    def init_weights(self):
        ...