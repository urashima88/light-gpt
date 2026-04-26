
import torch
import torch.nn as nn

from models.base_model import BaseModel
from models.components.linear import CastedLinear

class GPT(BaseModel):
    def __init__(self, model_cfg):
        super().__init__() 
        self.model_cfg = model_cfg

        self.tok_emb = nn.Embedding(model_cfg.vocab_size, model_cfg.model_dim)
        self.num_encoder_layers = model_cfg.num_encoder_layers
        self.num_decoder_layers = model_cfg.num_decoder_layers
        self.num_skip_weights = min(self.num_encoder_layers, self.num_decoder_layers)
        self.skip_weights = nn.Parameter(torch.ones(self.num_skip_weights, model_cfg.model_dim, dtype=torch.float32))

        self.blocks = nn.ModuleList()
        self.lm_head = None if model_cfg.tie_embeddings else CastedLinear(model_cfg.model_dim, model_cfg.vocab_size, bias=False)
        if self.lm_head is not None:
            self.lm_head._zero_init = True

    def forward(self, input_ids: torch.Tensor, **kwargs) -> torch.Tensor:
        ...

    def init_weights(self):
        ...
