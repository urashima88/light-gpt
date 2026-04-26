from logging import Logger

import torch

from models.base_model import BaseModel
from models.gpt import GPT
from models.components.linear import CastedLinear
from utils.exceptions import fatal


def create_model(model_cfg, logger: Logger):

    architecture = model_cfg.architecture.lower()
    match(architecture):
        case "gpt":
            model = GPT(model_cfg)
        case _:
            raise fatal(ValueError, f"architecture was not set or is incorrect", logger)

    return model

def prepare_model_for_training(
        model: BaseModel, 
        model_cfg,
        device: torch.device, 
        compute_dtype: torch.dtype,
    ):
    model.to(device, dtype=compute_dtype)
    for module in model.modules():
        if isinstance(module, CastedLinear):
            module.float()
    
    if model_cfg.restore_low_dim_params_in_fp32:
        model = restore_low_dim_params_to_fp32(model, model_cfg.control_tensor_name_patterns)   

    if model_cfg.compile.use:
        model = torch.compile(model, dynamic=model_cfg.compile.dynamic, fullgraph=model_cfg.compile.fullgraph)
    
    return model

def restore_low_dim_params_to_fp32(
        model: BaseModel, 
        control_tensor_name_patterns: list[str]
    ) -> None:
    # Keep small/control parameters in fp32 even when the model body runs in bf16.
    with torch.no_grad():
        for name, param in model.named_parameters():
            if (param.ndim < 2 or any(pattern in name for pattern in control_tensor_name_patterns)) and param.dtype != torch.float32:
                param.data = param.data.float()

    return model