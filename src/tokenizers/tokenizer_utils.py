import os

from utils.log import setup_logger
from tokenizers.sentencepiece_tokenizer import SentencePieceTokenizer
from utils.exceptions import fatal

logger = setup_logger(
    os.environ.get("LOG_LEVEL", "INFO"),
    bool(int(os.environ.get("USE_STREAM_HANDLER", 1))),
    bool(int(os.environ.get("USE_FILE_HANDLER", 0)))
)

def get_tokenizer(
    tokenizer_name: str,
    tokenizer_path: str
    ):
    tokenizer_name = tokenizer_name.lower()
    match(tokenizer_name):
        case "sentencepiece":
            if not tokenizer_path.endswith(".model"):
                raise fatal(ValueError, f"Script only setup for SentencePiece .model file: {tokenizer_path}", logger)
            tokenizer = SentencePieceTokenizer.from_model_file(tokenizer_path)
        case "huggingface":
            ...
        case "rustbpe":
            ...
        case _:
            raise fatal(ValueError, f"tokenizer_name was not set", logger)
    
    return tokenizer