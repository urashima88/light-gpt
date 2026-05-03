
import torch.nn as nn
from lib.ml.layers.linear.linear import CastedLinear


class Model(nn.Module):
    def __init__(self):
        super().__init__()
        self.linear1 = CastedLinear(20, 20)

    
    def forward(self, x):
        pass