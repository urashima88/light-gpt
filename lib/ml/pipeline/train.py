import os
import sys

import torch
# import wandb
from torch.nn.parallel import DistributedDataParallel as DDP

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
from metrics.byte_statistics.byte_statistics_utils import create_byte_statistics_calculator
from models.model_utils import create_model, prepare_model_for_training
from utils.common import count_model_params
from optimizers.optimizer_utils import set_optimizers

sys.excepthook = handle_exception


def main():
    cfg = load_config()

    logger = setup_logger(
        cfg.log.log_level,
        cfg.log.use_stream_handler,
        cfg.log.use_file_handler,
        cfg.log.logs_dir,
        cfg.ddp.rank,
    )

    COMPUTE_DTYPE, COMPUTE_DTYPE_REASON = detect_compute_dtype(cfg.dtype)

    logger.info(f"COMPUTE_DTYPE: {COMPUTE_DTYPE} ({COMPUTE_DTYPE_REASON})")
    
    is_ddp_requested, ddp_rank, ddp_local_rank, ddp_world_size, device = setup(cfg, logger)
    master_process = ddp_rank == 0
    synchronize = torch.cuda.synchronize if device == "cuda" else lambda: None
    get_max_memory_allocated = torch.cuda.max_memory_allocated if device == "cuda" else lambda: 0

    if device == "cuda":
        gpu_device_name = torch.cuda.get_device_name(0)
        gpu_peak_flops = get_peak_flops(gpu_device_name, logger)
        logger.info(f"GPU: {gpu_device_name} | Peak FLOPS (BF16): {gpu_peak_flops:.2e}")
    else:
        gpu_peak_flops = float('inf')

    use_dummy_wandb = cfg.run == "dummy" or not master_process
    wandb_run = DummyWandb() if use_dummy_wandb else ... # wandb.init(project="llm", name=args.run, config=user_cfg)

    using_fa3 = define_using_fa3()

    tokenizer = get_tokenizer(cfg.tokenizer_name, cfg.tokenizer_path, logger)
    tokenizer_vocab_size = tokenizer.get_vocab_size()
    if tokenizer_vocab_size != cfg.model.vocab_size:
        raise fatal(
            ValueError,
            f"VOCAB_SIZE={cfg.model.vocab_size} does not match tokenizer vocab_size={int(tokenizer_vocab_size)}",
            logger
        )
    logger.info(f"Vocab size: {tokenizer_vocab_size:,}")

    train_dataset = ...
    val_dataset = get_dataset(cfg.dataset_name, logger=logger, pattern=cfg.val_data, seq_len=cfg.training.train_seq_len)

    logger.info(f"val_dataset: tokens: {val_dataset.tokens.numel()-1}")

    used_metrics = set(cfg.training.metrics)
    logger.info(f"Used metrics: {','.join(used_metrics)}")
    if "bpb" in used_metrics:
        byte_stats_calc = create_byte_statistics_calculator(tokenizer=tokenizer, model_vocab_size=cfg.model.vocab_size, logger=logger)
        base_bytes_lut, has_leading_space_lut, is_boundary_token_lut = byte_stats_calc.build_luts(device)

    model = create_model(cfg.model, logger)
    model = prepare_model_for_training(model, cfg.model, device, COMPUTE_DTYPE)

    if is_ddp_requested:
        model = DDP(model, device_ids=[ddp_local_rank], broadcast_buffers=cfg.ddp.broadcast_buffers)

    optimizers = set_optimizers(model, cfg)

    n_params = count_model_params(model)
    logger.info(f"Model params: {n_params}")

if __name__ == "__main__":
    main()