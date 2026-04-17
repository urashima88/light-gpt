from logging import Logger

from metrics.byte_statistics.byte_statistics_calculator import ByteStatisticsCalculator
from metrics.byte_statistics.sentencepiece_byte_statistics import SentencePieceByteStatistics
from utils.exceptions import fatal
from tokenizers.sentencepiece_tokenizer import SentencePieceTokenizer


def create_byte_statistics_calculator(tokenizer, model_vocab_size: int, logger: Logger) -> ByteStatisticsCalculator:
    if isinstance(tokenizer, SentencePieceTokenizer):
        return SentencePieceByteStatistics(tokenizer, model_vocab_size, logger)
    else:
        raise fatal(TypeError, f"Unsupported tokenizer type: {type(tokenizer)}", logger)