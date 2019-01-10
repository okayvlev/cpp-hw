echo "Compressing png"
./huffman_testing compress samples/picture.png samples/dst.png;
echo "Size of the source file";
wc -c < "samples/picture.png";
echo "Size of the compressed file";
wc -c < "samples/dst.png";
./huffman_testing decompress samples/dst.png samples/picture2.png;
./compare.sh samples/picture.png samples/picture2.png;
echo
echo "Compressing empty file"
./huffman_testing compress samples/empty.txt samples/dst.txt;
echo "Size of the source file";
wc -c < "samples/empty.txt";
echo "Size of the compressed file";
wc -c < "samples/dst.txt";
./huffman_testing decompress samples/dst.txt samples/empty2.txt;
./compare.sh samples/empty.txt samples/empty2.txt;
echo
echo "Compressing lorem ipsum file"
./huffman_testing compress samples/lorem.txt samples/dst.txt;
echo "Size of the source file";
wc -c < "samples/lorem.txt";
echo "Size of the compressed file";
wc -c < "samples/dst.txt";
./huffman_testing decompress samples/dst.txt samples/lorem2.txt;
./compare.sh samples/lorem.txt samples/lorem2.txt;
echo
echo "Trying to decompress corrupted file"
./huffman_testing decompress samples/random.txt samples/random2.txt;
echo
echo "Compressing War and Peace.txt"
./huffman_testing compress samples/Warandpeace.txt samples/dst.txt;
echo "Size of the source file";
wc -c < "samples/Warandpeace.txt";
echo "Size of the compressed file";
wc -c < "samples/dst.txt";
./huffman_testing decompress samples/dst.txt samples/Warandpeace2.txt;
./compare.sh samples/Warandpeace.txt samples/Warandpeace2.txt;
echo
echo "Compressing War and Peace.pdf"
./huffman_testing compress samples/Warandpeace.pdf samples/dst.pdf;
echo "Size of the source file";
wc -c < "samples/Warandpeace.pdf";
echo "Size of the compressed file";
wc -c < "samples/dst.pdf";
./huffman_testing decompress samples/dst.pdf samples/Warandpeace2.pdf;
./compare.sh samples/Warandpeace.pdf samples/Warandpeace2.pdf;
