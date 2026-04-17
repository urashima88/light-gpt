from logging import Logger

from tokenizers.sentencepiece_tokenizer import SentencePieceTokenizer
from utils.exceptions import fatal


def get_tokenizer(
    tokenizer_name: str,
    tokenizer_path: str,
    logger: Logger
    ):
    tokenizer_name = tokenizer_name.lower()
    match(tokenizer_name):
        case "sentencepiece":
            if not tokenizer_path.endswith(".model"):
                raise fatal(ValueError, f"Script only setup for SentencePiece .model file: {tokenizer_path}", logger)
            tokenizer = SentencePieceTokenizer.from_model_file(tokenizer_path, logger)
        case "huggingface":
            ...
        case "rustbpe":
            ...
        case _:
            raise fatal(ValueError, f"tokenizer_name was not set or is incorrect", logger)
    
    return tokenizer