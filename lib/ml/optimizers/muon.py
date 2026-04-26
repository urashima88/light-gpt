
import torch

class Muon(torch.optim.Optimizer):
    def __init__(
            self, 
            params, 
            lr: float, 
            momentum: float, 
            backend_steps: int, 
            nesterov: bool = True
        ):
        super().__init__(
            params,
            dict(lr=lr, momentum=momentum, backend_steps=backend_steps, nesterov=nesterov),
        )

    @torch.no_grad()
    def step(self, closure=None):
        ...