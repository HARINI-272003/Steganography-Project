#include <stdio.h>
#include "encode.h"
#include "decode.h"
#include "types.h"
OperationType check_operation_type(char *);

int main(int argc, char *argv[])
{
    printf(MAGENTA"\n🛡️  STEGANOGRAPHY : SECURE DATA HIDING SYSTEM 🛡️\n"RESET);
    EncodeInfo encInfo; // Structure to store Encoding information
    DecodeInfo decInfo; // Structure to store Decoding information
    if (argc >= 4)      // For encoding the number of CLA count must be more than or equal to 4
    {
        if (check_operation_type(argv[1]) == e_encode) // Check whether encoding or decoding
        {
            if (read_and_validate_encode_args(argv, &encInfo) == e_success)
            {
                printf(ORANGE "\n\t🔐 ENCODING MODE ACTIVATED 🔐\n\n" RESET);
                if (do_encoding(&encInfo) == e_success)
                {
                    printf(YELLOW "\nENCODING COMPLETED SUCCESSFULLY 🎉\n\n" RESET);
                }
                else
                {
                    printf(RED "\nENCODING PROCESS FAILED ❌\n" RESET); // Failure message
                    return 0;
                }
            }
            else
            {
                return 0;
            }
        }
        else
        {
            printf(RED "\nERROR: Invalid operation type!!!\n" RESET); // Unsupported message
            return 0;
        }
    }

    else if (argc >= 3)
    {
        if (check_operation_type(argv[1]) == e_decode)
        {
            if (read_and_validate_decode_args(argv, &decInfo) == e_success)
            {
                printf(ORANGE "\n\t🔓 DECODING MODE ACTIVATED 🔓\n\n" RESET);
                if (do_decoding(&decInfo) == e_success)
                {
                    printf(YELLOW "\nDECODING COMPLETED SUCCESSFULLY 🎉\n\n" RESET);// Success message
                }
                else
                {
                    printf(RED "\nDECODING PROCESS FAILED ❌\n" RESET); // Failure message
                    return 0;
                }
            }
            else
            {
                return 0;
            }
        }
        else
        {
            printf(RED "\nERROR: Invalid operation type!!!\n" RESET); // Unsupported message
            return 0;
        }
    }
    else
    {
        printf(RED "\nERROR: Insufficient command line arguments!!!\n" RESET);
        return 0;
    }
}
OperationType check_operation_type(char *symbol)
{
    if (strcmp(symbol, "-e") == 0)
    {
        return e_encode;
    }
    else if (strcmp(symbol, "-d") == 0)
    {
        return e_decode;
    }
    else
    {
        return e_unsupported;
    }
}
