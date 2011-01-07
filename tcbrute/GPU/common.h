/*
 Copyright (c) 2005-2009 TrueCrypt Developers Association. All rights reserved.

 Governed by the TrueCrypt License 3.0 the full text of which is contained in
 the file License.txt included in TrueCrypt binary and source code distribution
 packages.
*/

#ifndef COMMON_H
#define COMMON_H

#define MIN_PASSWORD			1		// Minimum possible password length
#define MAX_PASSWORD			64		// Maximum possible password length

#define WHIRLPOOL_BLOCKSIZE		64
#define WHIRLPOOL_DIGESTSIZE	64

// Size of the salt (in bytes)
#define PKCS5_SALT_SIZE				64
// Size of the volume header area containing concatenated master key(s) and secondary key(s) (XTS mode)
#define MASTER_KEYDATA_SIZE			256

#define TC_VOLUME_HEADER_EFFECTIVE_SIZE			512

typedef int BOOL;

#define word32 u32

#ifdef _WIN32
        typedef __int8 int8;
        typedef __int16 int16;
        typedef __int32 int32;
        typedef __int64 int64;
        typedef unsigned __int8 byte;
        typedef unsigned __int16 uint16;
        typedef unsigned __int32 uint32;
        typedef unsigned __int64 uint64;

	typedef unsigned __int8 uint_8t;
	typedef unsigned __int16 uint_16t;
	typedef unsigned __int32 uint_32t;
	typedef uint64 uint_64t;
#else
	#ifdef _NVCC
        typedef signed char int8_t;
        typedef signed short int16_t;
        typedef signed int int32_t;
        typedef signed long int int64_t;
        typedef unsigned char uint8_t;
        typedef unsigned short uint16_t;
        typedef unsigned int uint32_t;
        typedef unsigned long int uint64_t;
	#endif

        typedef int8_t int8;
        typedef int16_t int16;
        typedef int32_t int32;
        typedef int64_t int64;
        typedef uint8_t byte;
        typedef uint16_t uint16;
        typedef uint32_t uint32;
        typedef uint64_t uint64;

        typedef uint8_t uint_8t;
        typedef uint16_t uint_16t;
        typedef uint32_t uint_32t;
        typedef uint64_t uint_64t;
#endif

typedef uint64	TC_LARGEST_COMPILER_UINT;

#define u4byte	u32
#define u1byte	unsigned char

typedef signed char s8;
typedef unsigned char u8;
typedef signed short s16;
typedef signed int s32;
typedef uint16 u16;
typedef uint32 u32;
typedef uint64 u64;
typedef int64 s64;

#define DIGESTBYTES 64
#define DIGESTBITS  (8*DIGESTBYTES) /* 512 */

#define WBLOCKBYTES 64
#define WBLOCKBITS  (8*WBLOCKBYTES) /* 512 */

#define LENGTHBYTES 32
#define LENGTHBITS  (8*LENGTHBYTES) /* 256 */

typedef struct NESSIEstruct {
    u8  bitLength[LENGTHBYTES]; /* global number of hashed bits (256-bit counter) */
    u8  buffer[WBLOCKBYTES];	/* buffer of data to hash */
    int bufferBits;		        /* current number of bits on the buffer */
    int bufferPos;		        /* current (possibly incomplete) byte slot on the buffer */
    u64 hash[DIGESTBYTES/8];    /* the hashing state */
} NESSIEstruct;

typedef NESSIEstruct WHIRLPOOL_CTX;

// The first PRF to try when mounting
#define FIRST_PRF_ID		1

// Hash algorithms (pseudorandom functions). 
enum
{
    RIPEMD160 = FIRST_PRF_ID,
    SHA512,
    WHIRLPOOL,
    SHA1,				// Deprecated/legacy
    HASH_ENUM_END_ID
};

// The last PRF to try when mounting and also the number of implemented PRFs
#define LAST_PRF_ID			SHA1//(HASH_ENUM_END_ID - 1)	

// The mode of operation used for newly created volumes and first to try when mounting
#define FIRST_MODE_OF_OPERATION_ID		1

// Modes of operation
enum
{
    /* If you add/remove a mode, update the following: GetMaxPkcs5OutSize(), EAInitMode() */
    XTS = FIRST_MODE_OF_OPERATION_ID,
    LRW,		// Deprecated/legacy
    CBC,		// Deprecated/legacy
    OUTER_CBC,	// Deprecated/legacy
    INNER_CBC,	// Deprecated/legacy
    MODE_ENUM_END_ID
};

// The last mode of operation to try when mounting and also the number of implemented modes
#define LAST_MODE_OF_OPERATION		(MODE_ENUM_END_ID - 1)

// Cipher IDs
enum
{
    NONE = 0,
    AES,
    SERPENT,			
    TWOFISH,			
    BLOWFISH,		// Deprecated/legacy
    CAST,			// Deprecated/legacy
    TRIPLEDES		// Deprecated/legacy
};

typedef struct
{
    // Modifying this structure can introduce incompatibility with previous versions
    u32 Length;
    unsigned char Text[MAX_PASSWORD + 1];
    char Pad[3]; // keep 64-bit alignment
} Password;

typedef union 
{
    struct 
    {
        u32 LowPart;
        u32 HighPart;
    };

    uint64 Value;

} UINT64_STRUCT;

// AES key schedule length
#define KS_LENGTH       60

typedef union
{   uint_32t l;
    uint_8t b[4];
} aes_inf;

typedef struct
{   uint_32t ks[KS_LENGTH];
    aes_inf inf;
} aes_encrypt_ctx;

typedef struct
{   uint_32t ks[KS_LENGTH];
    aes_inf inf;
} aes_decrypt_ctx;

typedef struct
{
    u4byte l_key[40];
    u4byte s_key[4];
    u4byte mk_tab[4 * 256];
    u4byte k_len;
} TwofishInstance;

#define AES_KS			(sizeof(aes_encrypt_ctx) + sizeof(aes_decrypt_ctx))
#define SERPENT_KS		(140 * 4)
#define TWOFISH_KS		sizeof(TwofishInstance)

#define MAX_EXPANDED_KEY	(AES_KS + SERPENT_KS + TWOFISH_KS)

typedef struct keyInfo_t
{
    int noIterations;					/* Number of times to iterate (PKCS-5) */
    int keyLength;						/* Length of the key */
    char userKey[MAX_PASSWORD];		/* Password (to which keyfiles may have been applied). WITHOUT +1 for the null terminator. */
    char salt[PKCS5_SALT_SIZE];		/* PKCS-5 salt */
    char master_keydata[MASTER_KEYDATA_SIZE];		/* Concatenated master primary and secondary key(s) (XTS mode). For LRW (deprecated/legacy), it contains the tweak key before the master key(s). For CBC (deprecated/legacy), it contains the IV seed before the master key(s). */
} KEY_INFO, *PKEY_INFO;

#define CBLK_LEN   16  /* encryption block length */
#define CBLK_LEN8  8

typedef struct
{
    /* union not used to support faster mounting */
    u32 gf_t128[CBLK_LEN * 2 / 2][16][CBLK_LEN / 4];
    u32 gf_t64[CBLK_LEN8 * 2][16][CBLK_LEN8 / 4];
} GfCtx;

typedef struct CRYPTO_INFO_t
{
    int ea;									/* Encryption algorithm ID */
    int mode;								/* Mode of operation (e.g., XTS) */
    u8 ks[MAX_EXPANDED_KEY];	/* Primary key schedule (if it is a cascade, it conatins multiple concatenated keys) */
    u8 ks2[MAX_EXPANDED_KEY];	/* Secondary key schedule (if cascade, multiple concatenated) for XTS mode. */

    BOOL hiddenVolume;						// Indicates whether the volume is mounted/mountable as hidden volume

    uint16 HeaderVersion;

    GfCtx gf_ctx; 

    u8 master_keydata[MASTER_KEYDATA_SIZE];	/* This holds the volume header area containing concatenated master key(s) and secondary key(s) (XTS mode). For LRW (deprecated/legacy), it contains the tweak key before the master key(s). For CBC (deprecated/legacy), it contains the IV seed before the master key(s). */
    u8 k2[MASTER_KEYDATA_SIZE];				/* For XTS, this contains the secondary key (if cascade, multiple concatenated). For LRW (deprecated/legacy), it contains the tweak key. For CBC (deprecated/legacy), it contains the IV seed. */
    u8 salt[PKCS5_SALT_SIZE];
    int noIterations;
    int pkcs5;

    uint64 volume_creation_time;	// Legacy
    uint64 header_creation_time;	// Legacy

    BOOL bProtectHiddenVolume;			// Indicates whether the volume contains a hidden volume to be protected against overwriting
    BOOL bHiddenVolProtectionAction;		// TRUE if a write operation has been denied by the driver in order to prevent the hidden volume from being overwritten (set to FALSE upon volume mount).
    
    uint64 volDataAreaOffset;		// Absolute position, in bytes, of the first data sector of the volume.

    uint64 hiddenVolumeSize;		// Size of the hidden volume excluding the header (in bytes). Set to 0 for standard volumes.
    uint64 hiddenVolumeOffset;	// Absolute position, in bytes, of the first hidden volume data sector within the host volume (provided that there is a hidden volume within). This must be set for all hidden volumes; in case of a normal volume, this variable is only used when protecting a hidden volume within it.
    uint64 hiddenVolumeProtectedSize;

    BOOL bPartitionInInactiveSysEncScope;	// If TRUE, the volume is a partition located on an encrypted system drive and mounted without pre-boot authentication.

    UINT64_STRUCT FirstDataUnitNo;			// First data unit number of the volume. This is 0 for file-hosted and non-system partition-hosted volumes. For partitions within key scope of system encryption this reflects real physical offset within the device (this is used e.g. when such a partition is mounted as a regular volume without pre-boot authentication).

    uint16 RequiredProgramVersion;
    BOOL LegacyVolume;

    uint32 SectorSize;

    UINT64_STRUCT VolumeSize;

    UINT64_STRUCT EncryptedAreaStart;
    UINT64_STRUCT EncryptedAreaLength;

    uint32 HeaderFlags;

} CRYPTO_INFO, *PCRYPTO_INFO;

typedef struct
{
    char u[WHIRLPOOL_DIGESTSIZE];
    char j[WHIRLPOOL_DIGESTSIZE], k[WHIRLPOOL_DIGESTSIZE];
    char init[128];
    WHIRLPOOL_CTX ictx, octx, tctx;
    char iwhi[WHIRLPOOL_DIGESTSIZE], owhi[WHIRLPOOL_DIGESTSIZE];
    char key[WHIRLPOOL_DIGESTSIZE];
    char buf[WHIRLPOOL_BLOCKSIZE];
    u64 K[8];        /* the round key */
    u64 block[8];    /* mu(buffer) */
    u64 state[8];    /* the cipher state */
    u64 L[8];
    u8 *buffer;
    KEY_INFO keyInfo;
    CRYPTO_INFO cryptoInfo;
    char dk[4][MASTER_KEYDATA_SIZE];
    unsigned char k_ipad[64];  // inner padding - key XORd with ipad
    unsigned char k_opad[64];  // outer padding - key XORd with opad
} THREAD_DATA, *PTHREAD_DATA;

typedef struct
{
    int tid, ea, mode, prf;
} THREAD_RESULT, *PTHREAD_RESULT;

typedef struct
{
    int Ciphers[4];			// Null terminated array of ciphers used by encryption algorithm
    int Modes[LAST_MODE_OF_OPERATION + 1];			// Null terminated array of modes of operation
    int FormatEnabled;
} EncryptionAlgorithm;

// Encryption algorithm configuration
// The following modes have been deprecated (legacy): LRW, CBC, INNER_CBC, OUTER_CBC
__constant__ const EncryptionAlgorithm EncryptionAlgorithms[] =
{
    //  Cipher(s)                     Modes						FormatEnabled
    { { 0,						0 }, { 0, 0, 0, 0 },				0 },	// Must be all-zero
    { { AES,					0 }, { XTS, LRW, CBC, 0 },			1 },
//    { { SERPENT,				0 }, { XTS, LRW, CBC, 0 },			1 },
    //{ { TWOFISH,				0 }, { XTS, LRW, CBC, 0 },			1 },
    //{ { TWOFISH, AES,			0 }, { XTS, LRW, OUTER_CBC, 0 },	1 },
    //{ { SERPENT, TWOFISH, AES,	0 }, { XTS, LRW, OUTER_CBC, 0 },	1 },
//    { { AES, SERPENT,			0 }, { XTS, LRW, OUTER_CBC, 0 },	1 },
    //{ { AES, TWOFISH, SERPENT,	0 }, { XTS, LRW, OUTER_CBC, 0 },	1 },
    //{ { SERPENT, TWOFISH,		0 }, { XTS, LRW, OUTER_CBC, 0 },	1 },
//    { { BLOWFISH,				0 }, { LRW, CBC, 0, 0 },			0 },	// Deprecated/legacy
//    { { CAST,					0 }, { LRW, CBC, 0, 0 },			0 },	// Deprecated/legacy
//    { { TRIPLEDES,				0 }, { LRW, CBC, 0, 0 },			0 },	// Deprecated/legacy
//    { { BLOWFISH, AES,			0 }, { INNER_CBC, 0, 0, 0 },		0 },	// Deprecated/legacy
//    { { SERPENT, BLOWFISH, AES,	0 }, { INNER_CBC, 0, 0, 0 },		0 },	// Deprecated/legacy
    { { 0,						0 }, { 0, 0, 0, 0 },				0 }		// Must be all-zero
};

#endif
