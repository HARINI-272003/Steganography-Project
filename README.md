# LSB Image Steganography in C

## Project Overview
This project implements Least Significant Bit (LSB) based image steganography using the C programming language. It hides secret data inside a BMP image without causing noticeable visual changes and allows secure extraction of the hidden data.

## Objective
To embed and extract secret files within an image using LSB manipulation while maintaining image quality.

## Technologies Used
- C Programming
- File Handling
- Bitwise Operations
- Structures
- Modular Programming
- BMP Image Format

## What is Steganography
Steganography is a technique used to hide confidential data inside another file such as an image, audio, or video so that the presence of the data is not easily detectable.

## Working Principle (LSB Technique)
- Each pixel in a BMP image is represented using bytes.
- The least significant bit of each byte is replaced with a bit of secret data.
- Since only the LSB is modified, visual distortion is minimal.

## Features
- Supports encoding and decoding of secret files
- Uses a magic string for authentication
- Preserves original image quality
- Modular and structured C code

## Encoding Process
1. Validate input arguments
2. Open source image and secret file
3. Embed magic string
4. Embed secret file size
5. Embed secret file extension
6. Embed secret file data
7. Generate stego image

## Decoding Process
1. Validate input arguments
2. Open stego image
3. Extract magic string
4. Extract secret file size
5. Extract secret file extension
6. Extract secret file data
7. Recreate original secret file

## Input Files
- Source Image: `.bmp`
- Secret File: `.txt / .c / .h / .sh`

## Output
- Stego Image (Encoded Image)
- Extracted Secret File (During Decoding)

## How to Compile
```bash
gcc *.c -o steganography

## Encoding
./steganography -e source.bmp secret.txt stego.bmp
## Decoding
./steganography -d stego.bmp
