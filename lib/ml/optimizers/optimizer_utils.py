
import torch

from optimizers.muon import Muon

def _split_parameters(model, model_cfg):
    block_named_params = list(model.blocks.named_parameters())
    matrix_params = [
        p
        for name, p in block_named_params
        if p.ndim == 2 and not any(pattern in name for pattern in model_cfg.model.control_tensor_name_patterns)
    ]
    scalar_params = [
        p
        for name, p in block_named_params
        if p.ndim < 2 or any(pattern in name for pattern in model_cfg.model.control_tensor_name_patters)
    ]

    if model.skip_weights.numel() > 0:
        scalar_params.append(model.skip_weights)

    return matrix_params, scalar_params

def _choose_optimizer(optimizer_name, params, optimizers_cfg, lr, base_lr = None):
    optimizer_name = optimizer_name.lower()
    match(optimizer_name):
        case "adam":
            optimizer = torch.optim.Adam(
                [{"params": [params], "lr": lr, "base_lr": base_lr}],
                betas=(optimizers_cfg.beta1,  optimizers_cfg.beta2),
                eps=optimizers_cfg.adam_eps,
                fused=optimizers_cfg.fused,
            )
        case "muon":
            optimizer = Muon(
                params,
                lr=lr,
                momentum=optimizers_cfg.muon_momentum,
                backend_steps=optimizers_cfg.muon_backend_steps,
            )
        case _:
            ...
    return optimizer

def set_optimizers(model, cfg):
    matrix_params, scalar_params = _split_parameters(model, cfg.model)
    token_lr = cfg.optimizers.tied_embed_lr if cfg.model.tie_embeddings else cfg.optimizers.embed_lr

    tok_optimizer = _choose_optimizer(
        cfg.optimizers.tok_optimizer,
        [model.tok_emb.weight],
        cfg.optimizers,
        token_lr,
        token_lr,
    )

    matrix_optimizer = _choose_optimizer(
        cfg.optimizers.matrix_optimizer,
        matrix_params,
        cfg.optimizers,
        cfg.optimizers.matrix_lr,
        cfg.optimizers.matrix_lr
    )
    for group in matrix_optimizer.param_groups:
        group["base_lr"] = cfg.optimizers.matrix_lr
        
    scalar_optimizer = _choose_optimizer(
        cfg.optimizers.scalar_optimizer,
        scalar_params,
        cfg.optimizers,
        cfg.optimizers.scalar_lr,
        cfg.optimizers.scalar_lr,
    )

    optimizers: list[torch.optim.Optimizer] = [tok_optimizer, matrix_optimizer, scalar_optimizer]
    if model.lm_head is not None:
        head_optimizer = _choose_optimizer(
           "adam",
            [model.lm_head.weight],
            cfg.optimizers,
            cfg.optimizers.head_lr,
            cfg.optimizers.head_lr,
        )
        optimizers.insert(1, head_optimizer)

    return optimizers