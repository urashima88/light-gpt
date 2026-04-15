import os

from metrics.byte_statistics.byte_statistics_calculator import ByteStatisticsCalculator
from metrics.byte_statistics.sentencepiece_byte_statistics import SentencePieceByteStatistics
from utils.log import setup_logger
from utils.exceptions import fatal
from tokenizers.sentencepiece_tokenizer import SentencePieceTokenizer

logger = setup_logger(
    os.environ.get("LOG_LEVEL", "INFO"),
    bool(int(os.environ.get("USE_STREAM_HANDLER", 1))),
    bool(int(os.environ.get("USE_FILE_HANDLER", 0)))
)

def create_byte_statistics_calculator(tokenizer, model_vocab_size: int) -> ByteStatisticsCalculator:
    if isinstance(tokenizer, SentencePieceTokenizer):
        return SentencePieceByteStatistics(tokenizer, model_vocab_size)
    else:
        raise fatal(TypeError, f"Unsupported tokenizer type: {type(tokenizer)}", logger)