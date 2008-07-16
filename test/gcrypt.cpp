/* bzflag
* Copyright (c) 1993 - 2008 Tim Riker
*
* This package is free software;  you can redistribute it and/or
* modify it under the terms of the license found in the file
* named COPYING that should have accompanied this file.
*
* THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
* IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
*/

#include "common.h"
#include "../bzauthd/RSA.h"
#include "../bzauthd/base64.h"

void nputs(const char *str, size_t len)
{
    for(size_t i = 0; i < len; i++)
        putchar(str[i]);
}

void md5test()
{
    int digest_len = (int)sRSAManager.md5len();

    uint8 *digest = new uint8[digest_len+1];
    memset(digest, 0, digest_len+1);
    uint8 buffer[] = "secret";
    size_t buffer_len = (int)strlen((const char*)buffer);

    sRSAManager.md5hash(buffer, buffer_len, digest);

    uint8 output[40];

    //for(int i = 0; i < digest_len; i++)
    //    printf("%X%X", digest[i]/16, digest[i]%16);
    base64::encode(digest, digest+digest_len, output);
    output[digest_len/2*3] = 0;

    printf("md5 base64 for %s is %s\n", buffer, output);

    delete[] digest;
}

void test_gcrypt()
{
    md5test(); return;

    if(!sRSAManager.initialize()) return;
    if(!sRSAManager.generateKeyPair()) return;

    // decompose the key into values that can be sent in packets
    size_t n_len;
    uint8 *key_n;
    uint16 e;
    if(!sRSAManager.getPublicKey().getValues(key_n, n_len, e)) return;
    if(e != 65537) return;

    // create the key from its components
    RSAPublicKey publicKey;
    if(!publicKey.setValues(key_n, n_len, e)) return;

    free(key_n);

    // encrypt a message
    char message[] = "let's see if this gets encrypted/decrypted properly";
    char *cipher = NULL;
    size_t cipher_len = 0;

    if(!publicKey.encrypt((uint8*)message, strlen(message), (uint8*&)cipher, cipher_len)) return;
    printf("encrypted: "); nputs((char*)cipher, cipher_len); printf("\n");

    // decrypt the cipher
    char *output = NULL;
    size_t output_len = 0;

    if(!sRSAManager.getSecretKey().decrypt((uint8*)cipher, cipher_len, (uint8*&)output, output_len)) return;
    printf("decrypted: "); nputs(output, output_len); printf("\n");

    gcry_free(cipher);
    gcry_free(output);
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8