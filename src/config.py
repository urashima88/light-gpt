import os
import argparse
import yaml
from typing import Any, Dict
from types import SimpleNamespace

from dotenv import load_dotenv

from utils.log import setup_logger


logger = setup_logger(
    os.environ.get("LOG_LEVEL", "INFO"),
    bool(int(os.environ.get("USE_STREAM_HANDLER", 1))),
    bool(int(os.environ.get("USE_FILE_HANDLER", 0)))
)

def load_config() -> SimpleNamespace:

    parser = argparse.ArgumentParser()
    parser.add_argument("--env", type=str, help="Path to .env file")
    parser.add_argument("--config", type=str, help="Path to YAML config file")
    args, unknown = parser.parse_known_args()

    load_dotenv(dotenv_path=args.env, override=True)
    env_vars = {}
    for key, value in os.environ.items():
        if key.startswith(('PATH=', 'USER=', 'HOME=', 'SHELL=', 'PWD=')):
            continue
        env_vars[key] = _parse_value(value)

    with open(args.config, 'r', encoding='utf-8') as f:
        config_dict = yaml.safe_load(f) or {}

    overrides = {}
    i = 0
    while i < len(unknown):
        arg = unknown[i]
        if arg.startswith('--'):
            key = arg[2:] 
            if i + 1 < len(unknown) and not unknown[i+1].startswith('--'):
                value = unknown[i+1]
                i += 2
            else:
                value = True 
                i += 1
            keys = key.split('.')
            d = overrides
            for k in keys[:-1]:
                d = d.setdefault(k, {})
            d[keys[-1]] = _parse_value(value)
        else:
            i += 1

    env_dict = {'env': env_vars}
    config_dict = _deep_update(config_dict, env_dict)
    config_dict = _deep_update(config_dict, overrides)

    config_dict = _deep_update(config_dict, overrides)
    return _dict_to_namespace(config_dict)
    
def _parse_value(val: str) -> Any:
    if isinstance(val, bool):
        return val
    lower = val.lower()
    if lower == 'true':
        return True
    if lower == 'false':
        return False
    try:
        return int(val)
    except ValueError:
        try:
            return float(val)
        except ValueError:
            return val
        
def _deep_update(base: Dict, updates: Dict) -> Dict:
    for k, v in updates.items():
        if isinstance(v, dict) and k in base and isinstance(base[k], dict):
            _deep_update(base[k], v)
        else:
            base[k] = v
    return base

def _dict_to_namespace(d: Dict) -> SimpleNamespace:
    ns = SimpleNamespace()
    for k, v in d.items():
        if isinstance(v, dict):
            setattr(ns, k, _dict_to_namespace(v))
        elif isinstance(v, list):
            setattr(ns, k, [_dict_to_namespace(item) if isinstance(item, dict) else item for item in v])
        else:
            setattr(ns, k, v)
    return ns