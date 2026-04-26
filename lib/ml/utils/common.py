

def count_model_params(model):
    return sum(p.numel() for p in model.parameters())    