#include <stdio.h>
#include <string.h>
#include "decode.h"
#include "types.h"
#include "common.h"

Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo)
{
    if (argv[2] == NULL)
    {
        printf(RED "❌ ERROR: Stego image file not specified\n" RESET);
        return e_failure;
    }
    if (strstr(argv[2], ".bmp") == NULL)
    {
        printf(RED "❌ ERROR: Stego image must be a .bmp file\n" RESET);
        return e_failure;
    }
    decInfo->stego_image_fname = argv[2];

    if (argv[3] != NULL)
    {
        strcpy(decInfo->output_fname, argv[3]);
        strtok(decInfo->output_fname, ".");
    }
    else
    {
        strcpy(decInfo->output_fname, "decoded_secret");
    }
    return e_success;
}

Status open_decoding_files(DecodeInfo *decInfo)
{
    decInfo->fptr_stego_image = fopen(decInfo->stego_image_fname, "r");
    if (decInfo->fptr_stego_image == NULL)
    {
        perror("fopen");
        fprintf(stderr, RED "❌ ERROR: Unable to open stego image %s\n" RESET,
                decInfo->stego_image_fname);
        return e_failure;
    }

    return e_success;
}

char decode_byte_from_lsb(char *image_buffer)
{
    char ch = 0;
    for (int i = 0; i < 8; i++)
    {
        ch = ch << 1;
        ch |= (image_buffer[i] & 1);
    }
    return ch;
}

Status decode_magic_string(DecodeInfo *decInfo)
{
    char imageBuffer[8];
    char decoded_magic[strlen(MAGIC_STRING) + 1];

    for (int i = 0; i < strlen(MAGIC_STRING); i++)
    {
        fread(imageBuffer, 1, 8, decInfo->fptr_stego_image);
        decoded_magic[i] = decode_byte_from_lsb(imageBuffer);
    }
    decoded_magic[strlen(MAGIC_STRING)] = '\0';

    if (strcmp(decoded_magic, MAGIC_STRING) != 0)
    {
        return e_failure;
    }

    printf(WHITE "\n🪄  Decoded Magic String : %s\n" RESET, decoded_magic);
    return e_success;
}

long decode_size_from_lsb(char *image_buffer)
{
    long size = 0;
    for (int i = 0; i < 32; i++)
    {
        size = size << 1;
        size |= (image_buffer[i] & 1);
    }
    return size;
}

Status decode_secret_file_extn_size(DecodeInfo *decInfo)
{
    char imageBuffer[32];

    if (fread(imageBuffer, 1, 32, decInfo->fptr_stego_image) != 32)
    {
        return e_failure;
    }
    decInfo->extn_size = decode_size_from_lsb(imageBuffer);

    printf(WHITE "\n📏 Decoded Extension Size : %d\n" RESET, decInfo->extn_size);
    return e_success;
}

Status decode_secret_file_extn(DecodeInfo *decInfo)
{
    char imageBuffer[8];
    for (int i = 0; i < decInfo->extn_size; i++)
    {
        if (fread(imageBuffer, 1, 8, decInfo->fptr_stego_image) != 8)
        {
            return e_failure;
        }
        decInfo->extn_secret_file[i] = decode_byte_from_lsb(imageBuffer);
    }
    decInfo->extn_secret_file[decInfo->extn_size] = '\0';

    printf(WHITE "\n🏷️  Decoded File Extension : %s\n" RESET,
           decInfo->extn_secret_file);
    return e_success;
}

Status decode_secret_file_size(DecodeInfo *decInfo)
{
    char imageBuffer[32];
    if (fread(imageBuffer, 1, 32, decInfo->fptr_stego_image) != 32)
    {
        return e_failure;
    }

    decInfo->size_secret_file = decode_size_from_lsb(imageBuffer);

    printf(WHITE "\n📦 Decoded Secret File Size : %ld bytes\n" RESET,
           decInfo->size_secret_file);
    return e_success;
}

Status decode_secret_file_data(DecodeInfo *decInfo)
{
    char ch;
    char buffer[8];

    printf(WHITE "\n🔓 Extracting Secret Data into : %s\n" RESET,
           decInfo->output_fname);

    for (long i = 0; i < decInfo->size_secret_file; i++)
    {
        if (fread(buffer, 1, 8, decInfo->fptr_stego_image) != 8)
        {
            printf(RED "❌ ERROR: Failed to read encoded data\n" RESET);
            return e_failure;
        }

        ch = decode_byte_from_lsb(buffer);

        if (fwrite(&ch, 1, 1, decInfo->fptr_output_file) != 1)
        {
            printf(RED "❌ ERROR: Failed to write decoded data\n" RESET);
            return e_failure;
        }
    }
    return e_success;
}

Status do_decoding(DecodeInfo *decInfo)
{
    if (open_decoding_files(decInfo) == e_failure)
    {
        return e_failure;
    }
    printf(WHITE "✔" RESET GREEN " Files opened successfully\n" RESET);

    fseek(decInfo->fptr_stego_image, 54, SEEK_SET);

    if (decode_magic_string(decInfo) == e_failure)
    {
        printf(RED "❌ ERROR: Magic string mismatch\n" RESET);
        return e_failure;
    }
    printf(WHITE "✔" RESET GREEN " Magic string verified\n" RESET);

    if (decode_secret_file_extn_size(decInfo) == e_failure)
    {
        printf(RED "❌ ERROR: Invalid extension size\n" RESET);
        return e_failure;
    }
    printf(WHITE "✔" RESET GREEN " Extension size decoded\n" RESET);

    if (decode_secret_file_extn(decInfo) == e_failure)
    {
        printf(RED "❌ ERROR: Invalid file extension\n" RESET);
        return e_failure;
    }
    printf(WHITE "✔" RESET GREEN " File extension decoded\n" RESET);

    strcat(decInfo->output_fname, decInfo->extn_secret_file);

    decInfo->fptr_output_file = fopen(decInfo->output_fname, "w");
    if (decInfo->fptr_output_file == NULL)
    {
        perror("fopen");
        printf(RED "❌ ERROR: Unable to create output file\n" RESET);
        return e_failure;
    }

    printf(WHITE "\n📄 Output File Created : %s\n" RESET, decInfo->output_fname);

    if (decode_secret_file_size(decInfo) == e_failure)
    {
        printf(RED "❌ ERROR: Invalid secret file size\n" RESET);
        return e_failure;
    }
    printf(WHITE "✔" RESET GREEN " Secret file size decoded\n" RESET);

    if (decode_secret_file_data(decInfo) == e_failure)
    {
        printf(RED "❌ ERROR: Secret file decoding failed\n" RESET);
        return e_failure;
    }
    printf(WHITE "✔" RESET GREEN " Secret file data decoded\n" RESET);

    fclose(decInfo->fptr_stego_image);
    fclose(decInfo->fptr_output_file);
    return e_success;
}
