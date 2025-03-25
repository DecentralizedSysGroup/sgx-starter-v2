#include <openenclave/enclave.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <string.h>
#include <stdlib.h>
#include "rsa_t.h"  // Generated from rsa.edl

#define TRACE_ENCLAVE(fmt, ...)     \
                                    \
    printf(                         \
        "RSA: ***%s(%d): " fmt "\n", \
        __FILE__,                   \
        __LINE__,                   \
        ##__VA_ARGS__)

void generate_rsa_key(uint8_t* pub_key, size_t pub_key_size,
                      uint8_t* evidence, size_t evidence_cap, size_t* evidence_used)
{
    OpenSSL_add_all_algorithms();

    EVP_PKEY_CTX* pctx = NULL;
    EVP_PKEY* pkey = NULL;
    BIO* bio_pub = NULL;
    char* pub_key_buf = NULL;
    size_t pub_key_len = 0;
    oe_result_t result;

    /* Initialize output buffers */
    memset(pub_key, 0, pub_key_size);
    memset(evidence, 0, evidence_cap);

    /* Generate RSA key pair using the EVP API */
    pctx = EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, NULL);
    if (!pctx)
        goto done;
    if (EVP_PKEY_keygen_init(pctx) <= 0)
        goto done;
    if (EVP_PKEY_CTX_set_rsa_keygen_bits(pctx, 2048) <= 0)
        goto done;
    if (EVP_PKEY_keygen(pctx, &pkey) <= 0)
        goto done;

    printf("2048-bit RSA key successfully generated\n");


    /* Export the public key (PEM format) into a memory BIO */
    bio_pub = BIO_new(BIO_s_mem());
    if (!bio_pub || PEM_write_bio_PUBKEY(bio_pub, pkey) != 1)
        goto done;

    pub_key_len = BIO_pending(bio_pub);
    if (pub_key_len >= pub_key_size)
        goto done;

    pub_key_buf = (char*)malloc(pub_key_len + 1);
    if (!pub_key_buf)
        goto done;
    BIO_read(bio_pub, pub_key_buf, pub_key_len);
    pub_key_buf[pub_key_len] = '\0';

    /* Copy the public key into the provided output buffer */
    memcpy(pub_key, pub_key_buf, pub_key_len);
    pub_key[pub_key_len] = '\0';

    printf("2048-bit RSA key successfully converted to PEM format: %ld bytes\n", pub_key_len);


    uint8_t report_data[32] = {0};
    size_t report_data_size = sizeof(report_data);

    int res = Sha256(pub_key_buf, pub_key_len, report_data);
    if(res != 0) {
        TRACE_ENCLAVE( "Sha256 calculation failed\n");
        oe_abort();
    }

    generate_attestation_report(report_data, report_data_size, evidence, evidence_cap, evidence_used);

    /* Delete the private key by freeing the EVP_PKEY structure */
    EVP_PKEY_free(pkey);
    pkey = NULL;

    printf("Secret key deleted!");



done:
    if (bio_pub)
        BIO_free(bio_pub);
    if (pub_key_buf)
        free(pub_key_buf);
    if (pctx)
        EVP_PKEY_CTX_free(pctx);
    if (pkey)
        EVP_PKEY_free(pkey);
}

int generate_attestation_report(uint8_t *pk, size_t pk_len, uint8_t* report_out, size_t report_out_cap, uint8_t* report_out_used) {
    oe_result_t result;
    int ret = -1;
    int res, i;
    uint32_t flags = OE_REPORT_FLAGS_REMOTE_ATTESTATION;

#ifndef OE_SIMULATION
    XXX not developed yet
    // result = oe_get_report(flags, (const uint8_t *) report_data, 
                // report_data_size, NULL, 0,  &report->data, &report->size);
#else 
    // create a report with reasonable values for testing purposes
    if (sizeof(oe_report_t) + pk_len > report_out_cap) {
        TRACE_ENCLAVE("report_out_too small. %ld needed, %ld available\n", 
            sizeof(oe_report_t) + pk_len,
            report_out_cap);
        oe_abort();
    }

    *report_out_used = sizeof(oe_report_t) + pk_len;

    oe_report_t * parsed_report = (oe_report_t *) report_out;
    parsed_report->size = sizeof(oe_report_t);
    parsed_report->type = OE_ENCLAVE_TYPE_SGX;
    parsed_report->report_data_size = pk_len;
    parsed_report->report_data = report_out + sizeof(oe_report_t); // pointer to the beginning of the pk
    memcpy(parsed_report->report_data, pk, pk_len);
    parsed_report->enclave_report_size = *report_out_used;
    parsed_report->enclave_report = report_out;

    parsed_report->identity.id_version = 0;
    parsed_report->identity.security_version = 0;
    parsed_report->identity.attributes = OE_SIMULATION;

    memset(parsed_report->identity.unique_id, 0xaa, OE_UNIQUE_ID_SIZE);
    memset(parsed_report->identity.signer_id, 0xbb, OE_SIGNER_ID_SIZE);
    memset(parsed_report->identity.product_id, 0xcc, OE_PRODUCT_ID_SIZE);
    result = OE_OK;
#endif

    if(OE_OK != result) {
        TRACE_ENCLAVE( "oe_get_report failed with %s\n", oe_result_str(result));
        oe_abort();
    }
    ret = 0;
    return ret;
}


int Sha256(const uint8_t* data, size_t data_size, uint8_t sha256[32])
{
    int ret = -1;
    EVP_MD_CTX* ctx = NULL;

    if (!(ctx = EVP_MD_CTX_new()))
    {
        printf("EVP_MD_CTX_new failed!");
        goto exit;
    }

    if (!EVP_DigestInit_ex(ctx, EVP_sha256(), NULL))
    {
        printf("EVP_DigestInit_ex failed!");
        goto exit;
    }

    if (!EVP_DigestUpdate(ctx, data, data_size))
    {
        printf("EVP_DigestUpdate failed!");
        goto exit;
    }

    if (!EVP_DigestFinal_ex(ctx, sha256, NULL))
    {
        printf("EVP_DigestFinal_ex failed!");
        goto exit;
    }

    ret = 0;
exit:
    if (ctx)
        EVP_MD_CTX_free(ctx);
    return ret;
}