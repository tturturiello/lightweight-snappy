all: snappy snappy_test test_BST test_max_buff test_varint foglio_per_test

snappy: snappy_decompression.c varint.c IO_utils.c cmd.c snappy_compression.c BST.c snappy_compression_tree.c
	gcc -o snappy snappy_decompression.c varint.c IO_utils.c cmd.c snappy_compression.c BST.c snappy_compression_tree.c -I

snappy_test: snappy_decompression.c varint.c IO_utils.c snappy_test.c snappy_compression.c BST.c snappy_compression_tree.c
	gcc -o snappy_test snappy_decompression.c varint.c IO_utils.c snappy_test.c snappy_compression.c BST.c snappy_compression_tree.c -I

test_BST: test_BST.c BST.c
	gcc -o test_BST test_BST.c BST.c -I

test_max_buff: test_max_buff.c
	gcc -o test_max_buff test_max_buff.c -I

test_varint: test_varint.c Varint.c
	gcc -o test_varint test_varint.c Varint.c -I

foglio_per_test: foglio_per_test.c IO_utils.c BST.c
	gcc -o foglio_per_test foglio_per_test.c IO_utils.c BST.c -I