import sentencepiece as spm
import os

from utils.log import setup_logger
from tokenizers.tokenizer import Tokenizer
from utils.exceptions import fatal

logger = setup_logger(
    os.environ.get("LOG_LEVEL", "INFO"),
    bool(int(os.environ.get("USE_STREAM_HANDLER", 1))),
    bool(int(os.environ.get("USE_FILE_HANDLER", 0)))
)

class SentencePieceTokenizer(Tokenizer):
    def __init__(self, sp_model: spm.SentencePieceProcessor):
        self.sp = sp_model

    @classmethod
    def from_model_file(cls, model_path: str):
        sp = spm.SentencePieceProcessor(model_file=model_path)
        return cls(sp)

    def encode(self, text, prepend=None, append=None):
        if not isinstance(text, str) and not isinstance(text, list):
            raise fatal(TypeError, f"Unsupported type {type(text)}", logger)

        ids = []
        if prepend is not None:
            prepend_id = prepend if isinstance(prepend, int) else self.encode_special(prepend)
            if isinstance(text, str):
                ids.append(prepend_id)
            else:
                for _ in range(len(text)):
                    ids.append([prepend_id])

        if isinstance(text, str):
            ids.extend(self.sp.EncodeAsIds(text))
        else:
            if prepend is not None:
                for i, t in enumerate(text):
                    ids[i].extend(self.sp.EncodeAsIds(t))
            else:
                ids = [self.sp.EncodeAsIds(t) for t in text]
        
        if append is not None:
            append_id = append if isinstance(append, int) else self.encode_special(append)
            if isinstance(text, str):
                ids.append(append_id)
            else:
                for row in ids:
                    row.append(append_id)
        return ids

    def decode(self, ids):
        return self.sp.DecodeIds(ids)

    def get_vocab_size(self):
        return self.sp.GetPieceSize()

    def get_bos_token_id(self):
        bos = self.encode_special("<s>")
        if bos is None:
            bos = 0
        return bos

    def encode_special(self, token_str):
        return self.sp.PieceToId(token_str)

    def save(self, directory):
        pass

    