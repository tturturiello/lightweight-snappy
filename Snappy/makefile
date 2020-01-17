all: snappy snappy_test test_BST test_varint

snappy: snappy_decompression.c varint.c IO_utils.c cmd.c snappy_compression.c BST.c snappy_compression_tree.c buffer_compression.c result.c
	gcc -o snappy snappy_decompression.c varint.c IO_utils.c cmd.c snappy_compression.c BST.c snappy_compression_tree.c buffer_compression.c result.c -I

snappy_test: snappy_decompression.c varint.c IO_utils.c snappy_test.c snappy_compression.c BST.c snappy_compression_tree.c buffer_compression.c result.c
	gcc -o snappy_test snappy_decompression.c varint.c IO_utils.c snappy_test.c snappy_compression.c BST.c snappy_compression_tree.c buffer_compression.c result.c -I

test_BST: test_BST.c BST.c
	gcc -o test_BST test_BST.c BST.c -I

test_varint: test_varint.c Varint.c
	gcc -o test_varint test_varint.c Varint.c -I