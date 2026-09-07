# Almost QR Code Reader

A small C++23 decoder for Model 2 QR codes. It reads a module grid, follows
the QR data path, removes the selected mask, corrects Version 1 errors, and
returns the encoded text.

## Input and Output

**Input:** a path to a Model 2 QR bitmap. Versions 1-4 are accepted, with
one pixel per module and the standard four-module quiet zone.

**Processing:** the reader converts the bitmap to a module grid and decodes
numeric, alphanumeric, byte, Kanji, or mixed segments. It supports every mask,
recovers damaged format information, and applies Reed-Solomon error correction
to Version 1 symbols.

**Output:** the decoded message as UTF-8 text, printed to standard output.

NOTE: The current image reader does not detect QR codes in ordinary photographs or
correct rotation, scaling, and perspective distortion.

## Build and Run

```sh
cmake -S . -B build -DPROJECT_TOPIC=QRCODE
cmake --build build --target qrcode
./build/qrcode/qrcode qrcode/qr01.png
```

The program prints the decoded message (or a short error).

## Decoding Flow

1. Load and binarize the image into a `BitGrid`.
2. Mark finder, timing, alignment, format, and other function modules.
3. Read the format information to select the mask and error-correction level.
4. Traverse data modules in the QR zig-zag order and remove the mask.
5. Correct Version 1 codewords or deinterleave Versions 2-4.
6. Decode each numeric, alphanumeric, byte, or Kanji segment.

## Tests

The project uses Catch2 v3 for its test cases.

NOTE: CTest registers the complete Catch2 test executable as one test, so CTest reports `1/1` while Catch2 runs all 14 test cases internally.

The tests cover:

- bit grids, bit streams, function patterns, and segment headers;
- data traversal for Versions 1-4 and all eight mask formulas;
- numeric, alphanumeric, byte, Kanji, and mixed-character fixtures;
- invalid input and unsupported dimensions or modes;
- recovery from three damaged format bits and three damaged Version 1 codewords.

```sh
cmake -S . -B build -DPROJECT_TOPIC=QRCODE -DBUILD_TESTING=ON
cmake --build build --target qrcode_tests
ctest --test-dir build --output-on-failure
```

## Documentation

Doxygen reads the README and public API comments, then generates the API
reference in HTML format.

```sh
cmake -S . -B build -DPROJECT_TOPIC=QRCODE -DBUILD_DOCS=ON
cmake --build build --target doc
```

Open `build/docs/html/index.html`.

## Modern C++

- `std::expected` for explicit success/error handling.
- `std::span` lets `BitStream` read bits without copying or owning them.
- `enum class` provides type safety and prevents mixing masks or modes with integers.
- `constexpr` improves runtime efficiency (Reed-Solomon correction is calculated at compile time).
- `std::ranges::all_of` and `std::popcount` simplify validation.
- `std::unique_ptr` with a custom deleter safely releases the loaded image.

The API docs contains only the operations used by the decoding pipeline.