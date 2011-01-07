#include "common.h"
#include "crypto.h"

__device__ void * mymemset ( void * ptr, int value, size_t num ) {
    size_t i;

    for (i = 0; i < num; i++)
        ((char*) ptr)[i] = value;

    return ptr;
}

#define memset mymemset

__device__ void * mymemcpy ( void * destination, const void * source, size_t num ) {
    size_t i;

    for (i = 0; i < num; i++)
        ((char*) destination)[i] = ((char*) source)[i];

    return destination;
}

#define memcpy mymemcpy

__device__ void crypto_open (PCRYPTO_INFO cryptoInfo)
{
    memset (cryptoInfo, 0, sizeof (CRYPTO_INFO));
    cryptoInfo->ea = -1;
}

__device__ void crypto_loadkey (PKEY_INFO keyInfo, char *lpszUserKey, int nUserKeyLen)
{
    keyInfo->keyLength = nUserKeyLen;
    burn (keyInfo->userKey, sizeof (keyInfo->userKey));
    memcpy (keyInfo->userKey, lpszUserKey, nUserKeyLen);
}

__device__ int get_pkcs5_iteration_count (int pkcs5_prf_id, BOOL bBoot)
{
    switch (pkcs5_prf_id)
    {
    case RIPEMD160:	
        return (bBoot ? 1000 : 2000);

    case SHA512:	
        return 1000;			

    case SHA1:		// Deprecated/legacy		
        return 2000;			

    case WHIRLPOOL:	
        return 1000;

    default:		
        TC_THROW_FATAL_EXCEPTION;	// Unknown/wrong ID
    }
    return 0;
}

__device__ void RMD160Init (RMD160_CTX *ctx)
{
    ctx->count = 0;
    ctx->state[0] = 0x67452301;
    ctx->state[1] = 0xefcdab89;
    ctx->state[2] = 0x98badcfe;
    ctx->state[3] = 0x10325476;
    ctx->state[4] = 0xc3d2e1f0;

}

__device__ word32 rotlFixed (word32 x, unsigned int y)
{
    return (word32)((x<<y) | (x>>(sizeof(word32)*8-y)));
}

#define F(x, y, z)    (x ^ y ^ z) 
#define G(x, y, z)    (z ^ (x & (y^z)))
#define H(x, y, z)    (z ^ (x | ~y))
#define I(x, y, z)    (y ^ (z & (x^y)))
#define J(x, y, z)    (x ^ (y | ~z))

#define k0 0UL
#define k1 0x5a827999UL
#define k2 0x6ed9eba1UL
#define k3 0x8f1bbcdcUL
#define k4 0xa953fd4eUL
#define k5 0x50a28be6UL
#define k6 0x5c4dd124UL
#define k7 0x6d703ef3UL
#define k8 0x7a6d76e9UL
#define k9 0UL

#define Subround(f, a, b, c, d, e, x, s, k) \
	a += f(b, c, d) + x + k; \
	a = rotlFixed((word32)a, s) + e;\
	c = rotlFixed((word32)c, 10U)

__device__ void RMD160Transform (u32 *digest, const u32 *data)
{
    const word32 *X = data;

    word32 a1, b1, c1, d1, e1, a2, b2, c2, d2, e2;
    a1 = a2 = digest[0];
    b1 = b2 = digest[1];
    c1 = c2 = digest[2];
    d1 = d2 = digest[3];
    e1 = e2 = digest[4];

    Subround(F, a1, b1, c1, d1, e1, X[ 0], 11, k0);
    Subround(F, e1, a1, b1, c1, d1, X[ 1], 14, k0);
    Subround(F, d1, e1, a1, b1, c1, X[ 2], 15, k0);
    Subround(F, c1, d1, e1, a1, b1, X[ 3], 12, k0);
    Subround(F, b1, c1, d1, e1, a1, X[ 4],  5, k0);
    Subround(F, a1, b1, c1, d1, e1, X[ 5],  8, k0);
    Subround(F, e1, a1, b1, c1, d1, X[ 6],  7, k0);
    Subround(F, d1, e1, a1, b1, c1, X[ 7],  9, k0);
    Subround(F, c1, d1, e1, a1, b1, X[ 8], 11, k0);
    Subround(F, b1, c1, d1, e1, a1, X[ 9], 13, k0);
    Subround(F, a1, b1, c1, d1, e1, X[10], 14, k0);
    Subround(F, e1, a1, b1, c1, d1, X[11], 15, k0);
    Subround(F, d1, e1, a1, b1, c1, X[12],  6, k0);
    Subround(F, c1, d1, e1, a1, b1, X[13],  7, k0);
    Subround(F, b1, c1, d1, e1, a1, X[14],  9, k0);
    Subround(F, a1, b1, c1, d1, e1, X[15],  8, k0);

    Subround(G, e1, a1, b1, c1, d1, X[ 7],  7, k1);
    Subround(G, d1, e1, a1, b1, c1, X[ 4],  6, k1);
    Subround(G, c1, d1, e1, a1, b1, X[13],  8, k1);
    Subround(G, b1, c1, d1, e1, a1, X[ 1], 13, k1);
    Subround(G, a1, b1, c1, d1, e1, X[10], 11, k1);
    Subround(G, e1, a1, b1, c1, d1, X[ 6],  9, k1);
    Subround(G, d1, e1, a1, b1, c1, X[15],  7, k1);
    Subround(G, c1, d1, e1, a1, b1, X[ 3], 15, k1);
    Subround(G, b1, c1, d1, e1, a1, X[12],  7, k1);
    Subround(G, a1, b1, c1, d1, e1, X[ 0], 12, k1);
    Subround(G, e1, a1, b1, c1, d1, X[ 9], 15, k1);
    Subround(G, d1, e1, a1, b1, c1, X[ 5],  9, k1);
    Subround(G, c1, d1, e1, a1, b1, X[ 2], 11, k1);
    Subround(G, b1, c1, d1, e1, a1, X[14],  7, k1);
    Subround(G, a1, b1, c1, d1, e1, X[11], 13, k1);
    Subround(G, e1, a1, b1, c1, d1, X[ 8], 12, k1);

    Subround(H, d1, e1, a1, b1, c1, X[ 3], 11, k2);
    Subround(H, c1, d1, e1, a1, b1, X[10], 13, k2);
    Subround(H, b1, c1, d1, e1, a1, X[14],  6, k2);
    Subround(H, a1, b1, c1, d1, e1, X[ 4],  7, k2);
    Subround(H, e1, a1, b1, c1, d1, X[ 9], 14, k2);
    Subround(H, d1, e1, a1, b1, c1, X[15],  9, k2);
    Subround(H, c1, d1, e1, a1, b1, X[ 8], 13, k2);
    Subround(H, b1, c1, d1, e1, a1, X[ 1], 15, k2);
    Subround(H, a1, b1, c1, d1, e1, X[ 2], 14, k2);
    Subround(H, e1, a1, b1, c1, d1, X[ 7],  8, k2);
    Subround(H, d1, e1, a1, b1, c1, X[ 0], 13, k2);
    Subround(H, c1, d1, e1, a1, b1, X[ 6],  6, k2);
    Subround(H, b1, c1, d1, e1, a1, X[13],  5, k2);
    Subround(H, a1, b1, c1, d1, e1, X[11], 12, k2);
    Subround(H, e1, a1, b1, c1, d1, X[ 5],  7, k2);
    Subround(H, d1, e1, a1, b1, c1, X[12],  5, k2);

    Subround(I, c1, d1, e1, a1, b1, X[ 1], 11, k3);
    Subround(I, b1, c1, d1, e1, a1, X[ 9], 12, k3);
    Subround(I, a1, b1, c1, d1, e1, X[11], 14, k3);
    Subround(I, e1, a1, b1, c1, d1, X[10], 15, k3);
    Subround(I, d1, e1, a1, b1, c1, X[ 0], 14, k3);
    Subround(I, c1, d1, e1, a1, b1, X[ 8], 15, k3);
    Subround(I, b1, c1, d1, e1, a1, X[12],  9, k3);
    Subround(I, a1, b1, c1, d1, e1, X[ 4],  8, k3);
    Subround(I, e1, a1, b1, c1, d1, X[13],  9, k3);
    Subround(I, d1, e1, a1, b1, c1, X[ 3], 14, k3);
    Subround(I, c1, d1, e1, a1, b1, X[ 7],  5, k3);
    Subround(I, b1, c1, d1, e1, a1, X[15],  6, k3);
    Subround(I, a1, b1, c1, d1, e1, X[14],  8, k3);
    Subround(I, e1, a1, b1, c1, d1, X[ 5],  6, k3);
    Subround(I, d1, e1, a1, b1, c1, X[ 6],  5, k3);
    Subround(I, c1, d1, e1, a1, b1, X[ 2], 12, k3);

    Subround(J, b1, c1, d1, e1, a1, X[ 4],  9, k4);
    Subround(J, a1, b1, c1, d1, e1, X[ 0], 15, k4);
    Subround(J, e1, a1, b1, c1, d1, X[ 5],  5, k4);
    Subround(J, d1, e1, a1, b1, c1, X[ 9], 11, k4);
    Subround(J, c1, d1, e1, a1, b1, X[ 7],  6, k4);
    Subround(J, b1, c1, d1, e1, a1, X[12],  8, k4);
    Subround(J, a1, b1, c1, d1, e1, X[ 2], 13, k4);
    Subround(J, e1, a1, b1, c1, d1, X[10], 12, k4);
    Subround(J, d1, e1, a1, b1, c1, X[14],  5, k4);
    Subround(J, c1, d1, e1, a1, b1, X[ 1], 12, k4);
    Subround(J, b1, c1, d1, e1, a1, X[ 3], 13, k4);
    Subround(J, a1, b1, c1, d1, e1, X[ 8], 14, k4);
    Subround(J, e1, a1, b1, c1, d1, X[11], 11, k4);
    Subround(J, d1, e1, a1, b1, c1, X[ 6],  8, k4);
    Subround(J, c1, d1, e1, a1, b1, X[15],  5, k4);
    Subround(J, b1, c1, d1, e1, a1, X[13],  6, k4);

    Subround(J, a2, b2, c2, d2, e2, X[ 5],  8, k5);
    Subround(J, e2, a2, b2, c2, d2, X[14],  9, k5);
    Subround(J, d2, e2, a2, b2, c2, X[ 7],  9, k5);
    Subround(J, c2, d2, e2, a2, b2, X[ 0], 11, k5);
    Subround(J, b2, c2, d2, e2, a2, X[ 9], 13, k5);
    Subround(J, a2, b2, c2, d2, e2, X[ 2], 15, k5);
    Subround(J, e2, a2, b2, c2, d2, X[11], 15, k5);
    Subround(J, d2, e2, a2, b2, c2, X[ 4],  5, k5);
    Subround(J, c2, d2, e2, a2, b2, X[13],  7, k5);
    Subround(J, b2, c2, d2, e2, a2, X[ 6],  7, k5);
    Subround(J, a2, b2, c2, d2, e2, X[15],  8, k5);
    Subround(J, e2, a2, b2, c2, d2, X[ 8], 11, k5);
    Subround(J, d2, e2, a2, b2, c2, X[ 1], 14, k5);
    Subround(J, c2, d2, e2, a2, b2, X[10], 14, k5);
    Subround(J, b2, c2, d2, e2, a2, X[ 3], 12, k5);
    Subround(J, a2, b2, c2, d2, e2, X[12],  6, k5);

    Subround(I, e2, a2, b2, c2, d2, X[ 6],  9, k6); 
    Subround(I, d2, e2, a2, b2, c2, X[11], 13, k6);
    Subround(I, c2, d2, e2, a2, b2, X[ 3], 15, k6);
    Subround(I, b2, c2, d2, e2, a2, X[ 7],  7, k6);
    Subround(I, a2, b2, c2, d2, e2, X[ 0], 12, k6);
    Subround(I, e2, a2, b2, c2, d2, X[13],  8, k6);
    Subround(I, d2, e2, a2, b2, c2, X[ 5],  9, k6);
    Subround(I, c2, d2, e2, a2, b2, X[10], 11, k6);
    Subround(I, b2, c2, d2, e2, a2, X[14],  7, k6);
    Subround(I, a2, b2, c2, d2, e2, X[15],  7, k6);
    Subround(I, e2, a2, b2, c2, d2, X[ 8], 12, k6);
    Subround(I, d2, e2, a2, b2, c2, X[12],  7, k6);
    Subround(I, c2, d2, e2, a2, b2, X[ 4],  6, k6);
    Subround(I, b2, c2, d2, e2, a2, X[ 9], 15, k6);
    Subround(I, a2, b2, c2, d2, e2, X[ 1], 13, k6);
    Subround(I, e2, a2, b2, c2, d2, X[ 2], 11, k6);

    Subround(H, d2, e2, a2, b2, c2, X[15],  9, k7);
    Subround(H, c2, d2, e2, a2, b2, X[ 5],  7, k7);
    Subround(H, b2, c2, d2, e2, a2, X[ 1], 15, k7);
    Subround(H, a2, b2, c2, d2, e2, X[ 3], 11, k7);
    Subround(H, e2, a2, b2, c2, d2, X[ 7],  8, k7);
    Subround(H, d2, e2, a2, b2, c2, X[14],  6, k7);
    Subround(H, c2, d2, e2, a2, b2, X[ 6],  6, k7);
    Subround(H, b2, c2, d2, e2, a2, X[ 9], 14, k7);
    Subround(H, a2, b2, c2, d2, e2, X[11], 12, k7);
    Subround(H, e2, a2, b2, c2, d2, X[ 8], 13, k7);
    Subround(H, d2, e2, a2, b2, c2, X[12],  5, k7);
    Subround(H, c2, d2, e2, a2, b2, X[ 2], 14, k7);
    Subround(H, b2, c2, d2, e2, a2, X[10], 13, k7);
    Subround(H, a2, b2, c2, d2, e2, X[ 0], 13, k7);
    Subround(H, e2, a2, b2, c2, d2, X[ 4],  7, k7);
    Subround(H, d2, e2, a2, b2, c2, X[13],  5, k7);

    Subround(G, c2, d2, e2, a2, b2, X[ 8], 15, k8);
    Subround(G, b2, c2, d2, e2, a2, X[ 6],  5, k8);
    Subround(G, a2, b2, c2, d2, e2, X[ 4],  8, k8);
    Subround(G, e2, a2, b2, c2, d2, X[ 1], 11, k8);
    Subround(G, d2, e2, a2, b2, c2, X[ 3], 14, k8);
    Subround(G, c2, d2, e2, a2, b2, X[11], 14, k8);
    Subround(G, b2, c2, d2, e2, a2, X[15],  6, k8);
    Subround(G, a2, b2, c2, d2, e2, X[ 0], 14, k8);
    Subround(G, e2, a2, b2, c2, d2, X[ 5],  6, k8);
    Subround(G, d2, e2, a2, b2, c2, X[12],  9, k8);
    Subround(G, c2, d2, e2, a2, b2, X[ 2], 12, k8);
    Subround(G, b2, c2, d2, e2, a2, X[13],  9, k8);
    Subround(G, a2, b2, c2, d2, e2, X[ 9], 12, k8);
    Subround(G, e2, a2, b2, c2, d2, X[ 7],  5, k8);
    Subround(G, d2, e2, a2, b2, c2, X[10], 15, k8);
    Subround(G, c2, d2, e2, a2, b2, X[14],  8, k8);

    Subround(F, b2, c2, d2, e2, a2, X[12],  8, k9);
    Subround(F, a2, b2, c2, d2, e2, X[15],  5, k9);
    Subround(F, e2, a2, b2, c2, d2, X[10], 12, k9);
    Subround(F, d2, e2, a2, b2, c2, X[ 4],  9, k9);
    Subround(F, c2, d2, e2, a2, b2, X[ 1], 12, k9);
    Subround(F, b2, c2, d2, e2, a2, X[ 5],  5, k9);
    Subround(F, a2, b2, c2, d2, e2, X[ 8], 14, k9);
    Subround(F, e2, a2, b2, c2, d2, X[ 7],  6, k9);
    Subround(F, d2, e2, a2, b2, c2, X[ 6],  8, k9);
    Subround(F, c2, d2, e2, a2, b2, X[ 2], 13, k9);
    Subround(F, b2, c2, d2, e2, a2, X[13],  6, k9);
    Subround(F, a2, b2, c2, d2, e2, X[14],  5, k9);
    Subround(F, e2, a2, b2, c2, d2, X[ 0], 15, k9);
    Subround(F, d2, e2, a2, b2, c2, X[ 3], 13, k9);
    Subround(F, c2, d2, e2, a2, b2, X[ 9], 11, k9);
    Subround(F, b2, c2, d2, e2, a2, X[11], 11, k9);

    c1        = digest[1] + c1 + d2;
    digest[1] = digest[2] + d1 + e2;
    digest[2] = digest[3] + e1 + a2;
    digest[3] = digest[4] + a1 + b2;
    digest[4] = digest[0] + b1 + c2;
    digest[0] = c1;
}
#undef k1
#undef k2
#undef k3

// Update context to reflect the concatenation of another buffer full
// of bytes.
__device__ void RMD160Update (RMD160_CTX *ctx, const unsigned char *input, u32 lenArg)
{
    uint64 len = lenArg, have, need;

    // Check how many bytes we already have and how many more we need.
    have = ctx->count >> 3;
    have &= (RIPEMD160_BLOCK_LENGTH - 1);
    need = RIPEMD160_BLOCK_LENGTH - have;

    // Update bitcount.
    ctx->count += len << 3;

    if (len >= need) {
        if (have != 0) {
            memcpy (ctx->buffer + have, input, (size_t) need);
            RMD160Transform ((uint32 *) ctx->state, (const uint32 *) ctx->buffer);
            input += need;
            len -= need;
            have = 0;
        }

        // Process data in RIPEMD160_BLOCK_LENGTH-byte chunks.
        while (len >= RIPEMD160_BLOCK_LENGTH) {
            RMD160Transform ((uint32 *) ctx->state, (const uint32 *) input);
            input += RIPEMD160_BLOCK_LENGTH;
            len -= RIPEMD160_BLOCK_LENGTH;
        }
    }

    // Handle any remaining bytes of data.
    if (len != 0)
        memcpy (ctx->buffer + have, input, (size_t) len);
}

// Pad pad to 64-byte boundary with the bit pattern
// 1 0* (64-bit count of bits processed, MSB-first)
__device__ void RMD160Pad(RMD160_CTX *ctx)
{
    byte count[8];
    uint32 padlen;

    // Convert count to 8 bytes in little endian order.
    PUT_64BIT_LE(count, ctx->count);

    // Pad out to 56 mod 64.
    padlen = RIPEMD160_BLOCK_LENGTH -
        (uint32)((ctx->count >> 3) & (RIPEMD160_BLOCK_LENGTH - 1));
    if (padlen < 1 + 8)
        padlen += RIPEMD160_BLOCK_LENGTH;
    RMD160Update(ctx, PADDING, padlen - 8);            // padlen - 8 <= 64
    RMD160Update(ctx, count, 8);
}

// Final wrapup--call RMD160Pad, fill in digest and zero out ctx.
__device__ void RMD160Final(unsigned char *digest, RMD160_CTX *ctx)
{
    int i;

    RMD160Pad(ctx);
    if (digest) {
        for (i = 0; i < 5; i++)
            PUT_32BIT_LE(digest + i * 4, ctx->state[i]);
        memset (ctx, 0, sizeof(*ctx));
    }
}

#define k_ipad td->k_ipad
#define k_opad td->k_opad
__device__ void hmac_ripemd160 (char *key, int keylen, char *input, int len, char *digest, PTHREAD_DATA td)
{
    RMD160_CTX context;
    unsigned char tk[RIPEMD160_DIGESTSIZE];
    int i;

    // If the key is longer than the hash algorithm block size,
    // let key = ripemd160(key), as per HMAC specifications.
    if (keylen > RIPEMD160_BLOCKSIZE) 
    {
        RMD160_CTX      tctx;

        RMD160Init(&tctx);
        RMD160Update(&tctx, (const unsigned char *) key, keylen);
        RMD160Final(tk, &tctx);

        key = (char *) tk;
        keylen = RIPEMD160_DIGESTSIZE;

        burn (&tctx, sizeof(tctx));	// Prevent leaks
    }

    // RMD160(K XOR opad, RMD160(K XOR ipad, text))
    // where K is an n byte key
    // ipad is the byte 0x36 repeated RIPEMD160_BLOCKSIZE times
    // opad is the byte 0x5c repeated RIPEMD160_BLOCKSIZE times
    // and text is the data being protected */

    // start out by storing key in pads
    memset(k_ipad, 54U, 64);
    memset(k_opad, 92U, 64);

    // XOR key with ipad and opad values
    for (i=0; i<keylen; i++) 
    {
        k_ipad[i] ^= (unsigned char)key[i];
        k_opad[i] ^= (unsigned char)key[i];
    }

    // perform inner RIPEMD-160

    RMD160Init(&context);           // init context for 1st pass
    RMD160Update(&context, k_ipad, RIPEMD160_BLOCKSIZE);  // start with inner pad
    RMD160Update(&context, (const unsigned char *) input, len); // then text of datagram
    RMD160Final((unsigned char *) digest, &context);         // finish up 1st pass

    // perform outer RIPEMD-160
    RMD160Init(&context);           // init context for 2nd pass
    RMD160Update(&context, k_opad, RIPEMD160_BLOCKSIZE);  // start with outer pad
    // results of 1st hash
    RMD160Update(&context, (const unsigned char *) digest, RIPEMD160_DIGESTSIZE);
    RMD160Final((unsigned char *) digest, &context);         // finish up 2nd pass

    // Prevent possible leaks.
    burn (tk, sizeof(tk));
    burn (&context, sizeof(context));
}
#undef k_ipad
#undef k_opad

__device__ void derive_u_ripemd160 (char *pwd, int pwd_len, char *salt, int salt_len, int iterations, char *u, int b, PTHREAD_DATA td)
{
    char j[RIPEMD160_DIGESTSIZE], k[RIPEMD160_DIGESTSIZE];
    char init[128];
    char counter[4];
    int c, i;

    // iteration 1
    memset (counter, 0, 4);
    counter[3] = (char) b;
    memcpy (init, salt, salt_len);	// salt
    memcpy (&init[salt_len], counter, 4);	// big-endian block number

    // remaining iterations
    for (c = 0; c < iterations; c++)
    {
		hmac_ripemd160 (pwd, pwd_len, !c ? init : j, !c ? salt_len + 4 : RIPEMD160_DIGESTSIZE, !c ? j : k, td);
		if (!c)
		memcpy (u, j, RIPEMD160_DIGESTSIZE);
		else
        for (i = 0; i < RIPEMD160_DIGESTSIZE; i++)
        {
            u[i] ^= k[i];
            j[i] = k[i];
        }
    }

    // Prevent possible leaks.
    burn (j, sizeof(j));
    burn (k, sizeof(k));
}

__device__ void derive_key_ripemd160 (char *pwd, int pwd_len, char *salt, int salt_len, int iterations, char *dk, int dklen, PTHREAD_DATA td)
{
    char u[RIPEMD160_DIGESTSIZE];
    int b, l, r;

    if (dklen % RIPEMD160_DIGESTSIZE)
    {
        l = 1 + dklen / RIPEMD160_DIGESTSIZE;
    }
    else
    {
        l = dklen / RIPEMD160_DIGESTSIZE;
    }

    r = dklen - (l - 1) * RIPEMD160_DIGESTSIZE;

    // first l - 1 blocks
    for (b = 1; b <= l; b++)
    {
        derive_u_ripemd160 (pwd, pwd_len, salt, salt_len, iterations, u, b, td);
		if (b < l)
		{
            memcpy (dk, u, RIPEMD160_DIGESTSIZE);
            dk += RIPEMD160_DIGESTSIZE;
		}
    }

    // last block
    memcpy (dk, u, r);


    // Prevent possible leaks.
    burn (u, sizeof(u));
}

__device__ int EAGetFirst ()
{
    return 1;
}

__device__ int EAGetNext (int previousEA)
{
    int id = previousEA + 1;
    if (EncryptionAlgorithms[id].Ciphers[0] != 0) return id;
    return 0;
}

// Returns the first mode of operation of EA
__device__ int EAGetFirstMode (int ea)
{
    return (EncryptionAlgorithms[ea].Modes[0]);
}

__device__ int EAGetNextMode (int ea, int previousModeId)
{
    int c, i = 0;
    while (c = EncryptionAlgorithms[ea].Modes[i++])
    {
        if (c == previousModeId) 
            return EncryptionAlgorithms[ea].Modes[i];
    }

    return 0;
}

// Returns TRUE if the mode of operation is supported for the encryption algorithm
__device__ BOOL EAIsModeSupported (int ea, int testedMode)
{
    int mode;

    for (mode = EAGetFirstMode (ea); mode != 0; mode = EAGetNextMode (ea, mode))
    {
        if (mode == testedMode)
            return TRUE;
    }
    return FALSE;
}

__device__ int EAGetFirstCipher (int ea)
{
    return EncryptionAlgorithms[ea].Ciphers[0];
}

__device__ int EAGetNextCipher (int ea, int previousCipherId)
{
    int c, i = 0;
    while (c = EncryptionAlgorithms[ea].Ciphers[i++])
    {
        if (c == previousCipherId) 
            return EncryptionAlgorithms[ea].Ciphers[i];
    }

    return 0;
}

// Returns number of ciphers in EA
__device__ int EAGetCipherCount (int ea)
{
    int i = 0;
    while (EncryptionAlgorithms[ea].Ciphers[i++]);

    return i - 1;
}

__device__ int EAGetLastCipher (int ea)
{
    int i = 0;
    while (EncryptionAlgorithms[ea].Ciphers[i++]);

    return EncryptionAlgorithms[ea].Ciphers[i - 2];
}

__device__ int EAGetPreviousCipher (int ea, int previousCipherId)
{
    int c, i = 0;

    if (EncryptionAlgorithms[ea].Ciphers[i++] == previousCipherId)
        return 0;

    while (c = EncryptionAlgorithms[ea].Ciphers[i++])
    {
        if (c == previousCipherId) 
            return EncryptionAlgorithms[ea].Ciphers[i - 2];
    }

    return 0;
}

__device__ const Cipher *CipherGet (int id)
{
    int i;
    for (i = 0; Ciphers[i].Id != 0; i++)
        if (Ciphers[i].Id == id)
            return &Ciphers[i];

    return NULL;
}

__device__ int CipherGetKeySize (int cipherId)
{
    return CipherGet (cipherId) -> KeySize;
}

// Returns sum of key sizes of all ciphers of the EA (in bytes)
__device__ int EAGetKeySize (int ea)
{
    int i = EAGetFirstCipher (ea);
    int size = CipherGetKeySize (i);

    while (i = EAGetNextCipher (ea, i))
    {
        size += CipherGetKeySize (i);
    }

    return size;
}

// Returns the largest key size needed by an EA for the specified mode of operation
__device__ int EAGetLargestKeyForMode (int mode)
{
    int ea, key = 0;

    for (ea = EAGetFirst (); ea != 0; ea = EAGetNext (ea))
    {
        if (!EAIsModeSupported (ea, mode))
            continue;

        if (EAGetKeySize (ea) >= key)
            key = EAGetKeySize (ea);
    }
    return key;
}

__device__ int GetMaxPkcs5OutSize (void)
{
    int size = 32;

    size = max (size, EAGetLargestKeyForMode (XTS) * 2);	// Sizes of primary + secondary keys

    size = max (size, LEGACY_VOL_IV_SIZE + EAGetLargestKeyForMode (LRW));		// Deprecated/legacy
    size = max (size, LEGACY_VOL_IV_SIZE + EAGetLargestKeyForMode (CBC));		// Deprecated/legacy
    size = max (size, LEGACY_VOL_IV_SIZE + EAGetLargestKeyForMode (OUTER_CBC));	// Deprecated/legacy
    size = max (size, LEGACY_VOL_IV_SIZE + EAGetLargestKeyForMode (INNER_CBC));	// Deprecated/legacy

    return size;
}

#define rotl32(x,n)   (((x) << n) | ((x) >> (32 - n)))
#define rotr32(x,n)   (((x) >> n) | ((x) << (32 - n)))
#define rotr64(x,n)   (((x) >> n) | ((x) << (64 - n)))
#define bswap_32(x) ((rotr32((x), 24) & 0x00ff00ff) | (rotr32((x), 8) & 0xff00ff00))
#define bswap_64(x) (((uint_64t)(bswap_32((uint_32t)(x)))) << 32 | bswap_32((uint_32t)((x) >> 32)))
#define bsw_32(p,n) \
    { int _i = (n); while(_i--) ((sha1_32t*)p)[_i] = bswap_32(((sha1_32t*)p)[_i]); }
#define bsw_64(p,n) \
    { int _i = (n); while(_i--) ((uint_64t*)p)[_i] = bswap_64(((uint_64t*)p)[_i]); }

#define s_0(x)  (rotr64((x), 28) ^ rotr64((x), 34) ^ rotr64((x), 39))
#define s_1(x)  (rotr64((x), 14) ^ rotr64((x), 18) ^ rotr64((x), 41))
#define g_0(x)  (rotr64((x),  1) ^ rotr64((x),  8) ^ ((x) >>  7))
#define g_1(x)  (rotr64((x), 19) ^ rotr64((x), 61) ^ ((x) >>  6))
#define k_0     k512

#define ch(x,y,z)       ((z) ^ ((x) & ((y) ^ (z))))
#define parity(x,y,z)   ((x) ^ (y) ^ (z))
#define maj(x,y,z)      (((x) & (y)) | ((z) & ((x) ^ (y))))

// round transforms for SHA256 and SHA512 compression functions

#define vf(n,i) v[(n - i) & 7]

#define hf(i) (p[i & 15] += \
    g_1(p[(i + 14) & 15]) + p[(i + 9) & 15] + g_0(p[(i + 1) & 15]))

#define v_cycle(i,j)                                \
    vf(7,i) += (j ? hf(i) : p[i]) + k_0[i+j]        \
    + s_1(vf(4,i)) + ch(vf(4,i),vf(5,i),vf(6,i));   \
    vf(3,i) += vf(7,i);                             \
    vf(7,i) += s_0(vf(0,i))+ maj(vf(0,i),vf(1,i),vf(2,i))

__device__ VOID_RETURN sha512_compile(sha512_ctx ctx[1])
{   uint_64t    v[8], *p = ctx->wbuf;
    uint_32t    j;

    memcpy(v, ctx->hash, 8 * sizeof(uint_64t));

    for(j = 0; j < 80; j += 16)
    {
        v_cycle( 0, j); v_cycle( 1, j);
        v_cycle( 2, j); v_cycle( 3, j);
        v_cycle( 4, j); v_cycle( 5, j);
        v_cycle( 6, j); v_cycle( 7, j);
        v_cycle( 8, j); v_cycle( 9, j);
        v_cycle(10, j); v_cycle(11, j);
        v_cycle(12, j); v_cycle(13, j);
        v_cycle(14, j); v_cycle(15, j);
    }

    ctx->hash[0] += v[0]; ctx->hash[1] += v[1];
    ctx->hash[2] += v[2]; ctx->hash[3] += v[3];
    ctx->hash[4] += v[4]; ctx->hash[5] += v[5];
    ctx->hash[6] += v[6]; ctx->hash[7] += v[7];
}

__device__ void sha_end2(unsigned char hval[], sha512_ctx ctx[1], const unsigned int hlen)
{   uint_32t    i = (uint_32t)(ctx->count[0] & SHA512_MASK);

    /* put bytes in the buffer in an order in which references to   */
    /* 32-bit words will put bytes with lower addresses into the    */
    /* top of 32 bit words on BOTH big and little endian machines   */
    bsw_64(ctx->wbuf, (i + 7) >> 3);

    /* we now need to mask valid bytes and add the padding which is */
    /* a single 1 bit and as many zero bits as necessary. Note that */
    /* we can always add the first padding byte here because the    */
    /* buffer always has at least one empty slot                    */
    ctx->wbuf[i >> 3] &= li_64(ffffffffffffff00) << 8 * (~i & 7);
    ctx->wbuf[i >> 3] |= li_64(0000000000000080) << 8 * (~i & 7);

    /* we need 17 or more empty byte positions, one for the padding */
    /* byte (above) and sixteen for the length count.  If there is  */
    /* not enough space pad and empty the buffer                    */
    if(i > SHA512_BLOCK_SIZE - 17)
    {
        if(i < 120) ctx->wbuf[15] = 0;
        sha512_compile(ctx);
        i = 0;
    }
    else
        i = (i >> 3) + 1;

    while(i < 14)
        ctx->wbuf[i++] = 0;

    /* the following 64-bit length fields are assembled in the      */
    /* wrong byte order on little endian machines but this is       */
    /* corrected later since they are only ever used as 64-bit      */
    /* word values.                                                 */
    ctx->wbuf[14] = (ctx->count[1] << 3) | (ctx->count[0] >> 61);
    ctx->wbuf[15] = ctx->count[0] << 3;
    sha512_compile(ctx);

    /* extract the hash value as bytes in case the hash buffer is   */
    /* misaligned for 32-bit words                                  */
    for(i = 0; i < hlen; ++i)
        hval[i] = (unsigned char)(ctx->hash[i >> 3] >> (8 * (~i & 7)));
}

__device__ VOID_RETURN sha512_begin(sha512_ctx ctx[1])
{
    ctx->count[0] = ctx->count[1] = 0;
    memcpy(ctx->hash, i512, 8 * sizeof(uint_64t));
}

__device__ VOID_RETURN sha512_end(unsigned char hval[], sha512_ctx ctx[1])
{
    sha_end2(hval, ctx, SHA512_DIGEST_SIZE);
}

/* Compile 128 bytes of hash data into SHA256 digest value  */
/* NOTE: this routine assumes that the byte order in the    */
/* ctx->wbuf[] at this point is in such an order that low   */
/* address bytes in the ORIGINAL byte stream placed in this */
/* buffer will now go to the high end of words on BOTH big  */
/* and little endian systems                                */

__device__ VOID_RETURN sha512_hash(const unsigned char data[], unsigned long len, sha512_ctx ctx[1])
{   uint_32t pos = (uint_32t)(ctx->count[0] & SHA512_MASK),
             space = SHA512_BLOCK_SIZE - pos;
    const unsigned char *sp = data;

    if((ctx->count[0] += len) < len)
        ++(ctx->count[1]);

    while(len >= space)     /* tranfer whole blocks while possible  */
    {
        memcpy(((unsigned char*)ctx->wbuf) + pos, sp, space);
        sp += space; len -= space; space = SHA512_BLOCK_SIZE; pos = 0;
        bsw_64(ctx->wbuf, SHA512_BLOCK_SIZE >> 3);
        sha512_compile(ctx);
    }

    memcpy(((unsigned char*)ctx->wbuf) + pos, sp, len);
}

__device__ void hmac_truncate
  (
      char *d1,		/* data to be truncated */
      char *d2,		/* truncated data */
      int len		/* length in bytes to keep */
)
{
    int i;
    for (i = 0; i < len; i++)
        d2[i] = d1[i];
}

__device__ void hmac_sha512
(
      char *k,		/* secret key */
      int lk,		/* length of the key in bytes */
      char *d,		/* data */
      int ld,		/* length of data in bytes */
      char *out,		/* output buffer, at least "t" bytes */
      int t
)
{
    sha512_ctx ictx, octx;
    char isha[SHA512_DIGESTSIZE], osha[SHA512_DIGESTSIZE];
    char key[SHA512_DIGESTSIZE];
    char buf[SHA512_BLOCKSIZE];
    int i;

    /* If the key is longer than the hash algorithm block size,
       let key = sha512(key), as per HMAC specifications. */
    if (lk > SHA512_BLOCKSIZE)
    {
        sha512_ctx tctx;

        sha512_begin (&tctx);
        sha512_hash ((unsigned char *) k, lk, &tctx);
        sha512_end ((unsigned char *) key, &tctx);

        k = key;
        lk = SHA512_DIGESTSIZE;

        burn (&tctx, sizeof(tctx));		// Prevent leaks
    }

    /**** Inner Digest ****/

    sha512_begin (&ictx);

    /* Pad the key for inner digest */
    for (i = 0; i < lk; ++i)
        buf[i] = (char) (k[i] ^ 0x36);
    for (i = lk; i < SHA512_BLOCKSIZE; ++i)
        buf[i] = 0x36;

    sha512_hash ((unsigned char *) buf, SHA512_BLOCKSIZE, &ictx);
    sha512_hash ((unsigned char *) d, ld, &ictx);

    sha512_end ((unsigned char *) isha, &ictx);

    /**** Outer Digest ****/

    sha512_begin (&octx);

    for (i = 0; i < lk; ++i)
        buf[i] = (char) (k[i] ^ 0x5C);
    for (i = lk; i < SHA512_BLOCKSIZE; ++i)
        buf[i] = 0x5C;

    sha512_hash ((unsigned char *) buf, SHA512_BLOCKSIZE, &octx);
    sha512_hash ((unsigned char *) isha, SHA512_DIGESTSIZE, &octx);

    sha512_end ((unsigned char *) osha, &octx);

    /* truncate and print the results */
    t = t > SHA512_DIGESTSIZE ? SHA512_DIGESTSIZE : t;
    hmac_truncate (osha, out, t);

    /* Prevent leaks */
    burn (&ictx, sizeof(ictx));
    burn (&octx, sizeof(octx));
    burn (isha, sizeof(isha));
    burn (osha, sizeof(osha));
    burn (buf, sizeof(buf));
    burn (key, sizeof(key));
}

__device__ void derive_u_sha512 (char *pwd, int pwd_len, char *salt, int salt_len, int iterations, char *u, int b)
{
    char j[SHA512_DIGESTSIZE], k[SHA512_DIGESTSIZE];
    char init[128];
    char counter[4];
    int c, i;

    /* iteration 1 */
    memset (counter, 0, 4);
    counter[3] = (char) b;
    memcpy (init, salt, salt_len);	/* salt */
    memcpy (&init[salt_len], counter, 4);	/* big-endian block number */

    /* remaining iterations */
    for (c = 0; c < iterations; c++)
    {
		hmac_sha512 (pwd, pwd_len, !c ? init : j, !c ? salt_len + 4 : SHA512_DIGESTSIZE, !c ? j : k, SHA512_DIGESTSIZE);
		if (!c)
			memcpy (u, j, SHA512_DIGESTSIZE);
		else
        for (i = 0; i < SHA512_DIGESTSIZE; i++)
        {
            u[i] ^= k[i];
            j[i] = k[i];
        }
    }

    /* Prevent possible leaks. */
    burn (j, sizeof(j));
    burn (k, sizeof(k));
}

__device__ void derive_key_sha512 (char *pwd, int pwd_len, char *salt, int salt_len, int iterations, char *dk, int dklen)
{
    char u[SHA512_DIGESTSIZE];
    int b, l, r;

    if (dklen % SHA512_DIGESTSIZE)
    {
        l = 1 + dklen / SHA512_DIGESTSIZE;
    }
    else
    {
        l = dklen / SHA512_DIGESTSIZE;
    }

    r = dklen - (l - 1) * SHA512_DIGESTSIZE;

    /* first l - 1 blocks */
    for (b = 1; b <= l; b++)
    {
        derive_u_sha512 (pwd, pwd_len, salt, salt_len, iterations, u, b);
		if (b < l)
		{
			memcpy (dk, u, SHA512_DIGESTSIZE);
			dk += SHA512_DIGESTSIZE;
		}
    }

    /* last block */
    memcpy (dk, u, r);


    /* Prevent possible leaks. */
    burn (u, sizeof(u));
}


__device__ void sha1_begin(sha1_ctx ctx[1])
{
    ctx->count[0] = ctx->count[1] = 0;
    ctx->hash[0] = 0x67452301;
    ctx->hash[1] = 0xefcdab89;
    ctx->hash[2] = 0x98badcfe;
    ctx->hash[3] = 0x10325476;
    ctx->hash[4] = 0xc3d2e1f0;
}

#define q(v,n)  v##n

#define one_cycle(v,a,b,c,d,e,f,k,h)            \
    q(v,e) += rotr32(q(v,a),27) +               \
              f(q(v,b),q(v,c),q(v,d)) + k + h;  \
    q(v,b)  = rotr32(q(v,b), 2)

#define five_cycle(v,f,k,i)                 \
    one_cycle(v, 0,1,2,3,4, f,k,hf(i  ));   \
    one_cycle(v, 4,0,1,2,3, f,k,hf(i+1));   \
    one_cycle(v, 3,4,0,1,2, f,k,hf(i+2));   \
    one_cycle(v, 2,3,4,0,1, f,k,hf(i+3));   \
    one_cycle(v, 1,2,3,4,0, f,k,hf(i+4))

__device__ void sha1_compile(sha1_ctx ctx[1])
{   sha1_32t    *w = ctx->wbuf;

    sha1_32t    v0, v1, v2, v3, v4;
    v0 = ctx->hash[0]; v1 = ctx->hash[1];
    v2 = ctx->hash[2]; v3 = ctx->hash[3];
    v4 = ctx->hash[4];

#undef  hf
#define hf(i)   w[i]

    five_cycle(v, ch, 0x5a827999,  0);
    five_cycle(v, ch, 0x5a827999,  5);
    five_cycle(v, ch, 0x5a827999, 10);
    one_cycle(v,0,1,2,3,4, ch, 0x5a827999, hf(15)); \

#undef  hf
#define hf(i) (w[(i) & 15] = rotl32(                    \
                 w[((i) + 13) & 15] ^ w[((i) + 8) & 15] \
               ^ w[((i) +  2) & 15] ^ w[(i) & 15], 1))

    one_cycle(v,4,0,1,2,3, ch, 0x5a827999, hf(16));
    one_cycle(v,3,4,0,1,2, ch, 0x5a827999, hf(17));
    one_cycle(v,2,3,4,0,1, ch, 0x5a827999, hf(18));
    one_cycle(v,1,2,3,4,0, ch, 0x5a827999, hf(19));

    five_cycle(v, parity, 0x6ed9eba1,  20);
    five_cycle(v, parity, 0x6ed9eba1,  25);
    five_cycle(v, parity, 0x6ed9eba1,  30);
    five_cycle(v, parity, 0x6ed9eba1,  35);

    five_cycle(v, maj, 0x8f1bbcdc,  40);
    five_cycle(v, maj, 0x8f1bbcdc,  45);
    five_cycle(v, maj, 0x8f1bbcdc,  50);
    five_cycle(v, maj, 0x8f1bbcdc,  55);

    five_cycle(v, parity, 0xca62c1d6,  60);
    five_cycle(v, parity, 0xca62c1d6,  65);
    five_cycle(v, parity, 0xca62c1d6,  70);
    five_cycle(v, parity, 0xca62c1d6,  75);

    ctx->hash[0] += v0; ctx->hash[1] += v1;
    ctx->hash[2] += v2; ctx->hash[3] += v3;
    ctx->hash[4] += v4;
}

/* SHA1 hash data in an array of bytes into hash buffer and */
/* call the hash_compile function as required.              */

__device__ void sha1_hash(const unsigned char data[], u32 len, sha1_ctx ctx[1])
{   sha1_32t pos = (sha1_32t)(ctx->count[0] & SHA1_MASK),
            space = SHA1_BLOCK_SIZE - pos;
    const unsigned char *sp = data;

    if((ctx->count[0] += len) < len)
        ++(ctx->count[1]);

    while(len >= space)     /* tranfer whole blocks if possible  */
    {
        memcpy(((unsigned char*)ctx->wbuf) + pos, sp, space);
        sp += space; len -= space; space = SHA1_BLOCK_SIZE; pos = 0;
        bsw_32(ctx->wbuf, SHA1_BLOCK_SIZE >> 2);
        sha1_compile(ctx);
    }

    memcpy(((unsigned char*)ctx->wbuf) + pos, sp, len);
}

/* SHA1 final padding and digest calculation  */

__device__ void sha1_end(unsigned char hval[], sha1_ctx ctx[1])
{   sha1_32t    i = (sha1_32t)(ctx->count[0] & SHA1_MASK);

    /* put bytes in the buffer in an order in which references to   */
    /* 32-bit words will put bytes with lower addresses into the    */
    /* top of 32 bit words on BOTH big and little endian machines   */
    bsw_32(ctx->wbuf, (i + 3) >> 2);

    /* we now need to mask valid bytes and add the padding which is */
    /* a single 1 bit and as many zero bits as necessary. Note that */
    /* we can always add the first padding byte here because the    */
    /* buffer always has at least one empty slot                    */
    ctx->wbuf[i >> 2] &= 0xffffff80 << 8 * (~i & 3);
    ctx->wbuf[i >> 2] |= 0x00000080 << 8 * (~i & 3);

    /* we need 9 or more empty positions, one for the padding byte  */
    /* (above) and eight for the length count. If there is not      */
    /* enough space, pad and empty the buffer                       */
    if(i > SHA1_BLOCK_SIZE - 9)
    {
        if(i < 60) ctx->wbuf[15] = 0;
        sha1_compile(ctx);
        i = 0;
    }
    else    /* compute a word index for the empty buffer positions  */
        i = (i >> 2) + 1;

    while(i < 14) /* and zero pad all but last two positions        */
        ctx->wbuf[i++] = 0;

    /* the following 32-bit length fields are assembled in the      */
    /* wrong byte order on little endian machines but this is       */
    /* corrected later since they are only ever used as 32-bit      */
    /* word values.                                                 */
    ctx->wbuf[14] = (ctx->count[1] << 3) | (ctx->count[0] >> 29);
    ctx->wbuf[15] = ctx->count[0] << 3;
    sha1_compile(ctx);

    /* extract the hash value as bytes in case the hash buffer is   */
    /* misaligned for 32-bit words                                  */
    for(i = 0; i < SHA1_DIGEST_SIZE; ++i)
        hval[i] = (unsigned char)(ctx->hash[i >> 2] >> (8 * (~i & 3)));
}

/* Deprecated/legacy */
__device__ void hmac_sha1
(
      char *k,		/* secret key */
      int lk,		/* length of the key in bytes */
      char *d,		/* data */
      int ld,		/* length of data in bytes */
      char *out,		/* output buffer, at least "t" bytes */
      int t
)
{
    sha1_ctx ictx, octx;
    char isha[SHA1_DIGESTSIZE], osha[SHA1_DIGESTSIZE];
    char key[SHA1_DIGESTSIZE];
    char buf[SHA1_BLOCKSIZE];
    int i;

    /* If the key is longer than the hash algorithm block size,
       let key = sha1(key), as per HMAC specifications. */
    if (lk > SHA1_BLOCKSIZE)
    {
        sha1_ctx tctx;

        sha1_begin (&tctx);
        sha1_hash ((unsigned char *) k, lk, &tctx);
        sha1_end ((unsigned char *) key, &tctx);

        k = key;
        lk = SHA1_DIGESTSIZE;

        burn (&tctx, sizeof(tctx));		// Prevent leaks
    }

    /**** Inner Digest ****/

    sha1_begin (&ictx);

    /* Pad the key for inner digest */
    for (i = 0; i < lk; ++i)
        buf[i] = (char) (k[i] ^ 0x36);
    for (i = lk; i < SHA1_BLOCKSIZE; ++i)
        buf[i] = 0x36;

    sha1_hash ((unsigned char *) buf, SHA1_BLOCKSIZE, &ictx);
    sha1_hash ((unsigned char *) d, ld, &ictx);

    sha1_end ((unsigned char *) isha, &ictx);

    /**** Outer Digest ****/

    sha1_begin (&octx);

    for (i = 0; i < lk; ++i)
        buf[i] = (char) (k[i] ^ 0x5C);
    for (i = lk; i < SHA1_BLOCKSIZE; ++i)
        buf[i] = 0x5C;

    sha1_hash ((unsigned char *) buf, SHA1_BLOCKSIZE, &octx);
    sha1_hash ((unsigned char *) isha, SHA1_DIGESTSIZE, &octx);

    sha1_end ((unsigned char *) osha, &octx);

    /* truncate and print the results */
    t = t > SHA1_DIGESTSIZE ? SHA1_DIGESTSIZE : t;
    hmac_truncate (osha, out, t);

    /* Prevent leaks */
    burn (&ictx, sizeof(ictx));
    burn (&octx, sizeof(octx));
    burn (isha, sizeof(isha));
    burn (osha, sizeof(osha));
    burn (buf, sizeof(buf));
    burn (key, sizeof(key));
}

/* Deprecated/legacy */
__device__ void derive_u_sha1 (char *pwd, int pwd_len, char *salt, int salt_len, int iterations, char *u, int b)
{
    char j[SHA1_DIGESTSIZE], k[SHA1_DIGESTSIZE];
    char init[128];
    char counter[4];
    int c, i;

    /* iteration 1 */
    memset (counter, 0, 4);
    counter[3] = (char) b;
    memcpy (init, salt, salt_len);	/* salt */
    memcpy (&init[salt_len], counter, 4);	/* big-endian block number */

    /* remaining iterations */
    for (c = 0; c < iterations; c++)
    {
		hmac_sha1 (pwd, pwd_len, !c ? init : j, !c ? salt_len + 4 : SHA1_DIGESTSIZE, !c ? j : k, SHA1_DIGESTSIZE);
		if (!c)
			memcpy (u, j, SHA1_DIGESTSIZE);
		else
        for (i = 0; i < SHA1_DIGESTSIZE; i++)
        {
            u[i] ^= k[i];
            j[i] = k[i];
        }
    }

    /* Prevent possible leaks. */
    burn (j, sizeof(j));
    burn (k, sizeof(k));
}

/* Deprecated/legacy */
__device__ void derive_key_sha1 (char *pwd, int pwd_len, char *salt, int salt_len, int iterations, char *dk, int dklen)
{
    char u[SHA1_DIGESTSIZE];
    int b, l, r;

    if (dklen % SHA1_DIGESTSIZE)
    {
        l = 1 + dklen / SHA1_DIGESTSIZE;
    }
    else
    {
        l = dklen / SHA1_DIGESTSIZE;
    }

    r = dklen - (l - 1) * SHA1_DIGESTSIZE;

    /* first l - 1 blocks */
    for (b = 1; b <= l; b++)
    {
        derive_u_sha1 (pwd, pwd_len, salt, salt_len, iterations, u, b);
		if (b < l)
		{
			memcpy (dk, u, SHA1_DIGESTSIZE);
			dk += SHA1_DIGESTSIZE;
		}
    }

    /* last block */
    memcpy (dk, u, r);


    /* Prevent possible leaks. */
    burn (u, sizeof(u));
}


__device__ void WHIRLPOOL_init(struct NESSIEstruct * const structpointer) {
    int i;

    memset(structpointer->bitLength, 0, 32);
    structpointer->bufferBits = structpointer->bufferPos = 0;
    structpointer->buffer[0] = 0; /* it's only necessary to cleanup buffer[bufferPos] */
    for (i = 0; i < 8; i++) {
        structpointer->hash[i] = 0L; /* initial value */
    }
}

/**
 * The core Whirlpool transform.
 */
__device__ void processBuffer(struct NESSIEstruct * const structpointer, PTHREAD_DATA td) {
    int i, r;

#define K td->K
#define block td->block
#define state td->state
#define L td->L
#define buffer td->buffer

    /*
     * map the buffer to a block:
     */
    for (i = 0; i < 8; i++, buffer += 8) {
        block[i] =
            (((u64)buffer[0]        ) << 56) ^
            (((u64)buffer[1] & 0xffL) << 48) ^
            (((u64)buffer[2] & 0xffL) << 40) ^
            (((u64)buffer[3] & 0xffL) << 32) ^
            (((u64)buffer[4] & 0xffL) << 24) ^
            (((u64)buffer[5] & 0xffL) << 16) ^
            (((u64)buffer[6] & 0xffL) <<  8) ^
            (((u64)buffer[7] & 0xffL)      );
    }
    /*
     * compute and apply K^0 to the cipher state:
     */
    state[0] = block[0] ^ (K[0] = structpointer->hash[0]);
    state[1] = block[1] ^ (K[1] = structpointer->hash[1]);
    state[2] = block[2] ^ (K[2] = structpointer->hash[2]);
    state[3] = block[3] ^ (K[3] = structpointer->hash[3]);
    state[4] = block[4] ^ (K[4] = structpointer->hash[4]);
    state[5] = block[5] ^ (K[5] = structpointer->hash[5]);
    state[6] = block[6] ^ (K[6] = structpointer->hash[6]);
    state[7] = block[7] ^ (K[7] = structpointer->hash[7]);
    /*
     * iterate over all rounds:
     */
    for (r = 1; r <= R; r++) {
        /*
         * compute K^r from K^{r-1}:
         */
        L[0] =
            C0[(int)(K[0] >> 56)       ] ^
            C1[(int)(K[7] >> 48) & 0xff] ^
            C2[(int)(K[6] >> 40) & 0xff] ^
            C3[(int)(K[5] >> 32) & 0xff] ^
            C4[(int)(K[4] >> 24) & 0xff] ^
            C5[(int)(K[3] >> 16) & 0xff] ^
            C6[(int)(K[2] >>  8) & 0xff] ^
            C7[(int)(K[1]      ) & 0xff] ^
            rc[r];
        L[1] =
            C0[(int)(K[1] >> 56)       ] ^
            C1[(int)(K[0] >> 48) & 0xff] ^
            C2[(int)(K[7] >> 40) & 0xff] ^
            C3[(int)(K[6] >> 32) & 0xff] ^
            C4[(int)(K[5] >> 24) & 0xff] ^
            C5[(int)(K[4] >> 16) & 0xff] ^
            C6[(int)(K[3] >>  8) & 0xff] ^
            C7[(int)(K[2]      ) & 0xff];
        L[2] =
            C0[(int)(K[2] >> 56)       ] ^
            C1[(int)(K[1] >> 48) & 0xff] ^
            C2[(int)(K[0] >> 40) & 0xff] ^
            C3[(int)(K[7] >> 32) & 0xff] ^
            C4[(int)(K[6] >> 24) & 0xff] ^
            C5[(int)(K[5] >> 16) & 0xff] ^
            C6[(int)(K[4] >>  8) & 0xff] ^
            C7[(int)(K[3]      ) & 0xff];
        L[3] =
            C0[(int)(K[3] >> 56)       ] ^
            C1[(int)(K[2] >> 48) & 0xff] ^
            C2[(int)(K[1] >> 40) & 0xff] ^
            C3[(int)(K[0] >> 32) & 0xff] ^
            C4[(int)(K[7] >> 24) & 0xff] ^
            C5[(int)(K[6] >> 16) & 0xff] ^
            C6[(int)(K[5] >>  8) & 0xff] ^
            C7[(int)(K[4]      ) & 0xff];
        L[4] =
            C0[(int)(K[4] >> 56)       ] ^
            C1[(int)(K[3] >> 48) & 0xff] ^
            C2[(int)(K[2] >> 40) & 0xff] ^
            C3[(int)(K[1] >> 32) & 0xff] ^
            C4[(int)(K[0] >> 24) & 0xff] ^
            C5[(int)(K[7] >> 16) & 0xff] ^
            C6[(int)(K[6] >>  8) & 0xff] ^
            C7[(int)(K[5]      ) & 0xff];
        L[5] =
            C0[(int)(K[5] >> 56)       ] ^
            C1[(int)(K[4] >> 48) & 0xff] ^
            C2[(int)(K[3] >> 40) & 0xff] ^
            C3[(int)(K[2] >> 32) & 0xff] ^
            C4[(int)(K[1] >> 24) & 0xff] ^
            C5[(int)(K[0] >> 16) & 0xff] ^
            C6[(int)(K[7] >>  8) & 0xff] ^
            C7[(int)(K[6]      ) & 0xff];
        L[6] =
            C0[(int)(K[6] >> 56)       ] ^
            C1[(int)(K[5] >> 48) & 0xff] ^
            C2[(int)(K[4] >> 40) & 0xff] ^
            C3[(int)(K[3] >> 32) & 0xff] ^
            C4[(int)(K[2] >> 24) & 0xff] ^
            C5[(int)(K[1] >> 16) & 0xff] ^
            C6[(int)(K[0] >>  8) & 0xff] ^
            C7[(int)(K[7]      ) & 0xff];
        L[7] =
            C0[(int)(K[7] >> 56)       ] ^
            C1[(int)(K[6] >> 48) & 0xff] ^
            C2[(int)(K[5] >> 40) & 0xff] ^
            C3[(int)(K[4] >> 32) & 0xff] ^
            C4[(int)(K[3] >> 24) & 0xff] ^
            C5[(int)(K[2] >> 16) & 0xff] ^
            C6[(int)(K[1] >>  8) & 0xff] ^
            C7[(int)(K[0]      ) & 0xff];
        K[0] = L[0];
        K[1] = L[1];
        K[2] = L[2];
        K[3] = L[3];
        K[4] = L[4];
        K[5] = L[5];
        K[6] = L[6];
        K[7] = L[7];
        /*
         * apply the r-th round transformation:
         */
        L[0] =
            C0[(int)(state[0] >> 56)       ] ^
            C1[(int)(state[7] >> 48) & 0xff] ^
            C2[(int)(state[6] >> 40) & 0xff] ^
            C3[(int)(state[5] >> 32) & 0xff] ^
            C4[(int)(state[4] >> 24) & 0xff] ^
            C5[(int)(state[3] >> 16) & 0xff] ^
            C6[(int)(state[2] >>  8) & 0xff] ^
            C7[(int)(state[1]      ) & 0xff] ^
            K[0];
        L[1] =
            C0[(int)(state[1] >> 56)       ] ^
            C1[(int)(state[0] >> 48) & 0xff] ^
            C2[(int)(state[7] >> 40) & 0xff] ^
            C3[(int)(state[6] >> 32) & 0xff] ^
            C4[(int)(state[5] >> 24) & 0xff] ^
            C5[(int)(state[4] >> 16) & 0xff] ^
            C6[(int)(state[3] >>  8) & 0xff] ^
            C7[(int)(state[2]      ) & 0xff] ^
            K[1];
        L[2] =
            C0[(int)(state[2] >> 56)       ] ^
            C1[(int)(state[1] >> 48) & 0xff] ^
            C2[(int)(state[0] >> 40) & 0xff] ^
            C3[(int)(state[7] >> 32) & 0xff] ^
            C4[(int)(state[6] >> 24) & 0xff] ^
            C5[(int)(state[5] >> 16) & 0xff] ^
            C6[(int)(state[4] >>  8) & 0xff] ^
            C7[(int)(state[3]      ) & 0xff] ^
            K[2];
        L[3] =
            C0[(int)(state[3] >> 56)       ] ^
            C1[(int)(state[2] >> 48) & 0xff] ^
            C2[(int)(state[1] >> 40) & 0xff] ^
            C3[(int)(state[0] >> 32) & 0xff] ^
            C4[(int)(state[7] >> 24) & 0xff] ^
            C5[(int)(state[6] >> 16) & 0xff] ^
            C6[(int)(state[5] >>  8) & 0xff] ^
            C7[(int)(state[4]      ) & 0xff] ^
            K[3];
        L[4] =
            C0[(int)(state[4] >> 56)       ] ^
            C1[(int)(state[3] >> 48) & 0xff] ^
            C2[(int)(state[2] >> 40) & 0xff] ^
            C3[(int)(state[1] >> 32) & 0xff] ^
            C4[(int)(state[0] >> 24) & 0xff] ^
            C5[(int)(state[7] >> 16) & 0xff] ^
            C6[(int)(state[6] >>  8) & 0xff] ^
            C7[(int)(state[5]      ) & 0xff] ^
            K[4];
        L[5] =
            C0[(int)(state[5] >> 56)       ] ^
            C1[(int)(state[4] >> 48) & 0xff] ^
            C2[(int)(state[3] >> 40) & 0xff] ^
            C3[(int)(state[2] >> 32) & 0xff] ^
            C4[(int)(state[1] >> 24) & 0xff] ^
            C5[(int)(state[0] >> 16) & 0xff] ^
            C6[(int)(state[7] >>  8) & 0xff] ^
            C7[(int)(state[6]      ) & 0xff] ^
            K[5];
        L[6] =
            C0[(int)(state[6] >> 56)       ] ^
            C1[(int)(state[5] >> 48) & 0xff] ^
            C2[(int)(state[4] >> 40) & 0xff] ^
            C3[(int)(state[3] >> 32) & 0xff] ^
            C4[(int)(state[2] >> 24) & 0xff] ^
            C5[(int)(state[1] >> 16) & 0xff] ^
            C6[(int)(state[0] >>  8) & 0xff] ^
            C7[(int)(state[7]      ) & 0xff] ^
            K[6];
        L[7] =
            C0[(int)(state[7] >> 56)       ] ^
            C1[(int)(state[6] >> 48) & 0xff] ^
            C2[(int)(state[5] >> 40) & 0xff] ^
            C3[(int)(state[4] >> 32) & 0xff] ^
            C4[(int)(state[3] >> 24) & 0xff] ^
            C5[(int)(state[2] >> 16) & 0xff] ^
            C6[(int)(state[1] >>  8) & 0xff] ^
            C7[(int)(state[0]      ) & 0xff] ^
            K[7];
        state[0] = L[0];
        state[1] = L[1];
        state[2] = L[2];
        state[3] = L[3];
        state[4] = L[4];
        state[5] = L[5];
        state[6] = L[6];
        state[7] = L[7];
    }
    /*
     * apply the Miyaguchi-Preneel compression function:
     */
    structpointer->hash[0] ^= state[0] ^ block[0];
    structpointer->hash[1] ^= state[1] ^ block[1];
    structpointer->hash[2] ^= state[2] ^ block[2];
    structpointer->hash[3] ^= state[3] ^ block[3];
    structpointer->hash[4] ^= state[4] ^ block[4];
    structpointer->hash[5] ^= state[5] ^ block[5];
    structpointer->hash[6] ^= state[6] ^ block[6];
    structpointer->hash[7] ^= state[7] ^ block[7];
}
#undef buffer
#undef K
#undef block
#undef state
#undef L

/**
 * Delivers input data to the hashing algorithm.
 *
 * @param    source        plaintext data to hash.
 * @param    sourceBits    how many bits of plaintext to process.
 *
 * This method maintains the invariant: bufferBits < DIGESTBITS
 */
__device__ void WHIRLPOOL_add(const unsigned char * const source,
               u32 sourceBits,
               struct NESSIEstruct * const structpointer, PTHREAD_DATA td) {
    /*
                       sourcePos
                       |
                       +-------+-------+-------
                          ||||||||||||||||||||| source
                       +-------+-------+-------
    +-------+-------+-------+-------+-------+-------
    ||||||||||||||||||||||                           buffer
    +-------+-------+-------+-------+-------+-------
                    |
                    bufferPos
    */
    int sourcePos    = 0; /* index of leftmost source u8 containing data (1 to 8 bits). */
    int sourceGap    = (8 - ((int)sourceBits & 7)) & 7; /* space on source[sourcePos]. */
    int bufferRem    = structpointer->bufferBits & 7; /* occupied bits on buffer[bufferPos]. */
    int i;
    u32 b, carry;
    u8 *buffer       = structpointer->buffer;
    u8 *bitLength    = structpointer->bitLength;
    int bufferBits   = structpointer->bufferBits;
    int bufferPos    = structpointer->bufferPos;

    /*
     * tally the length of the added data:
     */
    u64 value = sourceBits;
    for (i = 31, carry = 0; i >= 0 && (carry != 0 || value != LL(0)); i--) {
        carry += bitLength[i] + ((u32)value & 0xff);
        bitLength[i] = (u8)carry;
        carry >>= 8;
        value >>= 8;
    }
    /*
     * process data in chunks of 8 bits (a more efficient approach would be to take whole-word chunks):
     */
    while (sourceBits > 8) {
        /* N.B. at least source[sourcePos] and source[sourcePos+1] contain data. */
        /*
         * take a byte from the source:
         */
        b = ((source[sourcePos] << sourceGap) & 0xff) |
            ((source[sourcePos + 1] & 0xff) >> (8 - sourceGap));
        /*
         * process this byte:
         */
        buffer[bufferPos++] |= (u8)(b >> bufferRem);
        bufferBits += 8 - bufferRem; /* bufferBits = 8*bufferPos; */
        if (bufferBits == DIGESTBITS) {
            /*
             * process data block:
             */
            processBuffer(structpointer, td);
            /*
             * reset buffer:
             */
            bufferBits = bufferPos = 0;
        }
        buffer[bufferPos] = (u8) (b << (8 - bufferRem));
        bufferBits += bufferRem;
        /*
         * proceed to remaining data:
         */
        sourceBits -= 8;
        sourcePos++;
    }
    /* now 0 <= sourceBits <= 8;
     * furthermore, all data (if any is left) is in source[sourcePos].
     */
    if (sourceBits > 0) {
        b = (source[sourcePos] << sourceGap) & 0xff; /* bits are left-justified on b. */
        /*
         * process the remaining bits:
         */
        buffer[bufferPos] |= b >> bufferRem;
    } else {
        b = 0;
    }
    if (bufferRem + sourceBits < 8) {
        /*
         * all remaining data fits on buffer[bufferPos],
         * and there still remains some space.
         */
        bufferBits += sourceBits;
    } else {
        /*
         * buffer[bufferPos] is full:
         */
        bufferPos++;
        bufferBits += 8 - bufferRem; /* bufferBits = 8*bufferPos; */
        sourceBits -= 8 - bufferRem;
        /* now 0 <= sourceBits < 8;
         * furthermore, all data (if any is left) is in source[sourcePos].
         */
        if (bufferBits == DIGESTBITS) {
            /*
             * process data block:
             */
            processBuffer(structpointer, td);
            /*
             * reset buffer:
             */
            bufferBits = bufferPos = 0;
        }
        buffer[bufferPos] = (u8) (b << (8 - bufferRem));
        bufferBits += (int)sourceBits;
    }
    structpointer->bufferBits   = bufferBits;
    structpointer->bufferPos    = bufferPos;
}

/**
 * Get the hash value from the hashing state.
 * 
 * This method uses the invariant: bufferBits < DIGESTBITS
 */
__device__ void WHIRLPOOL_finalize(struct NESSIEstruct * const structpointer,
                    unsigned char * const result, PTHREAD_DATA td) {
    int i;
    u8 *buffer      = structpointer->buffer;
    u8 *bitLength   = structpointer->bitLength;
    int bufferBits  = structpointer->bufferBits;
    int bufferPos   = structpointer->bufferPos;
    u8 *digest      = result;

    /*
     * append a '1'-bit:
     */
    buffer[bufferPos] |= 0x80U >> (bufferBits & 7);
    bufferPos++; /* all remaining bits on the current u8 are set to zero. */
    /*
     * pad with zero bits to complete (N*WBLOCKBITS - LENGTHBITS) bits:
     */
    if (bufferPos > WBLOCKBYTES - LENGTHBYTES) {
        if (bufferPos < WBLOCKBYTES) {
            memset(&buffer[bufferPos], 0, WBLOCKBYTES - bufferPos);
        }
        /*
         * process data block:
         */
        processBuffer(structpointer, td);
        /*
         * reset buffer:
         */
        bufferPos = 0;
    }
    if (bufferPos < WBLOCKBYTES - LENGTHBYTES) {
        memset(&buffer[bufferPos], 0, (WBLOCKBYTES - LENGTHBYTES) - bufferPos);
    }
    bufferPos = WBLOCKBYTES - LENGTHBYTES;
    /*
     * append bit length of hashed data:
     */
    memcpy(&buffer[WBLOCKBYTES - LENGTHBYTES], bitLength, LENGTHBYTES);
    /*
     * process data block:
     */
    processBuffer(structpointer, td);
    /*
     * return the completed message digest:
     */
    for (i = 0; i < DIGESTBYTES/8; i++) {
        digest[0] = (u8)(structpointer->hash[i] >> 56);
        digest[1] = (u8)(structpointer->hash[i] >> 48);
        digest[2] = (u8)(structpointer->hash[i] >> 40);
        digest[3] = (u8)(structpointer->hash[i] >> 32);
        digest[4] = (u8)(structpointer->hash[i] >> 24);
        digest[5] = (u8)(structpointer->hash[i] >> 16);
        digest[6] = (u8)(structpointer->hash[i] >>  8);
        digest[7] = (u8)(structpointer->hash[i]      );
        digest += 8;
    }
    structpointer->bufferBits   = bufferBits;
    structpointer->bufferPos    = bufferPos;
}

__device__ void hmac_whirlpool
(
      char *k,		/* secret key */
      int lk,		/* length of the key in bytes */
      char *d,		/* data */
      int ld,		/* length of data in bytes */
      char *out,	/* output buffer, at least "t" bytes */
      int t, PTHREAD_DATA td
)
{
#define ictx td->ictx
#define octx td->octx
#define iwhi td->iwhi
#define owhi td->owhi
#define key td->key
#define buf td->buf
#define tctx td->tctx

    int i;

    /* If the key is longer than the hash algorithm block size,
       let key = whirlpool(key), as per HMAC specifications. */
    if (lk > WHIRLPOOL_BLOCKSIZE)
    {
        WHIRLPOOL_init (&tctx);
        WHIRLPOOL_add ((unsigned char *) k, lk * 8, &tctx, td);
        WHIRLPOOL_finalize (&tctx, (unsigned char *) key, td);

        k = key;
        lk = WHIRLPOOL_DIGESTSIZE;

        burn (&tctx, sizeof(tctx));		// Prevent leaks
    }

    /**** Inner Digest ****/

    WHIRLPOOL_init (&ictx);

    /* Pad the key for inner digest */
    for (i = 0; i < lk; ++i)
        buf[i] = (char) (k[i] ^ 0x36);
    for (i = lk; i < WHIRLPOOL_BLOCKSIZE; ++i)
        buf[i] = 0x36;

    WHIRLPOOL_add ((unsigned char *) buf, WHIRLPOOL_BLOCKSIZE * 8, &ictx, td);
    WHIRLPOOL_add ((unsigned char *) d, ld * 8, &ictx, td);

    WHIRLPOOL_finalize (&ictx, (unsigned char *) iwhi, td);

    /**** Outer Digest ****/

    WHIRLPOOL_init (&octx);

    for (i = 0; i < lk; ++i)
        buf[i] = (char) (k[i] ^ 0x5C);
    for (i = lk; i < WHIRLPOOL_BLOCKSIZE; ++i)
        buf[i] = 0x5C;

    WHIRLPOOL_add ((unsigned char *) buf, WHIRLPOOL_BLOCKSIZE * 8, &octx, td);
    WHIRLPOOL_add ((unsigned char *) iwhi, WHIRLPOOL_DIGESTSIZE * 8, &octx, td);

    WHIRLPOOL_finalize (&octx, (unsigned char *) owhi, td);

    /* truncate and print the results */
    t = t > WHIRLPOOL_DIGESTSIZE ? WHIRLPOOL_DIGESTSIZE : t;
    hmac_truncate (owhi, out, t);

    /* Prevent possible leaks. */
    burn (&ictx, sizeof(ictx));
    burn (&octx, sizeof(octx));
    burn (owhi, sizeof(owhi));
    burn (iwhi, sizeof(iwhi));
    burn (buf, sizeof(buf));
    burn (key, sizeof(key));
}
#undef ictx
#undef octx
#undef tctx
#undef iwhi
#undef owhi
#undef key
#undef buf

__device__ void derive_u_whirlpool (char *pwd, int pwd_len, char *salt, int salt_len, int iterations, char *u, int b, PTHREAD_DATA td)
{
#define j td->j
#define k td->k
#define init td->init

    char counter[4];
    int c, i;

    /* iteration 1 */
    memset (counter, 0, 4);
    counter[3] = (char) b;
    memcpy (init, salt, salt_len);	/* salt */
    memcpy (&init[salt_len], counter, 4);	/* big-endian block number */

    /* remaining iterations */
    for (c = 0; c < iterations; c++)
    {
		hmac_whirlpool (pwd, pwd_len, !c ? init : j, !c ? salt_len + 4 : WHIRLPOOL_DIGESTSIZE, !c ? j : k, WHIRLPOOL_DIGESTSIZE, td);
		if (!c)
			memcpy (u, j, WHIRLPOOL_DIGESTSIZE);
		else
        for (i = 0; i < WHIRLPOOL_DIGESTSIZE; i++)
        {
            u[i] ^= k[i];
            j[i] = k[i];
        }
    }

    /* Prevent possible leaks. */
    burn (j, sizeof(j));
    burn (k, sizeof(k));
}
#undef j
#undef k
#undef init

__device__ void derive_key_whirlpool (char *pwd, int pwd_len, char *salt, int salt_len, int iterations, char *dk, int dklen, PTHREAD_DATA td)
{
#define u td->u

    int b, l, r;

    if (dklen % WHIRLPOOL_DIGESTSIZE)
    {
        l = 1 + dklen / WHIRLPOOL_DIGESTSIZE;
    }
    else
    {
        l = dklen / WHIRLPOOL_DIGESTSIZE;
    }

    r = dklen - (l - 1) * WHIRLPOOL_DIGESTSIZE;

    /* first l - 1 blocks */
    for (b = 1; b <= l; b++)
    {
        derive_u_whirlpool (pwd, pwd_len, salt, salt_len, iterations, u, b, td);
        if (b < l)
        {
            memcpy (dk, u, WHIRLPOOL_DIGESTSIZE);
            dk += WHIRLPOOL_DIGESTSIZE;
        }
    }

    /* last block */
    memcpy (dk, u, r);


    /* Prevent possible leaks. */
    burn (u, sizeof(u));
}
#undef u

__device__ int CipherGetBlockSize (int cipherId)
{
    return CipherGet (cipherId) -> BlockSize;
}

__device__ int CipherGetKeyScheduleSize (int cipherId)
{
    return CipherGet (cipherId) -> KeyScheduleSize;
}

// Returns sum of key schedule sizes of all ciphers of the EA
__device__ int EAGetKeyScheduleSize (int ea)
{
    int i = EAGetFirstCipher(ea);
    int size = CipherGetKeyScheduleSize (i);

    while (i = EAGetNextCipher(ea, i))
    {
        size += CipherGetKeyScheduleSize (i);
    }

    return size;
}

#define vf1(x,r,c)  (x)
#define rf1(r,c)    (r)
#define rf2(r,c)    ((8+r-c)&3)

#define bval(x,n)       ((uint_8t)((x) >> (8 * (n))))

#define four_tables(x,tab,vf,rf,c) \
 (  tab[0][bval(vf(x,0,c),rf(0,c))] \
  ^ tab[1][bval(vf(x,1,c),rf(1,c))] \
  ^ tab[2][bval(vf(x,2,c),rf(2,c))] \
  ^ tab[3][bval(vf(x,3,c),rf(3,c))])

#define t_use(m,n) t_##m##n

#define ls_box(x,c)       four_tables(x,t_use(f,l),vf1,rf2,c)

#define kef8(k,i) \
{   k[8*(i)+ 8] = ss[0] ^= ls_box(ss[7],3) ^ t_use(r,c)[i]; \
    k[8*(i)+ 9] = ss[1] ^= ss[0]; \
    k[8*(i)+10] = ss[2] ^= ss[1]; \
    k[8*(i)+11] = ss[3] ^= ss[2]; \
}

#define ke8(k,i) \
{   kef8(k,i); \
    k[8*(i)+12] = ss[4] ^= ls_box(ss[3],0); \
    k[8*(i)+13] = ss[5] ^= ss[4]; \
    k[8*(i)+14] = ss[6] ^= ss[5]; \
    k[8*(i)+15] = ss[7] ^= ss[6]; \
}

#define word_in(x,c)    (*((uint_32t*)(x)+(c)))

__device__ AES_RETURN aes_encrypt_key256(const unsigned char *key, aes_encrypt_ctx cx[1])
{   uint_32t    ss[8];

    cx->ks[0] = ss[0] = word_in(key, 0);
    cx->ks[1] = ss[1] = word_in(key, 1);
    cx->ks[2] = ss[2] = word_in(key, 2);
    cx->ks[3] = ss[3] = word_in(key, 3);
    cx->ks[4] = ss[4] = word_in(key, 4);
    cx->ks[5] = ss[5] = word_in(key, 5);
    cx->ks[6] = ss[6] = word_in(key, 6);
    cx->ks[7] = ss[7] = word_in(key, 7);

    ke8(cx->ks, 0); ke8(cx->ks, 1);
    ke8(cx->ks, 2); ke8(cx->ks, 3);
    ke8(cx->ks, 4); ke8(cx->ks, 5);

    kef8(cx->ks, 6);
    cx->inf.l = 0;
    cx->inf.b[0] = 14 * 16;

    return EXIT_SUCCESS;
}

#define v(n,i)  ((n) - (i) + 2 * ((i) & 3))

#define kdf8(k,i) \
{   ss[0] ^= ls_box(ss[7],3) ^ t_use(r,c)[i]; k[v(56,(8*(i))+ 8)] = ff(ss[0]); \
    ss[1] ^= ss[0]; k[v(56,(8*(i))+ 9)] = ff(ss[1]); \
    ss[2] ^= ss[1]; k[v(56,(8*(i))+10)] = ff(ss[2]); \
    ss[3] ^= ss[2]; k[v(56,(8*(i))+11)] = ff(ss[3]); \
    ss[4] ^= ls_box(ss[3],0); k[v(56,(8*(i))+12)] = ff(ss[4]); \
    ss[5] ^= ss[4]; k[v(56,(8*(i))+13)] = ff(ss[5]); \
    ss[6] ^= ss[5]; k[v(56,(8*(i))+14)] = ff(ss[6]); \
    ss[7] ^= ss[6]; k[v(56,(8*(i))+15)] = ff(ss[7]); \
}

#define kd8(k,i) \
{   ss[8] = ls_box(ss[7],3) ^ t_use(r,c)[i]; \
    ss[0] ^= ss[8]; ss[8] = ff(ss[8]); k[v(56,(8*(i))+ 8)] = ss[8] ^= k[v(56,(8*(i)))]; \
    ss[1] ^= ss[0]; k[v(56,(8*(i))+ 9)] = ss[8] ^= k[v(56,(8*(i))+ 1)]; \
    ss[2] ^= ss[1]; k[v(56,(8*(i))+10)] = ss[8] ^= k[v(56,(8*(i))+ 2)]; \
    ss[3] ^= ss[2]; k[v(56,(8*(i))+11)] = ss[8] ^= k[v(56,(8*(i))+ 3)]; \
    ss[8] = ls_box(ss[3],0); \
    ss[4] ^= ss[8]; ss[8] = ff(ss[8]); k[v(56,(8*(i))+12)] = ss[8] ^= k[v(56,(8*(i))+ 4)]; \
    ss[5] ^= ss[4]; k[v(56,(8*(i))+13)] = ss[8] ^= k[v(56,(8*(i))+ 5)]; \
    ss[6] ^= ss[5]; k[v(56,(8*(i))+14)] = ss[8] ^= k[v(56,(8*(i))+ 6)]; \
    ss[7] ^= ss[6]; k[v(56,(8*(i))+15)] = ss[8] ^= k[v(56,(8*(i))+ 7)]; \
}

#define kdl8(k,i) \
{   ss[0] ^= ls_box(ss[7],3) ^ t_use(r,c)[i]; k[v(56,(8*(i))+ 8)] = ss[0]; \
    ss[1] ^= ss[0]; k[v(56,(8*(i))+ 9)] = ss[1]; \
    ss[2] ^= ss[1]; k[v(56,(8*(i))+10)] = ss[2]; \
    ss[3] ^= ss[2]; k[v(56,(8*(i))+11)] = ss[3]; \
}

#define inv_mcol(x)       four_tables(x,t_use(i,m),vf1,rf1,0)
#define ff(x)   inv_mcol(x)

__device__ AES_RETURN aes_decrypt_key256(const unsigned char *key, aes_decrypt_ctx cx[1])
{   uint_32t    ss[9];
    cx->ks[v(56,(0))] = ss[0] = word_in(key, 0);
    cx->ks[v(56,(1))] = ss[1] = word_in(key, 1);
    cx->ks[v(56,(2))] = ss[2] = word_in(key, 2);
    cx->ks[v(56,(3))] = ss[3] = word_in(key, 3);


    cx->ks[v(56,(4))] = ff(ss[4] = word_in(key, 4));
    cx->ks[v(56,(5))] = ff(ss[5] = word_in(key, 5));
    cx->ks[v(56,(6))] = ff(ss[6] = word_in(key, 6));
    cx->ks[v(56,(7))] = ff(ss[7] = word_in(key, 7));
    kdf8(cx->ks, 0); kd8(cx->ks, 1);
    kd8(cx->ks, 2);  kd8(cx->ks, 3);
    kd8(cx->ks, 4);  kd8(cx->ks, 5);
    kdl8(cx->ks, 6);

    cx->inf.l = 0;
    cx->inf.b[0] = 14 * 16;

    return EXIT_SUCCESS;
}

__device__ void LKf (u32 *k, unsigned int r, u32 *a, u32 *b, u32 *c, u32 *d)
{
    *a = k[r];
    *b = k[r + 1];
    *c = k[r + 2];
    *d = k[r + 3];
}

__device__ void SKf (u32 *k, unsigned int r, u32 *a, u32 *b, u32 *c, u32 *d)
{
    k[r + 4] = *a;
    k[r + 5] = *b;
    k[r + 6] = *c;
    k[r + 7] = *d;
}

#define rotlFixed(x,n)   (((x) << (n)) | ((x) >> (32 - (n))))
#define rotrFixed(x,n)   (((x) >> (n)) | ((x) << (32 - (n))))

__device__ void S0f (u32 *r0, u32 *r1, u32 *r2, u32 *r3, u32 *r4)
{
    *r3 ^= *r0;
    *r4 = *r1;
    *r1 &= *r3;
    *r4 ^= *r2;
    *r1 ^= *r0;
    *r0 |= *r3;
    *r0 ^= *r4;
    *r4 ^= *r3;
    *r3 ^= *r2;
    *r2 |= *r1;
    *r2 ^= *r4;
    *r4 = ~*r4;
    *r4 |= *r1;
    *r1 ^= *r3;
    *r1 ^= *r4;
    *r3 |= *r0;
    *r1 ^= *r3;
    *r4 ^= *r3;
}

__device__ void S1f (u32 *r0, u32 *r1, u32 *r2, u32 *r3, u32 *r4)
{        
    *r0 = ~*r0;   
    *r2 = ~*r2;   
    *r4 = *r0;
    *r0 &= *r1;
    *r2 ^= *r0;
    *r0 |= *r3;
    *r3 ^= *r2;
    *r1 ^= *r0;
    *r0 ^= *r4;
    *r4 |= *r1;
    *r1 ^= *r3;
    *r2 |= *r0;
    *r2 &= *r4;
    *r0 ^= *r1;
    *r1 &= *r2;
    *r1 ^= *r0;
    *r0 &= *r2;
    *r0 ^= *r4;
}

__device__ void S2f (u32 *r0, u32 *r1, u32 *r2, u32 *r3, u32 *r4)
{        
    *r4 = *r0;
    *r0 &= *r2;
    *r0 ^= *r3;
    *r2 ^= *r1;
    *r2 ^= *r0;
    *r3 |= *r4;
    *r3 ^= *r1;
    *r4 ^= *r2;
    *r1 = *r3;
    *r3 |= *r4;
    *r3 ^= *r0;
    *r0 &= *r1;
    *r4 ^= *r0;
    *r1 ^= *r3;
    *r1 ^= *r4;
    *r4 = ~*r4;   
}

__device__ void S3f (u32 *r0, u32 *r1, u32 *r2, u32 *r3, u32 *r4)
{        
    *r4 = *r0;
    *r0 |= *r3;
    *r3 ^= *r1;
    *r1 &= *r4;
    *r4 ^= *r2;
    *r2 ^= *r3;
    *r3 &= *r0;
    *r4 |= *r1;
    *r3 ^= *r4;
    *r0 ^= *r1;
    *r4 &= *r0;
    *r1 ^= *r3;
    *r4 ^= *r2;
    *r1 |= *r0;
    *r1 ^= *r2;
    *r0 ^= *r3;
    *r2 = *r1;
    *r1 |= *r3;
    *r1 ^= *r0;
}

__device__ void S4f (u32 *r0, u32 *r1, u32 *r2, u32 *r3, u32 *r4)
{        
    *r1 ^= *r3;
    *r3 = ~*r3;   
    *r2 ^= *r3;
    *r3 ^= *r0;
    *r4 = *r1;
    *r1 &= *r3;
    *r1 ^= *r2;
    *r4 ^= *r3;
    *r0 ^= *r4;
    *r2 &= *r4;
    *r2 ^= *r0;
    *r0 &= *r1;
    *r3 ^= *r0;
    *r4 |= *r1;
    *r4 ^= *r0;
    *r0 |= *r3;
    *r0 ^= *r2;
    *r2 &= *r3;
    *r0 = ~*r0;   
    *r4 ^= *r2;
}

__device__ void S5f (u32 *r0, u32 *r1, u32 *r2, u32 *r3, u32 *r4)
{        
    *r0 ^= *r1;
    *r1 ^= *r3;
    *r3 = ~*r3;   
    *r4 = *r1;
    *r1 &= *r0;
    *r2 ^= *r3;
    *r1 ^= *r2;
    *r2 |= *r4;
    *r4 ^= *r3;
    *r3 &= *r1;
    *r3 ^= *r0;
    *r4 ^= *r1;
    *r4 ^= *r2;
    *r2 ^= *r0;
    *r0 &= *r3;
    *r2 = ~*r2;   
    *r0 ^= *r4;
    *r4 |= *r3;
    *r2 ^= *r4;
}

__device__ void S6f (u32 *r0, u32 *r1, u32 *r2, u32 *r3, u32 *r4)
{        
    *r2 = ~*r2;   
    *r4 = *r3;
    *r3 &= *r0;
    *r0 ^= *r4;
    *r3 ^= *r2;
    *r2 |= *r4;
    *r1 ^= *r3;
    *r2 ^= *r0;
    *r0 |= *r1;
    *r2 ^= *r1;
    *r4 ^= *r0;
    *r0 |= *r3;
    *r0 ^= *r2;
    *r4 ^= *r3;
    *r4 ^= *r0;
    *r3 = ~*r3;   
    *r2 &= *r4;
    *r2 ^= *r3;
}

__device__ void S7f (u32 *r0, u32 *r1, u32 *r2, u32 *r3, u32 *r4)
{        
    *r4 = *r2;
    *r2 &= *r1;
    *r2 ^= *r3;
    *r3 &= *r1;
    *r4 ^= *r2;
    *r2 ^= *r1;
    *r1 ^= *r0;
    *r0 |= *r4;
    *r0 ^= *r2;
    *r3 ^= *r1;
    *r2 ^= *r3;
    *r3 &= *r0;
    *r3 ^= *r4;
    *r4 ^= *r2;
    *r2 &= *r0;
    *r4 = ~*r4;   
    *r2 ^= *r4;
    *r4 &= *r0;
    *r1 ^= *r3;
    *r4 ^= *r1;
}

__device__ void serpent_set_key(const u8 userKey[], int keylen, u8 *ks)
{
    u32 a,b,c,d,e;
    u32 *k = (u32 *)ks;
    u32 t;
    int i;

    for (i = 0; i < keylen / (int)sizeof(int); i++)
        k[i] = ((u32*)userKey)[i];

    if (keylen < 32)
        k[keylen/4] |= (u32)1 << ((keylen%4)*8);

    k += 8;
    t = k[-1];
    for (i = 0; i < 132; ++i)
        k[i] = t = rotlFixed(k[i-8] ^ k[i-5] ^ k[i-3] ^ t ^ 0x9e3779b9 ^ i, 11);
    k -= 20;

    for (i=0; i<4; i++)
    {
        LKf (k, 20, &a, &e, &b, &d); S3f (&a, &e, &b, &d, &c); SKf (k, 16, &e, &b, &d, &c);
        LKf (k, 24, &c, &b, &a, &e); S2f (&c, &b, &a, &e, &d); SKf (k, 20, &a, &e, &b, &d);
        LKf (k, 28, &b, &e, &c, &a); S1f (&b, &e, &c, &a, &d); SKf (k, 24, &c, &b, &a, &e);
        LKf (k, 32, &a, &b, &c, &d); S0f (&a, &b, &c, &d, &e); SKf (k, 28, &b, &e, &c, &a);
        k += 8*4;
        LKf (k,  4, &a, &c, &d, &b); S7f (&a, &c, &d, &b, &e); SKf (k,  0, &d, &e, &b, &a);
        LKf (k,  8, &a, &c, &b, &e); S6f (&a, &c, &b, &e, &d); SKf (k,  4, &a, &c, &d, &b);
        LKf (k, 12, &b, &a, &e, &c); S5f (&b, &a, &e, &c, &d); SKf (k,  8, &a, &c, &b, &e);
        LKf (k, 16, &e, &b, &d, &c); S4f (&e, &b, &d, &c, &a); SKf (k, 12, &b, &a, &e, &c);
    }
    LKf (k, 20, &a, &e, &b, &d); S3f (&a, &e, &b, &d, &c); SKf (k, 16, &e, &b, &d, &c);
}

#define G_MOD   0x0000014d

__device__ u4byte mds_rem(u4byte p0, u4byte p1)
{   u4byte  i, t, u;

    for(i = 0; i < 8; ++i)
    {
        t = p1 >> 24;   // get most significant coefficient
        
        p1 = (p1 << 8) | (p0 >> 24); p0 <<= 8;  // shift others up
            
        // multiply t by a (the primitive element - i.e. left shift)

        u = (t << 1); 
        
        if(t & 0x80)            // subtract modular polynomial on overflow
        
            u ^= G_MOD; 

        p1 ^= t ^ (u << 16);    // remove t * (a * x^2 + 1)  

        u ^= (t >> 1);          // form u = a * t + t / a = t * (a + 1 / a); 
        
        if(t & 0x01)            // add the modular polynomial on underflow
        
            u ^= G_MOD >> 1;

        p1 ^= (u << 24) | (u << 8); // remove t * (a + 1/a) * (x^3 + x)
    }

    return p1;
}

#define extract_byte(x,n)   ((u1byte)((x) >> (8 * n)))
#undef q
#define q(n,x)  q_tab[n][x]
#define mds(n,x)    m_tab[n][x]
#define rotr(x,n) (((x)>>(n))|((x)<<(32-(n))))
#define rotl(x,n) (((x)<<(n))|((x)>>(32-(n))))

#define q20(x)  q(0,q(0,x) ^ extract_byte(key[1],0)) ^ extract_byte(key[0],0)
#define q21(x)  q(0,q(1,x) ^ extract_byte(key[1],1)) ^ extract_byte(key[0],1)
#define q22(x)  q(1,q(0,x) ^ extract_byte(key[1],2)) ^ extract_byte(key[0],2)
#define q23(x)  q(1,q(1,x) ^ extract_byte(key[1],3)) ^ extract_byte(key[0],3)

#define q30(x)  q(0,q(0,q(1, x) ^ extract_byte(key[2],0)) ^ extract_byte(key[1],0)) ^ extract_byte(key[0],0)
#define q31(x)  q(0,q(1,q(1, x) ^ extract_byte(key[2],1)) ^ extract_byte(key[1],1)) ^ extract_byte(key[0],1)
#define q32(x)  q(1,q(0,q(0, x) ^ extract_byte(key[2],2)) ^ extract_byte(key[1],2)) ^ extract_byte(key[0],2)
#define q33(x)  q(1,q(1,q(0, x) ^ extract_byte(key[2],3)) ^ extract_byte(key[1],3)) ^ extract_byte(key[0],3)

#define q40(x)  q(0,q(0,q(1, q(1, x) ^ extract_byte(key[3],0)) ^ extract_byte(key[2],0)) ^ extract_byte(key[1],0)) ^ extract_byte(key[0],0)
#define q41(x)  q(0,q(1,q(1, q(0, x) ^ extract_byte(key[3],1)) ^ extract_byte(key[2],1)) ^ extract_byte(key[1],1)) ^ extract_byte(key[0],1)
#define q42(x)  q(1,q(0,q(0, q(0, x) ^ extract_byte(key[3],2)) ^ extract_byte(key[2],2)) ^ extract_byte(key[1],2)) ^ extract_byte(key[0],2)
#define q43(x)  q(1,q(1,q(0, q(1, x) ^ extract_byte(key[3],3)) ^ extract_byte(key[2],3)) ^ extract_byte(key[1],3)) ^ extract_byte(key[0],3)

__device__ void gen_mk_tab(TwofishInstance *instance, u4byte key[], u1byte** q_tab, u4byte** m_tab)
{   u4byte  i;
    u1byte  by;

    u4byte *mk_tab = instance->mk_tab;

    switch(instance->k_len)
    {
    case 2: for(i = 0; i < 256; ++i)
            {
                by = (u1byte)i;
                mk_tab[0 + 4*i] = mds(0, q20(by)); mk_tab[1 + 4*i] = mds(1, q21(by));
                mk_tab[2 + 4*i] = mds(2, q22(by)); mk_tab[3 + 4*i] = mds(3, q23(by));
            }
            break;
    
    case 3: for(i = 0; i < 256; ++i)
            {
                by = (u1byte)i;
                mk_tab[0 + 4*i] = mds(0, q30(by)); mk_tab[1 + 4*i] = mds(1, q31(by));
                mk_tab[2 + 4*i] = mds(2, q32(by)); mk_tab[3 + 4*i] = mds(3, q33(by));
            }
            break;
    
    case 4: for(i = 0; i < 256; ++i)
            {
                by = (u1byte)i;
                mk_tab[0 + 4*i] = mds(0, q40(by)); mk_tab[1 + 4*i] = mds(1, q41(by));
                mk_tab[2 + 4*i] = mds(2, q42(by)); mk_tab[3 + 4*i] = mds(3, q43(by));
            }
    }
}

__device__ u1byte qp(const u4byte n, const u1byte x)
{   u1byte  a0, a1, a2, a3, a4, b0, b1, b2, b3, b4;

    a0 = x >> 4; b0 = x & 15;
    a1 = a0 ^ b0; b1 = ror4[b0] ^ ashx[a0];
    a2 = qt0[n][a1]; b2 = qt1[n][b1];
    a3 = a2 ^ b2; b3 = ror4[b2] ^ ashx[a2];
    a4 = qt2[n][a3]; b4 = qt3[n][b3];
    return (b4 << 4) | a4;
}

__device__ u4byte h_fun(TwofishInstance *instance, const u4byte x, const u4byte key[], u1byte** q_tab, u4byte** m_tab)
{   u4byte  b0, b1, b2, b3;

    b0 = extract_byte(x, 0); b1 = extract_byte(x, 1); b2 = extract_byte(x, 2); b3 = extract_byte(x, 3);

    switch(instance->k_len)
    {
    case 4: b0 = q(1, (u1byte) b0) ^ extract_byte(key[3],0);
            b1 = q(0, (u1byte) b1) ^ extract_byte(key[3],1);
            b2 = q(0, (u1byte) b2) ^ extract_byte(key[3],2);
            b3 = q(1, (u1byte) b3) ^ extract_byte(key[3],3);
    case 3: b0 = q(1, (u1byte) b0) ^ extract_byte(key[2],0);
            b1 = q(1, (u1byte) b1) ^ extract_byte(key[2],1);
            b2 = q(0, (u1byte) b2) ^ extract_byte(key[2],2);
            b3 = q(0, (u1byte) b3) ^ extract_byte(key[2],3);
    case 2: b0 = q(0, (u1byte) (q(0, (u1byte) b0) ^ extract_byte(key[1],0))) ^ extract_byte(key[0],0);
            b1 = q(0, (u1byte) (q(1, (u1byte) b1) ^ extract_byte(key[1],1))) ^ extract_byte(key[0],1);
            b2 = q(1, (u1byte) (q(0, (u1byte) b2) ^ extract_byte(key[1],2))) ^ extract_byte(key[0],2);
            b3 = q(1, (u1byte) (q(1, (u1byte) b3) ^ extract_byte(key[1],3))) ^ extract_byte(key[0],3);
    }

    return  mds(0, b0) ^ mds(1, b1) ^ mds(2, b2) ^ mds(3, b3);
}

__device__ void gen_qtab(u1byte** q_tab)
{   u4byte  i;

    for(i = 0; i < 256; ++i)
    {
        q(0,i) = qp(0, (u1byte)i);
        q(1,i) = qp(1, (u1byte)i);
    }
}

#define ffm_5b(x)   ((x) ^ ((x) >> 2) ^ tab_5b[(x) & 3])
#define ffm_ef(x)   ((x) ^ ((x) >> 1) ^ ((x) >> 2) ^ tab_ef[(x) & 3])

__device__ void gen_mtab(u1byte** q_tab, u4byte** m_tab)
{   u4byte  i, f01, f5b, fef;
    
    for(i = 0; i < 256; ++i)
    {
        f01 = q(1,i); f5b = ffm_5b(f01); fef = ffm_ef(f01);
        m_tab[0][i] = f01 + (f5b << 8) + (fef << 16) + (fef << 24);
        m_tab[2][i] = f5b + (fef << 8) + (f01 << 16) + (fef << 24);

        f01 = q(0,i); f5b = ffm_5b(f01); fef = ffm_ef(f01);
        m_tab[1][i] = fef + (fef << 8) + (f5b << 16) + (f01 << 24);
        m_tab[3][i] = f5b + (f01 << 8) + (fef << 16) + (f5b << 24);
    }
}

/* initialise the key schedule from the user supplied key   */

__device__ u4byte *twofish_set_key(TwofishInstance *instance, const u4byte in_key[], const u4byte key_len,
    u1byte** q_tab, u4byte** m_tab, u4byte* qt_gen, u4byte* mt_gen)
{   u4byte  i, a, b, me_key[4], mo_key[4];
    u4byte *l_key, *s_key;

    l_key = instance->l_key;
    s_key = instance->s_key;

    if(!*qt_gen)
    {
        gen_qtab(q_tab); *qt_gen = 1;
    }

    if(!*mt_gen)
    {
        gen_mtab(q_tab, m_tab); *mt_gen = 1;
    }

    instance->k_len = key_len / 64;   /* 2, 3 or 4 */

    for(i = 0; i < instance->k_len; ++i)
    {
        a = in_key[i + i];     me_key[i] = a;
        b = in_key[i + i + 1]; mo_key[i] = b;
        s_key[instance->k_len - i - 1] = mds_rem(a, b);
    }

    for(i = 0; i < 40; i += 2)
    {
        a = 0x01010101 * i; b = a + 0x01010101;
        a = h_fun(instance, a, me_key, q_tab, m_tab);
        b = rotl(h_fun(instance, b, mo_key, q_tab, m_tab), 8);
        l_key[i] = a + b;
        l_key[i + 1] = rotl(a + 2 * b, 9);
    }

    gen_mk_tab(instance, s_key, q_tab, m_tab);

    return l_key;
}

#define ROUNDS 16
#define GETBYTE(x, y) (unsigned int)(byte)((x)>>(8*(y)))

__device__ void crypt_block(BF_KEY *key, const word32 in[2], word32 out[2])
{
    word32 left = in[0];
    word32 right = in[1];

    const word32 *const s=key->sbox;
    const word32 *p=key->pbox;

    unsigned i;

    left ^= p[0];

    for (i=0; i<ROUNDS/2; i++)
    {
        right ^= (((s[GETBYTE(left,3)] + s[256+GETBYTE(left,2)])
              ^ s[2*256+GETBYTE(left,1)]) + s[3*256+GETBYTE(left,0)])
              ^ p[2*i+1];

        left ^= (((s[GETBYTE(right,3)] + s[256+GETBYTE(right,2)])
             ^ s[2*256+GETBYTE(right,1)]) + s[3*256+GETBYTE(right,0)])
             ^ p[2*i+2];
    }

    right ^= p[ROUNDS+1];

    out[0] = right;
    out[1] = left;
}

__device__ void BlowfishSetKey (BF_KEY *key, int keylength, unsigned char *key_string)
{
    unsigned i, j=0, k;
    word32 data, dspace[2] = {0, 0};
    word32 *sbox = key->sbox;
    word32 *pbox = key->pbox;

    memcpy(pbox, p_init, sizeof(p_init));
    memcpy(sbox, s_init, sizeof(s_init));

    // Xor key string into encryption key vector
    for (i=0 ; i<ROUNDS+2 ; ++i)
    {
        data = 0 ;
        for (k=0 ; k<4 ; ++k )
            data = (data << 8) | key_string[j++ % keylength];
        pbox[i] ^= data;
    }

    crypt_block(key, dspace, pbox);

    for (i=0; i<ROUNDS; i+=2)
        crypt_block(key, pbox+i, pbox+i+2);

    crypt_block(key, pbox+ROUNDS, sbox);

    for (i=0; i<4*256-2; i+=2)
        crypt_block(key, sbox+i, sbox+i+2);

    for (i=0; i < ROUNDS+2; i++)
        key->pbox_dec[ROUNDS+1-i] = pbox[i];
}

__device__ u32 MirrorBytes32 (u32 x)
{
    u32 n = (u8) x;
    n <<= 8; n |= (u8) (x >> 8);
    n <<= 8; n |= (u8) (x >> 16);
    return (n << 8) | (u8) (x >> 24);
}

__device__ uint64 MirrorBytes64 (uint64 x)
{
    uint64 n = (u8) x;
    n <<= 8; n |= (u8) (x >> 8);
    n <<= 8; n |= (u8) (x >> 16);
    n <<= 8; n |= (u8) (x >> 24);
    n <<= 8; n |= (u8) (x >> 32);
    n <<= 8; n |= (u8) (x >> 40);
    n <<= 8; n |= (u8) (x >> 48);
    return (n << 8) | (u8) (x >> 56);
}

#define BE32(x) MirrorBytes32(x)
#define BE64(x) MirrorBytes64(x)

__device__ void Cast5SetKey (CAST_KEY *key, unsigned int keylength, const byte *userKey)
{
    unsigned int i;
    word32 *K = key->K;
    word32 X[4], Z[4];

    X[0] = BE32 (((word32 *)userKey)[0]);
    X[1] = BE32 (((word32 *)userKey)[1]);
    X[2] = BE32 (((word32 *)userKey)[2]);
    X[3] = BE32 (((word32 *)userKey)[3]);

#define x(i) GETBYTE(X[i/4], 3-i%4)
#define z(i) GETBYTE(Z[i/4], 3-i%4)

    for (i=0; i<=16; i+=16)
    {
        // this part is copied directly from RFC 2144 (with some search and replace) by Wei Dai
        Z[0] = X[0] ^ S[4][x(0xD)] ^ S[5][x(0xF)] ^ S[6][x(0xC)] ^ S[7][x(0xE)] ^ S[6][x(0x8)];
        Z[1] = X[2] ^ S[4][z(0x0)] ^ S[5][z(0x2)] ^ S[6][z(0x1)] ^ S[7][z(0x3)] ^ S[7][x(0xA)];
        Z[2] = X[3] ^ S[4][z(0x7)] ^ S[5][z(0x6)] ^ S[6][z(0x5)] ^ S[7][z(0x4)] ^ S[4][x(0x9)];
        Z[3] = X[1] ^ S[4][z(0xA)] ^ S[5][z(0x9)] ^ S[6][z(0xB)] ^ S[7][z(0x8)] ^ S[5][x(0xB)];
        K[i+0] = S[4][z(0x8)] ^ S[5][z(0x9)] ^ S[6][z(0x7)] ^ S[7][z(0x6)] ^ S[4][z(0x2)];
        K[i+1] = S[4][z(0xA)] ^ S[5][z(0xB)] ^ S[6][z(0x5)] ^ S[7][z(0x4)] ^ S[5][z(0x6)];
        K[i+2] = S[4][z(0xC)] ^ S[5][z(0xD)] ^ S[6][z(0x3)] ^ S[7][z(0x2)] ^ S[6][z(0x9)];
        K[i+3] = S[4][z(0xE)] ^ S[5][z(0xF)] ^ S[6][z(0x1)] ^ S[7][z(0x0)] ^ S[7][z(0xC)];
        X[0] = Z[2] ^ S[4][z(0x5)] ^ S[5][z(0x7)] ^ S[6][z(0x4)] ^ S[7][z(0x6)] ^ S[6][z(0x0)];
        X[1] = Z[0] ^ S[4][x(0x0)] ^ S[5][x(0x2)] ^ S[6][x(0x1)] ^ S[7][x(0x3)] ^ S[7][z(0x2)];
        X[2] = Z[1] ^ S[4][x(0x7)] ^ S[5][x(0x6)] ^ S[6][x(0x5)] ^ S[7][x(0x4)] ^ S[4][z(0x1)];
        X[3] = Z[3] ^ S[4][x(0xA)] ^ S[5][x(0x9)] ^ S[6][x(0xB)] ^ S[7][x(0x8)] ^ S[5][z(0x3)];
        K[i+4] = S[4][x(0x3)] ^ S[5][x(0x2)] ^ S[6][x(0xC)] ^ S[7][x(0xD)] ^ S[4][x(0x8)];
        K[i+5] = S[4][x(0x1)] ^ S[5][x(0x0)] ^ S[6][x(0xE)] ^ S[7][x(0xF)] ^ S[5][x(0xD)];
        K[i+6] = S[4][x(0x7)] ^ S[5][x(0x6)] ^ S[6][x(0x8)] ^ S[7][x(0x9)] ^ S[6][x(0x3)];
        K[i+7] = S[4][x(0x5)] ^ S[5][x(0x4)] ^ S[6][x(0xA)] ^ S[7][x(0xB)] ^ S[7][x(0x7)];
        Z[0] = X[0] ^ S[4][x(0xD)] ^ S[5][x(0xF)] ^ S[6][x(0xC)] ^ S[7][x(0xE)] ^ S[6][x(0x8)];
        Z[1] = X[2] ^ S[4][z(0x0)] ^ S[5][z(0x2)] ^ S[6][z(0x1)] ^ S[7][z(0x3)] ^ S[7][x(0xA)];
        Z[2] = X[3] ^ S[4][z(0x7)] ^ S[5][z(0x6)] ^ S[6][z(0x5)] ^ S[7][z(0x4)] ^ S[4][x(0x9)];
        Z[3] = X[1] ^ S[4][z(0xA)] ^ S[5][z(0x9)] ^ S[6][z(0xB)] ^ S[7][z(0x8)] ^ S[5][x(0xB)];
        K[i+8] = S[4][z(0x3)] ^ S[5][z(0x2)] ^ S[6][z(0xC)] ^ S[7][z(0xD)] ^ S[4][z(0x9)];
        K[i+9] = S[4][z(0x1)] ^ S[5][z(0x0)] ^ S[6][z(0xE)] ^ S[7][z(0xF)] ^ S[5][z(0xC)];
        K[i+10] = S[4][z(0x7)] ^ S[5][z(0x6)] ^ S[6][z(0x8)] ^ S[7][z(0x9)] ^ S[6][z(0x2)];
        K[i+11] = S[4][z(0x5)] ^ S[5][z(0x4)] ^ S[6][z(0xA)] ^ S[7][z(0xB)] ^ S[7][z(0x6)];
        X[0] = Z[2] ^ S[4][z(0x5)] ^ S[5][z(0x7)] ^ S[6][z(0x4)] ^ S[7][z(0x6)] ^ S[6][z(0x0)];
        X[1] = Z[0] ^ S[4][x(0x0)] ^ S[5][x(0x2)] ^ S[6][x(0x1)] ^ S[7][x(0x3)] ^ S[7][z(0x2)];
        X[2] = Z[1] ^ S[4][x(0x7)] ^ S[5][x(0x6)] ^ S[6][x(0x5)] ^ S[7][x(0x4)] ^ S[4][z(0x1)];
        X[3] = Z[3] ^ S[4][x(0xA)] ^ S[5][x(0x9)] ^ S[6][x(0xB)] ^ S[7][x(0x8)] ^ S[5][z(0x3)];
        K[i+12] = S[4][x(0x8)] ^ S[5][x(0x9)] ^ S[6][x(0x7)] ^ S[7][x(0x6)] ^ S[4][x(0x3)];
        K[i+13] = S[4][x(0xA)] ^ S[5][x(0xB)] ^ S[6][x(0x5)] ^ S[7][x(0x4)] ^ S[5][x(0x7)];
        K[i+14] = S[4][x(0xC)] ^ S[5][x(0xD)] ^ S[6][x(0x3)] ^ S[7][x(0x2)] ^ S[6][x(0x8)];
        K[i+15] = S[4][x(0xE)] ^ S[5][x(0xF)] ^ S[6][x(0x1)] ^ S[7][x(0x0)] ^ S[7][x(0xD)];
    }

    for (i=16; i<32; i++)
        K[i] &= 0x1f;
}

/* Set key (initialize key schedule array) */
__device__ void RawSetKey (int encryption, const byte *key, word32 *scheduledKey)
{
    byte buffer[56+56+8];
    byte *const pc1m=buffer;                 /* place to modify pc1 into */
    byte *const pcr=pc1m+56;                 /* place to rotate pc1 into */
    byte *const ks=pcr+56;
    int i,j,l;
    int m;

    for (j=0; j<56; j++) {          /* convert pc1 to bits of key */
        l=pc1[j]-1;             /* integer bit location  */
        m = l & 07;             /* find bit              */
        pc1m[j]=(key[l>>3] &    /* find which key byte l is in */
            bytebit[m])     /* and which bit of that byte */
            ? 1 : 0;        /* and store 1-bit result */
    }
    for (i=0; i<16; i++) {          /* key chunk for each iteration */
        memset(ks,0,8);         /* Clear key schedule */
        for (j=0; j<56; j++)    /* rotate pc1 the right amount */
            pcr[j] = pc1m[(l=j+totrot[i])<(j<28? 28 : 56) ? l: l-28];
        /* rotate left and right halves independently */
        for (j=0; j<48; j++){   /* select bits individually */
            /* check bit that goes to ks[j] */
            if (pcr[pc2[j]-1]){
                /* mask it in if it's there */
                l= j % 6;
                ks[j/6] |= bytebit[l] >> 2;
            }
        }
        /* Now convert to odd/even interleaved form for use in F */
        scheduledKey[2*i] = ((word32)ks[0] << 24)
            | ((word32)ks[2] << 16)
            | ((word32)ks[4] << 8)
            | ((word32)ks[6]);
        scheduledKey[2*i+1] = ((word32)ks[1] << 24)
            | ((word32)ks[3] << 16)
            | ((word32)ks[5] << 8)
            | ((word32)ks[7]);
    }
    
    if (!encryption)     // reverse key schedule order
        for (i=0; i<16; i+=2)
        {
            word32 b = scheduledKey[i];
            scheduledKey[i] = scheduledKey[32-2-i];
            scheduledKey[32-2-i] = b;

            b = scheduledKey[i+1];
            scheduledKey[i+1] = scheduledKey[32-1-i];
            scheduledKey[32-1-i] = b;
        }

    burn (buffer, sizeof (buffer));
}

__device__ void TripleDesSetKey (const byte *userKey, unsigned int length, TDES_KEY *ks)
{
    TDES_KEY *as = ks;

    RawSetKey (1, userKey + 0, as->k1);
    RawSetKey (1, userKey + 8, ks->k2);
    RawSetKey (1, userKey + 16, ks->k3);
    RawSetKey (0, userKey + 16, ks->k1d);
    RawSetKey (0, userKey + 8, ks->k2d);
    RawSetKey (0, userKey + 0, ks->k3d);
}

/* Return values: 0 = success, ERR_CIPHER_INIT_FAILURE (fatal), ERR_CIPHER_INIT_WEAK_KEY (non-fatal) */
__device__ int CipherInit (int cipher, unsigned char *key, u8 *ks, u1byte** q_tab, u4byte** m_tab, u4byte* qt_gen, u4byte* mt_gen)
{
    int retVal = ERR_SUCCESS;

    switch (cipher)
    {
    case AES:

        if (aes_encrypt_key256 (key, (aes_encrypt_ctx *) ks) != EXIT_SUCCESS)
            return ERR_CIPHER_INIT_FAILURE;

        if (aes_decrypt_key256 (key, (aes_decrypt_ctx *) (ks + sizeof(aes_encrypt_ctx))) != EXIT_SUCCESS)
            return ERR_CIPHER_INIT_FAILURE;
        break;

    case SERPENT:
        serpent_set_key (key, CipherGetKeySize(SERPENT) * 8, ks);
        break;
        
    case TWOFISH:
        twofish_set_key ((TwofishInstance *)ks, (const u4byte *)key, CipherGetKeySize(TWOFISH) * 8, q_tab, m_tab, qt_gen, mt_gen); // FIXME: crash here
        break;

    case BLOWFISH:
        /* Deprecated/legacy */
        BlowfishSetKey ((BF_KEY *)ks, CipherGetKeySize(BLOWFISH), key);
        break;

    case CAST:
        /* Deprecated/legacy */
        Cast5SetKey ((CAST_KEY *) ks, CipherGetKeySize(CAST), key);
        break;

    case TRIPLEDES:
        /* Deprecated/legacy */
        TripleDesSetKey (key, CipherGetKeySize (TRIPLEDES), (TDES_KEY *) ks);

        // Verify whether all three DES keys are mutually different
        if (((*((int64 *) key) ^ *((int64 *) key+1)) & 0xFEFEFEFEFEFEFEFEULL) == 0
        || ((*((int64 *) key+1) ^ *((int64 *) key+2)) & 0xFEFEFEFEFEFEFEFEULL) == 0
        || ((*((int64 *) key) ^ *((int64 *) key+2)) & 0xFEFEFEFEFEFEFEFEULL) == 0)
            retVal = ERR_CIPHER_INIT_WEAK_KEY;		// Non-fatal error

        break;

    default:
        // Unknown/wrong cipher ID
        return ERR_CIPHER_INIT_FAILURE;
    }

    return retVal;
}

// Return values: 0 = success, ERR_CIPHER_INIT_FAILURE (fatal), ERR_CIPHER_INIT_WEAK_KEY (non-fatal)
__device__ int EAInit (int ea, unsigned char *key, u8 *ks, u1byte** q_tab, u4byte** m_tab, u4byte* qt_gen, u4byte* mt_gen)
{
    int c, retVal = ERR_SUCCESS;

    if (ea == 0)
        return ERR_CIPHER_INIT_FAILURE;

    for (c = EAGetFirstCipher (ea); c != 0; c = EAGetNextCipher (ea, c))
    {
        switch (CipherInit (c, key, ks, q_tab, m_tab, qt_gen, mt_gen))
        {
        case ERR_CIPHER_INIT_FAILURE:
            return ERR_CIPHER_INIT_FAILURE;

        case ERR_CIPHER_INIT_WEAK_KEY:
            retVal = ERR_CIPHER_INIT_WEAK_KEY;		// Non-fatal error
            break;
        }

        key += CipherGetKeySize (c);
        ks += CipherGetKeyScheduleSize (c);
    }
    return retVal;
}

__device__ int IsBitSet128 (unsigned int bit, u8 *a)
{
    return a[(127 - bit) / 8] & (0x80 >> ((127 - bit) % 8));
}

__device__ int IsBitSet64 (unsigned int bit, u8 *a)
{
    return a[(63 - bit) / 8] & (0x80 >> ((63 - bit) % 8));
}

__device__ void SetBit128 (unsigned int bit, u8 *a)
{
    a[(127 - bit) / 8] |= 0x80 >> ((127 - bit) % 8);
}

__device__ void SetBit64 (unsigned int bit, u8 *a)
{
    a[(63 - bit) / 8] |= 0x80 >> ((63 - bit) % 8);
}

__device__ void MirrorBits128 (u8 *a)
{
    u8 t[128 / 8];
    int i;
    memset (t,0,16);
    for (i = 0; i < 128; i++)
    {
        if (IsBitSet128(i, a))
            SetBit128 (127 - i, t);
    }
    memcpy (a, t, sizeof (t));
    burn (t,sizeof (t));
}

__device__ void MirrorBits64 (u8 *a)
{
    u8 t[64 / 8];
    int i;
    memset (t,0,8);
    for (i = 0; i < 64; i++)
    {
        if (IsBitSet64(i, a))
            SetBit64 (63 - i, t);
    }
    memcpy (a, t, sizeof (t));
    burn (t,sizeof (t));
}

/* Multiply of a GF128 field element by x.   The field element  */
/* is held in an array of bytes in which field bits 8n..8n + 7  */
/* are held in byte[n], with lower indexed bits placed in the   */
/* more numerically significant bit positions in bytes.         */

/* This function multiples a field element x, in the polynomial */
/* field representation. It uses 32-bit word operations to gain */
/* speed but compensates for machine endianess and hence works  */
/* correctly on both styles of machine                          */

__device__ in_line void mul_x(mode(32t) x[4])
{   mode(32t)   t;

    bsw_32(x, 4);

    /* at this point the filed element bits 0..127 are set out  */
    /* as follows in 32-bit words (where the most significant   */
    /* (ms) numeric bits are to the left)                       */
    /*                                                          */
    /*            x[0]      x[1]      x[2]      x[3]            */
    /*          ms    ls  ms    ls  ms    ls  ms     ls         */
    /* field:   0 ... 31  32 .. 63  64 .. 95  96 .. 127         */

    t = gf_poly[x[3] & 1];          /* bit 127 of the element   */
    x[3] = (x[3] >> 1) | (x[2] << 31);  /* shift bits up by one */
    x[2] = (x[2] >> 1) | (x[1] << 31);  /* position             */
    x[1] = (x[1] >> 1) | (x[0] << 31);  /* if bit 7 is 1 xor in */
    x[0] = (x[0] >> 1) ^ t;             /* the field polynomial */
    bsw_32(x, 4);
}

__device__ in_line void mul_lex8(mode(32t) x[4])   /* mutiply with long words  */
{   mode(32t)   t = (x[3] >> 24);       /* in little endian format  */
    x[3] = (x[3] << 8) | (x[2] >> 24);
    x[2] = (x[2] << 8) | (x[1] >> 24);
    x[1] = (x[1] << 8) | (x[0] >> 24);
    x[0] = (x[0] << 8) ^ gft_le[t];
}

__device__ in_line void mul_x64(mode(32t) x[2])
{   mode(32t)   t;

    bsw_32(x, 2);

    /* at this point the filed element bits 0..127 are set out  */
    /* as follows in 32-bit words (where the most significant   */
    /* (ms) numeric bits are to the left)                       */
    /*                                                          */
    /*            x[0]      x[1]      x[2]      x[3]            */
    /*          ms    ls  ms    ls  ms    ls  ms     ls         */
    /* field:   0 ... 31  32 .. 63  64 .. 95  96 .. 127         */

    t = gf_poly64[x[1] & 1];          /* bit 127 of the element   */
                                        /* shift bits up by one */
                                        /* position             */
    x[1] = (x[1] >> 1) | (x[0] << 31);  /* if bit 7 is 1 xor in */
    x[0] = (x[0] >> 1) ^ t;             /* the field polynomial */
    bsw_32(x, 2);
}

__device__ in_line void mul_lex8_64(mode(32t) x[2])   /* mutiply with long words  */
{   mode(32t)   t = (x[1] >> 24);       /* in little endian format  */
    x[1] = (x[1] << 8) | (x[0] >> 24);
    x[0] = (x[0] << 8) ^ gft_le64[t];
}

#define mul_x8  mul_lex8
#define mul_x8_64  mul_lex8_64

__device__ void compile_8k_table(u8 *a, GfCtx8k *ctx)
{   int i, j, k;

    memset(ctx->gf_t8k, 0, 32 * 16 * 16);
    for(i = 0; i < 2 * CBLK_LEN; ++i)
    {
        if(i == 0)
        {
            memcpy(ctx->gf_t8k[1][8], a, CBLK_LEN);
            for(j = 4; j > 0; j >>= 1)
            {
                memcpy(ctx->gf_t8k[1][j], ctx->gf_t8k[1][j + j], CBLK_LEN);
                mul_x(ctx->gf_t8k[1][j]);
            }
            memcpy(ctx->gf_t8k[0][8], ctx->gf_t8k[1][1], CBLK_LEN);
            mul_x(ctx->gf_t8k[0][8]);
            for(j = 4; j > 0; j >>= 1)
            {
                memcpy(ctx->gf_t8k[0][j], ctx->gf_t8k[0][j + j], CBLK_LEN);
                mul_x(ctx->gf_t8k[0][j]);
            }
        }
        else if(i > 1)
            for(j = 8; j > 0; j >>= 1)
            {
                memcpy(ctx->gf_t8k[i][j], ctx->gf_t8k[i - 2][j], CBLK_LEN);
                mul_x8(ctx->gf_t8k[i][j]);
            }

        for(j = 2; j < 16; j += j)
        {
            mode(32t) *pj = ctx->gf_t8k[i][j];
            mode(32t) *pk = ctx->gf_t8k[i][1];
            mode(32t) *pl = ctx->gf_t8k[i][j + 1];

            for(k = 1; k < j; ++k)
            {
                *pl++ = pj[0] ^ *pk++;
                *pl++ = pj[1] ^ *pk++;
                *pl++ = pj[2] ^ *pk++;
                *pl++ = pj[3] ^ *pk++;
            }
        }
    }
}

__device__ void compile_4k_table64(u8 *a, GfCtx4k64 *ctx)
{   int i, j, k;

    memset(ctx->gf_t4k, 0, sizeof(ctx->gf_t4k));
    for(i = 0; i < 2 * CBLK_LEN8; ++i)
    {
        if(i == 0)
        {
            memcpy(ctx->gf_t4k[1][8], a, CBLK_LEN8);
            for(j = 4; j > 0; j >>= 1)
            {
                memcpy(ctx->gf_t4k[1][j], ctx->gf_t4k[1][j + j], CBLK_LEN8);
                mul_x64(ctx->gf_t4k[1][j]);
            }
            memcpy(ctx->gf_t4k[0][8], ctx->gf_t4k[1][1], CBLK_LEN8);
            mul_x64(ctx->gf_t4k[0][8]);
            for(j = 4; j > 0; j >>= 1)
            {
                memcpy(ctx->gf_t4k[0][j], ctx->gf_t4k[0][j + j], CBLK_LEN8);
                mul_x64(ctx->gf_t4k[0][j]);
            }
        }
        else if(i > 1)
            for(j = 8; j > 0; j >>= 1)
            {
                memcpy(ctx->gf_t4k[i][j], ctx->gf_t4k[i - 2][j], CBLK_LEN8);
                mul_x8_64(ctx->gf_t4k[i][j]);
            }

        for(j = 2; j < 16; j += j)
        {
            mode(32t) *pj = ctx->gf_t4k[i][j];
            mode(32t) *pk = ctx->gf_t4k[i][1];
            mode(32t) *pl = ctx->gf_t4k[i][j + 1];

            for(k = 1; k < j; ++k)
            {
                *pl++ = pj[0] ^ *pk++;
                *pl++ = pj[1] ^ *pk++;
                *pl++ = pj[2] ^ *pk++;
                *pl++ = pj[3] ^ *pk++;
            }
        }
    }
}

/* Allocate and initialize speed optimization table
   for multiplication by 64-bit operand in MSB-first mode */
__device__ int Gf128Tab64Init (u8 *a, GfCtx *ctx)
{
    GfCtx8k ctx8k;
    u8 am[16];
    int i, j;

    memcpy (am, a, 16);
    MirrorBits128 (am);
    compile_8k_table (am, &ctx8k);

    /* Convert 8k LSB-first table to 4k MSB-first */
    for (i = 16; i < 32; i++) 
    {
        for (j = 0; j < 16; j++) 
        {
            int jm = 0;
            jm |= (j & 0x1) << 3;
            jm |= (j & 0x2) << 1;
            jm |= (j & 0x4) >> 1;
            jm |= (j & 0x8) >> 3;

            memcpy (&ctx->gf_t128[i-16][jm], (unsigned char *)&ctx8k.gf_t8k[31-i][j], 16);
            MirrorBits128 ((unsigned char *)&ctx->gf_t128[i-16][jm]);
        }
    }

    burn (am, sizeof (am));

    return TRUE;
}

__device__ int Gf64TabInit (u8 *a, GfCtx *ctx)
{
    /* Deprecated/legacy */

    GfCtx4k64 ctx4k;
    u8 am[8];
    int i, j;

    memcpy (am, a, 8);
    MirrorBits64 (am);
    compile_4k_table64 (am, &ctx4k);

    /* Convert LSB-first table to MSB-first */
    for (i = 0; i < 16; i++) 
    {
        for (j = 0; j < 16; j++) 
        {
            int jm = 0;
            jm |= (j & 0x1) << 3;
            jm |= (j & 0x2) << 1;
            jm |= (j & 0x4) >> 1;
            jm |= (j & 0x8) >> 3;

            memcpy (&ctx->gf_t64[i][jm], (unsigned char *)&ctx4k.gf_t4k[15-i][j], 8);
            MirrorBits64 ((unsigned char *)&ctx->gf_t64[i][jm]);
        }
    }

    burn (am, sizeof (am));

    return TRUE;
}

__device__ BOOL EAInitMode (PCRYPTO_INFO ci, u1byte** q_tab, u4byte** m_tab, u4byte* qt_gen, u4byte* mt_gen)
{
    switch (ci->mode)
    {
    case XTS:
        // Secondary key schedule
        if (EAInit (ci->ea, ci->k2, ci->ks2, q_tab, m_tab, qt_gen, mt_gen) != ERR_SUCCESS)
            return FALSE;

        /* Note: XTS mode could potentially be initialized with a weak key causing all blocks in one data unit
        on the volume to be tweaked with zero tweaks (i.e. 512 bytes of the volume would be encrypted in ECB
        mode). However, to create a TrueCrypt volume with such a weak key, each human being on Earth would have
        to create approximately 11,378,125,361,078,862 (about eleven quadrillion) TrueCrypt volumes (provided 
        that the size of each of the volumes is 1024 terabytes). */
        break;

    case LRW:
        switch (CipherGetBlockSize (EAGetFirstCipher (ci->ea)))
        {
        case 8:
            /* Deprecated/legacy */
            return Gf64TabInit (ci->k2, &ci->gf_ctx);

        case 16:
            return Gf128Tab64Init (ci->k2, &ci->gf_ctx);

        default:
            TC_THROW_FATAL_EXCEPTION;
        }

        break;

    case CBC:
    case INNER_CBC:
    case OUTER_CBC:
        // The mode does not need to be initialized or is initialized elsewhere 
        return TRUE;

    default:		
        // Unknown/wrong ID
        TC_THROW_FATAL_EXCEPTION;
    }
    return TRUE;
}

__device__ uint32 GetHeaderField32 (byte *header, int offset)
{
    return BE32 (*(uint32 *) (header + offset));
}

#define etab_0(x)   t_fn[0][x]
#define etab_1(x)   t_fn[1][x]
#define etab_2(x)   t_fn[2][x]
#define etab_3(x)   t_fn[3][x]
#define eltab_0(x)  t_fl[0][x]
#define eltab_1(x)  t_fl[1][x]
#define eltab_2(x)  t_fl[2][x]
#define eltab_3(x)  t_fl[3][x]
#define eltab(n, x) eltab_##n(x)


__device__ void enc_round_c (uint_32t* eax, uint_32t* ebx, uint_32t* ecx, uint_32t* edx, uint_32t* ebp) {
    uint_32t key1, key2, tmp, esi, edi;
    uint_8t* ax = (uint_8t*)eax, *bx = (uint_8t*)ebx, *cx = (uint_8t*)ecx, *dx = (uint_8t*)edx;

    // ebp - key pointer, eax, ebx, ecx, edx - each contain 4 bytes (int) of outBlock, esi - 3rd int in key, edi - 4th int in key
    key1 = ebp[0];
    key2 = ebp[1];
    esi = ebp[2];
    edi = ebp[3];

    /* begin rnd_fun */
    *ebx = (*ebx << 16) | (*ebx >> 16);

    // al - 0, ah - 1, bl - 4, bh - 5, cl - 8, ch - 9, dl - 12, dh - 13
    esi ^= etab_0(cx[0]);
    esi ^= etab_1(dx[1]);
    esi ^= etab_3(bx[1]);

    edi ^= etab_0(dx[0]);
    edi ^= etab_1(ax[1]);
    edi ^= etab_2(bx[0]);

    tmp = etab_0(ax[0]); // ebp (restored later)

    *ebx >>= 16;
    *eax &= 0xffff0000;
    *eax |= *ebx;
    *edx >>= 16;

    tmp ^= etab_1(ax[1]);
    tmp ^= etab_3(dx[1]);

    *ebx = etab_2(dx[0]); // ebx
    *ebx ^= etab_1(cx[1]);
    *ebx ^= etab_0(ax[0]);

    *eax >>= 16;
    *ecx >>= 16;

    tmp ^= etab_2(cx[0]);
    edi ^= etab_3(cx[1]);
    esi ^= etab_2(ax[0]);
    *ebx ^= etab_3(ax[1]);
    /* end rnd_fun */

    *eax = tmp;

    *ecx = esi;
    *edx = edi;

    *eax ^= key1;
    *ebx ^= key2;
}

__device__ void enc_last_round_c (uint_32t* eax, uint_32t* ebx, uint_32t* ecx, uint_32t* edx, uint_32t* ebp) {
    uint_32t key1, key2, tmp, esi, edi;
    uint_8t* ax = (uint_8t*)eax, *bx = (uint_8t*)ebx, *cx = (uint_8t*)ecx, *dx = (uint_8t*)edx;

    // ebp - key pointer, eax, ebx, ecx, edx - each contain 4 bytes (int) of outBlock, esi - 3rd int in key, edi - 4th int in key
    key1 = ebp[0];
    key2 = ebp[1];
    esi = ebp[2];
    edi = ebp[3];

    /* begin rnd_fun */
    *ebx = (*ebx << 16) | (*ebx >> 16);

    // al - 0, ah - 1, bl - 4, bh - 5, cl - 8, ch - 9, dl - 12, dh - 13
    esi ^= eltab(0, cx[0]);
    esi ^= eltab(1, dx[1]);
    esi ^= eltab(3, bx[1]);

    edi ^= eltab(0, dx[0]);
    edi ^= eltab(1, ax[1]);
    edi ^= eltab(2, bx[0]);

    tmp = eltab(0, ax[0]); // ebp (restored later)

    *ebx >>= 16;
    *eax &= 0xffff0000;
    *eax |= *ebx;
    *edx >>= 16;

    tmp ^= eltab(1, ax[1]);
    tmp ^= eltab(3, dx[1]);

    *ebx = eltab(2, dx[0]); // ebx
    *ebx ^= eltab(1, cx[1]);
    *ebx ^= eltab(0, ax[0]);

    *eax >>= 16;
    *ecx >>= 16;

    tmp ^= eltab(2, cx[0]);
    edi ^= eltab(3, cx[1]);
    esi ^= eltab(2, ax[0]);
    *ebx ^= eltab(3, ax[1]);
    /* end rnd_fun */

    *eax = tmp;
    *ecx = esi;
    *edx = edi;

    *eax ^= key1;
    *ebx ^= key2;
}

__device__ AES_RETURN aes_encrypt_c(const u8 *inBlock, u8 *outBlock, void *ks)
{
    uint_32t* kp = (uint_32t *)ks; // key pointer
    uint_32t inf = *(kp + KS_LENGTH);
    uint_32t* o = (uint_32t*)outBlock;
    unsigned int i;

    // xor 4 bytes in inBlock with 4 bytes in ks, 4 times, store result in outBlock
    o[0] = ((uint_32t*)inBlock)[0] ^ kp[0];
    o[1] = ((uint_32t*)inBlock)[1] ^ kp[1];
    o[2] = ((uint_32t*)inBlock)[2] ^ kp[2];
    o[3] = ((uint_32t*)inBlock)[3] ^ kp[3];

    if (inf == 10 * 16 || inf == 12 * 16 || inf == 14 * 16) {
        for (i = 0; i < inf >> 4; i++) {
            kp += 4;

            if (i < (inf >> 4) - 1)
                enc_round_c(&o[0], &o[1], &o[2], &o[3], kp);
            else
                enc_last_round_c(&o[0], &o[1], &o[2], &o[3], kp);
        }
    } else {
        // error
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

#define g0_fun(x) ( mk_tab[0 + 4*extract_byte(x,0)] ^ mk_tab[1 + 4*extract_byte(x,1)] \
                      ^ mk_tab[2 + 4*extract_byte(x,2)] ^ mk_tab[3 + 4*extract_byte(x,3)] )
#define g1_fun(x) ( mk_tab[0 + 4*extract_byte(x,3)] ^ mk_tab[1 + 4*extract_byte(x,0)] \
                      ^ mk_tab[2 + 4*extract_byte(x,1)] ^ mk_tab[3 + 4*extract_byte(x,2)] )

#define f_rnd(i)                                                    \
    t1 = g1_fun(blk[1]); t0 = g0_fun(blk[0]);                       \
    blk[2] = rotr(blk[2] ^ (t0 + t1 + l_key[4 * (i) + 8]), 1);      \
    blk[3] = rotl(blk[3], 1) ^ (t0 + 2 * t1 + l_key[4 * (i) + 9]);  \
    t1 = g1_fun(blk[3]); t0 = g0_fun(blk[2]);                       \
    blk[0] = rotr(blk[0] ^ (t0 + t1 + l_key[4 * (i) + 10]), 1);     \
    blk[1] = rotl(blk[1], 1) ^ (t0 + 2 * t1 + l_key[4 * (i) + 11])

__device__ void twofish_encrypt(TwofishInstance *instance, const u4byte in_blk[4], u4byte out_blk[])
{   u4byte  t0, t1, blk[4];

    u4byte *l_key = instance->l_key;
    u4byte *mk_tab = instance->mk_tab;

    blk[0] = in_blk[0] ^ l_key[0];
    blk[1] = in_blk[1] ^ l_key[1];
    blk[2] = in_blk[2] ^ l_key[2];
    blk[3] = in_blk[3] ^ l_key[3];

    f_rnd(0); f_rnd(1); f_rnd(2); f_rnd(3);
    f_rnd(4); f_rnd(5); f_rnd(6); f_rnd(7);

    out_blk[0] = blk[2] ^ l_key[4];
    out_blk[1] = blk[3] ^ l_key[5];
    out_blk[2] = blk[0] ^ l_key[6];
    out_blk[3] = blk[1] ^ l_key[7];
}

__device__ void KXf (const u32 *k, unsigned int r, u32 *a, u32 *b, u32 *c, u32 *d)
{
    *a ^= k[r];
    *b ^= k[r + 1];
    *c ^= k[r + 2];
    *d ^= k[r + 3];
}

__device__ void LTf (uint32 *a, uint32 *b, uint32 *c, uint32 *d)
{
    *a = rotlFixed(*a, 13);
    *c = rotlFixed(*c, 3);
    *d = rotlFixed(*d ^ *c ^ (*a << 3), 7);
    *b = rotlFixed(*b ^ *a ^ *c, 1);
    *a = rotlFixed(*a ^ *b ^ *d, 5);
    *c = rotlFixed(*c ^ *d ^ (*b << 7), 22);
}

__device__ void serpent_encrypt(const u8 *inBlock, u8 *outBlock, u8 *ks)
{
    u32 a, b, c, d, e;
    unsigned int i=1;
    const u32 *k = (u32 *)ks + 8;
    u32 *in = (u32 *) inBlock;
    u32 *out = (u32 *) outBlock;

    a = in[0];
    b = in[1];
    c = in[2];
    d = in[3];

    do
    {
        KXf (k,  0, &a, &b, &c, &d); S0f (&a, &b, &c, &d, &e); LTf (&b, &e, &c, &a);
        KXf (k,  4, &b, &e, &c, &a); S1f (&b, &e, &c, &a, &d); LTf (&c, &b, &a, &e);
        KXf (k,  8, &c, &b, &a, &e); S2f (&c, &b, &a, &e, &d); LTf (&a, &e, &b, &d);
        KXf (k, 12, &a, &e, &b, &d); S3f (&a, &e, &b, &d, &c); LTf (&e, &b, &d, &c);
        KXf (k, 16, &e, &b, &d, &c); S4f (&e, &b, &d, &c, &a); LTf (&b, &a, &e, &c);
        KXf (k, 20, &b, &a, &e, &c); S5f (&b, &a, &e, &c, &d); LTf (&a, &c, &b, &e);
        KXf (k, 24, &a, &c, &b, &e); S6f (&a, &c, &b, &e, &d); LTf (&a, &c, &d, &b);
        KXf (k, 28, &a, &c, &d, &b); S7f (&a, &c, &d, &b, &e);

        if (i == 4)
            break;

        ++i;
        c = b;
        b = e;
        e = d;
        d = a;
        a = e;
        k += 32;
        LTf (&a,&b,&c,&d);
    }
    while (1);

    KXf (k, 32, &d, &e, &b, &a);
    
    out[0] = d;
    out[1] = e;
    out[2] = b;
    out[3] = a;
}

__device__ void BlowfishEncryptLE (unsigned char *inBlock, unsigned char *outBlock, BF_KEY *key, int encrypt)
{
    word32 left = ((word32 *) inBlock)[0];
    word32 right = ((word32 *) inBlock)[1];

    const word32 *const s = key->sbox;
    const word32 * p = encrypt ? key->pbox : key->pbox_dec;

    unsigned i;

    left ^= p[0];

    for (i=0; i<ROUNDS/2; i++)
    {
        right ^= (((s[GETBYTE(left,3)] + s[256+GETBYTE(left,2)])
              ^ s[2*256+GETBYTE(left,1)]) + s[3*256+GETBYTE(left,0)])
              ^ p[2*i+1];

        left ^= (((s[GETBYTE(right,3)] + s[256+GETBYTE(right,2)])
             ^ s[2*256+GETBYTE(right,1)]) + s[3*256+GETBYTE(right,0)])
             ^ p[2*i+2];
    }

    right ^= p[ROUNDS+1];

    ((word32 *) outBlock)[0] = right;
    ((word32 *) outBlock)[1] = left;
}

__device__ word32 rotlVariable (word32 x, unsigned int y)
{
    return (word32)((x<<y) | (x>>(sizeof(word32)*8-y)));
}

/* Macros to access 8-bit bytes out of a 32-bit word */
#define U8a(x) GETBYTE(x,3)
#define U8b(x) GETBYTE(x,2)
#define U8c(x) GETBYTE(x,1)
#define U8d(x) GETBYTE(x,0)
/* CAST uses three different round functions */
#define f1(l, r, km, kr) \
    t = rotlVariable(km + r, kr); \
    l ^= ((S[0][U8a(t)] ^ S[1][U8b(t)]) - \
     S[2][U8c(t)]) + S[3][U8d(t)];
#undef f2
#define f2(l, r, km, kr) \
    t = rotlVariable(km ^ r, kr); \
    l ^= ((S[0][U8a(t)] - S[1][U8b(t)]) + \
     S[2][U8c(t)]) ^ S[3][U8d(t)];
#undef f3
#define f3(l, r, km, kr) \
    t = rotlVariable(km - r, kr); \
    l ^= ((S[0][U8a(t)] + S[1][U8b(t)]) ^ \
     S[2][U8c(t)]) - S[3][U8d(t)];

#define F1(l, r, i, j) f1(l, r, K[i], K[i+j])
#define F2(l, r, i, j) f2(l, r, K[i], K[i+j])
#define F3(l, r, i, j) f3(l, r, K[i], K[i+j])

__device__ void Cast5Encrypt (const byte *inBlock, byte *outBlock, CAST_KEY *key)
{
    word32 l = BE32 (((word32 *)inBlock)[0]);
    word32 r = BE32 (((word32 *)inBlock)[1]);
    word32 *K = key->K;
    word32 t;

    /* Do the work */
    F1(l, r,  0, 16);
    F2(r, l,  1, 16);
    F3(l, r,  2, 16);
    F1(r, l,  3, 16);
    F2(l, r,  4, 16);
    F3(r, l,  5, 16);
    F1(l, r,  6, 16);
    F2(r, l,  7, 16);
    F3(l, r,  8, 16);
    F1(r, l,  9, 16);
    F2(l, r, 10, 16);
    F3(r, l, 11, 16);
    F1(l, r, 12, 16);
    F2(r, l, 13, 16);
    F3(l, r, 14, 16);
    F1(r, l, 15, 16);

    /* Put l,r into outblock */
    ((word32 *)outBlock)[0] = BE32 (r);
    ((word32 *)outBlock)[1] = BE32 (l);
}

__device__ void RawProcessBlock(word32 *l_, word32 *r_, const word32 *k)

{
    word32 l = *l_, r = *r_;
    const word32 *kptr=k;
    unsigned i;

    for (i=0; i<8; i++)
    {
        word32 work = rotrFixed(r, 4U) ^ kptr[4*i+0];
        l ^= Spbox[6][(work) & 0x3f]
          ^  Spbox[4][(work >> 8) & 0x3f]
          ^  Spbox[2][(work >> 16) & 0x3f]
          ^  Spbox[0][(work >> 24) & 0x3f];
        work = r ^ kptr[4*i+1];
        l ^= Spbox[7][(work) & 0x3f]
          ^  Spbox[5][(work >> 8) & 0x3f]
          ^  Spbox[3][(work >> 16) & 0x3f]
          ^  Spbox[1][(work >> 24) & 0x3f];

        work = rotrFixed(l, 4U) ^ kptr[4*i+2];
        r ^= Spbox[6][(work) & 0x3f]
          ^  Spbox[4][(work >> 8) & 0x3f]
          ^  Spbox[2][(work >> 16) & 0x3f]
          ^  Spbox[0][(work >> 24) & 0x3f];
        work = l ^ kptr[4*i+3];
        r ^= Spbox[7][(work) & 0x3f]
          ^  Spbox[5][(work >> 8) & 0x3f]
          ^  Spbox[3][(work >> 16) & 0x3f]
          ^  Spbox[1][(work >> 24) & 0x3f];
    }

    *l_ = l; *r_ = r;
}

__device__ void TripleDesEncrypt (byte *inBlock, byte *outBlock, TDES_KEY *key, int encrypt)
{
    word32 left = BE32 (((word32 *)inBlock)[0]);
    word32 right = BE32 (((word32 *)inBlock)[1]);
    word32 work;

    right = rotlFixed(right, 4U);
    work = (left ^ right) & 0xf0f0f0f0;
    left ^= work;
    right = rotrFixed(right^work, 20U);
    work = (left ^ right) & 0xffff0000;
    left ^= work;
    right = rotrFixed(right^work, 18U);
    work = (left ^ right) & 0x33333333;
    left ^= work;
    right = rotrFixed(right^work, 6U);
    work = (left ^ right) & 0x00ff00ff;
    left ^= work;
    right = rotlFixed(right^work, 9U);
    work = (left ^ right) & 0xaaaaaaaa;
    left = rotlFixed(left^work, 1U);
    right ^= work;

    RawProcessBlock (&left, &right, encrypt ? key->k1 : key->k1d);
    RawProcessBlock (&right, &left, !encrypt ? key->k2 : key->k2d);
    RawProcessBlock (&left, &right, encrypt ? key->k3 : key->k3d);
    
    right = rotrFixed(right, 1U);
    work = (left ^ right) & 0xaaaaaaaa;
    right ^= work;
    left = rotrFixed(left^work, 9U);
    work = (left ^ right) & 0x00ff00ff;
    right ^= work;
    left = rotlFixed(left^work, 6U);
    work = (left ^ right) & 0x33333333;
    right ^= work;
    left = rotlFixed(left^work, 18U);
    work = (left ^ right) & 0xffff0000;
    right ^= work;
    left = rotlFixed(left^work, 20U);
    work = (left ^ right) & 0xf0f0f0f0;
    right ^= work;
    left = rotrFixed(left^work, 4U);

    ((word32 *)outBlock)[0] = BE32 (right);
    ((word32 *)outBlock)[1] = BE32 (left);
}

__device__ void EncipherBlock(int cipher, void *data, void *ks)
{
    switch (cipher)
    {
    case AES:	
         aes_encrypt_c ((u8*)data, (u8*)data, ks);
        break;

    case TWOFISH:		twofish_encrypt ((TwofishInstance*)ks, (u4byte*)data, (u4byte*)data); break;
    case SERPENT:		serpent_encrypt ((u8*)data, (u8*)data, (u8*)ks); break;
    case BLOWFISH:		BlowfishEncryptLE ((unsigned char*)data, (unsigned char*)data, (BF_KEY*)ks, 1); break;	// Deprecated/legacy
    case CAST:			Cast5Encrypt ((byte*)data, (byte*)data, (CAST_KEY*)ks); break;			// Deprecated/legacy
    case TRIPLEDES:		TripleDesEncrypt ((byte*)data, (byte*)data, (TDES_KEY*)ks, 1); break;	// Deprecated/legacy
    default:			TC_THROW_FATAL_EXCEPTION;	// Unknown/wrong ID
    }
}

__device__ void ILTf (uint32 *a, uint32 *b, uint32 *c, uint32 *d)
{ 
    *c = rotrFixed(*c, 22);
    *a = rotrFixed(*a, 5);
    *c ^= *d ^ (*b << 7);
    *a ^= *b ^ *d;
    *b = rotrFixed(*b, 1);
    *d = rotrFixed(*d, 7) ^ *c ^ (*a << 3);
    *b ^= *a ^ *c;
    *c = rotrFixed(*c, 3);
    *a = rotrFixed(*a, 13);
}

// order of output from S-box functions
#define beforeS0(f) f(0,a,b,c,d,e)
#define afterS0(f) f(1,b,e,c,a,d)
#define afterS1(f) f(2,c,b,a,e,d)
#define afterS2(f) f(3,a,e,b,d,c)
#define afterS3(f) f(4,e,b,d,c,a)
#define afterS4(f) f(5,b,a,e,c,d)
#define afterS5(f) f(6,a,c,b,e,d)
#define afterS6(f) f(7,a,c,d,b,e)
#define afterS7(f) f(8,d,e,b,a,c)

// order of output from inverse S-box functions
#define beforeI7(f) f(8,a,b,c,d,e)
#define afterI7(f) f(7,d,a,b,e,c)
#define afterI6(f) f(6,a,b,c,e,d)
#define afterI5(f) f(5,b,d,e,c,a)
#define afterI4(f) f(4,b,c,e,a,d)
#define afterI3(f) f(3,a,b,e,c,d)
#define afterI2(f) f(2,b,d,e,c,a)
#define afterI1(f) f(1,a,b,c,e,d)
#define afterI0(f) f(0,a,d,b,e,c)

// inverse linear transformation
#define ILT(i,a,b,c,d,e)	{\
    c = rotrFixed(c, 22);	\
    a = rotrFixed(a, 5); 	\
    c ^= d ^ (b << 7);	\
    a ^= b ^ d; 		\
    b = rotrFixed(b, 1); 	\
    d = rotrFixed(d, 7) ^ c ^ (a << 3);	\
    b ^= a ^ c; 		\
    c = rotrFixed(c, 3); 	\
    a = rotrFixed(a, 13);}

#define I0(i, r0, r1, r2, r3, r4) \
       {           \
    r2 = ~r2;      \
    r4 = r1;   \
    r1 |= r0;   \
    r4 = ~r4;      \
    r1 ^= r2;   \
    r2 |= r4;   \
    r1 ^= r3;   \
    r0 ^= r4;   \
    r2 ^= r0;   \
    r0 &= r3;   \
    r4 ^= r0;   \
    r0 |= r1;   \
    r0 ^= r2;   \
    r3 ^= r4;   \
    r2 ^= r1;   \
    r3 ^= r0;   \
    r3 ^= r1;   \
    r2 &= r3;   \
    r4 ^= r2;   \
            }

#define I1(i, r0, r1, r2, r3, r4) \
       {           \
    r4 = r1;   \
    r1 ^= r3;   \
    r3 &= r1;   \
    r4 ^= r2;   \
    r3 ^= r0;   \
    r0 |= r1;   \
    r2 ^= r3;   \
    r0 ^= r4;   \
    r0 |= r2;   \
    r1 ^= r3;   \
    r0 ^= r1;   \
    r1 |= r3;   \
    r1 ^= r0;   \
    r4 = ~r4;      \
    r4 ^= r1;   \
    r1 |= r0;   \
    r1 ^= r0;   \
    r1 |= r4;   \
    r3 ^= r1;   \
            }

#define I2(i, r0, r1, r2, r3, r4) \
       {           \
    r2 ^= r3;   \
    r3 ^= r0;   \
    r4 = r3;   \
    r3 &= r2;   \
    r3 ^= r1;   \
    r1 |= r2;   \
    r1 ^= r4;   \
    r4 &= r3;   \
    r2 ^= r3;   \
    r4 &= r0;   \
    r4 ^= r2;   \
    r2 &= r1;   \
    r2 |= r0;   \
    r3 = ~r3;      \
    r2 ^= r3;   \
    r0 ^= r3;   \
    r0 &= r1;   \
    r3 ^= r4;   \
    r3 ^= r0;   \
            }

#define I3(i, r0, r1, r2, r3, r4) \
       {           \
    r4 = r2;   \
    r2 ^= r1;   \
    r1 &= r2;   \
    r1 ^= r0;   \
    r0 &= r4;   \
    r4 ^= r3;   \
    r3 |= r1;   \
    r3 ^= r2;   \
    r0 ^= r4;   \
    r2 ^= r0;   \
    r0 |= r3;   \
    r0 ^= r1;   \
    r4 ^= r2;   \
    r2 &= r3;   \
    r1 |= r3;   \
    r1 ^= r2;   \
    r4 ^= r0;   \
    r2 ^= r4;   \
            }

#define I4(i, r0, r1, r2, r3, r4) \
       {           \
    r4 = r2;   \
    r2 &= r3;   \
    r2 ^= r1;   \
    r1 |= r3;   \
    r1 &= r0;   \
    r4 ^= r2;   \
    r4 ^= r1;   \
    r1 &= r2;   \
    r0 = ~r0;      \
    r3 ^= r4;   \
    r1 ^= r3;   \
    r3 &= r0;   \
    r3 ^= r2;   \
    r0 ^= r1;   \
    r2 &= r0;   \
    r3 ^= r0;   \
    r2 ^= r4;   \
    r2 |= r3;   \
    r3 ^= r0;   \
    r2 ^= r1;   \
            }

#define I5(i, r0, r1, r2, r3, r4) \
       {           \
    r1 = ~r1;      \
    r4 = r3;   \
    r2 ^= r1;   \
    r3 |= r0;   \
    r3 ^= r2;   \
    r2 |= r1;   \
    r2 &= r0;   \
    r4 ^= r3;   \
    r2 ^= r4;   \
    r4 |= r0;   \
    r4 ^= r1;   \
    r1 &= r2;   \
    r1 ^= r3;   \
    r4 ^= r2;   \
    r3 &= r4;   \
    r4 ^= r1;   \
    r3 ^= r0;   \
    r3 ^= r4;   \
    r4 = ~r4;      \
            }

#define I6(i, r0, r1, r2, r3, r4) \
       {           \
    r0 ^= r2;   \
    r4 = r2;   \
    r2 &= r0;   \
    r4 ^= r3;   \
    r2 = ~r2;      \
    r3 ^= r1;   \
    r2 ^= r3;   \
    r4 |= r0;   \
    r0 ^= r2;   \
    r3 ^= r4;   \
    r4 ^= r1;   \
    r1 &= r3;   \
    r1 ^= r0;   \
    r0 ^= r3;   \
    r0 |= r2;   \
    r3 ^= r1;   \
    r4 ^= r0;   \
            }

#define I7(i, r0, r1, r2, r3, r4) \
       {           \
    r4 = r2;   \
    r2 ^= r0;   \
    r0 &= r3;   \
    r2 = ~r2;      \
    r4 |= r3;   \
    r3 ^= r1;   \
    r1 |= r0;   \
    r0 ^= r2;   \
    r2 &= r4;   \
    r1 ^= r2;   \
    r2 ^= r0;   \
    r0 |= r2;   \
    r3 &= r4;   \
    r0 ^= r3;   \
    r4 ^= r1;   \
    r3 ^= r4;   \
    r4 |= r0;   \
    r3 ^= r2;   \
    r4 ^= r2;   \
            }

__device__ void serpent_decrypt(const u8 *inBlock, u8 *outBlock, u8 *ks)
{
    u32 a, b, c, d, e;
    const u32 *k = (u32 *)ks + 104;
    unsigned int i=4;
    u32 *in = (u32 *) inBlock;
    u32 *out = (u32 *) outBlock;

    a = in[0];
    b = in[1];
    c = in[2];
    d = in[3];

    KXf (k, 32, &a, &b, &c, &d);
    goto start;

    do
    {
        c = b;
        b = d;
        d = e;
        k -= 32;
        beforeI7(ILT);
start:
        beforeI7(I7); KXf (k, 28, &d, &a, &b, &e);
        ILTf (&d, &a, &b, &e); afterI7(I6); KXf (k, 24, &a, &b, &c, &e); 
        ILTf (&a, &b, &c, &e); afterI6(I5); KXf (k, 20, &b, &d, &e, &c); 
        ILTf (&b, &d, &e, &c); afterI5(I4); KXf (k, 16, &b, &c, &e, &a); 
        ILTf (&b, &c, &e, &a); afterI4(I3); KXf (k, 12, &a, &b, &e, &c);
        ILTf (&a, &b, &e, &c); afterI3(I2); KXf (k, 8,  &b, &d, &e, &c);
        ILTf (&b, &d, &e, &c); afterI2(I1); KXf (k, 4,  &a, &b, &c, &e);
        ILTf (&a, &b, &c, &e); afterI1(I0); KXf (k, 0,  &a, &d, &b, &e);
    }
    while (--i != 0);
    
    out[0] = a;
    out[1] = d;
    out[2] = b;
    out[3] = e;
}

#define i_rnd(i)                                                        \
        t1 = g1_fun(blk[1]); t0 = g0_fun(blk[0]);                       \
        blk[2] = rotl(blk[2], 1) ^ (t0 + t1 + l_key[4 * (i) + 10]);     \
        blk[3] = rotr(blk[3] ^ (t0 + 2 * t1 + l_key[4 * (i) + 11]), 1); \
        t1 = g1_fun(blk[3]); t0 = g0_fun(blk[2]);                       \
        blk[0] = rotl(blk[0], 1) ^ (t0 + t1 + l_key[4 * (i) +  8]);     \
        blk[1] = rotr(blk[1] ^ (t0 + 2 * t1 + l_key[4 * (i) +  9]), 1)

__device__ void twofish_decrypt(TwofishInstance *instance, const u4byte in_blk[4], u4byte out_blk[4])
{   u4byte  t0, t1, blk[4];

    u4byte *l_key = instance->l_key;
    u4byte *mk_tab = instance->mk_tab;

    blk[0] = in_blk[0] ^ l_key[4];
    blk[1] = in_blk[1] ^ l_key[5];
    blk[2] = in_blk[2] ^ l_key[6];
    blk[3] = in_blk[3] ^ l_key[7];

    i_rnd(7); i_rnd(6); i_rnd(5); i_rnd(4);
    i_rnd(3); i_rnd(2); i_rnd(1); i_rnd(0);

    out_blk[0] = blk[2] ^ l_key[0];
    out_blk[1] = blk[3] ^ l_key[1];
    out_blk[2] = blk[0] ^ l_key[2];
    out_blk[3] = blk[1] ^ l_key[3]; 
}

#define AES_REV_DKS

#define dtab_0(x)   t_in[0][x]
#define dtab_1(x)   t_in[1][x]
#define dtab_2(x)   t_in[2][x]
#define dtab_3(x)   t_in[3][x]
#define dltab_0(x)  t_il[0][x]
#define dltab_1(x)  t_il[1][x]
#define dltab_2(x)  t_il[2][x]
#define dltab_3(x)  t_il[3][x]
#define dltab(n, x) dltab_##n(x)

__device__ void dec_round_c (uint_32t* eax, uint_32t* ebx, uint_32t* ecx, uint_32t* edx, uint_32t* ebp) {
    uint_32t key1, key2, tmp, esi, edi;
    uint_8t* ax = (uint_8t*)eax, *bx = (uint_8t*)ebx, *cx = (uint_8t*)ecx, *dx = (uint_8t*)edx;

    // ebp - key pointer, eax, ebx, ecx, edx - each contain 4 bytes (int) of outBlock, esi - 3rd int in key, edi - 4th int in key
    key1 = ebp[0];
    key2 = ebp[1];
    esi = ebp[2];
    edi = ebp[3];

    /* begin irn_fun */
    *eax = (*eax << 16) | (*eax >> 16);

    // al - 0, ah - 1, bl - 4, bh - 5, cl - 8, ch - 9, dl - 12, dh - 13
    esi ^= dtab_0(cx[0]);
    esi ^= dtab_1(bx[1]);
    esi ^= dtab_2(ax[0]);

    edi ^= dtab_0(dx[0]);
    edi ^= dtab_1(cx[1]);
    edi ^= dtab_3(ax[1]);

    tmp = dtab_0(bx[0]); // ebp (restored later)

    *eax >>= 16;
    *ebx &= 0xffff0000;
    *ebx |= *eax;
    *ecx >>= 16;

    tmp ^= dtab_1(bx[1]);
    tmp ^= dtab_3(cx[1]);

    *eax = dtab_2(cx[0]); // eax
    *eax ^= dtab_0(bx[0]);
    *eax ^= dtab_1(dx[1]);

    *ebx >>= 16;
    *edx >>= 16;

    esi ^= dtab_3(dx[1]);
    tmp ^= dtab_2(dx[0]);
    *eax ^= dtab_3(bx[1]);
    edi ^= dtab_2(bx[0]);
    /* end irn_fun */

    *ebx = tmp;
    *ecx = esi;
    *edx = edi;

    *eax ^= key1;
    *ebx ^= key2;
}

__device__ void dec_last_round_c (uint_32t* eax, uint_32t* ebx, uint_32t* ecx, uint_32t* edx, uint_32t* ebp) {
    uint_32t key1, key2, tmp, esi, edi;
    uint_8t* ax = (uint_8t*)eax, *bx = (uint_8t*)ebx, *cx = (uint_8t*)ecx, *dx = (uint_8t*)edx;

    // ebp - key pointer, eax, ebx, ecx, edx - each contain 4 bytes (int) of outBlock, esi - 3rd int in key, edi - 4th int in key
    key1 = ebp[0];
    key2 = ebp[1];
    esi = ebp[2];
    edi = ebp[3];

    /* begin irn_fun */
    *eax = (*eax << 16) | (*eax >> 16);

    // al - 0, ah - 1, bl - 4, bh - 5, cl - 8, ch - 9, dl - 12, dh - 13
    esi ^= dltab(0, cx[0]);
    esi ^= dltab(1, bx[1]);
    esi ^= dltab(2, ax[0]);

    edi ^= dltab(0, dx[0]);
    edi ^= dltab(1, cx[1]);
    edi ^= dltab(3, ax[1]);

    tmp = dltab(0, bx[0]); // ebp (restored later)

    *eax >>= 16;
    *ebx &= 0xffff0000;
    *ebx |= *eax;
    *ecx >>= 16;

    tmp ^= dltab(1, bx[1]);
    tmp ^= dltab(3, cx[1]);

    *eax = dltab(2, cx[0]); // eax
    *eax ^= dltab(0, bx[0]);
    *eax ^= dltab(1, dx[1]);

    *ebx >>= 16;
    *edx >>= 16;

    esi ^= dltab(3, dx[1]);
    tmp ^= dltab(2, dx[0]);
    *eax ^= dltab(3, bx[1]);
    edi ^= dltab(2, bx[0]);
    /* end irn_fun */

    *ebx = tmp;
    *ecx = esi;
    *edx = edi;

    *eax ^= key1;
    *ebx ^= key2;
}

__device__ AES_RETURN aes_decrypt_c(const u8 *inBlock, u8 *outBlock, void *ks)
{
    uint_32t* kp = (uint_32t *)ks; // key pointer
    uint_32t inf = *(kp + KS_LENGTH);
    uint_32t* o = (uint_32t*)outBlock;
    unsigned int i;

#ifndef  AES_REV_DKS
    kp += inf >> 2;
#endif

    // xor 4 bytes in inBlock with 4 bytes in ks, 4 times, store result in outBlock
    o[0] = ((uint_32t*)inBlock)[0] ^ kp[0];
    o[1] = ((uint_32t*)inBlock)[1] ^ kp[1];
    o[2] = ((uint_32t*)inBlock)[2] ^ kp[2];
    o[3] = ((uint_32t*)inBlock)[3] ^ kp[3];

    if (inf == 10 * 16 || inf == 12 * 16 || inf == 14 * 16) {
        for (i = 0; i < inf >> 4; i++) {
            #ifdef  AES_REV_DKS
                kp += 4;
            #else
                kp -= 4;
            #endif
            if (i < (inf >> 4) - 1)
                dec_round_c(&o[0], &o[1], &o[2], &o[3], kp);
            else
                dec_last_round_c(&o[0], &o[1], &o[2], &o[3], kp);
        }
    } else {
        // error
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

__device__ void Cast5Decrypt (const byte *inBlock, byte *outBlock, CAST_KEY *key)
{
    word32 r = BE32 (((word32 *)inBlock)[0]);
    word32 l = BE32 (((word32 *)inBlock)[1]);
    word32 *K = key->K;
    word32 t;

    /* Only do full 16 rounds if key length > 80 bits */
    F1(r, l, 15, 16);
    F3(l, r, 14, 16);
    F2(r, l, 13, 16);
    F1(l, r, 12, 16);
    F3(r, l, 11, 16);
    F2(l, r, 10, 16);
    F1(r, l,  9, 16);
    F3(l, r,  8, 16);
    F2(r, l,  7, 16);
    F1(l, r,  6, 16);
    F3(r, l,  5, 16);
    F2(l, r,  4, 16);
    F1(r, l,  3, 16);
    F3(l, r,  2, 16);
    F2(r, l,  1, 16);
    F1(l, r,  0, 16);
    /* Put l,r into outblock */
    ((word32 *)outBlock)[0] = BE32 (l);
    ((word32 *)outBlock)[1] = BE32 (r);
    /* Wipe clean */
    t = l = r = 0;
}

__device__ void DecipherBlock(int cipher, void *data, void *ks)
{
    switch (cipher)
    {
    case SERPENT:	serpent_decrypt ((u8*)data, (u8*)data, (u8*)ks); break;
    case TWOFISH:	twofish_decrypt ((TwofishInstance*)ks, (u4byte*)data, (u4byte*)data); break;

    case AES:
    {
        aes_decrypt_c ((u8*)data, (u8*)data, (void *) ((char *) ks + sizeof(aes_encrypt_ctx)));
        break;
    }

    case BLOWFISH:	BlowfishEncryptLE ((unsigned char*)data, (unsigned char*)data, (BF_KEY*)ks, 0); break;	// Deprecated/legacy
    case CAST:		Cast5Decrypt ((byte*)data, (byte*)data, (CAST_KEY*)ks); break;			// Deprecated/legacy
    case TRIPLEDES:	TripleDesEncrypt ((byte*)data, (byte*)data, (TDES_KEY*)ks, 0); break;	// Deprecated/legacy

    default:		TC_THROW_FATAL_EXCEPTION;	// Unknown/wrong ID
    }
}

__device__ void DecryptBufferXTS (u8 *buffer,
                       TC_LARGEST_COMPILER_UINT length,
                       const UINT64_STRUCT *startDataUnitNo,



                       unsigned int startCipherBlockNo,
                       u8 *ks,
                       u8 *ks2,
                       int cipher)
{
    u8 finalCarry;
    ALIGN(32) u8 whiteningValue [BYTES_PER_XTS_BLOCK];
    ALIGN(32) u8 byteBufUnitNo [BYTES_PER_XTS_BLOCK];
    u64 *whiteningValuePtr64 = (u64 *) whiteningValue;
    u64 *bufPtr = (u64 *) buffer;
    unsigned int startBlock = startCipherBlockNo, endBlock, block;
    TC_LARGEST_COMPILER_UINT blockCount, dataUnitNo;

    // Convert the 64-bit data unit number into a little-endian 16-byte array. 
    // Note that as we are converting a 64-bit number into a 16-byte array we can always zero the last 8 bytes.
    dataUnitNo = startDataUnitNo->Value;
    *((u64 *) byteBufUnitNo) = dataUnitNo;
    *((u64 *) byteBufUnitNo + 1) = 0;

    if (length % BYTES_PER_XTS_BLOCK)
        TC_THROW_FATAL_EXCEPTION;

    blockCount = length / BYTES_PER_XTS_BLOCK;

    // Process all blocks in the buffer
    while (blockCount > 0)
    {
        if (blockCount < BLOCKS_PER_XTS_DATA_UNIT)
            endBlock = startBlock + (unsigned int) blockCount;
        else
            endBlock = BLOCKS_PER_XTS_DATA_UNIT;

        whiteningValuePtr64 = (u64 *) whiteningValue;

        // Encrypt the data unit number using the secondary key (in order to generate the first 
        // whitening value for this data unit)
        *whiteningValuePtr64 = *((u64 *) byteBufUnitNo);
        *(whiteningValuePtr64 + 1) = 0;
        EncipherBlock (cipher, whiteningValue, ks2);

        // Generate (and apply) subsequent whitening values for blocks in this data unit and
        // decrypt all relevant blocks in this data unit
        for (block = 0; block < endBlock; block++)
        {
            if (block >= startBlock)
            {
                // Post-whitening
                *bufPtr++ ^= *whiteningValuePtr64++;
                *bufPtr-- ^= *whiteningValuePtr64--;

                // Actual decryption
                DecipherBlock (cipher, bufPtr, ks);

                // Pre-whitening
                *bufPtr++ ^= *whiteningValuePtr64++;
                *bufPtr++ ^= *whiteningValuePtr64;
            }
            else
                whiteningValuePtr64++;

            // Derive the next whitening value

            finalCarry = 
                (*whiteningValuePtr64 & 0x8000000000000000) ?
                135 : 0;

            *whiteningValuePtr64-- <<= 1;

            if (*whiteningValuePtr64 & 0x8000000000000000)
                *(whiteningValuePtr64 + 1) |= 1;	

            *whiteningValuePtr64 <<= 1;

            whiteningValue[0] ^= finalCarry;
        }

        blockCount -= endBlock - startBlock;
        startBlock = 0;
        dataUnitNo++;
        *((u64 *) byteBufUnitNo) = dataUnitNo;
    }

    FAST_ERASE64 (whiteningValue, sizeof (whiteningValue));
}

__device__ void Xor128 (u64 *a, u64 *b)
{
    *a++ ^= *b++;
    *a ^= *b;
}

__device__ void Xor64 (u64 *a, u64 *b)
{
    *a ^= *b;
}

#define lp32(x)      ((mode(32t)*)(x))

__device__ in_line void move_block_aligned( void *p, const void *q)
{
    lp32(p)[0] = lp32(q)[0], lp32(p)[1] = lp32(q)[1],
    lp32(p)[2] = lp32(q)[2], lp32(p)[3] = lp32(q)[3];
}

__device__ in_line void move_block_aligned64( void *p, const void *q)
{
    lp32(p)[0] = lp32(q)[0], lp32(p)[1] = lp32(q)[1];
}

__device__ in_line void xor_block_aligned( void *p, const void *q)
{
    lp32(p)[0] ^= lp32(q)[0], lp32(p)[1] ^= lp32(q)[1],
    lp32(p)[2] ^= lp32(q)[2], lp32(p)[3] ^= lp32(q)[3];
}

__device__ in_line void xor_block_aligned64( void *p, const void *q)
{
    lp32(p)[0] ^= lp32(q)[0], lp32(p)[1] ^= lp32(q)[1];
}

#define xor_8kt64(i)   \
    xor_block_aligned(r, ctx->gf_t128[i + i][a[i] & 15]); \
    xor_block_aligned(r, ctx->gf_t128[i + i + 1][a[i] >> 4])

/* Multiply a 128-bit number by a 64-bit number in the finite field GF(2^128) */
__device__ void Gf128MulBy64Tab (u8 a[8], u8 p[16], GfCtx *ctx)
{  
    ALIGN(32) u32 r[CBLK_LEN >> 2];

    move_block_aligned(r, ctx->gf_t128[7*2][a[7] & 15]);
    xor_block_aligned(r,  ctx->gf_t128[7*2+1][a[7] >> 4]);

    if (*(u16 *)a)
    {
        xor_8kt64(0);
        xor_8kt64(1);
    }
    if (a[2])
    {
        xor_8kt64(2);
    }
    xor_8kt64(3);
    xor_8kt64(4);
    xor_8kt64(5);
    xor_8kt64(6);

    move_block_aligned(p, r);
}

#define xor_8k64(i)   \
    xor_block_aligned64(r, ctx->gf_t64[i + i][a[i] & 15]); \
    xor_block_aligned64(r, ctx->gf_t64[i + i + 1][a[i] >> 4])

/* Multiply two 64-bit numbers in the finite field GF(2^64) */
__device__ void Gf64MulTab (unsigned char a[8], unsigned char p[8], GfCtx *ctx)
{  
    /* Deprecated/legacy */

    ALIGN(32) u32 r[CBLK_LEN8 >> 2];

    move_block_aligned64(r, ctx->gf_t64[7*2][a[7] & 15]);
    xor_block_aligned64(r,  ctx->gf_t64[7*2+1][a[7] >> 4]);

    if (*(u16 *)a)
    {
        xor_8k64(0);
        xor_8k64(1);
    }
    if (a[2])
    {
        xor_8k64(2);
    }
    xor_8k64(3);
    xor_8k64(4);
    xor_8k64(5);
    xor_8k64(6);

    move_block_aligned64(p, r);
}

__device__ void DecryptBufferLRW128 (byte *buffer, uint64 length, uint64 blockIndex, PCRYPTO_INFO cryptoInfo)
{
    /* Deprecated/legacy */

    int cipher = EAGetFirstCipher (cryptoInfo->ea);
    int cipherCount = EAGetCipherCount (cryptoInfo->ea);
    u8 *p = buffer;
    u8 *ks = cryptoInfo->ks;
    ALIGN(32) u8 i[8];
    ALIGN(32) u8 t[16];
    u64 b;
    u8 j;

    *(u64 *)i = BE64(blockIndex);

    if (length % 16)
        TC_THROW_FATAL_EXCEPTION;

    // Note that the maximum supported volume size is 8589934592 GB  (i.e., 2^63 bytes).

    for (b = 0; b < length >> 4; b++)
    {
        Gf128MulBy64Tab (i, t, &cryptoInfo->gf_ctx);
        Xor128 ((u64 *)p, (u64 *)t);

        if (cipherCount > 1)

        {
            // Cipher cascade
            ks = cryptoInfo->ks + EAGetKeyScheduleSize (cryptoInfo->ea);

            for (cipher = EAGetLastCipher (cryptoInfo->ea);
                cipher != 0;
                cipher = EAGetPreviousCipher (cryptoInfo->ea, cipher))
            {
                ks -= CipherGetKeyScheduleSize (cipher);
                DecipherBlock (cipher, p, ks);
            }
        }
        else
        {
            DecipherBlock (cipher, p, ks);
        }

        Xor128 ((u64 *)p, (u64 *)t);

        p += 16;

        if (i[7] != 0xff)
        {
            j = i[7];
            *(u64 *)i &= ~((j & 0xffffffffffffffff) << 56);
            j++;
            *(u64 *)i |= (j & 0xffffffffffffffff) << 56;
        }
        else
            *(u64 *)i = BE64 ( BE64(*(u64 *)i) + 1 );
    }

    FAST_ERASE64 (t, sizeof(t));
}

__device__ void DecryptBufferLRW64 (byte *buffer, uint64 length, uint64 blockIndex, PCRYPTO_INFO cryptoInfo)
{
    /* Deprecated/legacy */

    int cipher = EAGetFirstCipher (cryptoInfo->ea);
    u8 *p = buffer;
    u8 *ks = cryptoInfo->ks;
    ALIGN(32) u8 i[8];
    ALIGN(32) u8 t[8];
    u64 b;
    u8 j;

    *(u64 *)i = BE64(blockIndex);

    if (length % 8)
        TC_THROW_FATAL_EXCEPTION;

    for (b = 0; b < length >> 3; b++)
    {
        Gf64MulTab (i, t, &cryptoInfo->gf_ctx);
        Xor64 ((u64 *)p, (u64 *)t);

        DecipherBlock (cipher, p, ks);

        Xor64 ((u64 *)p, (u64 *)t);

        p += 8;

        if (i[7] != 0xff)
        {
            j = i[7];
            *(u64 *)i &= ~((j & 0xffffffffffffffff) << 56);
            j++;
            *(u64 *)i |= (j & 0xffffffffffffffff) << 56;
        }
        else
            *(u64 *)i = BE64 ( BE64(*(u64 *)i) + 1 );
    }

    FAST_ERASE64 (t, sizeof(t));
}

__device__ void DecryptBufferCBC (u32 *data,
         unsigned int len,
         u8 *ks,
         u32 *iv,
         u32 *whitening,
         int ea,
         int cipher)
{

    /* IMPORTANT: This function has been deprecated (legacy) */

    u32 bufIV[4];
    u64 i;
    u32 ct[4];
    int blockSize = CipherGetBlockSize (ea != 0 ? EAGetFirstCipher (ea) : cipher);

    if (len % blockSize)
        TC_THROW_FATAL_EXCEPTION;

    //  IV
    bufIV[0] = iv[0];
    bufIV[1] = iv[1];
    if (blockSize == 16)
    {
        bufIV[2] = iv[2];
        bufIV[3] = iv[3];
    }

    // Decrypt each block
    for (i = 0; i < len/blockSize; i++)
    {
        // Dewhitening
        data[0] ^= whitening[0];
        data[1] ^= whitening[1];
        if (blockSize == 16)
        {
            data[2] ^= whitening[0];
            data[3] ^= whitening[1];
        }

        // CBC
        ct[0] = data[0];
        ct[1] = data[1];
        if (blockSize == 16)
        {
            ct[2] = data[2];
            ct[3] = data[3];
        }

        if (ea != 0)
        {
            // Outer-CBC
            ks += EAGetKeyScheduleSize (ea);
            for (cipher = EAGetLastCipher (ea); cipher != 0; cipher = EAGetPreviousCipher (ea, cipher))
            {
                ks -= CipherGetKeyScheduleSize (cipher);
                DecipherBlock (cipher, data, ks);
            }
        }
        else
        {
            // CBC/inner-CBC
            DecipherBlock (cipher, data, ks);
        }

        // CBC
        data[0] ^= bufIV[0];
        data[1] ^= bufIV[1];
        bufIV[0] = ct[0];
        bufIV[1] = ct[1];
        if (blockSize == 16)
        {
            data[2] ^= bufIV[2];
            data[3] ^= bufIV[3];
            bufIV[2] = ct[2];
            bufIV[3] = ct[3];
        }

        data += blockSize / sizeof(*data);
    }
}

// DecryptBuffer
//
// buf:  data to be decrypted; the start of the buffer is assumed to be aligned with the start of a data unit.
// len:  number of bytes to decrypt; must be divisible by the block size (for cascaded ciphers, divisible 
//       by the largest block size used within the cascade)
__device__ void DecryptBuffer (u8 *buf, TC_LARGEST_COMPILER_UINT len, PCRYPTO_INFO cryptoInfo)
{
    switch (cryptoInfo->mode)
    {
    case XTS:
        {
            u8 *ks = cryptoInfo->ks + EAGetKeyScheduleSize (cryptoInfo->ea);
            u8 *ks2 = cryptoInfo->ks2 + EAGetKeyScheduleSize (cryptoInfo->ea);
            UINT64_STRUCT dataUnitNo;
            int cipher;

            // When encrypting/decrypting a buffer (typically a volume header) the sequential number
            // of the first XTS data unit in the buffer is always 0 and the start of the buffer is
            // always assumed to be aligned with the start of the data unit 0.
            dataUnitNo.LowPart = 0;
            dataUnitNo.HighPart = 0;

            for (cipher = EAGetLastCipher (cryptoInfo->ea);
                cipher != 0;
                cipher = EAGetPreviousCipher (cryptoInfo->ea, cipher))
            {
                ks -= CipherGetKeyScheduleSize (cipher);
                ks2 -= CipherGetKeyScheduleSize (cipher);

                DecryptBufferXTS (buf, len, &dataUnitNo, 0, ks, ks2, cipher);
            }
        }
        break;

    case LRW:

        /* Deprecated/legacy */

        switch (CipherGetBlockSize (EAGetFirstCipher (cryptoInfo->ea)))
        {
        case 8:
            DecryptBufferLRW64 (buf, (u64) len, 1, cryptoInfo);
            break;

        case 16:
            DecryptBufferLRW128 (buf, (u64) len, 1, cryptoInfo);
            break;

        default:
            TC_THROW_FATAL_EXCEPTION;
        }
        break;

    case CBC:
    case INNER_CBC:
        {
            /* Deprecated/legacy */

            u8 *ks = cryptoInfo->ks + EAGetKeyScheduleSize (cryptoInfo->ea);
            int cipher;
            for (cipher = EAGetLastCipher (cryptoInfo->ea);
                cipher != 0;
                cipher = EAGetPreviousCipher (cryptoInfo->ea, cipher))
            {
                ks -= CipherGetKeyScheduleSize (cipher);

                DecryptBufferCBC ((u32 *) buf,
                    (unsigned int) len,
                    ks,
                    (u32 *) cryptoInfo->k2,
                    (u32 *) &cryptoInfo->k2[8],
                    0,
                    cipher);
            }
        }
        break;

    case OUTER_CBC:

        /* Deprecated/legacy */

        DecryptBufferCBC ((u32 *) buf,
            (unsigned int) len,
            cryptoInfo->ks,
            (u32 *) cryptoInfo->k2,
            (u32 *) &cryptoInfo->k2[8],
            cryptoInfo->ea,
            0);

        break;

    default:		
        // Unknown/wrong ID
        TC_THROW_FATAL_EXCEPTION;
    }
}

#define keyInfo td->keyInfo
#define cryptoInfo (&td->cryptoInfo)

// --opencc-options -OPT:Olimit=180335 for optimization
// dk is the expected input for the next phase
// the kernel was split into 5 smaller kernels, because otherwise ptxas gives a memory allocation error
// x passwords are processed at a time, going through each of the five kernels, and then the process is repeated
// for the next x passwords until all passwords have been processed

#define dk td->dk[RIPEMD160 - 1]
// 9.4 seconds
__global__ static void reduceKernel_ripemd160 (char* d_EncryptedHeader, Password* d_Input, PTHREAD_DATA d_Output)
{
    const int     tid = blockIdx.x * blockDim.x + threadIdx.x;
    PTHREAD_DATA td = &d_Output[tid]; 

    crypto_loadkey (&keyInfo, (char*)d_Input[tid].Text, (int) d_Input[tid].Length);


    // PKCS5 is used to derive the primary header key(s) and secondary header key(s) (XTS mode) from the password
    memcpy (keyInfo.salt, d_EncryptedHeader + HEADER_SALT_OFFSET, PKCS5_SALT_SIZE);

    keyInfo.noIterations = get_pkcs5_iteration_count (RIPEMD160, 0);
    derive_key_ripemd160 (keyInfo.userKey, keyInfo.keyLength, keyInfo.salt,
        PKCS5_SALT_SIZE, keyInfo.noIterations, dk, GetMaxPkcs5OutSize(), td);
}

#undef dk
#define dk td->dk[SHA512 - 1]
// ~8 seconds
__global__ static void reduceKernel_sha512 (char* d_EncryptedHeader, Password* d_Input, PTHREAD_DATA d_Output)
{
    const int     tid = blockIdx.x * blockDim.x + threadIdx.x;
    PTHREAD_DATA td = &d_Output[tid]; 

    crypto_loadkey (&keyInfo, (char*)d_Input[tid].Text, (int) d_Input[tid].Length);

    // PKCS5 is used to derive the primary header key(s) and secondary header key(s) (XTS mode) from the password
    memcpy (keyInfo.salt, d_EncryptedHeader + HEADER_SALT_OFFSET, PKCS5_SALT_SIZE);

    keyInfo.noIterations = get_pkcs5_iteration_count (SHA512, 0);
    derive_key_sha512 (keyInfo.userKey, keyInfo.keyLength, keyInfo.salt,
        PKCS5_SALT_SIZE, keyInfo.noIterations, dk, GetMaxPkcs5OutSize());

#undef dk
}

#define dk td->dk[SHA1 - 1]
// ~10 seconds (27 seconds so far)
__global__ static void reduceKernel_sha1 (char* d_EncryptedHeader, Password* d_Input, PTHREAD_DATA d_Output)
{
    const int     tid = blockIdx.x * blockDim.x + threadIdx.x;
    PTHREAD_DATA td = &d_Output[tid];

    crypto_loadkey (&keyInfo, (char*)d_Input[tid].Text, (int) d_Input[tid].Length);

    // PKCS5 is used to derive the primary header key(s) and secondary header key(s) (XTS mode) from the password
    memcpy (keyInfo.salt, d_EncryptedHeader + HEADER_SALT_OFFSET, PKCS5_SALT_SIZE);

    keyInfo.noIterations = get_pkcs5_iteration_count (SHA1, 0);
    derive_key_sha1 (keyInfo.userKey, keyInfo.keyLength, keyInfo.salt,
        PKCS5_SALT_SIZE, keyInfo.noIterations, dk, GetMaxPkcs5OutSize());

#undef dk
}

#define dk td->dk[WHIRLPOOL - 1]
// slowest by far: ~31 seconds, total: ~58 seconds
__global__ static void reduceKernel_whirlpool (char* d_EncryptedHeader, Password* d_Input, PTHREAD_DATA d_Output)
{
    const int     tid = blockIdx.x * blockDim.x + threadIdx.x;
    PTHREAD_DATA td = &d_Output[tid];

    crypto_loadkey (&keyInfo, (char*)d_Input[tid].Text, (int) d_Input[tid].Length);

    // PKCS5 is used to derive the primary header key(s) and secondary header key(s) (XTS mode) from the password
    memcpy (keyInfo.salt, d_EncryptedHeader + HEADER_SALT_OFFSET, PKCS5_SALT_SIZE);

    keyInfo.noIterations = get_pkcs5_iteration_count (WHIRLPOOL, 0);
    derive_key_whirlpool (keyInfo.userKey, keyInfo.keyLength, keyInfo.salt,
        PKCS5_SALT_SIZE, keyInfo.noIterations, dk, GetMaxPkcs5OutSize(), td);

#undef dk
}

#define dk td->dk[enqPkcs5Prf - 1]
__global__ static void reduceKernel_final (char* d_EncryptedHeader, PTHREAD_DATA d_Input, PTHREAD_RESULT d_Output)
{
    const int     tid = blockIdx.x * blockDim.x + threadIdx.x;
    PTHREAD_DATA td = &d_Input[tid];

    ALIGN(32) char header[TC_VOLUME_HEADER_EFFECTIVE_SIZE];
    int enqPkcs5Prf;
    int primaryKeyOffset;
    ALIGN(32) u1byte  q_tab[2][256];
    ALIGN(32) u4byte  m_tab[4][256];
    u4byte  qt_gen = 0, mt_gen = 0;

    crypto_open(cryptoInfo);

    // Test all available PKCS5 PRFs
    for (enqPkcs5Prf = FIRST_PRF_ID; enqPkcs5Prf <= LAST_PRF_ID; ++enqPkcs5Prf)
    {
        BOOL lrw64InitDone = FALSE;		// Deprecated/legacy
        BOOL lrw128InitDone = FALSE;	// Deprecated/legacy
if (enqPkcs5Prf == SHA512/* || enqPkcs5Prf == WHIRLPOOL*/) continue;
        // Test all available modes of operation
        for (cryptoInfo->mode = FIRST_MODE_OF_OPERATION_ID;
            cryptoInfo->mode <= LAST_MODE_OF_OPERATION;
            cryptoInfo->mode++)
        {
            switch (cryptoInfo->mode)
            {
            case LRW:
            case CBC:
            case INNER_CBC:
            case OUTER_CBC:

                // For LRW (deprecated/legacy), copy the tweak key 
                // For CBC (deprecated/legacy), copy the IV/whitening seed 
                memcpy (cryptoInfo->k2, dk, LEGACY_VOL_IV_SIZE);
                primaryKeyOffset = LEGACY_VOL_IV_SIZE;
                break;

            default:
                primaryKeyOffset = 0;
            }

            // Test all available encryption algorithms
            for (cryptoInfo->ea = EAGetFirst ();
                cryptoInfo->ea != 0;
                cryptoInfo->ea = EAGetNext (cryptoInfo->ea))
            {
                int blockSize;

                if (!EAIsModeSupported (cryptoInfo->ea, cryptoInfo->mode))
                    continue;	// This encryption algorithm has never been available with this mode of operation

                blockSize = CipherGetBlockSize (EAGetFirstCipher (cryptoInfo->ea));

                if (EAInit (cryptoInfo->ea, (unsigned char *)(dk + primaryKeyOffset), cryptoInfo->ks, (u1byte**)q_tab, (u4byte**)m_tab, &qt_gen, &mt_gen) == ERR_CIPHER_INIT_FAILURE)
                    goto ret;

                // Init objects related to the mode of operation

                if (cryptoInfo->mode == XTS)
                {
                    // Copy the secondary key (if cascade, multiple concatenated)
                    memcpy (cryptoInfo->k2, dk + EAGetKeySize (cryptoInfo->ea), EAGetKeySize (cryptoInfo->ea));

                    // Secondary key schedule
                    if (!EAInitMode (cryptoInfo, (u1byte**)q_tab, (u4byte**)m_tab, &qt_gen, &mt_gen))
                    {
                        goto ret;
                    }
                }
                else if (cryptoInfo->mode == LRW
                    && (blockSize == 8 && !lrw64InitDone || blockSize == 16 && !lrw128InitDone))
                {
                    // Deprecated/legacy

                    if (!EAInitMode (cryptoInfo,(u1byte**)q_tab, (u4byte**)m_tab, &qt_gen, &mt_gen))
                    {
                        goto ret;
                    }

                    if (blockSize == 8)
                        lrw64InitDone = TRUE;
                    else if (blockSize == 16)
                        lrw128InitDone = TRUE;
                }

                // Copy the header for decryption
                memcpy (header, d_EncryptedHeader, sizeof (header));

                // Try to decrypt header
                DecryptBuffer ((unsigned char*)(header + HEADER_ENCRYPTED_DATA_OFFSET), HEADER_ENCRYPTED_DATA_SIZE, cryptoInfo); // fixme: crash here due to twofish

                // Magic 'TRUE'
                if (GetHeaderField32 ((byte*)header, TC_HEADER_OFFSET_MAGIC) == 0x54525545){
                    d_Output->tid = tid;
                    d_Output->ea = cryptoInfo->ea;
                    d_Output->mode = cryptoInfo->mode;
                    d_Output->prf = enqPkcs5Prf;
                    goto ret;
                }
            }
        }
    }

ret:
    return;
}

extern "C" void launch_reduceKernel_ripemd160(char* d_EncryptedHeader, Password* d_Input, PTHREAD_DATA d_Output, int BLOCK_N, int THREAD_N)
{
    reduceKernel_ripemd160<<<BLOCK_N, THREAD_N>>>(d_EncryptedHeader, d_Input, d_Output);
}

extern "C" void launch_reduceKernel_sha512(char* d_EncryptedHeader, Password* d_Input, PTHREAD_DATA d_Output, int BLOCK_N, int THREAD_N)
{
    reduceKernel_sha512<<<BLOCK_N, THREAD_N>>>(d_EncryptedHeader, d_Input, d_Output);
}

extern "C" void launch_reduceKernel_sha1(char* d_EncryptedHeader, Password* d_Input, PTHREAD_DATA d_Output, int BLOCK_N, int THREAD_N)
{
    reduceKernel_sha1<<<BLOCK_N, THREAD_N>>>(d_EncryptedHeader, d_Input, d_Output);
}

extern "C" void launch_reduceKernel_whirlpool(char* d_EncryptedHeader, Password* d_Input, PTHREAD_DATA d_Output, int BLOCK_N, int THREAD_N)
{
    reduceKernel_whirlpool<<<BLOCK_N, THREAD_N>>>(d_EncryptedHeader, d_Input, d_Output);
}

extern "C" void launch_reduceKernel_final(char *d_EncryptedHeader, PTHREAD_DATA d_Input, PTHREAD_RESULT d_Output, int BLOCK_N, int THREAD_N)
{
    reduceKernel_final<<<BLOCK_N, THREAD_N>>>(d_EncryptedHeader, d_Input, d_Output);
}
