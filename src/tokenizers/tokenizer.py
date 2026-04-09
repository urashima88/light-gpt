import os
from abc import ABC, abstractmethod
from typing import List, Union, Optional

from utils.log import setup_logger

logger = setup_logger(
    os.environ.get("LOG_LEVEL", "INFO"),
    bool(int(os.environ.get("USE_STREAM_HANDLER", 1))),
    bool(int(os.environ.get("USE_FILE_HANDLER", 0)))
)

class Tokenizer(ABC):
    @abstractmethod
    def encode(
        self, 
        text: Union[str, List[str]], 
        prepend: Optional[Union[str, int]] = None, 
        append: Optional[Union[str, int]] = None
    ) -> Union[List[int], List[List[int]]]:
        pass

    @abstractmethod
    def decode(self, ids: List[int]) -> str:
        pass

    @abstractmethod
    def get_vocab_size(self) -> int:
        pass

    @abstractmethod
    def get_bos_token_id(self) -> int:
        pass

    @abstractmethod
    def encode_special(self, token_str: str) -> int:
        pass

    def get_vocab_size(self):
        ...

    def save(self, directory: str) -> None:
        raise NotImplementedError("save() not supported for this tokenizer")

    @classmethod
    def from_pretrained(cls, name_or_path: str, **kwargs):
        raise NotImplementedError("from_pretrained() not supported for this tokenizer")

    @classmethod
    def from_directory(cls, directory: str):
        raise NotImplementedError("from_directory() not supported for this tokenizer")

    @classmethod
    def train_from_iterator(cls, text_iterator, vocab_size: int, **kwargs):
        raise NotImplementedError("train_from_iterator() not supported for this tokenizer")

def get_tokenizer(
    tokenizer_name: str,
    tokenizer_path: str
    ):
    match(tokenizer_name):
        case "SentencePiece":
            if not tokenizer_path.endswith(".model"):
                logger.error("")
                raise ValueError(f"Script only setup for SentencePiece .model file: {tokenizer_path}")
            ...
        case "HuggingFace":
            ...
        case "RustBPE":
            ...
        case _:
            logger.error("")
            raise ValueError(f"tokenizer_name was not set")
    