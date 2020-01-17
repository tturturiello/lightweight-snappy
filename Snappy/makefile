all: snappy snappy_test test_BST test_varint

snappy: snappy_decompression.c varint.c IO_utils.c cmd.c snappy_compression.c BST.c snappy_compression_tree.c buffer_compression.c result.c
	gcc -o snappy snappy_decompression.c varint.c IO_utils.c cmd.c snappy_compression.c BST.c snappy_compression_tree.c buffer_compression.c result.c

snappy_test: snappy_decompression.c varint.c IO_utils.c snappy_test.c snappy_compression.c buffer_compression.c result.c
	gcc -o snappy_test snappy_decompression.c varint.c IO_utils.c snappy_test.c snappy_compression.c buffer_compression.c result.c

test_BST: test_BST.c BST.c
	gcc -o test_BST test_BST.c BST.c

test_varint: test_varint.c varint.c
	gcc -o test_varint test_varint.c varint.c