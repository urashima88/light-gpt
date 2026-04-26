import os
import random
from logging import Logger

import numpy as np
import torch
import torch.distributed as dist
from torch.backends.cuda import (
    enable_cudnn_sdp, 
    enable_flash_sdp, 
    enable_math_sdp, 
    enable_mem_efficient_sdp
)

from utils.device import autodetect_device_type

def setup(cfg, logger: Logger):
    device = autodetect_device_type(logger) if cfg.device == "" else cfg.device

    assert device in ["cuda", "mps", "cpu"]
    if device == "cuda":
        assert torch.cuda.is_available()
    if device == "mps":
        assert torch.backends.mps.is_available()

    random.seed(cfg.seed)
    np.random.seed(cfg.seed)
    torch.manual_seed(cfg.seed)
    if device == "cuda":
        torch.cuda.manual_seed_all(cfg.seed)
        # uses tf32 instead of fp32 for matmuls, see https://docs.pytorch.org/docs/stable/generated/torch.set_float32_matmul_precision.html
        torch.set_float32_matmul_precision(cfg.matmul_tf32)
        torch.backends.cudnn.allow_tf32 = cfg.cudnn_tf32

        enable_cudnn_sdp(cfg.cudnn_sdp)
        enable_flash_sdp(cfg.flash_sdp)
        enable_mem_efficient_sdp(cfg.mem_efficient_sdp)
        enable_math_sdp(cfg.math_sdp)

    if cfg.ddp.use and device == "cuda":
        device = torch.device("cuda", cfg.ddp.local_rank)
        torch.cuda.set_device(device)
        dist.init_process_group(backend="nccl", device_id=device)
        dist.barrier()
    else:
        device = torch.device(device)

    if cfg.ddp.rank == 0:
        logger.info(f"Distributed world size: {cfg.ddp.world_size}")
    
    return cfg.ddp.use, cfg.ddp.rank, cfg.ddp.local_rank, cfg.ddp.world_size, device