#include <stdio.h>
#include <string.h>
#include "encode.h"
#include "types.h"
#include "common.h"

uint get_image_size_for_bmp(FILE *fptr_image)
{
    uint width, height;
    long curr_pos = ftell(fptr_image);

    fseek(fptr_image, 18, SEEK_SET);
    fread(&width, sizeof(int), 1, fptr_image);
    printf(WHITE "\n📐 Image Width  : %u px\n" RESET, width);

    fread(&height, sizeof(int), 1, fptr_image);
    printf(WHITE "📐 Image Height : %u px\n" RESET, height);

    fseek(fptr_image, curr_pos, SEEK_SET);
    return width * height * 3;
}

uint get_file_size(FILE *fptr)
{
    fseek(fptr, 0, SEEK_END);
    long size = ftell(fptr);
    rewind(fptr);
    return (uint)size;
}

Status read_and_validate_encode_args(char *argv[], EncodeInfo *encInfo)
{
    if (strstr(argv[2], ".bmp") == NULL)
    {
        printf(RED "❌ ERROR: Source image must be a .bmp file\n" RESET);
        return e_failure;
    }
    encInfo->src_image_fname = argv[2];

    if (argv[3] == NULL)
    {
        printf(RED "❌ ERROR: Secret file not provided\n" RESET);
        return e_failure;
    }

    char *ext = strchr(argv[3], '.');
    if (ext == NULL || (strcmp(ext, ".txt") != 0 && strcmp(ext, ".c") != 0 && strcmp(ext, ".h") != 0 && strcmp(ext, ".sh") != 0))
    {
        printf(RED "ERROR: Secret file must be .txt / .c / .h / .sh\n" RESET);
        return e_failure;
    }

    encInfo->secret_fname = argv[3];

    strcpy(encInfo->extn_secret_file, ext);

    if (argv[4] != NULL)
    {
        if (strstr(argv[4], ".bmp") == NULL)
        {
            printf(RED "❌ ERROR: Output file must be a .bmp image\n" RESET);
            return e_failure;
        }
        encInfo->stego_image_fname = argv[4];
    }
    else
    {
        encInfo->stego_image_fname = "stego.bmp";
    }

    return e_success;
}

Status encode_byte_to_lsb(char data, char *image_buffer)
{
    for (int i = 0; i < 8; i++)
    {
        image_buffer[i] = (image_buffer[i] & 0xFE) | ((data >> (7 - i)) & 1);
    }
    return e_success;
}

Status encode_size_to_lsb(int size, char *imageBuffer)
{
    for (int i = 0; i < 32; i++)
    {
        imageBuffer[i] = (imageBuffer[i] & 0xFE) | ((size >> (31 - i)) & 1);
    }
    return e_success;
}

Status open_files(EncodeInfo *encInfo)
{
    encInfo->fptr_src_image = fopen(encInfo->src_image_fname, "r");
    if (encInfo->fptr_src_image == NULL)
    {
        perror("fopen");
        fprintf(stderr, RED "❌ ERROR: Unable to open source image %s\n" RESET, encInfo->src_image_fname);
        return e_failure;
    }

    encInfo->fptr_secret = fopen(encInfo->secret_fname, "r");
    if (encInfo->fptr_secret == NULL)
    {
        perror("fopen");
        fprintf(stderr, RED "❌ ERROR: Unable to open secret file %s\n" RESET, encInfo->secret_fname);
        return e_failure;
    }

    encInfo->fptr_stego_image = fopen(encInfo->stego_image_fname, "w");
    if (encInfo->fptr_stego_image == NULL)
    {
        perror("fopen");
        fprintf(stderr, RED "❌ ERROR: Unable to create stego image %s\n" RESET, encInfo->stego_image_fname);
        return e_failure;
    }

    printf(WHITE "🖼️  Source Image File      : %s\n" RESET, encInfo->src_image_fname);
    printf(WHITE "📄 Secret File             : %s\n" RESET, encInfo->secret_fname);
    printf(WHITE "🧪 Stego Image Output File : %s\n" RESET, encInfo->stego_image_fname);

    return e_success;
}

Status check_capacity(EncodeInfo *encInfo)
{
    encInfo->image_capacity = get_image_size_for_bmp(encInfo->fptr_src_image);
    printf(WHITE "\n📊 Total Image Capacity : %d bits\n" RESET, encInfo->image_capacity);

    encInfo->size_secret_file = get_file_size(encInfo->fptr_secret);

    char *ext = strchr(encInfo->secret_fname, '.');
    strcpy(encInfo->extn_secret_file, ext);

    int magic_len = strlen(MAGIC_STRING);
    int extn_len = strlen(encInfo->extn_secret_file);

    uint required_bits = (magic_len * 8) + 32 + (extn_len * 8) + 32 + (encInfo->size_secret_file * 8);

    if (encInfo->image_capacity < required_bits)
    {
        printf(RED "❌ ERROR: Image does not have enough capacity\n" RESET);
        return e_failure;
    }

    printf(WHITE "📊 Required Capacity : %u bits\n" RESET, required_bits);
    return e_success;
}

Status copy_bmp_header(FILE *fptr_src_image, FILE *fptr_dest_image)
{
    char imageBuffer[54];

    rewind(fptr_src_image);
    fread(imageBuffer, 1, 54, fptr_src_image);
    fwrite(imageBuffer, 1, 54, fptr_dest_image);

    if (ftell(fptr_src_image) != ftell(fptr_dest_image))
    {
        printf(RED "❌ ERROR: BMP header copy failed\n" RESET);
        return e_failure;
    }
    return e_success;
}

Status encode_magic_string(const char *magic_string, EncodeInfo *encInfo)
{
    char buffer[8];

    for (int i = 0; magic_string[i] != '\0'; i++)
    {
        fread(buffer, 1, 8, encInfo->fptr_src_image);
        encode_byte_to_lsb(magic_string[i], buffer);
        fwrite(buffer, 1, 8, encInfo->fptr_stego_image);
    }

    if (ftell(encInfo->fptr_src_image) != ftell(encInfo->fptr_stego_image))
    {
        printf(RED "❌ ERROR: Magic string encoding failed\n" RESET);
        return e_failure;
    }
    return e_success;
}

Status encode_secret_file_extn_size(int size, EncodeInfo *encInfo)
{
    char imageBuffer[32];

    fread(imageBuffer, 1, 32, encInfo->fptr_src_image);
    encode_size_to_lsb(size, imageBuffer);
    fwrite(imageBuffer, 1, 32, encInfo->fptr_stego_image);

    if (ftell(encInfo->fptr_src_image) != ftell(encInfo->fptr_stego_image))
    {
        printf(RED "❌ ERROR: Extension size encoding failed\n" RESET);
        return e_failure;
    }
    return e_success;
}

Status encode_secret_file_extn(const char *file_extn, EncodeInfo *encInfo)
{
    char imageBuffer[8];

    for (int i = 0; file_extn[i] != '\0'; i++)
    {
        fread(imageBuffer, 1, 8, encInfo->fptr_src_image);
        encode_byte_to_lsb(file_extn[i], imageBuffer);
        fwrite(imageBuffer, 1, 8, encInfo->fptr_stego_image);
    }

    if (ftell(encInfo->fptr_src_image) != ftell(encInfo->fptr_stego_image))
    {
        printf(RED "❌ ERROR: File extension encoding failed\n" RESET);
        return e_failure;
    }
    return e_success;
}

Status encode_secret_file_size(long file_size, EncodeInfo *encInfo)
{
    char imageBuffer[32];

    fread(imageBuffer, 1, 32, encInfo->fptr_src_image);
    encode_size_to_lsb((int)file_size, imageBuffer);
    fwrite(imageBuffer, 1, 32, encInfo->fptr_stego_image);

    if (ftell(encInfo->fptr_src_image) != ftell(encInfo->fptr_stego_image))
    {
        printf(RED "❌ ERROR: Secret file size encoding failed\n" RESET);
        return e_failure;
    }
    return e_success;
}

Status encode_secret_file_data(EncodeInfo *encInfo)
{
    char imageBuffer[8];
    char ch;

    rewind(encInfo->fptr_secret);

    for (long i = 0; i < encInfo->size_secret_file; i++)
    {
        fread(&ch, 1, 1, encInfo->fptr_secret);
        fread(imageBuffer, 1, 8, encInfo->fptr_src_image);
        encode_byte_to_lsb(ch, imageBuffer);
        fwrite(imageBuffer, 1, 8, encInfo->fptr_stego_image);
    }

    if (ftell(encInfo->fptr_src_image) != ftell(encInfo->fptr_stego_image))
    {
        printf(RED "❌ ERROR: Secret data encoding failed\n" RESET);
        return e_failure;
    }
    return e_success;
}

Status copy_remaining_img_data(FILE *fptr_src, FILE *fptr_dest)
{
    char buffer[1024];
    int bytes;

    while ((bytes = fread(buffer, 1, sizeof(buffer), fptr_src)) > 0)
    {
        if (fwrite(buffer, 1, bytes, fptr_dest) != bytes)
        {
            printf(RED "❌ ERROR: Remaining image data copy failed\n" RESET);
            return e_failure;
        }
    }
    return e_success;
}

Status do_encoding(EncodeInfo *encInfo)
{
    if (open_files(encInfo) == e_failure)
        return e_failure;
    printf(WHITE "✔" RESET GREEN " Files opened successfully\n" RESET);

    if (check_capacity(encInfo) == e_failure)
        return e_failure;
    printf(WHITE "✔" RESET GREEN " Image capacity verified\n" RESET);

    if (copy_bmp_header(encInfo->fptr_src_image, encInfo->fptr_stego_image) == e_failure)
        return e_failure;
    printf(WHITE "✔" RESET GREEN " BMP header copied\n" RESET);

    printf(WHITE "\n🪄  Encoding Magic String : %s\n" RESET, MAGIC_STRING);
    if (encode_magic_string(MAGIC_STRING, encInfo) == e_failure)
        return e_failure;
    printf(WHITE "✔" RESET GREEN " Magic string encoded\n\n" RESET);

    int extn_len = strlen(encInfo->extn_secret_file);

    if (encode_secret_file_extn_size(extn_len, encInfo) == e_failure)
        return e_failure;
    printf(WHITE "📏 Extension Size : %d\n" RESET, extn_len);
    printf(WHITE "✔" RESET GREEN " Extension size encoded\n\n" RESET);

    if (encode_secret_file_extn(encInfo->extn_secret_file, encInfo) == e_failure)
        return e_failure;
    printf(WHITE "🏷️  File Extension : %s\n" RESET, encInfo->extn_secret_file);
    printf(WHITE "✔" RESET GREEN " Extension encoded\n\n" RESET);

    if (encode_secret_file_size(encInfo->size_secret_file, encInfo) == e_failure)
        return e_failure;
    printf(WHITE "📦 Secret File Size : %ld bytes\n" RESET, encInfo->size_secret_file);
    printf(WHITE "✔" RESET GREEN " File size encoded\n\n" RESET);

    printf(WHITE "\n🔐 Encoding Secret Data from : %s\n" RESET, encInfo->secret_fname);
    if (encode_secret_file_data(encInfo) == e_failure)
        return e_failure;
    printf(WHITE "✔" RESET GREEN " Secret data encoded\n" RESET);

    if (copy_remaining_img_data(encInfo->fptr_src_image, encInfo->fptr_stego_image) == e_failure)
        return e_failure;
    printf(WHITE "✔" RESET GREEN " Remaining image data copied\n" RESET);

    fclose(encInfo->fptr_src_image);
    fclose(encInfo->fptr_secret);
    fclose(encInfo->fptr_stego_image);

    return e_success;
}
