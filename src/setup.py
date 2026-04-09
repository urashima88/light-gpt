import os
import random

import numpy as np
import torch
import torch.distributed as dist
from torch.backends.cuda import (
    enable_cudnn_sdp, 
    enable_flash_sdp, 
    enable_math_sdp, 
    enable_mem_efficient_sdp
)

from utils.device import print0
from utils.log import setup_logger

logger = setup_logger(
    os.environ.get("LOG_LEVEL", "INFO"),
    bool(int(os.environ.get("USE_STREAM_HANDLER", 1))),
    bool(int(os.environ.get("USE_FILE_HANDLER", 0)))
)

def autodetect_device_type() -> str:
    if torch.cuda.is_available():
        device = "cuda"
    elif torch.backends.mps.is_available():
        device = "mps"
    else:
        device = "cpu"
    print0(f"Autodetected device type: {device}")
    return device


def is_ddp_requested() -> bool:
    return all(k in os.environ for k in ("RANK", "LOCAL_RANK", "WORLD_SIZE"))

def get_dist_info():
    if is_ddp_requested():
        assert all(var in os.environ for var in ['RANK', 'LOCAL_RANK', 'WORLD_SIZE'])
        ddp_rank = int(os.environ['RANK'])
        ddp_local_rank = int(os.environ['LOCAL_RANK'])
        ddp_world_size = int(os.environ['WORLD_SIZE'])
        return True, ddp_rank, ddp_local_rank, ddp_world_size
    else:
        return False, 0, 0, 1

def setup(cfg):

    device = autodetect_device_type() if cfg.device == "" else cfg.device

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

    is_ddp_requested, ddp_rank, ddp_local_rank, ddp_world_size = get_dist_info()
    if is_ddp_requested and device == "cuda":
        device = torch.device("cuda", ddp_local_rank)
        torch.cuda.set_device(device)
        dist.init_process_group(backend="nccl", device_id=device)
        dist.barrier()
    else:
        device = torch.device(device)

    if ddp_rank == 0:
        logger.info(f"Distributed world size: {ddp_world_size}")
    
    return is_ddp_requested, ddp_rank, ddp_local_rank, ddp_world_size, device