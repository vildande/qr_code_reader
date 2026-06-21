## Intitial setup
### Set the project topic
set(PROJECT_TOPIC
    QRCODE


### building
then, i ran (created build folder and ran there):


mkdir build && cd build

cmake -DPROJECT_TOPIC=QRCODE ..
make

### run the project (with the provided input)
./qrcode/qrcode ../qrcode/qr01.png


### Generate own qrcode. check the zxing-writer and zxing-reader
help:
./_deps/zxing-build/example/ZXingWriter

generate qr (format, text, output):
./_deps/zxing-build/example/ZXingWriter "QR Code Model 2" "Encode this" out.png

to read:
./_deps/zxing-build/example/ZXingReader out.png


### NOTE: during submission comment out add_subdirectory(qrcode)
elseif(PROJECT_TOPIC STREQUAL QRCODE)
  add_subdirectory(qrcode)



## The project...

### bit grid is qr code -> vector of bytes
src/bit_grid.h
src/bit_grid.cpp

### zxing grid reader now works with our bit grid (not ZXing internal bitmap)
src/zxing_grid_reader.h
src/zxing_grid_reader.cpp

