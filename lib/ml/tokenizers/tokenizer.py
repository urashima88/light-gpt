import os
from abc import ABC, abstractmethod
from typing import List, Union, Optional

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
    
