echo "Compressing png"
./huffman_testing compress samples/picture.png samples/dst.png;
./huffman_testing decompress samples/dst.png samples/picture2.png;
./compare.sh samples/picture.png samples/picture2.png;

echo "Compressing empty file"
./huffman_testing compress samples/empty.txt samples/dst.txt;
./huffman_testing decompress samples/dst.txt samples/empty2.txt;
./compare.sh samples/empty.txt samples/empty2.txt;

echo "Compressing lorem ipsum file"
./huffman_testing compress samples/lorem.txt samples/dst.txt;
./huffman_testing decompress samples/dst.txt samples/lorem2.txt;
./compare.sh samples/lorem.txt samples/lorem2.txt;

echo "Trying to decompress corrupted file"
./huffman_testing decompress samples/random.txt samples/random2.txt;