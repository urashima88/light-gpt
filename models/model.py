import torch.nn as nn


class Model(nn.Module):
    def __init__(self):
        super().__init__()
        self.linear1 = nn.Linear(in_features=None, out_features=None, bias='True', device='None', dtype='None')
        self.linear2 = nn.Linear(in_features=0, out_features=0, bias=True, device=None, dtype=None)

    
    def forward(self, x):
        x = self.linear2(x)
        x = self.linear1(x)
        return x
