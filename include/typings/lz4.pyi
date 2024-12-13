def compress(data: bytes) -> bytes:
    """Compress the given data into LZ4 block format.
    
    This function is equivalent to `lz4.block.compress` of https://pypi.org/project/lz4/.
    """

def decompress(data: bytes) -> bytes:
    """Decompress the given LZ4 block format data produced by `lz4.compress()`.
    
    This function is equivalent to `lz4.block.decompress` of https://pypi.org/project/lz4/.
    """
