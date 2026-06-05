#include <stdio.h>
#include <string.h>
#include "api.h"
#include "drng.h"

DRNG_ctx drng_algorithm;

int main(void) {
    unsigned char pk[CRYPTO_PUBLICKEYBYTES];
    unsigned char sk[CRYPTO_SECRETKEYBYTES];
    unsigned char ct[CRYPTO_CIPHERTEXTBYTES];
    unsigned char ss_a[CRYPTO_BYTES];
    unsigned char ss_b[CRYPTO_BYTES];
    unsigned char ss_b2[CRYPTO_BYTES];
    int pass = 1;

    init_random_number(&drng_algorithm, NULL, 0);

    /* ---- Test 1: Roundtrip ---- */
    printf("Test 1: Keypair -> Encaps -> Decaps roundtrip... ");
    crypto_kem_keypair(pk, sk);
    crypto_kem_enc(ct, ss_a, pk);
    crypto_kem_dec(ss_b, ct, sk);

    if (memcmp(ss_a, ss_b, CRYPTO_BYTES) == 0) {
        printf("PASS\n");
    } else {
        printf("FAIL\n");
        pass = 0;
    }

    /* ---- Test 2: Bit-flip CT -> decaps no crash ---- */
    printf("Test 2: Bit-flipped CT decaps (no crash)... ");
    ct[0] ^= 1;
    crypto_kem_dec(ss_b, ct, sk);
    ct[0] ^= 1;
    printf("PASS\n");

    /* ---- Test 3: Deterministic invalid SS ---- */
    printf("Test 3: Deterministic invalid SS... ");
    ct[0] ^= 0x80;
    crypto_kem_dec(ss_b, ct, sk);
    crypto_kem_dec(ss_b2, ct, sk);
    ct[0] ^= 0x80;

    if (memcmp(ss_b, ss_b2, CRYPTO_BYTES) == 0) {
        printf("PASS\n");
    } else {
        printf("FAIL (non-deterministic)\n");
        pass = 0;
    }

    /* ---- Test 4: Invalid SS != valid SS ---- */
    printf("Test 4: Invalid SS != valid SS... ");
    if (memcmp(ss_a, ss_b, CRYPTO_BYTES) != 0) {
        printf("PASS\n");
    } else {
        printf("FAIL\n");
        pass = 0;
    }

    /* ---- Test 5: Deterministic Encaps ---- */
    printf("Test 5: Deterministic Encaps... ");
    unsigned char coins_kp[CRYPTO_KEYPAIRCOINBYTES];
    unsigned char coins_enc[CRYPTO_ENCCOINBYTES];
    memset(coins_kp, 0x42, CRYPTO_KEYPAIRCOINBYTES);
    memset(coins_enc, 0x42, CRYPTO_ENCCOINBYTES);

    crypto_kem_keypair_derand(pk, sk, coins_kp);
    crypto_kem_enc_derand(ct, ss_a, pk, coins_enc);
    crypto_kem_enc_derand(ct, ss_b, pk, coins_enc);

    if (memcmp(ss_a, ss_b, CRYPTO_BYTES) == 0) {
        printf("PASS\n");
    } else {
        printf("FAIL (non-deterministic)\n");
        pass = 0;
    }

    if (pass) {
        printf("\nAll tests PASSED.\n");
        return 0;
    } else {
        printf("\nSome tests FAILED.\n");
        return 1;
    }
}
