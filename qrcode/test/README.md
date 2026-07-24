# QR Code Tests

The test suite uses Catch2 v3 and is registered with CTest.

```sh
cmake -S . -B build -DPROJECT_TOPIC=QRCODE -DBUILD_TESTING=ON
cmake --build build --target qrcode_tests
ctest --test-dir build --output-on-failure
```

The tests cover the bit grid and stream, function-pattern map, all eight mask
formulas, Version 1-4 data traversal, segment headers, decoding modes, format
BCH recovery, and Version 1 Reed-Solomon correction. Images in `data/` were
generated with the ZXing writer as independent Version 2-4 and Kanji fixtures.
