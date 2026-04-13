import os
import sys

import torch
# import wandb

from config.load import load_config
from setup import setup
from utils.device import (
    get_peak_flops,
    detect_compute_dtype
)
from utils.log import setup_logger, DummyWandb
from flash_attention import define_using_fa3
from tokenizers.tokenizer_utils import get_tokenizer
from data.data_utils import get_dataset
from utils.exceptions import fatal, handle_exception

logger = setup_logger(
    os.environ.get("LOG_LEVEL", "INFO"),
    bool(int(os.environ.get("USE_STREAM_HANDLER", 1))),
    bool(int(os.environ.get("USE_FILE_HANDLER", 0)))
)
sys.excepthook = handle_exception


def main():
    cfg = load_config()
    COMPUTE_DTYPE, COMPUTE_DTYPE_REASON = detect_compute_dtype(cfg.dtype)

    logger.info(f"COMPUTE_DTYPE: {COMPUTE_DTYPE} ({COMPUTE_DTYPE_REASON})")
    
    is_ddp_requested, ddp_rank, ddp_local_rank, ddp_world_size, device = setup(cfg)
    master_process = ddp_rank == 0
    synchronize = torch.cuda.synchronize if device == "cuda" else lambda: None
    get_max_memory_allocated = torch.cuda.max_memory_allocated if device == "cuda" else lambda: 0

    if device == "cuda":
        gpu_device_name = torch.cuda.get_device_name(0)
        gpu_peak_flops = get_peak_flops(gpu_device_name)
        logger.info(f"GPU: {gpu_device_name} | Peak FLOPS (BF16): {gpu_peak_flops:.2e}")
    else:
        gpu_peak_flops = float('inf')

    use_dummy_wandb = cfg.run == "dummy" or not master_process
    wandb_run = DummyWandb() if use_dummy_wandb else ... # wandb.init(project="llm", name=args.run, config=user_cfg)

    using_fa3 = define_using_fa3()

    tokenizer = get_tokenizer(cfg.tokenizer_name, cfg.tokenizer_path)
    tokenizer_vocab_size = tokenizer.get_vocab_size()
    if tokenizer_vocab_size != cfg.model.vocab_size:
        raise fatal(
            ValueError,
            f"VOCAB_SIZE={cfg.model.vocab_size} does not match tokenizer vocab_size={int(tokenizer_vocab_size)}",
            logger
        )
    logger.info(f"Vocab size: {tokenizer_vocab_size:,}")

    train_dataset = ...
    val_dataset = get_dataset(cfg.dataset_name, pattern=cfg.val_data, seq_len=cfg.training.train_seq_len)

    logger.info(f"val_dataset: tokens: {val_dataset.tokens.numel()-1}")


if __name__ == "__main__":
    main()