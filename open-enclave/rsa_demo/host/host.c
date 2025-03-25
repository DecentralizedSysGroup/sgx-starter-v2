#include <openenclave/host.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "rsa_u.h"

#define PUB_KEY_SIZE 2048
#define report_SIZE 4096

static bool check_simulate_opt(int* argc, const char* argv[])
{
    for (int i = 0; i < *argc; i++)
    {
        if (strcmp(argv[i], "--simulate") == 0)
        {
            printf("Running in simulation mode\n");
            memmove(&argv[i], &argv[i + 1], (*argc - i) * sizeof(char*));
            (*argc)--;
            return true;
        }
    }
    return false;
}

int main(int argc, const char* argv[])
{
    oe_result_t result;
    int ret = 1;
    oe_enclave_t* enclave = NULL;
    uint32_t flags = OE_ENCLAVE_FLAG_DEBUG;

    if (check_simulate_opt(&argc, argv))
        flags |= OE_ENCLAVE_FLAG_SIMULATE;

    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s enclave.signed [--simulate]\n", argv[0]);
        return 1;
    }

    result = oe_create_rsa_enclave(argv[1], OE_ENCLAVE_TYPE_AUTO, flags, NULL, 0, &enclave);
    if (result != OE_OK)
    {
        fprintf(stderr, "oe_create_rsa_enclave failed: %s\n", oe_result_str(result));
        return 1;
    }

    uint8_t pub_key[PUB_KEY_SIZE] = {0};
    uint8_t report[report_SIZE] = {0};
    size_t report_used;

    result = generate_rsa_key(enclave, pub_key, sizeof(pub_key), report, sizeof(report), &report_used);
    if (result != OE_OK)
    {
        fprintf(stderr, "generate_rsa_key failed: %s\n", oe_result_str(result));
        goto cleanup;
    }

    printf("Public Key (PEM):\n%s\n", pub_key);

    // Dump report in hex format.
    printf("SGX Report (hex dump):\n");
    for (size_t i = 0; i < report_used; i++)
    {
        if (i % 16 == 0)
            printf("\n");
        printf("%02x ", report[i]);
    }
    printf("\n");

    FILE* f = fopen("report.bin", "wb");
    if (!f)
    {
        perror("fopen");
        goto cleanup;
    }

    fwrite(report, 1, report_used, f);
    fclose(f);
    printf("report written to report.bin\n");

    ret = 0;

cleanup:
    if (enclave)
        oe_terminate_enclave(enclave);
    return ret;
}