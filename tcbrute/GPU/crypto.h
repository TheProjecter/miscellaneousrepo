/*
 Copyright (c) 2005-2009 TrueCrypt Developers Association. All rights reserved.

 Governed by the TrueCrypt License 3.0 the full text of which is contained in
 the file License.txt included in TrueCrypt binary and source code distribution
 packages.
*/

#ifndef CRYPTO_H
#define CRYPTO_H

#pragma region "crypto stuff"

#define TC_HEADER_OFFSET_MAGIC					64

// Size of the deprecated volume header item containing either an IV seed (CBC mode) or tweak key (LRW mode)
#define LEGACY_VOL_IV_SIZE			32

#define RIPEMD160_BLOCKSIZE		64
#define RIPEMD160_DIGESTSIZE	20

#define SHA1_BLOCKSIZE			64	
#define SHA1_DIGESTSIZE			20

#define SHA512_BLOCKSIZE		128
#define SHA512_DIGESTSIZE		64

#define SHA384_DIGEST_SIZE  48
#define SHA384_BLOCK_SIZE  128
#define SHA512_DIGEST_SIZE  64
#define SHA512_BLOCK_SIZE  128
#define SHA2_MAX_DIGEST_SIZE    SHA512_DIGEST_SIZE

#define SHA1_BLOCK_SIZE  64
#define SHA1_DIGEST_SIZE 20

#define AES_BLOCK_SIZE  16  /* the AES block size in bytes          */
#define RC_LENGTH   (5 * (AES_BLOCK_SIZE / 4 - 2))

#define SHA512_MASK (SHA512_BLOCK_SIZE - 1)
#define SHA1_MASK   (SHA1_BLOCK_SIZE - 1)

#define HEADER_ENCRYPTED_DATA_SIZE			(TC_VOLUME_HEADER_EFFECTIVE_SIZE - HEADER_ENCRYPTED_DATA_OFFSET)

// Volume header field offsets
#define	HEADER_SALT_OFFSET					0
#define HEADER_ENCRYPTED_DATA_OFFSET		PKCS5_SALT_SIZE
#define	HEADER_MASTER_KEYDATA_OFFSET		256

#define EXIT_SUCCESS    0
#define EXIT_FAILURE    1

#ifndef NULL
#define NULL 0
#endif

#ifdef _MSC_VER
#define ALIGN(x) __declspec(align(x))
#else 
#define ALIGN(x) __attribute__ ((aligned (x)))
#endif

#define FALSE               0
#define TRUE                1
#define TC_THROW_FATAL_EXCEPTION	*(char *) 0 = 0

/*
 * The number of rounds of the internal dedicated block cipher.
 */
#define R 10

// Erase macros
#define FAST_ERASE64(mem,size) do { volatile uint64 *burnm = (volatile uint64 *)(mem); int burnc = size >> 3; while (burnc--) *burnm++ = 0; } while (0)
#define burn(mem,size) do { volatile char *burnm = (volatile char *)(mem); int burnc = size; while (burnc--) *burnm++ = 0; } while (0)

// Ciphertext/plaintext block size for XTS mode (in bytes)
#define BYTES_PER_XTS_BLOCK			16

// Encryption data unit size, which may differ from the sector size and must always be 512
#define ENCRYPTION_DATA_UNIT_SIZE	512

// Number of ciphertext/plaintext blocks per XTS data unit
#define BLOCKS_PER_XTS_DATA_UNIT	(ENCRYPTION_DATA_UNIT_SIZE / BYTES_PER_XTS_BLOCK)

#define in_line __inline

#define RIPEMD160_BLOCK_LENGTH 64

typedef struct RMD160Context
{
    u32 state[5];
    uint64 count;
    unsigned char buffer[RIPEMD160_BLOCK_LENGTH];
} RMD160_CTX;

typedef struct
{
    u32 gf_t8k[CBLK_LEN * 2][16][CBLK_LEN / 4];
} GfCtx8k;

typedef struct
{
        u32 gf_t4k[CBLK_LEN8 * 2][16][CBLK_LEN / 4];
} GfCtx4k64;

#define PUT_64BIT_LE(cp, value) do {                                    \
    (cp)[7] = (byte) ((value) >> 56);                                        \
    (cp)[6] = (byte) ((value) >> 48);                                        \
    (cp)[5] = (byte) ((value) >> 40);                                        \
    (cp)[4] = (byte) ((value) >> 32);                                        \
    (cp)[3] = (byte) ((value) >> 24);                                        \
    (cp)[2] = (byte) ((value) >> 16);                                        \
    (cp)[1] = (byte) ((value) >> 8);                                         \
    (cp)[0] = (byte) (value); } while (0)

#define PUT_32BIT_LE(cp, value) do {                                    \
    (cp)[3] = (byte) ((value) >> 24);                                        \
    (cp)[2] = (byte) ((value) >> 16);                                        \
    (cp)[1] = (byte) ((value) >> 8);                                         \
    (cp)[0] = (byte) (value); } while (0)

enum
{
    /* WARNING: ADD ANY NEW CODES AT THE END (DO NOT INSERT THEM BETWEEN EXISTING). DO *NOT* DELETE ANY 
    EXISTING CODES! Changing these values or their meanings may cause incompatibility with other versions
    (for example, if a new version of the TrueCrypt installer receives an error code from an installed 
    driver whose version is lower, it will report and interpret the error incorrectly). */

    ERR_SUCCESS								= 0,
    ERR_OS_ERROR							= 1,
    ERR_OUTOFMEMORY							= 2,
    ERR_PASSWORD_WRONG						= 3,
    ERR_VOL_FORMAT_BAD						= 4,
    ERR_DRIVE_NOT_FOUND						= 5,
    ERR_FILES_OPEN							= 6,
    ERR_VOL_SIZE_WRONG						= 7,
    ERR_COMPRESSION_NOT_SUPPORTED			= 8,
    ERR_PASSWORD_CHANGE_VOL_TYPE			= 9,
    ERR_PASSWORD_CHANGE_VOL_VERSION			= 10,
    ERR_VOL_SEEKING							= 11,
    ERR_VOL_WRITING							= 12,
    ERR_FILES_OPEN_LOCK						= 13,
    ERR_VOL_READING							= 14,
    ERR_DRIVER_VERSION						= 15,
    ERR_NEW_VERSION_REQUIRED				= 16,
    ERR_CIPHER_INIT_FAILURE					= 17,
    ERR_CIPHER_INIT_WEAK_KEY				= 18,
    ERR_SELF_TESTS_FAILED					= 19,
    ERR_SECTOR_SIZE_INCOMPATIBLE			= 20,
    ERR_VOL_ALREADY_MOUNTED					= 21,
    ERR_NO_FREE_DRIVES						= 22,
    ERR_FILE_OPEN_FAILED					= 23,
    ERR_VOL_MOUNT_FAILED					= 24,
    DEPRECATED_ERR_INVALID_DEVICE			= 25,
    ERR_ACCESS_DENIED						= 26,
    ERR_MODE_INIT_FAILED					= 27,
    ERR_DONT_REPORT							= 28,
    ERR_ENCRYPTION_NOT_COMPLETED			= 29,
    ERR_PARAMETER_INCORRECT					= 30,
    ERR_SYS_HIDVOL_HEAD_REENC_MODE_WRONG	= 31,
    ERR_NONSYS_INPLACE_ENC_INCOMPLETE		= 32,
    ERR_USER_ABORT							= 33
};

typedef struct
{
    int Id;					// Cipher ID
    int BlockSize;			// Block size (bytes)
    int KeySize;			// Key size (bytes)
    int KeyScheduleSize;	// Scheduled key size (bytes)
} Cipher;

typedef struct BF_KEY_STRUCT
{
    u32 pbox[18];
    u32 pbox_dec[18];
    u32 sbox[4*256];
} BF_KEY;

typedef struct CAST_KEY_STRUCT
{
    u32 K[32];
} CAST_KEY;

typedef struct TRIPLE_DES_KEY_STRUCT
{
    u32 k1[32];
    u32 k2[32];
    u32 k3[32];
    u32 k1d[32];
    u32 k2d[32];
    u32 k3d[32];
} TDES_KEY;

/* type to hold the SHA384 (and SHA512) context */

typedef struct
{   uint_64t count[2];
    uint_64t hash[8];
    uint_64t wbuf[16];
} sha512_ctx;

#define VOID_RETURN void
#define AES_RETURN int

#define li_64(h) 0x##h##ull

typedef   u32    sha1_32t;

/* type to hold the SHA256 context  */

typedef struct
{   sha1_32t count[2];
    sha1_32t hash[5];
    sha1_32t wbuf[16];
} sha1_ctx;

#ifdef _WIN32
#define LL(v)   (v##ui64)
#else
#define LL(v) (v##ull)
#endif

#define ONE64   LL(0xffffffffffffffff)

#define mode(x)			GM_##x

typedef u32 mode(32t);
typedef uint64 mode(64t);

#define gf_dat(q) {\
    q(0x00), q(0x01), q(0x02), q(0x03), q(0x04), q(0x05), q(0x06), q(0x07),\
    q(0x08), q(0x09), q(0x0a), q(0x0b), q(0x0c), q(0x0d), q(0x0e), q(0x0f),\
    q(0x10), q(0x11), q(0x12), q(0x13), q(0x14), q(0x15), q(0x16), q(0x17),\
    q(0x18), q(0x19), q(0x1a), q(0x1b), q(0x1c), q(0x1d), q(0x1e), q(0x1f),\
    q(0x20), q(0x21), q(0x22), q(0x23), q(0x24), q(0x25), q(0x26), q(0x27),\
    q(0x28), q(0x29), q(0x2a), q(0x2b), q(0x2c), q(0x2d), q(0x2e), q(0x2f),\
    q(0x30), q(0x31), q(0x32), q(0x33), q(0x34), q(0x35), q(0x36), q(0x37),\
    q(0x38), q(0x39), q(0x3a), q(0x3b), q(0x3c), q(0x3d), q(0x3e), q(0x3f),\
    q(0x40), q(0x41), q(0x42), q(0x43), q(0x44), q(0x45), q(0x46), q(0x47),\
    q(0x48), q(0x49), q(0x4a), q(0x4b), q(0x4c), q(0x4d), q(0x4e), q(0x4f),\
    q(0x50), q(0x51), q(0x52), q(0x53), q(0x54), q(0x55), q(0x56), q(0x57),\
    q(0x58), q(0x59), q(0x5a), q(0x5b), q(0x5c), q(0x5d), q(0x5e), q(0x5f),\
    q(0x60), q(0x61), q(0x62), q(0x63), q(0x64), q(0x65), q(0x66), q(0x67),\
    q(0x68), q(0x69), q(0x6a), q(0x6b), q(0x6c), q(0x6d), q(0x6e), q(0x6f),\
    q(0x70), q(0x71), q(0x72), q(0x73), q(0x74), q(0x75), q(0x76), q(0x77),\
    q(0x78), q(0x79), q(0x7a), q(0x7b), q(0x7c), q(0x7d), q(0x7e), q(0x7f),\
    q(0x80), q(0x81), q(0x82), q(0x83), q(0x84), q(0x85), q(0x86), q(0x87),\
    q(0x88), q(0x89), q(0x8a), q(0x8b), q(0x8c), q(0x8d), q(0x8e), q(0x8f),\
    q(0x90), q(0x91), q(0x92), q(0x93), q(0x94), q(0x95), q(0x96), q(0x97),\
    q(0x98), q(0x99), q(0x9a), q(0x9b), q(0x9c), q(0x9d), q(0x9e), q(0x9f),\
    q(0xa0), q(0xa1), q(0xa2), q(0xa3), q(0xa4), q(0xa5), q(0xa6), q(0xa7),\
    q(0xa8), q(0xa9), q(0xaa), q(0xab), q(0xac), q(0xad), q(0xae), q(0xaf),\
    q(0xb0), q(0xb1), q(0xb2), q(0xb3), q(0xb4), q(0xb5), q(0xb6), q(0xb7),\
    q(0xb8), q(0xb9), q(0xba), q(0xbb), q(0xbc), q(0xbd), q(0xbe), q(0xbf),\
    q(0xc0), q(0xc1), q(0xc2), q(0xc3), q(0xc4), q(0xc5), q(0xc6), q(0xc7),\
    q(0xc8), q(0xc9), q(0xca), q(0xcb), q(0xcc), q(0xcd), q(0xce), q(0xcf),\
    q(0xd0), q(0xd1), q(0xd2), q(0xd3), q(0xd4), q(0xd5), q(0xd6), q(0xd7),\
    q(0xd8), q(0xd9), q(0xda), q(0xdb), q(0xdc), q(0xdd), q(0xde), q(0xdf),\
    q(0xe0), q(0xe1), q(0xe2), q(0xe3), q(0xe4), q(0xe5), q(0xe6), q(0xe7),\
    q(0xe8), q(0xe9), q(0xea), q(0xeb), q(0xec), q(0xed), q(0xee), q(0xef),\
    q(0xf0), q(0xf1), q(0xf2), q(0xf3), q(0xf4), q(0xf5), q(0xf6), q(0xf7),\
    q(0xf8), q(0xf9), q(0xfa), q(0xfb), q(0xfc), q(0xfd), q(0xfe), q(0xff) }

/* given the value i in 0..255 as the byte overflow when a a field  */
/* element in GHASH is multipled by x^8, this function will return  */
/* the values that are generated in the lo 16-bit word of the field */
/* value by applying the modular polynomial. The values lo_byte and */
/* hi_byte are returned via the macro xp_fun(lo_byte, hi_byte) so   */
/* that the values can be assembled into memory as required by a    */
/* suitable definition of this macro operating on the table above   */
#define xp_fun(x,y)    ((mode(32t))(y)) | (((mode(32t))(x)) << 8)
#define xp(i) xp_fun( \
    (i & 0x80 ? 0xe1 : 0) ^ (i & 0x40 ? 0x70 : 0) ^ \
    (i & 0x20 ? 0x38 : 0) ^ (i & 0x10 ? 0x1c : 0) ^ \
    (i & 0x08 ? 0x0e : 0) ^ (i & 0x04 ? 0x07 : 0) ^ \
    (i & 0x02 ? 0x03 : 0) ^ (i & 0x01 ? 0x01 : 0),  \
    (i & 0x80 ? 0x00 : 0) ^ (i & 0x40 ? 0x80 : 0) ^ \
    (i & 0x20 ? 0x40 : 0) ^ (i & 0x10 ? 0x20 : 0) ^ \
    (i & 0x08 ? 0x10 : 0) ^ (i & 0x04 ? 0x08 : 0) ^ \
    (i & 0x02 ? 0x84 : 0) ^ (i & 0x01 ? 0xc2 : 0) )
#define xp64(i) xp_fun( \
    (i & 0x80 ? 0xd8 : 0) ^ (i & 0x40 ? 0x6c : 0) ^ \
    (i & 0x20 ? 0x36 : 0) ^ (i & 0x10 ? 0x1b : 0) ^ \
    (i & 0x08 ? 0x0d : 0) ^ (i & 0x04 ? 0x06 : 0) ^ \
    (i & 0x02 ? 0x03 : 0) ^ (i & 0x01 ? 0x01 : 0),  \
    (i & 0x80 ? 0x00 : 0) ^ (i & 0x40 ? 0x00 : 0) ^ \
    (i & 0x20 ? 0x00 : 0) ^ (i & 0x10 ? 0x00 : 0) ^ \
    (i & 0x08 ? 0x80 : 0) ^ (i & 0x04 ? 0xc0 : 0) ^ \
    (i & 0x02 ? 0x60 : 0) ^ (i & 0x01 ? 0xb0 : 0) )
#pragma endregion

#pragma region "Whirlpool constants"
/*
 * Though Whirlpool is endianness-neutral, the encryption tables are listed
 * in BIG-ENDIAN format, which is adopted throughout this implementation
 * (but little-endian notation would be equally suitable if consistently
 * employed).
 */

__constant__ u64 C0[256] = {
    LL(0x18186018c07830d8), LL(0x23238c2305af4626), LL(0xc6c63fc67ef991b8), LL(0xe8e887e8136fcdfb),
    LL(0x878726874ca113cb), LL(0xb8b8dab8a9626d11), LL(0x0101040108050209), LL(0x4f4f214f426e9e0d),
    LL(0x3636d836adee6c9b), LL(0xa6a6a2a6590451ff), LL(0xd2d26fd2debdb90c), LL(0xf5f5f3f5fb06f70e),
    LL(0x7979f979ef80f296), LL(0x6f6fa16f5fcede30), LL(0x91917e91fcef3f6d), LL(0x52525552aa07a4f8),
    LL(0x60609d6027fdc047), LL(0xbcbccabc89766535), LL(0x9b9b569baccd2b37), LL(0x8e8e028e048c018a),
    LL(0xa3a3b6a371155bd2), LL(0x0c0c300c603c186c), LL(0x7b7bf17bff8af684), LL(0x3535d435b5e16a80),
    LL(0x1d1d741de8693af5), LL(0xe0e0a7e05347ddb3), LL(0xd7d77bd7f6acb321), LL(0xc2c22fc25eed999c),
    LL(0x2e2eb82e6d965c43), LL(0x4b4b314b627a9629), LL(0xfefedffea321e15d), LL(0x575741578216aed5),
    LL(0x15155415a8412abd), LL(0x7777c1779fb6eee8), LL(0x3737dc37a5eb6e92), LL(0xe5e5b3e57b56d79e),
    LL(0x9f9f469f8cd92313), LL(0xf0f0e7f0d317fd23), LL(0x4a4a354a6a7f9420), LL(0xdada4fda9e95a944),
    LL(0x58587d58fa25b0a2), LL(0xc9c903c906ca8fcf), LL(0x2929a429558d527c), LL(0x0a0a280a5022145a),
    LL(0xb1b1feb1e14f7f50), LL(0xa0a0baa0691a5dc9), LL(0x6b6bb16b7fdad614), LL(0x85852e855cab17d9),
    LL(0xbdbdcebd8173673c), LL(0x5d5d695dd234ba8f), LL(0x1010401080502090), LL(0xf4f4f7f4f303f507),
    LL(0xcbcb0bcb16c08bdd), LL(0x3e3ef83eedc67cd3), LL(0x0505140528110a2d), LL(0x676781671fe6ce78),
    LL(0xe4e4b7e47353d597), LL(0x27279c2725bb4e02), LL(0x4141194132588273), LL(0x8b8b168b2c9d0ba7),
    LL(0xa7a7a6a7510153f6), LL(0x7d7de97dcf94fab2), LL(0x95956e95dcfb3749), LL(0xd8d847d88e9fad56),
    LL(0xfbfbcbfb8b30eb70), LL(0xeeee9fee2371c1cd), LL(0x7c7ced7cc791f8bb), LL(0x6666856617e3cc71),
    LL(0xdddd53dda68ea77b), LL(0x17175c17b84b2eaf), LL(0x4747014702468e45), LL(0x9e9e429e84dc211a),
    LL(0xcaca0fca1ec589d4), LL(0x2d2db42d75995a58), LL(0xbfbfc6bf9179632e), LL(0x07071c07381b0e3f),
    LL(0xadad8ead012347ac), LL(0x5a5a755aea2fb4b0), LL(0x838336836cb51bef), LL(0x3333cc3385ff66b6),
    LL(0x636391633ff2c65c), LL(0x02020802100a0412), LL(0xaaaa92aa39384993), LL(0x7171d971afa8e2de),
    LL(0xc8c807c80ecf8dc6), LL(0x19196419c87d32d1), LL(0x494939497270923b), LL(0xd9d943d9869aaf5f),
    LL(0xf2f2eff2c31df931), LL(0xe3e3abe34b48dba8), LL(0x5b5b715be22ab6b9), LL(0x88881a8834920dbc),
    LL(0x9a9a529aa4c8293e), LL(0x262698262dbe4c0b), LL(0x3232c8328dfa64bf), LL(0xb0b0fab0e94a7d59),
    LL(0xe9e983e91b6acff2), LL(0x0f0f3c0f78331e77), LL(0xd5d573d5e6a6b733), LL(0x80803a8074ba1df4),
    LL(0xbebec2be997c6127), LL(0xcdcd13cd26de87eb), LL(0x3434d034bde46889), LL(0x48483d487a759032),
    LL(0xffffdbffab24e354), LL(0x7a7af57af78ff48d), LL(0x90907a90f4ea3d64), LL(0x5f5f615fc23ebe9d),
    LL(0x202080201da0403d), LL(0x6868bd6867d5d00f), LL(0x1a1a681ad07234ca), LL(0xaeae82ae192c41b7),
    LL(0xb4b4eab4c95e757d), LL(0x54544d549a19a8ce), LL(0x93937693ece53b7f), LL(0x222288220daa442f),
    LL(0x64648d6407e9c863), LL(0xf1f1e3f1db12ff2a), LL(0x7373d173bfa2e6cc), LL(0x12124812905a2482),
    LL(0x40401d403a5d807a), LL(0x0808200840281048), LL(0xc3c32bc356e89b95), LL(0xecec97ec337bc5df),
    LL(0xdbdb4bdb9690ab4d), LL(0xa1a1bea1611f5fc0), LL(0x8d8d0e8d1c830791), LL(0x3d3df43df5c97ac8),
    LL(0x97976697ccf1335b), LL(0x0000000000000000), LL(0xcfcf1bcf36d483f9), LL(0x2b2bac2b4587566e),
    LL(0x7676c57697b3ece1), LL(0x8282328264b019e6), LL(0xd6d67fd6fea9b128), LL(0x1b1b6c1bd87736c3),
    LL(0xb5b5eeb5c15b7774), LL(0xafaf86af112943be), LL(0x6a6ab56a77dfd41d), LL(0x50505d50ba0da0ea),
    LL(0x45450945124c8a57), LL(0xf3f3ebf3cb18fb38), LL(0x3030c0309df060ad), LL(0xefef9bef2b74c3c4),
    LL(0x3f3ffc3fe5c37eda), LL(0x55554955921caac7), LL(0xa2a2b2a2791059db), LL(0xeaea8fea0365c9e9),
    LL(0x656589650fecca6a), LL(0xbabad2bab9686903), LL(0x2f2fbc2f65935e4a), LL(0xc0c027c04ee79d8e),
    LL(0xdede5fdebe81a160), LL(0x1c1c701ce06c38fc), LL(0xfdfdd3fdbb2ee746), LL(0x4d4d294d52649a1f),
    LL(0x92927292e4e03976), LL(0x7575c9758fbceafa), LL(0x06061806301e0c36), LL(0x8a8a128a249809ae),
    LL(0xb2b2f2b2f940794b), LL(0xe6e6bfe66359d185), LL(0x0e0e380e70361c7e), LL(0x1f1f7c1ff8633ee7),
    LL(0x6262956237f7c455), LL(0xd4d477d4eea3b53a), LL(0xa8a89aa829324d81), LL(0x96966296c4f43152),
    LL(0xf9f9c3f99b3aef62), LL(0xc5c533c566f697a3), LL(0x2525942535b14a10), LL(0x59597959f220b2ab),
    LL(0x84842a8454ae15d0), LL(0x7272d572b7a7e4c5), LL(0x3939e439d5dd72ec), LL(0x4c4c2d4c5a619816),
    LL(0x5e5e655eca3bbc94), LL(0x7878fd78e785f09f), LL(0x3838e038ddd870e5), LL(0x8c8c0a8c14860598),
    LL(0xd1d163d1c6b2bf17), LL(0xa5a5aea5410b57e4), LL(0xe2e2afe2434dd9a1), LL(0x616199612ff8c24e),
    LL(0xb3b3f6b3f1457b42), LL(0x2121842115a54234), LL(0x9c9c4a9c94d62508), LL(0x1e1e781ef0663cee),
    LL(0x4343114322528661), LL(0xc7c73bc776fc93b1), LL(0xfcfcd7fcb32be54f), LL(0x0404100420140824),
    LL(0x51515951b208a2e3), LL(0x99995e99bcc72f25), LL(0x6d6da96d4fc4da22), LL(0x0d0d340d68391a65),
    LL(0xfafacffa8335e979), LL(0xdfdf5bdfb684a369), LL(0x7e7ee57ed79bfca9), LL(0x242490243db44819),
    LL(0x3b3bec3bc5d776fe), LL(0xabab96ab313d4b9a), LL(0xcece1fce3ed181f0), LL(0x1111441188552299),
    LL(0x8f8f068f0c890383), LL(0x4e4e254e4a6b9c04), LL(0xb7b7e6b7d1517366), LL(0xebeb8beb0b60cbe0),
    LL(0x3c3cf03cfdcc78c1), LL(0x81813e817cbf1ffd), LL(0x94946a94d4fe3540), LL(0xf7f7fbf7eb0cf31c),
    LL(0xb9b9deb9a1676f18), LL(0x13134c13985f268b), LL(0x2c2cb02c7d9c5851), LL(0xd3d36bd3d6b8bb05),
    LL(0xe7e7bbe76b5cd38c), LL(0x6e6ea56e57cbdc39), LL(0xc4c437c46ef395aa), LL(0x03030c03180f061b),
    LL(0x565645568a13acdc), LL(0x44440d441a49885e), LL(0x7f7fe17fdf9efea0), LL(0xa9a99ea921374f88),
    LL(0x2a2aa82a4d825467), LL(0xbbbbd6bbb16d6b0a), LL(0xc1c123c146e29f87), LL(0x53535153a202a6f1),
    LL(0xdcdc57dcae8ba572), LL(0x0b0b2c0b58271653), LL(0x9d9d4e9d9cd32701), LL(0x6c6cad6c47c1d82b),
    LL(0x3131c43195f562a4), LL(0x7474cd7487b9e8f3), LL(0xf6f6fff6e309f115), LL(0x464605460a438c4c),
    LL(0xacac8aac092645a5), LL(0x89891e893c970fb5), LL(0x14145014a04428b4), LL(0xe1e1a3e15b42dfba),
    LL(0x16165816b04e2ca6), LL(0x3a3ae83acdd274f7), LL(0x6969b9696fd0d206), LL(0x09092409482d1241),
    LL(0x7070dd70a7ade0d7), LL(0xb6b6e2b6d954716f), LL(0xd0d067d0ceb7bd1e), LL(0xeded93ed3b7ec7d6),
    LL(0xcccc17cc2edb85e2), LL(0x424215422a578468), LL(0x98985a98b4c22d2c), LL(0xa4a4aaa4490e55ed),
    LL(0x2828a0285d885075), LL(0x5c5c6d5cda31b886), LL(0xf8f8c7f8933fed6b), LL(0x8686228644a411c2),
};

__constant__ u64 C1[256] = {
    LL(0xd818186018c07830), LL(0x2623238c2305af46), LL(0xb8c6c63fc67ef991), LL(0xfbe8e887e8136fcd),
    LL(0xcb878726874ca113), LL(0x11b8b8dab8a9626d), LL(0x0901010401080502), LL(0x0d4f4f214f426e9e),
    LL(0x9b3636d836adee6c), LL(0xffa6a6a2a6590451), LL(0x0cd2d26fd2debdb9), LL(0x0ef5f5f3f5fb06f7),
    LL(0x967979f979ef80f2), LL(0x306f6fa16f5fcede), LL(0x6d91917e91fcef3f), LL(0xf852525552aa07a4),
    LL(0x4760609d6027fdc0), LL(0x35bcbccabc897665), LL(0x379b9b569baccd2b), LL(0x8a8e8e028e048c01),
    LL(0xd2a3a3b6a371155b), LL(0x6c0c0c300c603c18), LL(0x847b7bf17bff8af6), LL(0x803535d435b5e16a),
    LL(0xf51d1d741de8693a), LL(0xb3e0e0a7e05347dd), LL(0x21d7d77bd7f6acb3), LL(0x9cc2c22fc25eed99),
    LL(0x432e2eb82e6d965c), LL(0x294b4b314b627a96), LL(0x5dfefedffea321e1), LL(0xd5575741578216ae),
    LL(0xbd15155415a8412a), LL(0xe87777c1779fb6ee), LL(0x923737dc37a5eb6e), LL(0x9ee5e5b3e57b56d7),
    LL(0x139f9f469f8cd923), LL(0x23f0f0e7f0d317fd), LL(0x204a4a354a6a7f94), LL(0x44dada4fda9e95a9),
    LL(0xa258587d58fa25b0), LL(0xcfc9c903c906ca8f), LL(0x7c2929a429558d52), LL(0x5a0a0a280a502214),
    LL(0x50b1b1feb1e14f7f), LL(0xc9a0a0baa0691a5d), LL(0x146b6bb16b7fdad6), LL(0xd985852e855cab17),
    LL(0x3cbdbdcebd817367), LL(0x8f5d5d695dd234ba), LL(0x9010104010805020), LL(0x07f4f4f7f4f303f5),
    LL(0xddcbcb0bcb16c08b), LL(0xd33e3ef83eedc67c), LL(0x2d0505140528110a), LL(0x78676781671fe6ce),
    LL(0x97e4e4b7e47353d5), LL(0x0227279c2725bb4e), LL(0x7341411941325882), LL(0xa78b8b168b2c9d0b),
    LL(0xf6a7a7a6a7510153), LL(0xb27d7de97dcf94fa), LL(0x4995956e95dcfb37), LL(0x56d8d847d88e9fad),
    LL(0x70fbfbcbfb8b30eb), LL(0xcdeeee9fee2371c1), LL(0xbb7c7ced7cc791f8), LL(0x716666856617e3cc),
    LL(0x7bdddd53dda68ea7), LL(0xaf17175c17b84b2e), LL(0x454747014702468e), LL(0x1a9e9e429e84dc21),
    LL(0xd4caca0fca1ec589), LL(0x582d2db42d75995a), LL(0x2ebfbfc6bf917963), LL(0x3f07071c07381b0e),
    LL(0xacadad8ead012347), LL(0xb05a5a755aea2fb4), LL(0xef838336836cb51b), LL(0xb63333cc3385ff66),
    LL(0x5c636391633ff2c6), LL(0x1202020802100a04), LL(0x93aaaa92aa393849), LL(0xde7171d971afa8e2),
    LL(0xc6c8c807c80ecf8d), LL(0xd119196419c87d32), LL(0x3b49493949727092), LL(0x5fd9d943d9869aaf),
    LL(0x31f2f2eff2c31df9), LL(0xa8e3e3abe34b48db), LL(0xb95b5b715be22ab6), LL(0xbc88881a8834920d),
    LL(0x3e9a9a529aa4c829), LL(0x0b262698262dbe4c), LL(0xbf3232c8328dfa64), LL(0x59b0b0fab0e94a7d),
    LL(0xf2e9e983e91b6acf), LL(0x770f0f3c0f78331e), LL(0x33d5d573d5e6a6b7), LL(0xf480803a8074ba1d),
    LL(0x27bebec2be997c61), LL(0xebcdcd13cd26de87), LL(0x893434d034bde468), LL(0x3248483d487a7590),
    LL(0x54ffffdbffab24e3), LL(0x8d7a7af57af78ff4), LL(0x6490907a90f4ea3d), LL(0x9d5f5f615fc23ebe),
    LL(0x3d202080201da040), LL(0x0f6868bd6867d5d0), LL(0xca1a1a681ad07234), LL(0xb7aeae82ae192c41),
    LL(0x7db4b4eab4c95e75), LL(0xce54544d549a19a8), LL(0x7f93937693ece53b), LL(0x2f222288220daa44),
    LL(0x6364648d6407e9c8), LL(0x2af1f1e3f1db12ff), LL(0xcc7373d173bfa2e6), LL(0x8212124812905a24),
    LL(0x7a40401d403a5d80), LL(0x4808082008402810), LL(0x95c3c32bc356e89b), LL(0xdfecec97ec337bc5),
    LL(0x4ddbdb4bdb9690ab), LL(0xc0a1a1bea1611f5f), LL(0x918d8d0e8d1c8307), LL(0xc83d3df43df5c97a),
    LL(0x5b97976697ccf133), LL(0x0000000000000000), LL(0xf9cfcf1bcf36d483), LL(0x6e2b2bac2b458756),
    LL(0xe17676c57697b3ec), LL(0xe68282328264b019), LL(0x28d6d67fd6fea9b1), LL(0xc31b1b6c1bd87736),
    LL(0x74b5b5eeb5c15b77), LL(0xbeafaf86af112943), LL(0x1d6a6ab56a77dfd4), LL(0xea50505d50ba0da0),
    LL(0x5745450945124c8a), LL(0x38f3f3ebf3cb18fb), LL(0xad3030c0309df060), LL(0xc4efef9bef2b74c3),
    LL(0xda3f3ffc3fe5c37e), LL(0xc755554955921caa), LL(0xdba2a2b2a2791059), LL(0xe9eaea8fea0365c9),
    LL(0x6a656589650fecca), LL(0x03babad2bab96869), LL(0x4a2f2fbc2f65935e), LL(0x8ec0c027c04ee79d),
    LL(0x60dede5fdebe81a1), LL(0xfc1c1c701ce06c38), LL(0x46fdfdd3fdbb2ee7), LL(0x1f4d4d294d52649a),
    LL(0x7692927292e4e039), LL(0xfa7575c9758fbcea), LL(0x3606061806301e0c), LL(0xae8a8a128a249809),
    LL(0x4bb2b2f2b2f94079), LL(0x85e6e6bfe66359d1), LL(0x7e0e0e380e70361c), LL(0xe71f1f7c1ff8633e),
    LL(0x556262956237f7c4), LL(0x3ad4d477d4eea3b5), LL(0x81a8a89aa829324d), LL(0x5296966296c4f431),
    LL(0x62f9f9c3f99b3aef), LL(0xa3c5c533c566f697), LL(0x102525942535b14a), LL(0xab59597959f220b2),
    LL(0xd084842a8454ae15), LL(0xc57272d572b7a7e4), LL(0xec3939e439d5dd72), LL(0x164c4c2d4c5a6198),
    LL(0x945e5e655eca3bbc), LL(0x9f7878fd78e785f0), LL(0xe53838e038ddd870), LL(0x988c8c0a8c148605),
    LL(0x17d1d163d1c6b2bf), LL(0xe4a5a5aea5410b57), LL(0xa1e2e2afe2434dd9), LL(0x4e616199612ff8c2),
    LL(0x42b3b3f6b3f1457b), LL(0x342121842115a542), LL(0x089c9c4a9c94d625), LL(0xee1e1e781ef0663c),
    LL(0x6143431143225286), LL(0xb1c7c73bc776fc93), LL(0x4ffcfcd7fcb32be5), LL(0x2404041004201408),
    LL(0xe351515951b208a2), LL(0x2599995e99bcc72f), LL(0x226d6da96d4fc4da), LL(0x650d0d340d68391a),
    LL(0x79fafacffa8335e9), LL(0x69dfdf5bdfb684a3), LL(0xa97e7ee57ed79bfc), LL(0x19242490243db448),
    LL(0xfe3b3bec3bc5d776), LL(0x9aabab96ab313d4b), LL(0xf0cece1fce3ed181), LL(0x9911114411885522),
    LL(0x838f8f068f0c8903), LL(0x044e4e254e4a6b9c), LL(0x66b7b7e6b7d15173), LL(0xe0ebeb8beb0b60cb),
    LL(0xc13c3cf03cfdcc78), LL(0xfd81813e817cbf1f), LL(0x4094946a94d4fe35), LL(0x1cf7f7fbf7eb0cf3),
    LL(0x18b9b9deb9a1676f), LL(0x8b13134c13985f26), LL(0x512c2cb02c7d9c58), LL(0x05d3d36bd3d6b8bb),
    LL(0x8ce7e7bbe76b5cd3), LL(0x396e6ea56e57cbdc), LL(0xaac4c437c46ef395), LL(0x1b03030c03180f06),
    LL(0xdc565645568a13ac), LL(0x5e44440d441a4988), LL(0xa07f7fe17fdf9efe), LL(0x88a9a99ea921374f),
    LL(0x672a2aa82a4d8254), LL(0x0abbbbd6bbb16d6b), LL(0x87c1c123c146e29f), LL(0xf153535153a202a6),
    LL(0x72dcdc57dcae8ba5), LL(0x530b0b2c0b582716), LL(0x019d9d4e9d9cd327), LL(0x2b6c6cad6c47c1d8),
    LL(0xa43131c43195f562), LL(0xf37474cd7487b9e8), LL(0x15f6f6fff6e309f1), LL(0x4c464605460a438c),
    LL(0xa5acac8aac092645), LL(0xb589891e893c970f), LL(0xb414145014a04428), LL(0xbae1e1a3e15b42df),
    LL(0xa616165816b04e2c), LL(0xf73a3ae83acdd274), LL(0x066969b9696fd0d2), LL(0x4109092409482d12),
    LL(0xd77070dd70a7ade0), LL(0x6fb6b6e2b6d95471), LL(0x1ed0d067d0ceb7bd), LL(0xd6eded93ed3b7ec7),
    LL(0xe2cccc17cc2edb85), LL(0x68424215422a5784), LL(0x2c98985a98b4c22d), LL(0xeda4a4aaa4490e55),
    LL(0x752828a0285d8850), LL(0x865c5c6d5cda31b8), LL(0x6bf8f8c7f8933fed), LL(0xc28686228644a411),
};

__constant__ u64 C2[256] = {
    LL(0x30d818186018c078), LL(0x462623238c2305af), LL(0x91b8c6c63fc67ef9), LL(0xcdfbe8e887e8136f),
    LL(0x13cb878726874ca1), LL(0x6d11b8b8dab8a962), LL(0x0209010104010805), LL(0x9e0d4f4f214f426e),
    LL(0x6c9b3636d836adee), LL(0x51ffa6a6a2a65904), LL(0xb90cd2d26fd2debd), LL(0xf70ef5f5f3f5fb06),
    LL(0xf2967979f979ef80), LL(0xde306f6fa16f5fce), LL(0x3f6d91917e91fcef), LL(0xa4f852525552aa07),
    LL(0xc04760609d6027fd), LL(0x6535bcbccabc8976), LL(0x2b379b9b569baccd), LL(0x018a8e8e028e048c),
    LL(0x5bd2a3a3b6a37115), LL(0x186c0c0c300c603c), LL(0xf6847b7bf17bff8a), LL(0x6a803535d435b5e1),
    LL(0x3af51d1d741de869), LL(0xddb3e0e0a7e05347), LL(0xb321d7d77bd7f6ac), LL(0x999cc2c22fc25eed),
    LL(0x5c432e2eb82e6d96), LL(0x96294b4b314b627a), LL(0xe15dfefedffea321), LL(0xaed5575741578216),
    LL(0x2abd15155415a841), LL(0xeee87777c1779fb6), LL(0x6e923737dc37a5eb), LL(0xd79ee5e5b3e57b56),
    LL(0x23139f9f469f8cd9), LL(0xfd23f0f0e7f0d317), LL(0x94204a4a354a6a7f), LL(0xa944dada4fda9e95),
    LL(0xb0a258587d58fa25), LL(0x8fcfc9c903c906ca), LL(0x527c2929a429558d), LL(0x145a0a0a280a5022),
    LL(0x7f50b1b1feb1e14f), LL(0x5dc9a0a0baa0691a), LL(0xd6146b6bb16b7fda), LL(0x17d985852e855cab),
    LL(0x673cbdbdcebd8173), LL(0xba8f5d5d695dd234), LL(0x2090101040108050), LL(0xf507f4f4f7f4f303),
    LL(0x8bddcbcb0bcb16c0), LL(0x7cd33e3ef83eedc6), LL(0x0a2d050514052811), LL(0xce78676781671fe6),
    LL(0xd597e4e4b7e47353), LL(0x4e0227279c2725bb), LL(0x8273414119413258), LL(0x0ba78b8b168b2c9d),
    LL(0x53f6a7a7a6a75101), LL(0xfab27d7de97dcf94), LL(0x374995956e95dcfb), LL(0xad56d8d847d88e9f),
    LL(0xeb70fbfbcbfb8b30), LL(0xc1cdeeee9fee2371), LL(0xf8bb7c7ced7cc791), LL(0xcc716666856617e3),
    LL(0xa77bdddd53dda68e), LL(0x2eaf17175c17b84b), LL(0x8e45474701470246), LL(0x211a9e9e429e84dc),
    LL(0x89d4caca0fca1ec5), LL(0x5a582d2db42d7599), LL(0x632ebfbfc6bf9179), LL(0x0e3f07071c07381b),
    LL(0x47acadad8ead0123), LL(0xb4b05a5a755aea2f), LL(0x1bef838336836cb5), LL(0x66b63333cc3385ff),
    LL(0xc65c636391633ff2), LL(0x041202020802100a), LL(0x4993aaaa92aa3938), LL(0xe2de7171d971afa8),
    LL(0x8dc6c8c807c80ecf), LL(0x32d119196419c87d), LL(0x923b494939497270), LL(0xaf5fd9d943d9869a),
    LL(0xf931f2f2eff2c31d), LL(0xdba8e3e3abe34b48), LL(0xb6b95b5b715be22a), LL(0x0dbc88881a883492),
    LL(0x293e9a9a529aa4c8), LL(0x4c0b262698262dbe), LL(0x64bf3232c8328dfa), LL(0x7d59b0b0fab0e94a),
    LL(0xcff2e9e983e91b6a), LL(0x1e770f0f3c0f7833), LL(0xb733d5d573d5e6a6), LL(0x1df480803a8074ba),
    LL(0x6127bebec2be997c), LL(0x87ebcdcd13cd26de), LL(0x68893434d034bde4), LL(0x903248483d487a75),
    LL(0xe354ffffdbffab24), LL(0xf48d7a7af57af78f), LL(0x3d6490907a90f4ea), LL(0xbe9d5f5f615fc23e),
    LL(0x403d202080201da0), LL(0xd00f6868bd6867d5), LL(0x34ca1a1a681ad072), LL(0x41b7aeae82ae192c),
    LL(0x757db4b4eab4c95e), LL(0xa8ce54544d549a19), LL(0x3b7f93937693ece5), LL(0x442f222288220daa),
    LL(0xc86364648d6407e9), LL(0xff2af1f1e3f1db12), LL(0xe6cc7373d173bfa2), LL(0x248212124812905a),
    LL(0x807a40401d403a5d), LL(0x1048080820084028), LL(0x9b95c3c32bc356e8), LL(0xc5dfecec97ec337b),
    LL(0xab4ddbdb4bdb9690), LL(0x5fc0a1a1bea1611f), LL(0x07918d8d0e8d1c83), LL(0x7ac83d3df43df5c9),
    LL(0x335b97976697ccf1), LL(0x0000000000000000), LL(0x83f9cfcf1bcf36d4), LL(0x566e2b2bac2b4587),
    LL(0xece17676c57697b3), LL(0x19e68282328264b0), LL(0xb128d6d67fd6fea9), LL(0x36c31b1b6c1bd877),
    LL(0x7774b5b5eeb5c15b), LL(0x43beafaf86af1129), LL(0xd41d6a6ab56a77df), LL(0xa0ea50505d50ba0d),
    LL(0x8a5745450945124c), LL(0xfb38f3f3ebf3cb18), LL(0x60ad3030c0309df0), LL(0xc3c4efef9bef2b74),
    LL(0x7eda3f3ffc3fe5c3), LL(0xaac755554955921c), LL(0x59dba2a2b2a27910), LL(0xc9e9eaea8fea0365),
    LL(0xca6a656589650fec), LL(0x6903babad2bab968), LL(0x5e4a2f2fbc2f6593), LL(0x9d8ec0c027c04ee7),
    LL(0xa160dede5fdebe81), LL(0x38fc1c1c701ce06c), LL(0xe746fdfdd3fdbb2e), LL(0x9a1f4d4d294d5264),
    LL(0x397692927292e4e0), LL(0xeafa7575c9758fbc), LL(0x0c3606061806301e), LL(0x09ae8a8a128a2498),
    LL(0x794bb2b2f2b2f940), LL(0xd185e6e6bfe66359), LL(0x1c7e0e0e380e7036), LL(0x3ee71f1f7c1ff863),
    LL(0xc4556262956237f7), LL(0xb53ad4d477d4eea3), LL(0x4d81a8a89aa82932), LL(0x315296966296c4f4),
    LL(0xef62f9f9c3f99b3a), LL(0x97a3c5c533c566f6), LL(0x4a102525942535b1), LL(0xb2ab59597959f220),
    LL(0x15d084842a8454ae), LL(0xe4c57272d572b7a7), LL(0x72ec3939e439d5dd), LL(0x98164c4c2d4c5a61),
    LL(0xbc945e5e655eca3b), LL(0xf09f7878fd78e785), LL(0x70e53838e038ddd8), LL(0x05988c8c0a8c1486),
    LL(0xbf17d1d163d1c6b2), LL(0x57e4a5a5aea5410b), LL(0xd9a1e2e2afe2434d), LL(0xc24e616199612ff8),
    LL(0x7b42b3b3f6b3f145), LL(0x42342121842115a5), LL(0x25089c9c4a9c94d6), LL(0x3cee1e1e781ef066),
    LL(0x8661434311432252), LL(0x93b1c7c73bc776fc), LL(0xe54ffcfcd7fcb32b), LL(0x0824040410042014),
    LL(0xa2e351515951b208), LL(0x2f2599995e99bcc7), LL(0xda226d6da96d4fc4), LL(0x1a650d0d340d6839),
    LL(0xe979fafacffa8335), LL(0xa369dfdf5bdfb684), LL(0xfca97e7ee57ed79b), LL(0x4819242490243db4),
    LL(0x76fe3b3bec3bc5d7), LL(0x4b9aabab96ab313d), LL(0x81f0cece1fce3ed1), LL(0x2299111144118855),
    LL(0x03838f8f068f0c89), LL(0x9c044e4e254e4a6b), LL(0x7366b7b7e6b7d151), LL(0xcbe0ebeb8beb0b60),
    LL(0x78c13c3cf03cfdcc), LL(0x1ffd81813e817cbf), LL(0x354094946a94d4fe), LL(0xf31cf7f7fbf7eb0c),
    LL(0x6f18b9b9deb9a167), LL(0x268b13134c13985f), LL(0x58512c2cb02c7d9c), LL(0xbb05d3d36bd3d6b8),
    LL(0xd38ce7e7bbe76b5c), LL(0xdc396e6ea56e57cb), LL(0x95aac4c437c46ef3), LL(0x061b03030c03180f),
    LL(0xacdc565645568a13), LL(0x885e44440d441a49), LL(0xfea07f7fe17fdf9e), LL(0x4f88a9a99ea92137),
    LL(0x54672a2aa82a4d82), LL(0x6b0abbbbd6bbb16d), LL(0x9f87c1c123c146e2), LL(0xa6f153535153a202),
    LL(0xa572dcdc57dcae8b), LL(0x16530b0b2c0b5827), LL(0x27019d9d4e9d9cd3), LL(0xd82b6c6cad6c47c1),
    LL(0x62a43131c43195f5), LL(0xe8f37474cd7487b9), LL(0xf115f6f6fff6e309), LL(0x8c4c464605460a43),
    LL(0x45a5acac8aac0926), LL(0x0fb589891e893c97), LL(0x28b414145014a044), LL(0xdfbae1e1a3e15b42),
    LL(0x2ca616165816b04e), LL(0x74f73a3ae83acdd2), LL(0xd2066969b9696fd0), LL(0x124109092409482d),
    LL(0xe0d77070dd70a7ad), LL(0x716fb6b6e2b6d954), LL(0xbd1ed0d067d0ceb7), LL(0xc7d6eded93ed3b7e),
    LL(0x85e2cccc17cc2edb), LL(0x8468424215422a57), LL(0x2d2c98985a98b4c2), LL(0x55eda4a4aaa4490e),
    LL(0x50752828a0285d88), LL(0xb8865c5c6d5cda31), LL(0xed6bf8f8c7f8933f), LL(0x11c28686228644a4),
};

__constant__ u64 C3[256] = {
    LL(0x7830d818186018c0), LL(0xaf462623238c2305), LL(0xf991b8c6c63fc67e), LL(0x6fcdfbe8e887e813),
    LL(0xa113cb878726874c), LL(0x626d11b8b8dab8a9), LL(0x0502090101040108), LL(0x6e9e0d4f4f214f42),
    LL(0xee6c9b3636d836ad), LL(0x0451ffa6a6a2a659), LL(0xbdb90cd2d26fd2de), LL(0x06f70ef5f5f3f5fb),
    LL(0x80f2967979f979ef), LL(0xcede306f6fa16f5f), LL(0xef3f6d91917e91fc), LL(0x07a4f852525552aa),
    LL(0xfdc04760609d6027), LL(0x766535bcbccabc89), LL(0xcd2b379b9b569bac), LL(0x8c018a8e8e028e04),
    LL(0x155bd2a3a3b6a371), LL(0x3c186c0c0c300c60), LL(0x8af6847b7bf17bff), LL(0xe16a803535d435b5),
    LL(0x693af51d1d741de8), LL(0x47ddb3e0e0a7e053), LL(0xacb321d7d77bd7f6), LL(0xed999cc2c22fc25e),
    LL(0x965c432e2eb82e6d), LL(0x7a96294b4b314b62), LL(0x21e15dfefedffea3), LL(0x16aed55757415782),
    LL(0x412abd15155415a8), LL(0xb6eee87777c1779f), LL(0xeb6e923737dc37a5), LL(0x56d79ee5e5b3e57b),
    LL(0xd923139f9f469f8c), LL(0x17fd23f0f0e7f0d3), LL(0x7f94204a4a354a6a), LL(0x95a944dada4fda9e),
    LL(0x25b0a258587d58fa), LL(0xca8fcfc9c903c906), LL(0x8d527c2929a42955), LL(0x22145a0a0a280a50),
    LL(0x4f7f50b1b1feb1e1), LL(0x1a5dc9a0a0baa069), LL(0xdad6146b6bb16b7f), LL(0xab17d985852e855c),
    LL(0x73673cbdbdcebd81), LL(0x34ba8f5d5d695dd2), LL(0x5020901010401080), LL(0x03f507f4f4f7f4f3),
    LL(0xc08bddcbcb0bcb16), LL(0xc67cd33e3ef83eed), LL(0x110a2d0505140528), LL(0xe6ce78676781671f),
    LL(0x53d597e4e4b7e473), LL(0xbb4e0227279c2725), LL(0x5882734141194132), LL(0x9d0ba78b8b168b2c),
    LL(0x0153f6a7a7a6a751), LL(0x94fab27d7de97dcf), LL(0xfb374995956e95dc), LL(0x9fad56d8d847d88e),
    LL(0x30eb70fbfbcbfb8b), LL(0x71c1cdeeee9fee23), LL(0x91f8bb7c7ced7cc7), LL(0xe3cc716666856617),
    LL(0x8ea77bdddd53dda6), LL(0x4b2eaf17175c17b8), LL(0x468e454747014702), LL(0xdc211a9e9e429e84),
    LL(0xc589d4caca0fca1e), LL(0x995a582d2db42d75), LL(0x79632ebfbfc6bf91), LL(0x1b0e3f07071c0738),
    LL(0x2347acadad8ead01), LL(0x2fb4b05a5a755aea), LL(0xb51bef838336836c), LL(0xff66b63333cc3385),
    LL(0xf2c65c636391633f), LL(0x0a04120202080210), LL(0x384993aaaa92aa39), LL(0xa8e2de7171d971af),
    LL(0xcf8dc6c8c807c80e), LL(0x7d32d119196419c8), LL(0x70923b4949394972), LL(0x9aaf5fd9d943d986),
    LL(0x1df931f2f2eff2c3), LL(0x48dba8e3e3abe34b), LL(0x2ab6b95b5b715be2), LL(0x920dbc88881a8834),
    LL(0xc8293e9a9a529aa4), LL(0xbe4c0b262698262d), LL(0xfa64bf3232c8328d), LL(0x4a7d59b0b0fab0e9),
    LL(0x6acff2e9e983e91b), LL(0x331e770f0f3c0f78), LL(0xa6b733d5d573d5e6), LL(0xba1df480803a8074),
    LL(0x7c6127bebec2be99), LL(0xde87ebcdcd13cd26), LL(0xe468893434d034bd), LL(0x75903248483d487a),
    LL(0x24e354ffffdbffab), LL(0x8ff48d7a7af57af7), LL(0xea3d6490907a90f4), LL(0x3ebe9d5f5f615fc2),
    LL(0xa0403d202080201d), LL(0xd5d00f6868bd6867), LL(0x7234ca1a1a681ad0), LL(0x2c41b7aeae82ae19),
    LL(0x5e757db4b4eab4c9), LL(0x19a8ce54544d549a), LL(0xe53b7f93937693ec), LL(0xaa442f222288220d),
    LL(0xe9c86364648d6407), LL(0x12ff2af1f1e3f1db), LL(0xa2e6cc7373d173bf), LL(0x5a24821212481290),
    LL(0x5d807a40401d403a), LL(0x2810480808200840), LL(0xe89b95c3c32bc356), LL(0x7bc5dfecec97ec33),
    LL(0x90ab4ddbdb4bdb96), LL(0x1f5fc0a1a1bea161), LL(0x8307918d8d0e8d1c), LL(0xc97ac83d3df43df5),
    LL(0xf1335b97976697cc), LL(0x0000000000000000), LL(0xd483f9cfcf1bcf36), LL(0x87566e2b2bac2b45),
    LL(0xb3ece17676c57697), LL(0xb019e68282328264), LL(0xa9b128d6d67fd6fe), LL(0x7736c31b1b6c1bd8),
    LL(0x5b7774b5b5eeb5c1), LL(0x2943beafaf86af11), LL(0xdfd41d6a6ab56a77), LL(0x0da0ea50505d50ba),
    LL(0x4c8a574545094512), LL(0x18fb38f3f3ebf3cb), LL(0xf060ad3030c0309d), LL(0x74c3c4efef9bef2b),
    LL(0xc37eda3f3ffc3fe5), LL(0x1caac75555495592), LL(0x1059dba2a2b2a279), LL(0x65c9e9eaea8fea03),
    LL(0xecca6a656589650f), LL(0x686903babad2bab9), LL(0x935e4a2f2fbc2f65), LL(0xe79d8ec0c027c04e),
    LL(0x81a160dede5fdebe), LL(0x6c38fc1c1c701ce0), LL(0x2ee746fdfdd3fdbb), LL(0x649a1f4d4d294d52),
    LL(0xe0397692927292e4), LL(0xbceafa7575c9758f), LL(0x1e0c360606180630), LL(0x9809ae8a8a128a24),
    LL(0x40794bb2b2f2b2f9), LL(0x59d185e6e6bfe663), LL(0x361c7e0e0e380e70), LL(0x633ee71f1f7c1ff8),
    LL(0xf7c4556262956237), LL(0xa3b53ad4d477d4ee), LL(0x324d81a8a89aa829), LL(0xf4315296966296c4),
    LL(0x3aef62f9f9c3f99b), LL(0xf697a3c5c533c566), LL(0xb14a102525942535), LL(0x20b2ab59597959f2),
    LL(0xae15d084842a8454), LL(0xa7e4c57272d572b7), LL(0xdd72ec3939e439d5), LL(0x6198164c4c2d4c5a),
    LL(0x3bbc945e5e655eca), LL(0x85f09f7878fd78e7), LL(0xd870e53838e038dd), LL(0x8605988c8c0a8c14),
    LL(0xb2bf17d1d163d1c6), LL(0x0b57e4a5a5aea541), LL(0x4dd9a1e2e2afe243), LL(0xf8c24e616199612f),
    LL(0x457b42b3b3f6b3f1), LL(0xa542342121842115), LL(0xd625089c9c4a9c94), LL(0x663cee1e1e781ef0),
    LL(0x5286614343114322), LL(0xfc93b1c7c73bc776), LL(0x2be54ffcfcd7fcb3), LL(0x1408240404100420),
    LL(0x08a2e351515951b2), LL(0xc72f2599995e99bc), LL(0xc4da226d6da96d4f), LL(0x391a650d0d340d68),
    LL(0x35e979fafacffa83), LL(0x84a369dfdf5bdfb6), LL(0x9bfca97e7ee57ed7), LL(0xb44819242490243d),
    LL(0xd776fe3b3bec3bc5), LL(0x3d4b9aabab96ab31), LL(0xd181f0cece1fce3e), LL(0x5522991111441188),
    LL(0x8903838f8f068f0c), LL(0x6b9c044e4e254e4a), LL(0x517366b7b7e6b7d1), LL(0x60cbe0ebeb8beb0b),
    LL(0xcc78c13c3cf03cfd), LL(0xbf1ffd81813e817c), LL(0xfe354094946a94d4), LL(0x0cf31cf7f7fbf7eb),
    LL(0x676f18b9b9deb9a1), LL(0x5f268b13134c1398), LL(0x9c58512c2cb02c7d), LL(0xb8bb05d3d36bd3d6),
    LL(0x5cd38ce7e7bbe76b), LL(0xcbdc396e6ea56e57), LL(0xf395aac4c437c46e), LL(0x0f061b03030c0318),
    LL(0x13acdc565645568a), LL(0x49885e44440d441a), LL(0x9efea07f7fe17fdf), LL(0x374f88a9a99ea921),
    LL(0x8254672a2aa82a4d), LL(0x6d6b0abbbbd6bbb1), LL(0xe29f87c1c123c146), LL(0x02a6f153535153a2),
    LL(0x8ba572dcdc57dcae), LL(0x2716530b0b2c0b58), LL(0xd327019d9d4e9d9c), LL(0xc1d82b6c6cad6c47),
    LL(0xf562a43131c43195), LL(0xb9e8f37474cd7487), LL(0x09f115f6f6fff6e3), LL(0x438c4c464605460a),
    LL(0x2645a5acac8aac09), LL(0x970fb589891e893c), LL(0x4428b414145014a0), LL(0x42dfbae1e1a3e15b),
    LL(0x4e2ca616165816b0), LL(0xd274f73a3ae83acd), LL(0xd0d2066969b9696f), LL(0x2d12410909240948),
    LL(0xade0d77070dd70a7), LL(0x54716fb6b6e2b6d9), LL(0xb7bd1ed0d067d0ce), LL(0x7ec7d6eded93ed3b),
    LL(0xdb85e2cccc17cc2e), LL(0x578468424215422a), LL(0xc22d2c98985a98b4), LL(0x0e55eda4a4aaa449),
    LL(0x8850752828a0285d), LL(0x31b8865c5c6d5cda), LL(0x3fed6bf8f8c7f893), LL(0xa411c28686228644),
};

__constant__ u64 C4[256] = {
    LL(0xc07830d818186018), LL(0x05af462623238c23), LL(0x7ef991b8c6c63fc6), LL(0x136fcdfbe8e887e8),
    LL(0x4ca113cb87872687), LL(0xa9626d11b8b8dab8), LL(0x0805020901010401), LL(0x426e9e0d4f4f214f),
    LL(0xadee6c9b3636d836), LL(0x590451ffa6a6a2a6), LL(0xdebdb90cd2d26fd2), LL(0xfb06f70ef5f5f3f5),
    LL(0xef80f2967979f979), LL(0x5fcede306f6fa16f), LL(0xfcef3f6d91917e91), LL(0xaa07a4f852525552),
    LL(0x27fdc04760609d60), LL(0x89766535bcbccabc), LL(0xaccd2b379b9b569b), LL(0x048c018a8e8e028e),
    LL(0x71155bd2a3a3b6a3), LL(0x603c186c0c0c300c), LL(0xff8af6847b7bf17b), LL(0xb5e16a803535d435),
    LL(0xe8693af51d1d741d), LL(0x5347ddb3e0e0a7e0), LL(0xf6acb321d7d77bd7), LL(0x5eed999cc2c22fc2),
    LL(0x6d965c432e2eb82e), LL(0x627a96294b4b314b), LL(0xa321e15dfefedffe), LL(0x8216aed557574157),
    LL(0xa8412abd15155415), LL(0x9fb6eee87777c177), LL(0xa5eb6e923737dc37), LL(0x7b56d79ee5e5b3e5),
    LL(0x8cd923139f9f469f), LL(0xd317fd23f0f0e7f0), LL(0x6a7f94204a4a354a), LL(0x9e95a944dada4fda),
    LL(0xfa25b0a258587d58), LL(0x06ca8fcfc9c903c9), LL(0x558d527c2929a429), LL(0x5022145a0a0a280a),
    LL(0xe14f7f50b1b1feb1), LL(0x691a5dc9a0a0baa0), LL(0x7fdad6146b6bb16b), LL(0x5cab17d985852e85),
    LL(0x8173673cbdbdcebd), LL(0xd234ba8f5d5d695d), LL(0x8050209010104010), LL(0xf303f507f4f4f7f4),
    LL(0x16c08bddcbcb0bcb), LL(0xedc67cd33e3ef83e), LL(0x28110a2d05051405), LL(0x1fe6ce7867678167),
    LL(0x7353d597e4e4b7e4), LL(0x25bb4e0227279c27), LL(0x3258827341411941), LL(0x2c9d0ba78b8b168b),
    LL(0x510153f6a7a7a6a7), LL(0xcf94fab27d7de97d), LL(0xdcfb374995956e95), LL(0x8e9fad56d8d847d8),
    LL(0x8b30eb70fbfbcbfb), LL(0x2371c1cdeeee9fee), LL(0xc791f8bb7c7ced7c), LL(0x17e3cc7166668566),
    LL(0xa68ea77bdddd53dd), LL(0xb84b2eaf17175c17), LL(0x02468e4547470147), LL(0x84dc211a9e9e429e),
    LL(0x1ec589d4caca0fca), LL(0x75995a582d2db42d), LL(0x9179632ebfbfc6bf), LL(0x381b0e3f07071c07),
    LL(0x012347acadad8ead), LL(0xea2fb4b05a5a755a), LL(0x6cb51bef83833683), LL(0x85ff66b63333cc33),
    LL(0x3ff2c65c63639163), LL(0x100a041202020802), LL(0x39384993aaaa92aa), LL(0xafa8e2de7171d971),
    LL(0x0ecf8dc6c8c807c8), LL(0xc87d32d119196419), LL(0x7270923b49493949), LL(0x869aaf5fd9d943d9),
    LL(0xc31df931f2f2eff2), LL(0x4b48dba8e3e3abe3), LL(0xe22ab6b95b5b715b), LL(0x34920dbc88881a88),
    LL(0xa4c8293e9a9a529a), LL(0x2dbe4c0b26269826), LL(0x8dfa64bf3232c832), LL(0xe94a7d59b0b0fab0),
    LL(0x1b6acff2e9e983e9), LL(0x78331e770f0f3c0f), LL(0xe6a6b733d5d573d5), LL(0x74ba1df480803a80),
    LL(0x997c6127bebec2be), LL(0x26de87ebcdcd13cd), LL(0xbde468893434d034), LL(0x7a75903248483d48),
    LL(0xab24e354ffffdbff), LL(0xf78ff48d7a7af57a), LL(0xf4ea3d6490907a90), LL(0xc23ebe9d5f5f615f),
    LL(0x1da0403d20208020), LL(0x67d5d00f6868bd68), LL(0xd07234ca1a1a681a), LL(0x192c41b7aeae82ae),
    LL(0xc95e757db4b4eab4), LL(0x9a19a8ce54544d54), LL(0xece53b7f93937693), LL(0x0daa442f22228822),
    LL(0x07e9c86364648d64), LL(0xdb12ff2af1f1e3f1), LL(0xbfa2e6cc7373d173), LL(0x905a248212124812),
    LL(0x3a5d807a40401d40), LL(0x4028104808082008), LL(0x56e89b95c3c32bc3), LL(0x337bc5dfecec97ec),
    LL(0x9690ab4ddbdb4bdb), LL(0x611f5fc0a1a1bea1), LL(0x1c8307918d8d0e8d), LL(0xf5c97ac83d3df43d),
    LL(0xccf1335b97976697), LL(0x0000000000000000), LL(0x36d483f9cfcf1bcf), LL(0x4587566e2b2bac2b),
    LL(0x97b3ece17676c576), LL(0x64b019e682823282), LL(0xfea9b128d6d67fd6), LL(0xd87736c31b1b6c1b),
    LL(0xc15b7774b5b5eeb5), LL(0x112943beafaf86af), LL(0x77dfd41d6a6ab56a), LL(0xba0da0ea50505d50),
    LL(0x124c8a5745450945), LL(0xcb18fb38f3f3ebf3), LL(0x9df060ad3030c030), LL(0x2b74c3c4efef9bef),
    LL(0xe5c37eda3f3ffc3f), LL(0x921caac755554955), LL(0x791059dba2a2b2a2), LL(0x0365c9e9eaea8fea),
    LL(0x0fecca6a65658965), LL(0xb9686903babad2ba), LL(0x65935e4a2f2fbc2f), LL(0x4ee79d8ec0c027c0),
    LL(0xbe81a160dede5fde), LL(0xe06c38fc1c1c701c), LL(0xbb2ee746fdfdd3fd), LL(0x52649a1f4d4d294d),
    LL(0xe4e0397692927292), LL(0x8fbceafa7575c975), LL(0x301e0c3606061806), LL(0x249809ae8a8a128a),
    LL(0xf940794bb2b2f2b2), LL(0x6359d185e6e6bfe6), LL(0x70361c7e0e0e380e), LL(0xf8633ee71f1f7c1f),
    LL(0x37f7c45562629562), LL(0xeea3b53ad4d477d4), LL(0x29324d81a8a89aa8), LL(0xc4f4315296966296),
    LL(0x9b3aef62f9f9c3f9), LL(0x66f697a3c5c533c5), LL(0x35b14a1025259425), LL(0xf220b2ab59597959),
    LL(0x54ae15d084842a84), LL(0xb7a7e4c57272d572), LL(0xd5dd72ec3939e439), LL(0x5a6198164c4c2d4c),
    LL(0xca3bbc945e5e655e), LL(0xe785f09f7878fd78), LL(0xddd870e53838e038), LL(0x148605988c8c0a8c),
    LL(0xc6b2bf17d1d163d1), LL(0x410b57e4a5a5aea5), LL(0x434dd9a1e2e2afe2), LL(0x2ff8c24e61619961),
    LL(0xf1457b42b3b3f6b3), LL(0x15a5423421218421), LL(0x94d625089c9c4a9c), LL(0xf0663cee1e1e781e),
    LL(0x2252866143431143), LL(0x76fc93b1c7c73bc7), LL(0xb32be54ffcfcd7fc), LL(0x2014082404041004),
    LL(0xb208a2e351515951), LL(0xbcc72f2599995e99), LL(0x4fc4da226d6da96d), LL(0x68391a650d0d340d),
    LL(0x8335e979fafacffa), LL(0xb684a369dfdf5bdf), LL(0xd79bfca97e7ee57e), LL(0x3db4481924249024),
    LL(0xc5d776fe3b3bec3b), LL(0x313d4b9aabab96ab), LL(0x3ed181f0cece1fce), LL(0x8855229911114411),
    LL(0x0c8903838f8f068f), LL(0x4a6b9c044e4e254e), LL(0xd1517366b7b7e6b7), LL(0x0b60cbe0ebeb8beb),
    LL(0xfdcc78c13c3cf03c), LL(0x7cbf1ffd81813e81), LL(0xd4fe354094946a94), LL(0xeb0cf31cf7f7fbf7),
    LL(0xa1676f18b9b9deb9), LL(0x985f268b13134c13), LL(0x7d9c58512c2cb02c), LL(0xd6b8bb05d3d36bd3),
    LL(0x6b5cd38ce7e7bbe7), LL(0x57cbdc396e6ea56e), LL(0x6ef395aac4c437c4), LL(0x180f061b03030c03),
    LL(0x8a13acdc56564556), LL(0x1a49885e44440d44), LL(0xdf9efea07f7fe17f), LL(0x21374f88a9a99ea9),
    LL(0x4d8254672a2aa82a), LL(0xb16d6b0abbbbd6bb), LL(0x46e29f87c1c123c1), LL(0xa202a6f153535153),
    LL(0xae8ba572dcdc57dc), LL(0x582716530b0b2c0b), LL(0x9cd327019d9d4e9d), LL(0x47c1d82b6c6cad6c),
    LL(0x95f562a43131c431), LL(0x87b9e8f37474cd74), LL(0xe309f115f6f6fff6), LL(0x0a438c4c46460546),
    LL(0x092645a5acac8aac), LL(0x3c970fb589891e89), LL(0xa04428b414145014), LL(0x5b42dfbae1e1a3e1),
    LL(0xb04e2ca616165816), LL(0xcdd274f73a3ae83a), LL(0x6fd0d2066969b969), LL(0x482d124109092409),
    LL(0xa7ade0d77070dd70), LL(0xd954716fb6b6e2b6), LL(0xceb7bd1ed0d067d0), LL(0x3b7ec7d6eded93ed),
    LL(0x2edb85e2cccc17cc), LL(0x2a57846842421542), LL(0xb4c22d2c98985a98), LL(0x490e55eda4a4aaa4),
    LL(0x5d8850752828a028), LL(0xda31b8865c5c6d5c), LL(0x933fed6bf8f8c7f8), LL(0x44a411c286862286),
};

__constant__ u64 C5[256] = {
    LL(0x18c07830d8181860), LL(0x2305af462623238c), LL(0xc67ef991b8c6c63f), LL(0xe8136fcdfbe8e887),
    LL(0x874ca113cb878726), LL(0xb8a9626d11b8b8da), LL(0x0108050209010104), LL(0x4f426e9e0d4f4f21),
    LL(0x36adee6c9b3636d8), LL(0xa6590451ffa6a6a2), LL(0xd2debdb90cd2d26f), LL(0xf5fb06f70ef5f5f3),
    LL(0x79ef80f2967979f9), LL(0x6f5fcede306f6fa1), LL(0x91fcef3f6d91917e), LL(0x52aa07a4f8525255),
    LL(0x6027fdc04760609d), LL(0xbc89766535bcbcca), LL(0x9baccd2b379b9b56), LL(0x8e048c018a8e8e02),
    LL(0xa371155bd2a3a3b6), LL(0x0c603c186c0c0c30), LL(0x7bff8af6847b7bf1), LL(0x35b5e16a803535d4),
    LL(0x1de8693af51d1d74), LL(0xe05347ddb3e0e0a7), LL(0xd7f6acb321d7d77b), LL(0xc25eed999cc2c22f),
    LL(0x2e6d965c432e2eb8), LL(0x4b627a96294b4b31), LL(0xfea321e15dfefedf), LL(0x578216aed5575741),
    LL(0x15a8412abd151554), LL(0x779fb6eee87777c1), LL(0x37a5eb6e923737dc), LL(0xe57b56d79ee5e5b3),
    LL(0x9f8cd923139f9f46), LL(0xf0d317fd23f0f0e7), LL(0x4a6a7f94204a4a35), LL(0xda9e95a944dada4f),
    LL(0x58fa25b0a258587d), LL(0xc906ca8fcfc9c903), LL(0x29558d527c2929a4), LL(0x0a5022145a0a0a28),
    LL(0xb1e14f7f50b1b1fe), LL(0xa0691a5dc9a0a0ba), LL(0x6b7fdad6146b6bb1), LL(0x855cab17d985852e),
    LL(0xbd8173673cbdbdce), LL(0x5dd234ba8f5d5d69), LL(0x1080502090101040), LL(0xf4f303f507f4f4f7),
    LL(0xcb16c08bddcbcb0b), LL(0x3eedc67cd33e3ef8), LL(0x0528110a2d050514), LL(0x671fe6ce78676781),
    LL(0xe47353d597e4e4b7), LL(0x2725bb4e0227279c), LL(0x4132588273414119), LL(0x8b2c9d0ba78b8b16),
    LL(0xa7510153f6a7a7a6), LL(0x7dcf94fab27d7de9), LL(0x95dcfb374995956e), LL(0xd88e9fad56d8d847),
    LL(0xfb8b30eb70fbfbcb), LL(0xee2371c1cdeeee9f), LL(0x7cc791f8bb7c7ced), LL(0x6617e3cc71666685),
    LL(0xdda68ea77bdddd53), LL(0x17b84b2eaf17175c), LL(0x4702468e45474701), LL(0x9e84dc211a9e9e42),
    LL(0xca1ec589d4caca0f), LL(0x2d75995a582d2db4), LL(0xbf9179632ebfbfc6), LL(0x07381b0e3f07071c),
    LL(0xad012347acadad8e), LL(0x5aea2fb4b05a5a75), LL(0x836cb51bef838336), LL(0x3385ff66b63333cc),
    LL(0x633ff2c65c636391), LL(0x02100a0412020208), LL(0xaa39384993aaaa92), LL(0x71afa8e2de7171d9),
    LL(0xc80ecf8dc6c8c807), LL(0x19c87d32d1191964), LL(0x497270923b494939), LL(0xd9869aaf5fd9d943),
    LL(0xf2c31df931f2f2ef), LL(0xe34b48dba8e3e3ab), LL(0x5be22ab6b95b5b71), LL(0x8834920dbc88881a),
    LL(0x9aa4c8293e9a9a52), LL(0x262dbe4c0b262698), LL(0x328dfa64bf3232c8), LL(0xb0e94a7d59b0b0fa),
    LL(0xe91b6acff2e9e983), LL(0x0f78331e770f0f3c), LL(0xd5e6a6b733d5d573), LL(0x8074ba1df480803a),
    LL(0xbe997c6127bebec2), LL(0xcd26de87ebcdcd13), LL(0x34bde468893434d0), LL(0x487a75903248483d),
    LL(0xffab24e354ffffdb), LL(0x7af78ff48d7a7af5), LL(0x90f4ea3d6490907a), LL(0x5fc23ebe9d5f5f61),
    LL(0x201da0403d202080), LL(0x6867d5d00f6868bd), LL(0x1ad07234ca1a1a68), LL(0xae192c41b7aeae82),
    LL(0xb4c95e757db4b4ea), LL(0x549a19a8ce54544d), LL(0x93ece53b7f939376), LL(0x220daa442f222288),
    LL(0x6407e9c86364648d), LL(0xf1db12ff2af1f1e3), LL(0x73bfa2e6cc7373d1), LL(0x12905a2482121248),
    LL(0x403a5d807a40401d), LL(0x0840281048080820), LL(0xc356e89b95c3c32b), LL(0xec337bc5dfecec97),
    LL(0xdb9690ab4ddbdb4b), LL(0xa1611f5fc0a1a1be), LL(0x8d1c8307918d8d0e), LL(0x3df5c97ac83d3df4),
    LL(0x97ccf1335b979766), LL(0x0000000000000000), LL(0xcf36d483f9cfcf1b), LL(0x2b4587566e2b2bac),
    LL(0x7697b3ece17676c5), LL(0x8264b019e6828232), LL(0xd6fea9b128d6d67f), LL(0x1bd87736c31b1b6c),
    LL(0xb5c15b7774b5b5ee), LL(0xaf112943beafaf86), LL(0x6a77dfd41d6a6ab5), LL(0x50ba0da0ea50505d),
    LL(0x45124c8a57454509), LL(0xf3cb18fb38f3f3eb), LL(0x309df060ad3030c0), LL(0xef2b74c3c4efef9b),
    LL(0x3fe5c37eda3f3ffc), LL(0x55921caac7555549), LL(0xa2791059dba2a2b2), LL(0xea0365c9e9eaea8f),
    LL(0x650fecca6a656589), LL(0xbab9686903babad2), LL(0x2f65935e4a2f2fbc), LL(0xc04ee79d8ec0c027),
    LL(0xdebe81a160dede5f), LL(0x1ce06c38fc1c1c70), LL(0xfdbb2ee746fdfdd3), LL(0x4d52649a1f4d4d29),
    LL(0x92e4e03976929272), LL(0x758fbceafa7575c9), LL(0x06301e0c36060618), LL(0x8a249809ae8a8a12),
    LL(0xb2f940794bb2b2f2), LL(0xe66359d185e6e6bf), LL(0x0e70361c7e0e0e38), LL(0x1ff8633ee71f1f7c),
    LL(0x6237f7c455626295), LL(0xd4eea3b53ad4d477), LL(0xa829324d81a8a89a), LL(0x96c4f43152969662),
    LL(0xf99b3aef62f9f9c3), LL(0xc566f697a3c5c533), LL(0x2535b14a10252594), LL(0x59f220b2ab595979),
    LL(0x8454ae15d084842a), LL(0x72b7a7e4c57272d5), LL(0x39d5dd72ec3939e4), LL(0x4c5a6198164c4c2d),
    LL(0x5eca3bbc945e5e65), LL(0x78e785f09f7878fd), LL(0x38ddd870e53838e0), LL(0x8c148605988c8c0a),
    LL(0xd1c6b2bf17d1d163), LL(0xa5410b57e4a5a5ae), LL(0xe2434dd9a1e2e2af), LL(0x612ff8c24e616199),
    LL(0xb3f1457b42b3b3f6), LL(0x2115a54234212184), LL(0x9c94d625089c9c4a), LL(0x1ef0663cee1e1e78),
    LL(0x4322528661434311), LL(0xc776fc93b1c7c73b), LL(0xfcb32be54ffcfcd7), LL(0x0420140824040410),
    LL(0x51b208a2e3515159), LL(0x99bcc72f2599995e), LL(0x6d4fc4da226d6da9), LL(0x0d68391a650d0d34),
    LL(0xfa8335e979fafacf), LL(0xdfb684a369dfdf5b), LL(0x7ed79bfca97e7ee5), LL(0x243db44819242490),
    LL(0x3bc5d776fe3b3bec), LL(0xab313d4b9aabab96), LL(0xce3ed181f0cece1f), LL(0x1188552299111144),
    LL(0x8f0c8903838f8f06), LL(0x4e4a6b9c044e4e25), LL(0xb7d1517366b7b7e6), LL(0xeb0b60cbe0ebeb8b),
    LL(0x3cfdcc78c13c3cf0), LL(0x817cbf1ffd81813e), LL(0x94d4fe354094946a), LL(0xf7eb0cf31cf7f7fb),
    LL(0xb9a1676f18b9b9de), LL(0x13985f268b13134c), LL(0x2c7d9c58512c2cb0), LL(0xd3d6b8bb05d3d36b),
    LL(0xe76b5cd38ce7e7bb), LL(0x6e57cbdc396e6ea5), LL(0xc46ef395aac4c437), LL(0x03180f061b03030c),
    LL(0x568a13acdc565645), LL(0x441a49885e44440d), LL(0x7fdf9efea07f7fe1), LL(0xa921374f88a9a99e),
    LL(0x2a4d8254672a2aa8), LL(0xbbb16d6b0abbbbd6), LL(0xc146e29f87c1c123), LL(0x53a202a6f1535351),
    LL(0xdcae8ba572dcdc57), LL(0x0b582716530b0b2c), LL(0x9d9cd327019d9d4e), LL(0x6c47c1d82b6c6cad),
    LL(0x3195f562a43131c4), LL(0x7487b9e8f37474cd), LL(0xf6e309f115f6f6ff), LL(0x460a438c4c464605),
    LL(0xac092645a5acac8a), LL(0x893c970fb589891e), LL(0x14a04428b4141450), LL(0xe15b42dfbae1e1a3),
    LL(0x16b04e2ca6161658), LL(0x3acdd274f73a3ae8), LL(0x696fd0d2066969b9), LL(0x09482d1241090924),
    LL(0x70a7ade0d77070dd), LL(0xb6d954716fb6b6e2), LL(0xd0ceb7bd1ed0d067), LL(0xed3b7ec7d6eded93),
    LL(0xcc2edb85e2cccc17), LL(0x422a578468424215), LL(0x98b4c22d2c98985a), LL(0xa4490e55eda4a4aa),
    LL(0x285d8850752828a0), LL(0x5cda31b8865c5c6d), LL(0xf8933fed6bf8f8c7), LL(0x8644a411c2868622),
};

__constant__ u64 C6[256] = {
    LL(0x6018c07830d81818), LL(0x8c2305af46262323), LL(0x3fc67ef991b8c6c6), LL(0x87e8136fcdfbe8e8),
    LL(0x26874ca113cb8787), LL(0xdab8a9626d11b8b8), LL(0x0401080502090101), LL(0x214f426e9e0d4f4f),
    LL(0xd836adee6c9b3636), LL(0xa2a6590451ffa6a6), LL(0x6fd2debdb90cd2d2), LL(0xf3f5fb06f70ef5f5),
    LL(0xf979ef80f2967979), LL(0xa16f5fcede306f6f), LL(0x7e91fcef3f6d9191), LL(0x5552aa07a4f85252),
    LL(0x9d6027fdc0476060), LL(0xcabc89766535bcbc), LL(0x569baccd2b379b9b), LL(0x028e048c018a8e8e),
    LL(0xb6a371155bd2a3a3), LL(0x300c603c186c0c0c), LL(0xf17bff8af6847b7b), LL(0xd435b5e16a803535),
    LL(0x741de8693af51d1d), LL(0xa7e05347ddb3e0e0), LL(0x7bd7f6acb321d7d7), LL(0x2fc25eed999cc2c2),
    LL(0xb82e6d965c432e2e), LL(0x314b627a96294b4b), LL(0xdffea321e15dfefe), LL(0x41578216aed55757),
    LL(0x5415a8412abd1515), LL(0xc1779fb6eee87777), LL(0xdc37a5eb6e923737), LL(0xb3e57b56d79ee5e5),
    LL(0x469f8cd923139f9f), LL(0xe7f0d317fd23f0f0), LL(0x354a6a7f94204a4a), LL(0x4fda9e95a944dada),
    LL(0x7d58fa25b0a25858), LL(0x03c906ca8fcfc9c9), LL(0xa429558d527c2929), LL(0x280a5022145a0a0a),
    LL(0xfeb1e14f7f50b1b1), LL(0xbaa0691a5dc9a0a0), LL(0xb16b7fdad6146b6b), LL(0x2e855cab17d98585),
    LL(0xcebd8173673cbdbd), LL(0x695dd234ba8f5d5d), LL(0x4010805020901010), LL(0xf7f4f303f507f4f4),
    LL(0x0bcb16c08bddcbcb), LL(0xf83eedc67cd33e3e), LL(0x140528110a2d0505), LL(0x81671fe6ce786767),
    LL(0xb7e47353d597e4e4), LL(0x9c2725bb4e022727), LL(0x1941325882734141), LL(0x168b2c9d0ba78b8b),
    LL(0xa6a7510153f6a7a7), LL(0xe97dcf94fab27d7d), LL(0x6e95dcfb37499595), LL(0x47d88e9fad56d8d8),
    LL(0xcbfb8b30eb70fbfb), LL(0x9fee2371c1cdeeee), LL(0xed7cc791f8bb7c7c), LL(0x856617e3cc716666),
    LL(0x53dda68ea77bdddd), LL(0x5c17b84b2eaf1717), LL(0x014702468e454747), LL(0x429e84dc211a9e9e),
    LL(0x0fca1ec589d4caca), LL(0xb42d75995a582d2d), LL(0xc6bf9179632ebfbf), LL(0x1c07381b0e3f0707),
    LL(0x8ead012347acadad), LL(0x755aea2fb4b05a5a), LL(0x36836cb51bef8383), LL(0xcc3385ff66b63333),
    LL(0x91633ff2c65c6363), LL(0x0802100a04120202), LL(0x92aa39384993aaaa), LL(0xd971afa8e2de7171),
    LL(0x07c80ecf8dc6c8c8), LL(0x6419c87d32d11919), LL(0x39497270923b4949), LL(0x43d9869aaf5fd9d9),
    LL(0xeff2c31df931f2f2), LL(0xabe34b48dba8e3e3), LL(0x715be22ab6b95b5b), LL(0x1a8834920dbc8888),
    LL(0x529aa4c8293e9a9a), LL(0x98262dbe4c0b2626), LL(0xc8328dfa64bf3232), LL(0xfab0e94a7d59b0b0),
    LL(0x83e91b6acff2e9e9), LL(0x3c0f78331e770f0f), LL(0x73d5e6a6b733d5d5), LL(0x3a8074ba1df48080),
    LL(0xc2be997c6127bebe), LL(0x13cd26de87ebcdcd), LL(0xd034bde468893434), LL(0x3d487a7590324848),
    LL(0xdbffab24e354ffff), LL(0xf57af78ff48d7a7a), LL(0x7a90f4ea3d649090), LL(0x615fc23ebe9d5f5f),
    LL(0x80201da0403d2020), LL(0xbd6867d5d00f6868), LL(0x681ad07234ca1a1a), LL(0x82ae192c41b7aeae),
    LL(0xeab4c95e757db4b4), LL(0x4d549a19a8ce5454), LL(0x7693ece53b7f9393), LL(0x88220daa442f2222),
    LL(0x8d6407e9c8636464), LL(0xe3f1db12ff2af1f1), LL(0xd173bfa2e6cc7373), LL(0x4812905a24821212),
    LL(0x1d403a5d807a4040), LL(0x2008402810480808), LL(0x2bc356e89b95c3c3), LL(0x97ec337bc5dfecec),
    LL(0x4bdb9690ab4ddbdb), LL(0xbea1611f5fc0a1a1), LL(0x0e8d1c8307918d8d), LL(0xf43df5c97ac83d3d),
    LL(0x6697ccf1335b9797), LL(0x0000000000000000), LL(0x1bcf36d483f9cfcf), LL(0xac2b4587566e2b2b),
    LL(0xc57697b3ece17676), LL(0x328264b019e68282), LL(0x7fd6fea9b128d6d6), LL(0x6c1bd87736c31b1b),
    LL(0xeeb5c15b7774b5b5), LL(0x86af112943beafaf), LL(0xb56a77dfd41d6a6a), LL(0x5d50ba0da0ea5050),
    LL(0x0945124c8a574545), LL(0xebf3cb18fb38f3f3), LL(0xc0309df060ad3030), LL(0x9bef2b74c3c4efef),
    LL(0xfc3fe5c37eda3f3f), LL(0x4955921caac75555), LL(0xb2a2791059dba2a2), LL(0x8fea0365c9e9eaea),
    LL(0x89650fecca6a6565), LL(0xd2bab9686903baba), LL(0xbc2f65935e4a2f2f), LL(0x27c04ee79d8ec0c0),
    LL(0x5fdebe81a160dede), LL(0x701ce06c38fc1c1c), LL(0xd3fdbb2ee746fdfd), LL(0x294d52649a1f4d4d),
    LL(0x7292e4e039769292), LL(0xc9758fbceafa7575), LL(0x1806301e0c360606), LL(0x128a249809ae8a8a),
    LL(0xf2b2f940794bb2b2), LL(0xbfe66359d185e6e6), LL(0x380e70361c7e0e0e), LL(0x7c1ff8633ee71f1f),
    LL(0x956237f7c4556262), LL(0x77d4eea3b53ad4d4), LL(0x9aa829324d81a8a8), LL(0x6296c4f431529696),
    LL(0xc3f99b3aef62f9f9), LL(0x33c566f697a3c5c5), LL(0x942535b14a102525), LL(0x7959f220b2ab5959),
    LL(0x2a8454ae15d08484), LL(0xd572b7a7e4c57272), LL(0xe439d5dd72ec3939), LL(0x2d4c5a6198164c4c),
    LL(0x655eca3bbc945e5e), LL(0xfd78e785f09f7878), LL(0xe038ddd870e53838), LL(0x0a8c148605988c8c),
    LL(0x63d1c6b2bf17d1d1), LL(0xaea5410b57e4a5a5), LL(0xafe2434dd9a1e2e2), LL(0x99612ff8c24e6161),
    LL(0xf6b3f1457b42b3b3), LL(0x842115a542342121), LL(0x4a9c94d625089c9c), LL(0x781ef0663cee1e1e),
    LL(0x1143225286614343), LL(0x3bc776fc93b1c7c7), LL(0xd7fcb32be54ffcfc), LL(0x1004201408240404),
    LL(0x5951b208a2e35151), LL(0x5e99bcc72f259999), LL(0xa96d4fc4da226d6d), LL(0x340d68391a650d0d),
    LL(0xcffa8335e979fafa), LL(0x5bdfb684a369dfdf), LL(0xe57ed79bfca97e7e), LL(0x90243db448192424),
    LL(0xec3bc5d776fe3b3b), LL(0x96ab313d4b9aabab), LL(0x1fce3ed181f0cece), LL(0x4411885522991111),
    LL(0x068f0c8903838f8f), LL(0x254e4a6b9c044e4e), LL(0xe6b7d1517366b7b7), LL(0x8beb0b60cbe0ebeb),
    LL(0xf03cfdcc78c13c3c), LL(0x3e817cbf1ffd8181), LL(0x6a94d4fe35409494), LL(0xfbf7eb0cf31cf7f7),
    LL(0xdeb9a1676f18b9b9), LL(0x4c13985f268b1313), LL(0xb02c7d9c58512c2c), LL(0x6bd3d6b8bb05d3d3),
    LL(0xbbe76b5cd38ce7e7), LL(0xa56e57cbdc396e6e), LL(0x37c46ef395aac4c4), LL(0x0c03180f061b0303),
    LL(0x45568a13acdc5656), LL(0x0d441a49885e4444), LL(0xe17fdf9efea07f7f), LL(0x9ea921374f88a9a9),
    LL(0xa82a4d8254672a2a), LL(0xd6bbb16d6b0abbbb), LL(0x23c146e29f87c1c1), LL(0x5153a202a6f15353),
    LL(0x57dcae8ba572dcdc), LL(0x2c0b582716530b0b), LL(0x4e9d9cd327019d9d), LL(0xad6c47c1d82b6c6c),
    LL(0xc43195f562a43131), LL(0xcd7487b9e8f37474), LL(0xfff6e309f115f6f6), LL(0x05460a438c4c4646),
    LL(0x8aac092645a5acac), LL(0x1e893c970fb58989), LL(0x5014a04428b41414), LL(0xa3e15b42dfbae1e1),
    LL(0x5816b04e2ca61616), LL(0xe83acdd274f73a3a), LL(0xb9696fd0d2066969), LL(0x2409482d12410909),
    LL(0xdd70a7ade0d77070), LL(0xe2b6d954716fb6b6), LL(0x67d0ceb7bd1ed0d0), LL(0x93ed3b7ec7d6eded),
    LL(0x17cc2edb85e2cccc), LL(0x15422a5784684242), LL(0x5a98b4c22d2c9898), LL(0xaaa4490e55eda4a4),
    LL(0xa0285d8850752828), LL(0x6d5cda31b8865c5c), LL(0xc7f8933fed6bf8f8), LL(0x228644a411c28686),
};

__constant__ u64 C7[256] = {
    LL(0x186018c07830d818), LL(0x238c2305af462623), LL(0xc63fc67ef991b8c6), LL(0xe887e8136fcdfbe8),
    LL(0x8726874ca113cb87), LL(0xb8dab8a9626d11b8), LL(0x0104010805020901), LL(0x4f214f426e9e0d4f),
    LL(0x36d836adee6c9b36), LL(0xa6a2a6590451ffa6), LL(0xd26fd2debdb90cd2), LL(0xf5f3f5fb06f70ef5),
    LL(0x79f979ef80f29679), LL(0x6fa16f5fcede306f), LL(0x917e91fcef3f6d91), LL(0x525552aa07a4f852),
    LL(0x609d6027fdc04760), LL(0xbccabc89766535bc), LL(0x9b569baccd2b379b), LL(0x8e028e048c018a8e),
    LL(0xa3b6a371155bd2a3), LL(0x0c300c603c186c0c), LL(0x7bf17bff8af6847b), LL(0x35d435b5e16a8035),
    LL(0x1d741de8693af51d), LL(0xe0a7e05347ddb3e0), LL(0xd77bd7f6acb321d7), LL(0xc22fc25eed999cc2),
    LL(0x2eb82e6d965c432e), LL(0x4b314b627a96294b), LL(0xfedffea321e15dfe), LL(0x5741578216aed557),
    LL(0x155415a8412abd15), LL(0x77c1779fb6eee877), LL(0x37dc37a5eb6e9237), LL(0xe5b3e57b56d79ee5),
    LL(0x9f469f8cd923139f), LL(0xf0e7f0d317fd23f0), LL(0x4a354a6a7f94204a), LL(0xda4fda9e95a944da),
    LL(0x587d58fa25b0a258), LL(0xc903c906ca8fcfc9), LL(0x29a429558d527c29), LL(0x0a280a5022145a0a),
    LL(0xb1feb1e14f7f50b1), LL(0xa0baa0691a5dc9a0), LL(0x6bb16b7fdad6146b), LL(0x852e855cab17d985),
    LL(0xbdcebd8173673cbd), LL(0x5d695dd234ba8f5d), LL(0x1040108050209010), LL(0xf4f7f4f303f507f4),
    LL(0xcb0bcb16c08bddcb), LL(0x3ef83eedc67cd33e), LL(0x05140528110a2d05), LL(0x6781671fe6ce7867),
    LL(0xe4b7e47353d597e4), LL(0x279c2725bb4e0227), LL(0x4119413258827341), LL(0x8b168b2c9d0ba78b),
    LL(0xa7a6a7510153f6a7), LL(0x7de97dcf94fab27d), LL(0x956e95dcfb374995), LL(0xd847d88e9fad56d8),
    LL(0xfbcbfb8b30eb70fb), LL(0xee9fee2371c1cdee), LL(0x7ced7cc791f8bb7c), LL(0x66856617e3cc7166),
    LL(0xdd53dda68ea77bdd), LL(0x175c17b84b2eaf17), LL(0x47014702468e4547), LL(0x9e429e84dc211a9e),
    LL(0xca0fca1ec589d4ca), LL(0x2db42d75995a582d), LL(0xbfc6bf9179632ebf), LL(0x071c07381b0e3f07),
    LL(0xad8ead012347acad), LL(0x5a755aea2fb4b05a), LL(0x8336836cb51bef83), LL(0x33cc3385ff66b633),
    LL(0x6391633ff2c65c63), LL(0x020802100a041202), LL(0xaa92aa39384993aa), LL(0x71d971afa8e2de71),
    LL(0xc807c80ecf8dc6c8), LL(0x196419c87d32d119), LL(0x4939497270923b49), LL(0xd943d9869aaf5fd9),
    LL(0xf2eff2c31df931f2), LL(0xe3abe34b48dba8e3), LL(0x5b715be22ab6b95b), LL(0x881a8834920dbc88),
    LL(0x9a529aa4c8293e9a), LL(0x2698262dbe4c0b26), LL(0x32c8328dfa64bf32), LL(0xb0fab0e94a7d59b0),
    LL(0xe983e91b6acff2e9), LL(0x0f3c0f78331e770f), LL(0xd573d5e6a6b733d5), LL(0x803a8074ba1df480),
    LL(0xbec2be997c6127be), LL(0xcd13cd26de87ebcd), LL(0x34d034bde4688934), LL(0x483d487a75903248),
    LL(0xffdbffab24e354ff), LL(0x7af57af78ff48d7a), LL(0x907a90f4ea3d6490), LL(0x5f615fc23ebe9d5f),
    LL(0x2080201da0403d20), LL(0x68bd6867d5d00f68), LL(0x1a681ad07234ca1a), LL(0xae82ae192c41b7ae),
    LL(0xb4eab4c95e757db4), LL(0x544d549a19a8ce54), LL(0x937693ece53b7f93), LL(0x2288220daa442f22),
    LL(0x648d6407e9c86364), LL(0xf1e3f1db12ff2af1), LL(0x73d173bfa2e6cc73), LL(0x124812905a248212),
    LL(0x401d403a5d807a40), LL(0x0820084028104808), LL(0xc32bc356e89b95c3), LL(0xec97ec337bc5dfec),
    LL(0xdb4bdb9690ab4ddb), LL(0xa1bea1611f5fc0a1), LL(0x8d0e8d1c8307918d), LL(0x3df43df5c97ac83d),
    LL(0x976697ccf1335b97), LL(0x0000000000000000), LL(0xcf1bcf36d483f9cf), LL(0x2bac2b4587566e2b),
    LL(0x76c57697b3ece176), LL(0x82328264b019e682), LL(0xd67fd6fea9b128d6), LL(0x1b6c1bd87736c31b),
    LL(0xb5eeb5c15b7774b5), LL(0xaf86af112943beaf), LL(0x6ab56a77dfd41d6a), LL(0x505d50ba0da0ea50),
    LL(0x450945124c8a5745), LL(0xf3ebf3cb18fb38f3), LL(0x30c0309df060ad30), LL(0xef9bef2b74c3c4ef),
    LL(0x3ffc3fe5c37eda3f), LL(0x554955921caac755), LL(0xa2b2a2791059dba2), LL(0xea8fea0365c9e9ea),
    LL(0x6589650fecca6a65), LL(0xbad2bab9686903ba), LL(0x2fbc2f65935e4a2f), LL(0xc027c04ee79d8ec0),
    LL(0xde5fdebe81a160de), LL(0x1c701ce06c38fc1c), LL(0xfdd3fdbb2ee746fd), LL(0x4d294d52649a1f4d),
    LL(0x927292e4e0397692), LL(0x75c9758fbceafa75), LL(0x061806301e0c3606), LL(0x8a128a249809ae8a),
    LL(0xb2f2b2f940794bb2), LL(0xe6bfe66359d185e6), LL(0x0e380e70361c7e0e), LL(0x1f7c1ff8633ee71f),
    LL(0x62956237f7c45562), LL(0xd477d4eea3b53ad4), LL(0xa89aa829324d81a8), LL(0x966296c4f4315296),
    LL(0xf9c3f99b3aef62f9), LL(0xc533c566f697a3c5), LL(0x25942535b14a1025), LL(0x597959f220b2ab59),
    LL(0x842a8454ae15d084), LL(0x72d572b7a7e4c572), LL(0x39e439d5dd72ec39), LL(0x4c2d4c5a6198164c),
    LL(0x5e655eca3bbc945e), LL(0x78fd78e785f09f78), LL(0x38e038ddd870e538), LL(0x8c0a8c148605988c),
    LL(0xd163d1c6b2bf17d1), LL(0xa5aea5410b57e4a5), LL(0xe2afe2434dd9a1e2), LL(0x6199612ff8c24e61),
    LL(0xb3f6b3f1457b42b3), LL(0x21842115a5423421), LL(0x9c4a9c94d625089c), LL(0x1e781ef0663cee1e),
    LL(0x4311432252866143), LL(0xc73bc776fc93b1c7), LL(0xfcd7fcb32be54ffc), LL(0x0410042014082404),
    LL(0x515951b208a2e351), LL(0x995e99bcc72f2599), LL(0x6da96d4fc4da226d), LL(0x0d340d68391a650d),
    LL(0xfacffa8335e979fa), LL(0xdf5bdfb684a369df), LL(0x7ee57ed79bfca97e), LL(0x2490243db4481924),
    LL(0x3bec3bc5d776fe3b), LL(0xab96ab313d4b9aab), LL(0xce1fce3ed181f0ce), LL(0x1144118855229911),
    LL(0x8f068f0c8903838f), LL(0x4e254e4a6b9c044e), LL(0xb7e6b7d1517366b7), LL(0xeb8beb0b60cbe0eb),
    LL(0x3cf03cfdcc78c13c), LL(0x813e817cbf1ffd81), LL(0x946a94d4fe354094), LL(0xf7fbf7eb0cf31cf7),
    LL(0xb9deb9a1676f18b9), LL(0x134c13985f268b13), LL(0x2cb02c7d9c58512c), LL(0xd36bd3d6b8bb05d3),
    LL(0xe7bbe76b5cd38ce7), LL(0x6ea56e57cbdc396e), LL(0xc437c46ef395aac4), LL(0x030c03180f061b03),
    LL(0x5645568a13acdc56), LL(0x440d441a49885e44), LL(0x7fe17fdf9efea07f), LL(0xa99ea921374f88a9),
    LL(0x2aa82a4d8254672a), LL(0xbbd6bbb16d6b0abb), LL(0xc123c146e29f87c1), LL(0x535153a202a6f153),
    LL(0xdc57dcae8ba572dc), LL(0x0b2c0b582716530b), LL(0x9d4e9d9cd327019d), LL(0x6cad6c47c1d82b6c),
    LL(0x31c43195f562a431), LL(0x74cd7487b9e8f374), LL(0xf6fff6e309f115f6), LL(0x4605460a438c4c46),
    LL(0xac8aac092645a5ac), LL(0x891e893c970fb589), LL(0x145014a04428b414), LL(0xe1a3e15b42dfbae1),
    LL(0x165816b04e2ca616), LL(0x3ae83acdd274f73a), LL(0x69b9696fd0d20669), LL(0x092409482d124109),
    LL(0x70dd70a7ade0d770), LL(0xb6e2b6d954716fb6), LL(0xd067d0ceb7bd1ed0), LL(0xed93ed3b7ec7d6ed),
    LL(0xcc17cc2edb85e2cc), LL(0x4215422a57846842), LL(0x985a98b4c22d2c98), LL(0xa4aaa4490e55eda4),
    LL(0x28a0285d88507528), LL(0x5c6d5cda31b8865c), LL(0xf8c7f8933fed6bf8), LL(0x86228644a411c286),
};

__constant__ u64 rc[R + 1] = {
    LL(0x0000000000000000),
    LL(0x1823c6e887b8014f),
    LL(0x36a6d2f5796f9152),
    LL(0x60bc9b8ea30c7b35),
    LL(0x1de0d7c22e4bfe57),
    LL(0x157737e59ff04ada),
    LL(0x58c9290ab1a06b85),
    LL(0xbd5d10f4cb3e0567),
    LL(0xe427418ba77d95d8),
    LL(0xfbee7c66dd17479e),
    LL(0xca2dbf07ad5a8333),
};
#pragma endregion

#pragma region "Blowfish constants"
__constant__ u32 p_init[16+2] =
{
  608135816U, 2242054355U,  320440878U,   57701188U,
 2752067618U,  698298832U,  137296536U, 3964562569U,
 1160258022U,  953160567U, 3193202383U,  887688300U,
 3232508343U, 3380367581U, 1065670069U, 3041331479U,
 2450970073U, 2306472731U
} ;

__constant__ u32 s_init[4*256] = {
 3509652390U, 2564797868U,  805139163U, 3491422135U,
 3101798381U, 1780907670U, 3128725573U, 4046225305U,
  614570311U, 3012652279U,  134345442U, 2240740374U,
 1667834072U, 1901547113U, 2757295779U, 4103290238U,
  227898511U, 1921955416U, 1904987480U, 2182433518U,
 2069144605U, 3260701109U, 2620446009U,  720527379U,
 3318853667U,  677414384U, 3393288472U, 3101374703U,
 2390351024U, 1614419982U, 1822297739U, 2954791486U,
 3608508353U, 3174124327U, 2024746970U, 1432378464U,
 3864339955U, 2857741204U, 1464375394U, 1676153920U,
 1439316330U,  715854006U, 3033291828U,  289532110U,
 2706671279U, 2087905683U, 3018724369U, 1668267050U,
  732546397U, 1947742710U, 3462151702U, 2609353502U,
 2950085171U, 1814351708U, 2050118529U,  680887927U,
  999245976U, 1800124847U, 3300911131U, 1713906067U,
 1641548236U, 4213287313U, 1216130144U, 1575780402U,
 4018429277U, 3917837745U, 3693486850U, 3949271944U,
  596196993U, 3549867205U,  258830323U, 2213823033U,
  772490370U, 2760122372U, 1774776394U, 2652871518U,
  566650946U, 4142492826U, 1728879713U, 2882767088U,
 1783734482U, 3629395816U, 2517608232U, 2874225571U,
 1861159788U,  326777828U, 3124490320U, 2130389656U,
 2716951837U,  967770486U, 1724537150U, 2185432712U,
 2364442137U, 1164943284U, 2105845187U,  998989502U,
 3765401048U, 2244026483U, 1075463327U, 1455516326U,
 1322494562U,  910128902U,  469688178U, 1117454909U,
  936433444U, 3490320968U, 3675253459U, 1240580251U,
  122909385U, 2157517691U,  634681816U, 4142456567U,
 3825094682U, 3061402683U, 2540495037U,   79693498U,
 3249098678U, 1084186820U, 1583128258U,  426386531U,
 1761308591U, 1047286709U,  322548459U,  995290223U,
 1845252383U, 2603652396U, 3431023940U, 2942221577U,
 3202600964U, 3727903485U, 1712269319U,  422464435U,
 3234572375U, 1170764815U, 3523960633U, 3117677531U,
 1434042557U,  442511882U, 3600875718U, 1076654713U,
 1738483198U, 4213154764U, 2393238008U, 3677496056U,
 1014306527U, 4251020053U,  793779912U, 2902807211U,
  842905082U, 4246964064U, 1395751752U, 1040244610U,
 2656851899U, 3396308128U,  445077038U, 3742853595U,
 3577915638U,  679411651U, 2892444358U, 2354009459U,
 1767581616U, 3150600392U, 3791627101U, 3102740896U,
  284835224U, 4246832056U, 1258075500U,  768725851U,
 2589189241U, 3069724005U, 3532540348U, 1274779536U,
 3789419226U, 2764799539U, 1660621633U, 3471099624U,
 4011903706U,  913787905U, 3497959166U,  737222580U,
 2514213453U, 2928710040U, 3937242737U, 1804850592U,
 3499020752U, 2949064160U, 2386320175U, 2390070455U,
 2415321851U, 4061277028U, 2290661394U, 2416832540U,
 1336762016U, 1754252060U, 3520065937U, 3014181293U,
  791618072U, 3188594551U, 3933548030U, 2332172193U,
 3852520463U, 3043980520U,  413987798U, 3465142937U,
 3030929376U, 4245938359U, 2093235073U, 3534596313U,
  375366246U, 2157278981U, 2479649556U,  555357303U,
 3870105701U, 2008414854U, 3344188149U, 4221384143U,
 3956125452U, 2067696032U, 3594591187U, 2921233993U,
    2428461U,  544322398U,  577241275U, 1471733935U,
  610547355U, 4027169054U, 1432588573U, 1507829418U,
 2025931657U, 3646575487U,  545086370U,   48609733U,
 2200306550U, 1653985193U,  298326376U, 1316178497U,
 3007786442U, 2064951626U,  458293330U, 2589141269U,
 3591329599U, 3164325604U,  727753846U, 2179363840U,
  146436021U, 1461446943U, 4069977195U,  705550613U,
 3059967265U, 3887724982U, 4281599278U, 3313849956U,
 1404054877U, 2845806497U,  146425753U, 1854211946U,

 1266315497U, 3048417604U, 3681880366U, 3289982499U,
 2909710000U, 1235738493U, 2632868024U, 2414719590U,
 3970600049U, 1771706367U, 1449415276U, 3266420449U,
  422970021U, 1963543593U, 2690192192U, 3826793022U,
 1062508698U, 1531092325U, 1804592342U, 2583117782U,
 2714934279U, 4024971509U, 1294809318U, 4028980673U,
 1289560198U, 2221992742U, 1669523910U,   35572830U,
  157838143U, 1052438473U, 1016535060U, 1802137761U,
 1753167236U, 1386275462U, 3080475397U, 2857371447U,
 1040679964U, 2145300060U, 2390574316U, 1461121720U,
 2956646967U, 4031777805U, 4028374788U,   33600511U,
 2920084762U, 1018524850U,  629373528U, 3691585981U,
 3515945977U, 2091462646U, 2486323059U,  586499841U,
  988145025U,  935516892U, 3367335476U, 2599673255U,
 2839830854U,  265290510U, 3972581182U, 2759138881U,
 3795373465U, 1005194799U,  847297441U,  406762289U,
 1314163512U, 1332590856U, 1866599683U, 4127851711U,
  750260880U,  613907577U, 1450815602U, 3165620655U,
 3734664991U, 3650291728U, 3012275730U, 3704569646U,
 1427272223U,  778793252U, 1343938022U, 2676280711U,
 2052605720U, 1946737175U, 3164576444U, 3914038668U,
 3967478842U, 3682934266U, 1661551462U, 3294938066U,
 4011595847U,  840292616U, 3712170807U,  616741398U,
  312560963U,  711312465U, 1351876610U,  322626781U,
 1910503582U,  271666773U, 2175563734U, 1594956187U,
   70604529U, 3617834859U, 1007753275U, 1495573769U,
 4069517037U, 2549218298U, 2663038764U,  504708206U,
 2263041392U, 3941167025U, 2249088522U, 1514023603U,
 1998579484U, 1312622330U,  694541497U, 2582060303U,
 2151582166U, 1382467621U,  776784248U, 2618340202U,
 3323268794U, 2497899128U, 2784771155U,  503983604U,
 4076293799U,  907881277U,  423175695U,  432175456U,
 1378068232U, 4145222326U, 3954048622U, 3938656102U,
 3820766613U, 2793130115U, 2977904593U,   26017576U,
 3274890735U, 3194772133U, 1700274565U, 1756076034U,
 4006520079U, 3677328699U,  720338349U, 1533947780U,
  354530856U,  688349552U, 3973924725U, 1637815568U,
  332179504U, 3949051286U,   53804574U, 2852348879U,
 3044236432U, 1282449977U, 3583942155U, 3416972820U,
 4006381244U, 1617046695U, 2628476075U, 3002303598U,
 1686838959U,  431878346U, 2686675385U, 1700445008U,
 1080580658U, 1009431731U,  832498133U, 3223435511U,
 2605976345U, 2271191193U, 2516031870U, 1648197032U,
 4164389018U, 2548247927U,  300782431U,  375919233U,
  238389289U, 3353747414U, 2531188641U, 2019080857U,
 1475708069U,  455242339U, 2609103871U,  448939670U,
 3451063019U, 1395535956U, 2413381860U, 1841049896U,
 1491858159U,  885456874U, 4264095073U, 4001119347U,
 1565136089U, 3898914787U, 1108368660U,  540939232U,
 1173283510U, 2745871338U, 3681308437U, 4207628240U,
 3343053890U, 4016749493U, 1699691293U, 1103962373U,
 3625875870U, 2256883143U, 3830138730U, 1031889488U,
 3479347698U, 1535977030U, 4236805024U, 3251091107U,
 2132092099U, 1774941330U, 1199868427U, 1452454533U,
  157007616U, 2904115357U,  342012276U,  595725824U,
 1480756522U,  206960106U,  497939518U,  591360097U,
  863170706U, 2375253569U, 3596610801U, 1814182875U,
 2094937945U, 3421402208U, 1082520231U, 3463918190U,
 2785509508U,  435703966U, 3908032597U, 1641649973U,
 2842273706U, 3305899714U, 1510255612U, 2148256476U,
 2655287854U, 3276092548U, 4258621189U,  236887753U,
 3681803219U,  274041037U, 1734335097U, 3815195456U,
 3317970021U, 1899903192U, 1026095262U, 4050517792U,
  356393447U, 2410691914U, 3873677099U, 3682840055U,

 3913112168U, 2491498743U, 4132185628U, 2489919796U,
 1091903735U, 1979897079U, 3170134830U, 3567386728U,
 3557303409U,  857797738U, 1136121015U, 1342202287U,
  507115054U, 2535736646U,  337727348U, 3213592640U,
 1301675037U, 2528481711U, 1895095763U, 1721773893U,
 3216771564U,   62756741U, 2142006736U,  835421444U,
 2531993523U, 1442658625U, 3659876326U, 2882144922U,
  676362277U, 1392781812U,  170690266U, 3921047035U,
 1759253602U, 3611846912U, 1745797284U,  664899054U,
 1329594018U, 3901205900U, 3045908486U, 2062866102U,
 2865634940U, 3543621612U, 3464012697U, 1080764994U,
  553557557U, 3656615353U, 3996768171U,  991055499U,
  499776247U, 1265440854U,  648242737U, 3940784050U,
  980351604U, 3713745714U, 1749149687U, 3396870395U,
 4211799374U, 3640570775U, 1161844396U, 3125318951U,
 1431517754U,  545492359U, 4268468663U, 3499529547U,
 1437099964U, 2702547544U, 3433638243U, 2581715763U,
 2787789398U, 1060185593U, 1593081372U, 2418618748U,
 4260947970U,   69676912U, 2159744348U,   86519011U,
 2512459080U, 3838209314U, 1220612927U, 3339683548U,
  133810670U, 1090789135U, 1078426020U, 1569222167U,
  845107691U, 3583754449U, 4072456591U, 1091646820U,
  628848692U, 1613405280U, 3757631651U,  526609435U,
  236106946U,   48312990U, 2942717905U, 3402727701U,
 1797494240U,  859738849U,  992217954U, 4005476642U,
 2243076622U, 3870952857U, 3732016268U,  765654824U,
 3490871365U, 2511836413U, 1685915746U, 3888969200U,
 1414112111U, 2273134842U, 3281911079U, 4080962846U,
  172450625U, 2569994100U,  980381355U, 4109958455U,
 2819808352U, 2716589560U, 2568741196U, 3681446669U,
 3329971472U, 1835478071U,  660984891U, 3704678404U,
 4045999559U, 3422617507U, 3040415634U, 1762651403U,
 1719377915U, 3470491036U, 2693910283U, 3642056355U,
 3138596744U, 1364962596U, 2073328063U, 1983633131U,
  926494387U, 3423689081U, 2150032023U, 4096667949U,
 1749200295U, 3328846651U,  309677260U, 2016342300U,
 1779581495U, 3079819751U,  111262694U, 1274766160U,
  443224088U,  298511866U, 1025883608U, 3806446537U,
 1145181785U,  168956806U, 3641502830U, 3584813610U,
 1689216846U, 3666258015U, 3200248200U, 1692713982U,
 2646376535U, 4042768518U, 1618508792U, 1610833997U,
 3523052358U, 4130873264U, 2001055236U, 3610705100U,
 2202168115U, 4028541809U, 2961195399U, 1006657119U,
 2006996926U, 3186142756U, 1430667929U, 3210227297U,
 1314452623U, 4074634658U, 4101304120U, 2273951170U,
 1399257539U, 3367210612U, 3027628629U, 1190975929U,
 2062231137U, 2333990788U, 2221543033U, 2438960610U,
 1181637006U,  548689776U, 2362791313U, 3372408396U,
 3104550113U, 3145860560U,  296247880U, 1970579870U,
 3078560182U, 3769228297U, 1714227617U, 3291629107U,
 3898220290U,  166772364U, 1251581989U,  493813264U,
  448347421U,  195405023U, 2709975567U,  677966185U,
 3703036547U, 1463355134U, 2715995803U, 1338867538U,
 1343315457U, 2802222074U, 2684532164U,  233230375U,
 2599980071U, 2000651841U, 3277868038U, 1638401717U,
 4028070440U, 3237316320U,    6314154U,  819756386U,
  300326615U,  590932579U, 1405279636U, 3267499572U,

 3150704214U, 2428286686U, 3959192993U, 3461946742U,
 1862657033U, 1266418056U,  963775037U, 2089974820U,
 2263052895U, 1917689273U,  448879540U, 3550394620U,
 3981727096U,  150775221U, 3627908307U, 1303187396U,
  508620638U, 2975983352U, 2726630617U, 1817252668U,
 1876281319U, 1457606340U,  908771278U, 3720792119U,
 3617206836U, 2455994898U, 1729034894U, 1080033504U,

  976866871U, 3556439503U, 2881648439U, 1522871579U,
 1555064734U, 1336096578U, 3548522304U, 2579274686U,
 3574697629U, 3205460757U, 3593280638U, 3338716283U,
 3079412587U,  564236357U, 2993598910U, 1781952180U,
 1464380207U, 3163844217U, 3332601554U, 1699332808U,
 1393555694U, 1183702653U, 3581086237U, 1288719814U,
  691649499U, 2847557200U, 2895455976U, 3193889540U,
 2717570544U, 1781354906U, 1676643554U, 2592534050U,
 3230253752U, 1126444790U, 2770207658U, 2633158820U,
 2210423226U, 2615765581U, 2414155088U, 3127139286U,
  673620729U, 2805611233U, 1269405062U, 4015350505U,
 3341807571U, 4149409754U, 1057255273U, 2012875353U,
 2162469141U, 2276492801U, 2601117357U,  993977747U,
 3918593370U, 2654263191U,  753973209U,   36408145U,
 2530585658U,   25011837U, 3520020182U, 2088578344U,
  530523599U, 2918365339U, 1524020338U, 1518925132U,
 3760827505U, 3759777254U, 1202760957U, 3985898139U,
 3906192525U,  674977740U, 4174734889U, 2031300136U,
 2019492241U, 3983892565U, 4153806404U, 3822280332U,
  352677332U, 2297720250U,   60907813U,   90501309U,
 3286998549U, 1016092578U, 2535922412U, 2839152426U,
  457141659U,  509813237U, 4120667899U,  652014361U,
 1966332200U, 2975202805U,   55981186U, 2327461051U,
  676427537U, 3255491064U, 2882294119U, 3433927263U,
 1307055953U,  942726286U,  933058658U, 2468411793U,
 3933900994U, 4215176142U, 1361170020U, 2001714738U,
 2830558078U, 3274259782U, 1222529897U, 1679025792U,
 2729314320U, 3714953764U, 1770335741U,  151462246U,
 3013232138U, 1682292957U, 1483529935U,  471910574U,
 1539241949U,  458788160U, 3436315007U, 1807016891U,
 3718408830U,  978976581U, 1043663428U, 3165965781U,
 1927990952U, 4200891579U, 2372276910U, 3208408903U,
 3533431907U, 1412390302U, 2931980059U, 4132332400U,
 1947078029U, 3881505623U, 4168226417U, 2941484381U,
 1077988104U, 1320477388U,  886195818U,   18198404U,
 3786409000U, 2509781533U,  112762804U, 3463356488U,
 1866414978U,  891333506U,   18488651U,  661792760U,
 1628790961U, 3885187036U, 3141171499U,  876946877U,
 2693282273U, 1372485963U,  791857591U, 2686433993U,
 3759982718U, 3167212022U, 3472953795U, 2716379847U,
  445679433U, 3561995674U, 3504004811U, 3574258232U,
   54117162U, 3331405415U, 2381918588U, 3769707343U,
 4154350007U, 1140177722U, 4074052095U,  668550556U,
 3214352940U,  367459370U,  261225585U, 2610173221U,
 4209349473U, 3468074219U, 3265815641U,  314222801U,
 3066103646U, 3808782860U,  282218597U, 3406013506U,
 3773591054U,  379116347U, 1285071038U,  846784868U,
 2669647154U, 3771962079U, 3550491691U, 2305946142U,
  453669953U, 1268987020U, 3317592352U, 3279303384U,
 3744833421U, 2610507566U, 3859509063U,  266596637U,
 3847019092U,  517658769U, 3462560207U, 3443424879U,
  370717030U, 4247526661U, 2224018117U, 4143653529U,
 4112773975U, 2788324899U, 2477274417U, 1456262402U,
 2901442914U, 1517677493U, 1846949527U, 2295493580U,
 3734397586U, 2176403920U, 1280348187U, 1908823572U,
 3871786941U,  846861322U, 1172426758U, 3287448474U,
 3383383037U, 1655181056U, 3139813346U,  901632758U,
 1897031941U, 2986607138U, 3066810236U, 3447102507U,
 1393639104U,  373351379U,  950779232U,  625454576U,
 3124240540U, 4148612726U, 2007998917U,  544563296U,
 2244738638U, 2330496472U, 2058025392U, 1291430526U,
  424198748U,   50039436U,   29584100U, 3605783033U,
 2429876329U, 2791104160U, 1057563949U, 3255363231U,
 3075367218U, 3463963227U, 1469046755U,  985887462U
};
#pragma endregion

#pragma region "CAST constants"
// CAST S-boxes

__constant__ word32 S[8][256] = {
{
    0x30FB40D4UL, 0x9FA0FF0BUL, 0x6BECCD2FUL, 0x3F258C7AUL,
    0x1E213F2FUL, 0x9C004DD3UL, 0x6003E540UL, 0xCF9FC949UL,
    0xBFD4AF27UL, 0x88BBBDB5UL, 0xE2034090UL, 0x98D09675UL,
    0x6E63A0E0UL, 0x15C361D2UL, 0xC2E7661DUL, 0x22D4FF8EUL,
    0x28683B6FUL, 0xC07FD059UL, 0xFF2379C8UL, 0x775F50E2UL,
    0x43C340D3UL, 0xDF2F8656UL, 0x887CA41AUL, 0xA2D2BD2DUL,
    0xA1C9E0D6UL, 0x346C4819UL, 0x61B76D87UL, 0x22540F2FUL,
    0x2ABE32E1UL, 0xAA54166BUL, 0x22568E3AUL, 0xA2D341D0UL,
    0x66DB40C8UL, 0xA784392FUL, 0x004DFF2FUL, 0x2DB9D2DEUL,
    0x97943FACUL, 0x4A97C1D8UL, 0x527644B7UL, 0xB5F437A7UL,
    0xB82CBAEFUL, 0xD751D159UL, 0x6FF7F0EDUL, 0x5A097A1FUL,
    0x827B68D0UL, 0x90ECF52EUL, 0x22B0C054UL, 0xBC8E5935UL,
    0x4B6D2F7FUL, 0x50BB64A2UL, 0xD2664910UL, 0xBEE5812DUL,
    0xB7332290UL, 0xE93B159FUL, 0xB48EE411UL, 0x4BFF345DUL,
    0xFD45C240UL, 0xAD31973FUL, 0xC4F6D02EUL, 0x55FC8165UL,
    0xD5B1CAADUL, 0xA1AC2DAEUL, 0xA2D4B76DUL, 0xC19B0C50UL,
    0x882240F2UL, 0x0C6E4F38UL, 0xA4E4BFD7UL, 0x4F5BA272UL,
    0x564C1D2FUL, 0xC59C5319UL, 0xB949E354UL, 0xB04669FEUL,
    0xB1B6AB8AUL, 0xC71358DDUL, 0x6385C545UL, 0x110F935DUL,
    0x57538AD5UL, 0x6A390493UL, 0xE63D37E0UL, 0x2A54F6B3UL,
    0x3A787D5FUL, 0x6276A0B5UL, 0x19A6FCDFUL, 0x7A42206AUL,
    0x29F9D4D5UL, 0xF61B1891UL, 0xBB72275EUL, 0xAA508167UL,
    0x38901091UL, 0xC6B505EBUL, 0x84C7CB8CUL, 0x2AD75A0FUL,
    0x874A1427UL, 0xA2D1936BUL, 0x2AD286AFUL, 0xAA56D291UL,
    0xD7894360UL, 0x425C750DUL, 0x93B39E26UL, 0x187184C9UL,
    0x6C00B32DUL, 0x73E2BB14UL, 0xA0BEBC3CUL, 0x54623779UL,
    0x64459EABUL, 0x3F328B82UL, 0x7718CF82UL, 0x59A2CEA6UL,
    0x04EE002EUL, 0x89FE78E6UL, 0x3FAB0950UL, 0x325FF6C2UL,
    0x81383F05UL, 0x6963C5C8UL, 0x76CB5AD6UL, 0xD49974C9UL,
    0xCA180DCFUL, 0x380782D5UL, 0xC7FA5CF6UL, 0x8AC31511UL,
    0x35E79E13UL, 0x47DA91D0UL, 0xF40F9086UL, 0xA7E2419EUL,
    0x31366241UL, 0x051EF495UL, 0xAA573B04UL, 0x4A805D8DUL,
    0x548300D0UL, 0x00322A3CUL, 0xBF64CDDFUL, 0xBA57A68EUL,
    0x75C6372BUL, 0x50AFD341UL, 0xA7C13275UL, 0x915A0BF5UL,
    0x6B54BFABUL, 0x2B0B1426UL, 0xAB4CC9D7UL, 0x449CCD82UL,
    0xF7FBF265UL, 0xAB85C5F3UL, 0x1B55DB94UL, 0xAAD4E324UL,
    0xCFA4BD3FUL, 0x2DEAA3E2UL, 0x9E204D02UL, 0xC8BD25ACUL,
    0xEADF55B3UL, 0xD5BD9E98UL, 0xE31231B2UL, 0x2AD5AD6CUL,
    0x954329DEUL, 0xADBE4528UL, 0xD8710F69UL, 0xAA51C90FUL,
    0xAA786BF6UL, 0x22513F1EUL, 0xAA51A79BUL, 0x2AD344CCUL,
    0x7B5A41F0UL, 0xD37CFBADUL, 0x1B069505UL, 0x41ECE491UL,
    0xB4C332E6UL, 0x032268D4UL, 0xC9600ACCUL, 0xCE387E6DUL,
    0xBF6BB16CUL, 0x6A70FB78UL, 0x0D03D9C9UL, 0xD4DF39DEUL,
    0xE01063DAUL, 0x4736F464UL, 0x5AD328D8UL, 0xB347CC96UL,
    0x75BB0FC3UL, 0x98511BFBUL, 0x4FFBCC35UL, 0xB58BCF6AUL,
    0xE11F0ABCUL, 0xBFC5FE4AUL, 0xA70AEC10UL, 0xAC39570AUL,
    0x3F04442FUL, 0x6188B153UL, 0xE0397A2EUL, 0x5727CB79UL,
    0x9CEB418FUL, 0x1CACD68DUL, 0x2AD37C96UL, 0x0175CB9DUL,
    0xC69DFF09UL, 0xC75B65F0UL, 0xD9DB40D8UL, 0xEC0E7779UL,
    0x4744EAD4UL, 0xB11C3274UL, 0xDD24CB9EUL, 0x7E1C54BDUL,
    0xF01144F9UL, 0xD2240EB1UL, 0x9675B3FDUL, 0xA3AC3755UL,
    0xD47C27AFUL, 0x51C85F4DUL, 0x56907596UL, 0xA5BB15E6UL,
    0x580304F0UL, 0xCA042CF1UL, 0x011A37EAUL, 0x8DBFAADBUL,
    0x35BA3E4AUL, 0x3526FFA0UL, 0xC37B4D09UL, 0xBC306ED9UL,
    0x98A52666UL, 0x5648F725UL, 0xFF5E569DUL, 0x0CED63D0UL,
    0x7C63B2CFUL, 0x700B45E1UL, 0xD5EA50F1UL, 0x85A92872UL,
    0xAF1FBDA7UL, 0xD4234870UL, 0xA7870BF3UL, 0x2D3B4D79UL,
    0x42E04198UL, 0x0CD0EDE7UL, 0x26470DB8UL, 0xF881814CUL,
    0x474D6AD7UL, 0x7C0C5E5CUL, 0xD1231959UL, 0x381B7298UL,
    0xF5D2F4DBUL, 0xAB838653UL, 0x6E2F1E23UL, 0x83719C9EUL,
    0xBD91E046UL, 0x9A56456EUL, 0xDC39200CUL, 0x20C8C571UL,
    0x962BDA1CUL, 0xE1E696FFUL, 0xB141AB08UL, 0x7CCA89B9UL,
    0x1A69E783UL, 0x02CC4843UL, 0xA2F7C579UL, 0x429EF47DUL,
    0x427B169CUL, 0x5AC9F049UL, 0xDD8F0F00UL, 0x5C8165BFUL
},

{
    0x1F201094UL, 0xEF0BA75BUL, 0x69E3CF7EUL, 0x393F4380UL,
    0xFE61CF7AUL, 0xEEC5207AUL, 0x55889C94UL, 0x72FC0651UL,
    0xADA7EF79UL, 0x4E1D7235UL, 0xD55A63CEUL, 0xDE0436BAUL,
    0x99C430EFUL, 0x5F0C0794UL, 0x18DCDB7DUL, 0xA1D6EFF3UL,
    0xA0B52F7BUL, 0x59E83605UL, 0xEE15B094UL, 0xE9FFD909UL,
    0xDC440086UL, 0xEF944459UL, 0xBA83CCB3UL, 0xE0C3CDFBUL,
    0xD1DA4181UL, 0x3B092AB1UL, 0xF997F1C1UL, 0xA5E6CF7BUL,
    0x01420DDBUL, 0xE4E7EF5BUL, 0x25A1FF41UL, 0xE180F806UL,
    0x1FC41080UL, 0x179BEE7AUL, 0xD37AC6A9UL, 0xFE5830A4UL,
    0x98DE8B7FUL, 0x77E83F4EUL, 0x79929269UL, 0x24FA9F7BUL,
    0xE113C85BUL, 0xACC40083UL, 0xD7503525UL, 0xF7EA615FUL,
    0x62143154UL, 0x0D554B63UL, 0x5D681121UL, 0xC866C359UL,
    0x3D63CF73UL, 0xCEE234C0UL, 0xD4D87E87UL, 0x5C672B21UL,
    0x071F6181UL, 0x39F7627FUL, 0x361E3084UL, 0xE4EB573BUL,
    0x602F64A4UL, 0xD63ACD9CUL, 0x1BBC4635UL, 0x9E81032DUL,
    0x2701F50CUL, 0x99847AB4UL, 0xA0E3DF79UL, 0xBA6CF38CUL,
    0x10843094UL, 0x2537A95EUL, 0xF46F6FFEUL, 0xA1FF3B1FUL,
    0x208CFB6AUL, 0x8F458C74UL, 0xD9E0A227UL, 0x4EC73A34UL,
    0xFC884F69UL, 0x3E4DE8DFUL, 0xEF0E0088UL, 0x3559648DUL,
    0x8A45388CUL, 0x1D804366UL, 0x721D9BFDUL, 0xA58684BBUL,
    0xE8256333UL, 0x844E8212UL, 0x128D8098UL, 0xFED33FB4UL,
    0xCE280AE1UL, 0x27E19BA5UL, 0xD5A6C252UL, 0xE49754BDUL,
    0xC5D655DDUL, 0xEB667064UL, 0x77840B4DUL, 0xA1B6A801UL,
    0x84DB26A9UL, 0xE0B56714UL, 0x21F043B7UL, 0xE5D05860UL,
    0x54F03084UL, 0x066FF472UL, 0xA31AA153UL, 0xDADC4755UL,
    0xB5625DBFUL, 0x68561BE6UL, 0x83CA6B94UL, 0x2D6ED23BUL,
    0xECCF01DBUL, 0xA6D3D0BAUL, 0xB6803D5CUL, 0xAF77A709UL,
    0x33B4A34CUL, 0x397BC8D6UL, 0x5EE22B95UL, 0x5F0E5304UL,
    0x81ED6F61UL, 0x20E74364UL, 0xB45E1378UL, 0xDE18639BUL,
    0x881CA122UL, 0xB96726D1UL, 0x8049A7E8UL, 0x22B7DA7BUL,
    0x5E552D25UL, 0x5272D237UL, 0x79D2951CUL, 0xC60D894CUL,
    0x488CB402UL, 0x1BA4FE5BUL, 0xA4B09F6BUL, 0x1CA815CFUL,
    0xA20C3005UL, 0x8871DF63UL, 0xB9DE2FCBUL, 0x0CC6C9E9UL,
    0x0BEEFF53UL, 0xE3214517UL, 0xB4542835UL, 0x9F63293CUL,
    0xEE41E729UL, 0x6E1D2D7CUL, 0x50045286UL, 0x1E6685F3UL,
    0xF33401C6UL, 0x30A22C95UL, 0x31A70850UL, 0x60930F13UL,
    0x73F98417UL, 0xA1269859UL, 0xEC645C44UL, 0x52C877A9UL,
    0xCDFF33A6UL, 0xA02B1741UL, 0x7CBAD9A2UL, 0x2180036FUL,
    0x50D99C08UL, 0xCB3F4861UL, 0xC26BD765UL, 0x64A3F6ABUL,
    0x80342676UL, 0x25A75E7BUL, 0xE4E6D1FCUL, 0x20C710E6UL,
    0xCDF0B680UL, 0x17844D3BUL, 0x31EEF84DUL, 0x7E0824E4UL,
    0x2CCB49EBUL, 0x846A3BAEUL, 0x8FF77888UL, 0xEE5D60F6UL,
    0x7AF75673UL, 0x2FDD5CDBUL, 0xA11631C1UL, 0x30F66F43UL,
    0xB3FAEC54UL, 0x157FD7FAUL, 0xEF8579CCUL, 0xD152DE58UL,
    0xDB2FFD5EUL, 0x8F32CE19UL, 0x306AF97AUL, 0x02F03EF8UL,
    0x99319AD5UL, 0xC242FA0FUL, 0xA7E3EBB0UL, 0xC68E4906UL,
    0xB8DA230CUL, 0x80823028UL, 0xDCDEF3C8UL, 0xD35FB171UL,
    0x088A1BC8UL, 0xBEC0C560UL, 0x61A3C9E8UL, 0xBCA8F54DUL,
    0xC72FEFFAUL, 0x22822E99UL, 0x82C570B4UL, 0xD8D94E89UL,
    0x8B1C34BCUL, 0x301E16E6UL, 0x273BE979UL, 0xB0FFEAA6UL,
    0x61D9B8C6UL, 0x00B24869UL, 0xB7FFCE3FUL, 0x08DC283BUL,
    0x43DAF65AUL, 0xF7E19798UL, 0x7619B72FUL, 0x8F1C9BA4UL,
    0xDC8637A0UL, 0x16A7D3B1UL, 0x9FC393B7UL, 0xA7136EEBUL,
    0xC6BCC63EUL, 0x1A513742UL, 0xEF6828BCUL, 0x520365D6UL,
    0x2D6A77ABUL, 0x3527ED4BUL, 0x821FD216UL, 0x095C6E2EUL,
    0xDB92F2FBUL, 0x5EEA29CBUL, 0x145892F5UL, 0x91584F7FUL,
    0x5483697BUL, 0x2667A8CCUL, 0x85196048UL, 0x8C4BACEAUL,
    0x833860D4UL, 0x0D23E0F9UL, 0x6C387E8AUL, 0x0AE6D249UL,
    0xB284600CUL, 0xD835731DUL, 0xDCB1C647UL, 0xAC4C56EAUL,
    0x3EBD81B3UL, 0x230EABB0UL, 0x6438BC87UL, 0xF0B5B1FAUL,
    0x8F5EA2B3UL, 0xFC184642UL, 0x0A036B7AUL, 0x4FB089BDUL,
    0x649DA589UL, 0xA345415EUL, 0x5C038323UL, 0x3E5D3BB9UL,
    0x43D79572UL, 0x7E6DD07CUL, 0x06DFDF1EUL, 0x6C6CC4EFUL,
    0x7160A539UL, 0x73BFBE70UL, 0x83877605UL, 0x4523ECF1UL
},

{
    0x8DEFC240UL, 0x25FA5D9FUL, 0xEB903DBFUL, 0xE810C907UL,
    0x47607FFFUL, 0x369FE44BUL, 0x8C1FC644UL, 0xAECECA90UL,
    0xBEB1F9BFUL, 0xEEFBCAEAUL, 0xE8CF1950UL, 0x51DF07AEUL,
    0x920E8806UL, 0xF0AD0548UL, 0xE13C8D83UL, 0x927010D5UL,
    0x11107D9FUL, 0x07647DB9UL, 0xB2E3E4D4UL, 0x3D4F285EUL,
    0xB9AFA820UL, 0xFADE82E0UL, 0xA067268BUL, 0x8272792EUL,
    0x553FB2C0UL, 0x489AE22BUL, 0xD4EF9794UL, 0x125E3FBCUL,
    0x21FFFCEEUL, 0x825B1BFDUL, 0x9255C5EDUL, 0x1257A240UL,
    0x4E1A8302UL, 0xBAE07FFFUL, 0x528246E7UL, 0x8E57140EUL,
    0x3373F7BFUL, 0x8C9F8188UL, 0xA6FC4EE8UL, 0xC982B5A5UL,
    0xA8C01DB7UL, 0x579FC264UL, 0x67094F31UL, 0xF2BD3F5FUL,
    0x40FFF7C1UL, 0x1FB78DFCUL, 0x8E6BD2C1UL, 0x437BE59BUL,
    0x99B03DBFUL, 0xB5DBC64BUL, 0x638DC0E6UL, 0x55819D99UL,
    0xA197C81CUL, 0x4A012D6EUL, 0xC5884A28UL, 0xCCC36F71UL,
    0xB843C213UL, 0x6C0743F1UL, 0x8309893CUL, 0x0FEDDD5FUL,
    0x2F7FE850UL, 0xD7C07F7EUL, 0x02507FBFUL, 0x5AFB9A04UL,
    0xA747D2D0UL, 0x1651192EUL, 0xAF70BF3EUL, 0x58C31380UL,
    0x5F98302EUL, 0x727CC3C4UL, 0x0A0FB402UL, 0x0F7FEF82UL,
    0x8C96FDADUL, 0x5D2C2AAEUL, 0x8EE99A49UL, 0x50DA88B8UL,
    0x8427F4A0UL, 0x1EAC5790UL, 0x796FB449UL, 0x8252DC15UL,
    0xEFBD7D9BUL, 0xA672597DUL, 0xADA840D8UL, 0x45F54504UL,
    0xFA5D7403UL, 0xE83EC305UL, 0x4F91751AUL, 0x925669C2UL,
    0x23EFE941UL, 0xA903F12EUL, 0x60270DF2UL, 0x0276E4B6UL,
    0x94FD6574UL, 0x927985B2UL, 0x8276DBCBUL, 0x02778176UL,
    0xF8AF918DUL, 0x4E48F79EUL, 0x8F616DDFUL, 0xE29D840EUL,
    0x842F7D83UL, 0x340CE5C8UL, 0x96BBB682UL, 0x93B4B148UL,
    0xEF303CABUL, 0x984FAF28UL, 0x779FAF9BUL, 0x92DC560DUL,
    0x224D1E20UL, 0x8437AA88UL, 0x7D29DC96UL, 0x2756D3DCUL,
    0x8B907CEEUL, 0xB51FD240UL, 0xE7C07CE3UL, 0xE566B4A1UL,
    0xC3E9615EUL, 0x3CF8209DUL, 0x6094D1E3UL, 0xCD9CA341UL,
    0x5C76460EUL, 0x00EA983BUL, 0xD4D67881UL, 0xFD47572CUL,
    0xF76CEDD9UL, 0xBDA8229CUL, 0x127DADAAUL, 0x438A074EUL,
    0x1F97C090UL, 0x081BDB8AUL, 0x93A07EBEUL, 0xB938CA15UL,
    0x97B03CFFUL, 0x3DC2C0F8UL, 0x8D1AB2ECUL, 0x64380E51UL,
    0x68CC7BFBUL, 0xD90F2788UL, 0x12490181UL, 0x5DE5FFD4UL,
    0xDD7EF86AUL, 0x76A2E214UL, 0xB9A40368UL, 0x925D958FUL,
    0x4B39FFFAUL, 0xBA39AEE9UL, 0xA4FFD30BUL, 0xFAF7933BUL,
    0x6D498623UL, 0x193CBCFAUL, 0x27627545UL, 0x825CF47AUL,
    0x61BD8BA0UL, 0xD11E42D1UL, 0xCEAD04F4UL, 0x127EA392UL,
    0x10428DB7UL, 0x8272A972UL, 0x9270C4A8UL, 0x127DE50BUL,
    0x285BA1C8UL, 0x3C62F44FUL, 0x35C0EAA5UL, 0xE805D231UL,
    0x428929FBUL, 0xB4FCDF82UL, 0x4FB66A53UL, 0x0E7DC15BUL,
    0x1F081FABUL, 0x108618AEUL, 0xFCFD086DUL, 0xF9FF2889UL,
    0x694BCC11UL, 0x236A5CAEUL, 0x12DECA4DUL, 0x2C3F8CC5UL,
    0xD2D02DFEUL, 0xF8EF5896UL, 0xE4CF52DAUL, 0x95155B67UL,
    0x494A488CUL, 0xB9B6A80CUL, 0x5C8F82BCUL, 0x89D36B45UL,
    0x3A609437UL, 0xEC00C9A9UL, 0x44715253UL, 0x0A874B49UL,
    0xD773BC40UL, 0x7C34671CUL, 0x02717EF6UL, 0x4FEB5536UL,
    0xA2D02FFFUL, 0xD2BF60C4UL, 0xD43F03C0UL, 0x50B4EF6DUL,
    0x07478CD1UL, 0x006E1888UL, 0xA2E53F55UL, 0xB9E6D4BCUL,
    0xA2048016UL, 0x97573833UL, 0xD7207D67UL, 0xDE0F8F3DUL,
    0x72F87B33UL, 0xABCC4F33UL, 0x7688C55DUL, 0x7B00A6B0UL,
    0x947B0001UL, 0x570075D2UL, 0xF9BB88F8UL, 0x8942019EUL,
    0x4264A5FFUL, 0x856302E0UL, 0x72DBD92BUL, 0xEE971B69UL,
    0x6EA22FDEUL, 0x5F08AE2BUL, 0xAF7A616DUL, 0xE5C98767UL,
    0xCF1FEBD2UL, 0x61EFC8C2UL, 0xF1AC2571UL, 0xCC8239C2UL,
    0x67214CB8UL, 0xB1E583D1UL, 0xB7DC3E62UL, 0x7F10BDCEUL,
    0xF90A5C38UL, 0x0FF0443DUL, 0x606E6DC6UL, 0x60543A49UL,
    0x5727C148UL, 0x2BE98A1DUL, 0x8AB41738UL, 0x20E1BE24UL,
    0xAF96DA0FUL, 0x68458425UL, 0x99833BE5UL, 0x600D457DUL,
    0x282F9350UL, 0x8334B362UL, 0xD91D1120UL, 0x2B6D8DA0UL,
    0x642B1E31UL, 0x9C305A00UL, 0x52BCE688UL, 0x1B03588AUL,
    0xF7BAEFD5UL, 0x4142ED9CUL, 0xA4315C11UL, 0x83323EC5UL,
    0xDFEF4636UL, 0xA133C501UL, 0xE9D3531CUL, 0xEE353783UL
},

{
    0x9DB30420UL, 0x1FB6E9DEUL, 0xA7BE7BEFUL, 0xD273A298UL,
    0x4A4F7BDBUL, 0x64AD8C57UL, 0x85510443UL, 0xFA020ED1UL,
    0x7E287AFFUL, 0xE60FB663UL, 0x095F35A1UL, 0x79EBF120UL,
    0xFD059D43UL, 0x6497B7B1UL, 0xF3641F63UL, 0x241E4ADFUL,
    0x28147F5FUL, 0x4FA2B8CDUL, 0xC9430040UL, 0x0CC32220UL,
    0xFDD30B30UL, 0xC0A5374FUL, 0x1D2D00D9UL, 0x24147B15UL,
    0xEE4D111AUL, 0x0FCA5167UL, 0x71FF904CUL, 0x2D195FFEUL,
    0x1A05645FUL, 0x0C13FEFEUL, 0x081B08CAUL, 0x05170121UL,
    0x80530100UL, 0xE83E5EFEUL, 0xAC9AF4F8UL, 0x7FE72701UL,
    0xD2B8EE5FUL, 0x06DF4261UL, 0xBB9E9B8AUL, 0x7293EA25UL,
    0xCE84FFDFUL, 0xF5718801UL, 0x3DD64B04UL, 0xA26F263BUL,
    0x7ED48400UL, 0x547EEBE6UL, 0x446D4CA0UL, 0x6CF3D6F5UL,
    0x2649ABDFUL, 0xAEA0C7F5UL, 0x36338CC1UL, 0x503F7E93UL,
    0xD3772061UL, 0x11B638E1UL, 0x72500E03UL, 0xF80EB2BBUL,
    0xABE0502EUL, 0xEC8D77DEUL, 0x57971E81UL, 0xE14F6746UL,
    0xC9335400UL, 0x6920318FUL, 0x081DBB99UL, 0xFFC304A5UL,
    0x4D351805UL, 0x7F3D5CE3UL, 0xA6C866C6UL, 0x5D5BCCA9UL,
    0xDAEC6FEAUL, 0x9F926F91UL, 0x9F46222FUL, 0x3991467DUL,
    0xA5BF6D8EUL, 0x1143C44FUL, 0x43958302UL, 0xD0214EEBUL,
    0x022083B8UL, 0x3FB6180CUL, 0x18F8931EUL, 0x281658E6UL,
    0x26486E3EUL, 0x8BD78A70UL, 0x7477E4C1UL, 0xB506E07CUL,
    0xF32D0A25UL, 0x79098B02UL, 0xE4EABB81UL, 0x28123B23UL,
    0x69DEAD38UL, 0x1574CA16UL, 0xDF871B62UL, 0x211C40B7UL,
    0xA51A9EF9UL, 0x0014377BUL, 0x041E8AC8UL, 0x09114003UL,
    0xBD59E4D2UL, 0xE3D156D5UL, 0x4FE876D5UL, 0x2F91A340UL,
    0x557BE8DEUL, 0x00EAE4A7UL, 0x0CE5C2ECUL, 0x4DB4BBA6UL,
    0xE756BDFFUL, 0xDD3369ACUL, 0xEC17B035UL, 0x06572327UL,
    0x99AFC8B0UL, 0x56C8C391UL, 0x6B65811CUL, 0x5E146119UL,
    0x6E85CB75UL, 0xBE07C002UL, 0xC2325577UL, 0x893FF4ECUL,
    0x5BBFC92DUL, 0xD0EC3B25UL, 0xB7801AB7UL, 0x8D6D3B24UL,
    0x20C763EFUL, 0xC366A5FCUL, 0x9C382880UL, 0x0ACE3205UL,
    0xAAC9548AUL, 0xECA1D7C7UL, 0x041AFA32UL, 0x1D16625AUL,
    0x6701902CUL, 0x9B757A54UL, 0x31D477F7UL, 0x9126B031UL,
    0x36CC6FDBUL, 0xC70B8B46UL, 0xD9E66A48UL, 0x56E55A79UL,
    0x026A4CEBUL, 0x52437EFFUL, 0x2F8F76B4UL, 0x0DF980A5UL,
    0x8674CDE3UL, 0xEDDA04EBUL, 0x17A9BE04UL, 0x2C18F4DFUL,
    0xB7747F9DUL, 0xAB2AF7B4UL, 0xEFC34D20UL, 0x2E096B7CUL,
    0x1741A254UL, 0xE5B6A035UL, 0x213D42F6UL, 0x2C1C7C26UL,
    0x61C2F50FUL, 0x6552DAF9UL, 0xD2C231F8UL, 0x25130F69UL,
    0xD8167FA2UL, 0x0418F2C8UL, 0x001A96A6UL, 0x0D1526ABUL,
    0x63315C21UL, 0x5E0A72ECUL, 0x49BAFEFDUL, 0x187908D9UL,
    0x8D0DBD86UL, 0x311170A7UL, 0x3E9B640CUL, 0xCC3E10D7UL,
    0xD5CAD3B6UL, 0x0CAEC388UL, 0xF73001E1UL, 0x6C728AFFUL,
    0x71EAE2A1UL, 0x1F9AF36EUL, 0xCFCBD12FUL, 0xC1DE8417UL,
    0xAC07BE6BUL, 0xCB44A1D8UL, 0x8B9B0F56UL, 0x013988C3UL,
    0xB1C52FCAUL, 0xB4BE31CDUL, 0xD8782806UL, 0x12A3A4E2UL,
    0x6F7DE532UL, 0x58FD7EB6UL, 0xD01EE900UL, 0x24ADFFC2UL,
    0xF4990FC5UL, 0x9711AAC5UL, 0x001D7B95UL, 0x82E5E7D2UL,
    0x109873F6UL, 0x00613096UL, 0xC32D9521UL, 0xADA121FFUL,
    0x29908415UL, 0x7FBB977FUL, 0xAF9EB3DBUL, 0x29C9ED2AUL,
    0x5CE2A465UL, 0xA730F32CUL, 0xD0AA3FE8UL, 0x8A5CC091UL,
    0xD49E2CE7UL, 0x0CE454A9UL, 0xD60ACD86UL, 0x015F1919UL,
    0x77079103UL, 0xDEA03AF6UL, 0x78A8565EUL, 0xDEE356DFUL,
    0x21F05CBEUL, 0x8B75E387UL, 0xB3C50651UL, 0xB8A5C3EFUL,
    0xD8EEB6D2UL, 0xE523BE77UL, 0xC2154529UL, 0x2F69EFDFUL,
    0xAFE67AFBUL, 0xF470C4B2UL, 0xF3E0EB5BUL, 0xD6CC9876UL,
    0x39E4460CUL, 0x1FDA8538UL, 0x1987832FUL, 0xCA007367UL,
    0xA99144F8UL, 0x296B299EUL, 0x492FC295UL, 0x9266BEABUL,
    0xB5676E69UL, 0x9BD3DDDAUL, 0xDF7E052FUL, 0xDB25701CUL,
    0x1B5E51EEUL, 0xF65324E6UL, 0x6AFCE36CUL, 0x0316CC04UL,
    0x8644213EUL, 0xB7DC59D0UL, 0x7965291FUL, 0xCCD6FD43UL,
    0x41823979UL, 0x932BCDF6UL, 0xB657C34DUL, 0x4EDFD282UL,
    0x7AE5290CUL, 0x3CB9536BUL, 0x851E20FEUL, 0x9833557EUL,
    0x13ECF0B0UL, 0xD3FFB372UL, 0x3F85C5C1UL, 0x0AEF7ED2UL
},

{
    0x7EC90C04UL, 0x2C6E74B9UL, 0x9B0E66DFUL, 0xA6337911UL,
    0xB86A7FFFUL, 0x1DD358F5UL, 0x44DD9D44UL, 0x1731167FUL,
    0x08FBF1FAUL, 0xE7F511CCUL, 0xD2051B00UL, 0x735ABA00UL,
    0x2AB722D8UL, 0x386381CBUL, 0xACF6243AUL, 0x69BEFD7AUL,
    0xE6A2E77FUL, 0xF0C720CDUL, 0xC4494816UL, 0xCCF5C180UL,
    0x38851640UL, 0x15B0A848UL, 0xE68B18CBUL, 0x4CAADEFFUL,
    0x5F480A01UL, 0x0412B2AAUL, 0x259814FCUL, 0x41D0EFE2UL,
    0x4E40B48DUL, 0x248EB6FBUL, 0x8DBA1CFEUL, 0x41A99B02UL,
    0x1A550A04UL, 0xBA8F65CBUL, 0x7251F4E7UL, 0x95A51725UL,
    0xC106ECD7UL, 0x97A5980AUL, 0xC539B9AAUL, 0x4D79FE6AUL,
    0xF2F3F763UL, 0x68AF8040UL, 0xED0C9E56UL, 0x11B4958BUL,
    0xE1EB5A88UL, 0x8709E6B0UL, 0xD7E07156UL, 0x4E29FEA7UL,
    0x6366E52DUL, 0x02D1C000UL, 0xC4AC8E05UL, 0x9377F571UL,
    0x0C05372AUL, 0x578535F2UL, 0x2261BE02UL, 0xD642A0C9UL,
    0xDF13A280UL, 0x74B55BD2UL, 0x682199C0UL, 0xD421E5ECUL,
    0x53FB3CE8UL, 0xC8ADEDB3UL, 0x28A87FC9UL, 0x3D959981UL,
    0x5C1FF900UL, 0xFE38D399UL, 0x0C4EFF0BUL, 0x062407EAUL,
    0xAA2F4FB1UL, 0x4FB96976UL, 0x90C79505UL, 0xB0A8A774UL,
    0xEF55A1FFUL, 0xE59CA2C2UL, 0xA6B62D27UL, 0xE66A4263UL,
    0xDF65001FUL, 0x0EC50966UL, 0xDFDD55BCUL, 0x29DE0655UL,
    0x911E739AUL, 0x17AF8975UL, 0x32C7911CUL, 0x89F89468UL,
    0x0D01E980UL, 0x524755F4UL, 0x03B63CC9UL, 0x0CC844B2UL,
    0xBCF3F0AAUL, 0x87AC36E9UL, 0xE53A7426UL, 0x01B3D82BUL,
    0x1A9E7449UL, 0x64EE2D7EUL, 0xCDDBB1DAUL, 0x01C94910UL,
    0xB868BF80UL, 0x0D26F3FDUL, 0x9342EDE7UL, 0x04A5C284UL,
    0x636737B6UL, 0x50F5B616UL, 0xF24766E3UL, 0x8ECA36C1UL,
    0x136E05DBUL, 0xFEF18391UL, 0xFB887A37UL, 0xD6E7F7D4UL,
    0xC7FB7DC9UL, 0x3063FCDFUL, 0xB6F589DEUL, 0xEC2941DAUL,
    0x26E46695UL, 0xB7566419UL, 0xF654EFC5UL, 0xD08D58B7UL,
    0x48925401UL, 0xC1BACB7FUL, 0xE5FF550FUL, 0xB6083049UL,
    0x5BB5D0E8UL, 0x87D72E5AUL, 0xAB6A6EE1UL, 0x223A66CEUL,
    0xC62BF3CDUL, 0x9E0885F9UL, 0x68CB3E47UL, 0x086C010FUL,
    0xA21DE820UL, 0xD18B69DEUL, 0xF3F65777UL, 0xFA02C3F6UL,
    0x407EDAC3UL, 0xCBB3D550UL, 0x1793084DUL, 0xB0D70EBAUL,
    0x0AB378D5UL, 0xD951FB0CUL, 0xDED7DA56UL, 0x4124BBE4UL,
    0x94CA0B56UL, 0x0F5755D1UL, 0xE0E1E56EUL, 0x6184B5BEUL,
    0x580A249FUL, 0x94F74BC0UL, 0xE327888EUL, 0x9F7B5561UL,
    0xC3DC0280UL, 0x05687715UL, 0x646C6BD7UL, 0x44904DB3UL,
    0x66B4F0A3UL, 0xC0F1648AUL, 0x697ED5AFUL, 0x49E92FF6UL,
    0x309E374FUL, 0x2CB6356AUL, 0x85808573UL, 0x4991F840UL,
    0x76F0AE02UL, 0x083BE84DUL, 0x28421C9AUL, 0x44489406UL,
    0x736E4CB8UL, 0xC1092910UL, 0x8BC95FC6UL, 0x7D869CF4UL,
    0x134F616FUL, 0x2E77118DUL, 0xB31B2BE1UL, 0xAA90B472UL,
    0x3CA5D717UL, 0x7D161BBAUL, 0x9CAD9010UL, 0xAF462BA2UL,
    0x9FE459D2UL, 0x45D34559UL, 0xD9F2DA13UL, 0xDBC65487UL,
    0xF3E4F94EUL, 0x176D486FUL, 0x097C13EAUL, 0x631DA5C7UL,
    0x445F7382UL, 0x175683F4UL, 0xCDC66A97UL, 0x70BE0288UL,
    0xB3CDCF72UL, 0x6E5DD2F3UL, 0x20936079UL, 0x459B80A5UL,
    0xBE60E2DBUL, 0xA9C23101UL, 0xEBA5315CUL, 0x224E42F2UL,
    0x1C5C1572UL, 0xF6721B2CUL, 0x1AD2FFF3UL, 0x8C25404EUL,
    0x324ED72FUL, 0x4067B7FDUL, 0x0523138EUL, 0x5CA3BC78UL,
    0xDC0FD66EUL, 0x75922283UL, 0x784D6B17UL, 0x58EBB16EUL,
    0x44094F85UL, 0x3F481D87UL, 0xFCFEAE7BUL, 0x77B5FF76UL,
    0x8C2302BFUL, 0xAAF47556UL, 0x5F46B02AUL, 0x2B092801UL,
    0x3D38F5F7UL, 0x0CA81F36UL, 0x52AF4A8AUL, 0x66D5E7C0UL,
    0xDF3B0874UL, 0x95055110UL, 0x1B5AD7A8UL, 0xF61ED5ADUL,
    0x6CF6E479UL, 0x20758184UL, 0xD0CEFA65UL, 0x88F7BE58UL,
    0x4A046826UL, 0x0FF6F8F3UL, 0xA09C7F70UL, 0x5346ABA0UL,
    0x5CE96C28UL, 0xE176EDA3UL, 0x6BAC307FUL, 0x376829D2UL,
    0x85360FA9UL, 0x17E3FE2AUL, 0x24B79767UL, 0xF5A96B20UL,
    0xD6CD2595UL, 0x68FF1EBFUL, 0x7555442CUL, 0xF19F06BEUL,
    0xF9E0659AUL, 0xEEB9491DUL, 0x34010718UL, 0xBB30CAB8UL,
    0xE822FE15UL, 0x88570983UL, 0x750E6249UL, 0xDA627E55UL,
    0x5E76FFA8UL, 0xB1534546UL, 0x6D47DE08UL, 0xEFE9E7D4UL
},

{
    0xF6FA8F9DUL, 0x2CAC6CE1UL, 0x4CA34867UL, 0xE2337F7CUL,
    0x95DB08E7UL, 0x016843B4UL, 0xECED5CBCUL, 0x325553ACUL,
    0xBF9F0960UL, 0xDFA1E2EDUL, 0x83F0579DUL, 0x63ED86B9UL,
    0x1AB6A6B8UL, 0xDE5EBE39UL, 0xF38FF732UL, 0x8989B138UL,
    0x33F14961UL, 0xC01937BDUL, 0xF506C6DAUL, 0xE4625E7EUL,
    0xA308EA99UL, 0x4E23E33CUL, 0x79CBD7CCUL, 0x48A14367UL,
    0xA3149619UL, 0xFEC94BD5UL, 0xA114174AUL, 0xEAA01866UL,
    0xA084DB2DUL, 0x09A8486FUL, 0xA888614AUL, 0x2900AF98UL,
    0x01665991UL, 0xE1992863UL, 0xC8F30C60UL, 0x2E78EF3CUL,
    0xD0D51932UL, 0xCF0FEC14UL, 0xF7CA07D2UL, 0xD0A82072UL,
    0xFD41197EUL, 0x9305A6B0UL, 0xE86BE3DAUL, 0x74BED3CDUL,
    0x372DA53CUL, 0x4C7F4448UL, 0xDAB5D440UL, 0x6DBA0EC3UL,
    0x083919A7UL, 0x9FBAEED9UL, 0x49DBCFB0UL, 0x4E670C53UL,
    0x5C3D9C01UL, 0x64BDB941UL, 0x2C0E636AUL, 0xBA7DD9CDUL,
    0xEA6F7388UL, 0xE70BC762UL, 0x35F29ADBUL, 0x5C4CDD8DUL,
    0xF0D48D8CUL, 0xB88153E2UL, 0x08A19866UL, 0x1AE2EAC8UL,
    0x284CAF89UL, 0xAA928223UL, 0x9334BE53UL, 0x3B3A21BFUL,
    0x16434BE3UL, 0x9AEA3906UL, 0xEFE8C36EUL, 0xF890CDD9UL,
    0x80226DAEUL, 0xC340A4A3UL, 0xDF7E9C09UL, 0xA694A807UL,
    0x5B7C5ECCUL, 0x221DB3A6UL, 0x9A69A02FUL, 0x68818A54UL,
    0xCEB2296FUL, 0x53C0843AUL, 0xFE893655UL, 0x25BFE68AUL,
    0xB4628ABCUL, 0xCF222EBFUL, 0x25AC6F48UL, 0xA9A99387UL,
    0x53BDDB65UL, 0xE76FFBE7UL, 0xE967FD78UL, 0x0BA93563UL,
    0x8E342BC1UL, 0xE8A11BE9UL, 0x4980740DUL, 0xC8087DFCUL,
    0x8DE4BF99UL, 0xA11101A0UL, 0x7FD37975UL, 0xDA5A26C0UL,
    0xE81F994FUL, 0x9528CD89UL, 0xFD339FEDUL, 0xB87834BFUL,
    0x5F04456DUL, 0x22258698UL, 0xC9C4C83BUL, 0x2DC156BEUL,
    0x4F628DAAUL, 0x57F55EC5UL, 0xE2220ABEUL, 0xD2916EBFUL,
    0x4EC75B95UL, 0x24F2C3C0UL, 0x42D15D99UL, 0xCD0D7FA0UL,
    0x7B6E27FFUL, 0xA8DC8AF0UL, 0x7345C106UL, 0xF41E232FUL,
    0x35162386UL, 0xE6EA8926UL, 0x3333B094UL, 0x157EC6F2UL,
    0x372B74AFUL, 0x692573E4UL, 0xE9A9D848UL, 0xF3160289UL,
    0x3A62EF1DUL, 0xA787E238UL, 0xF3A5F676UL, 0x74364853UL,
    0x20951063UL, 0x4576698DUL, 0xB6FAD407UL, 0x592AF950UL,
    0x36F73523UL, 0x4CFB6E87UL, 0x7DA4CEC0UL, 0x6C152DAAUL,
    0xCB0396A8UL, 0xC50DFE5DUL, 0xFCD707ABUL, 0x0921C42FUL,
    0x89DFF0BBUL, 0x5FE2BE78UL, 0x448F4F33UL, 0x754613C9UL,
    0x2B05D08DUL, 0x48B9D585UL, 0xDC049441UL, 0xC8098F9BUL,
    0x7DEDE786UL, 0xC39A3373UL, 0x42410005UL, 0x6A091751UL,
    0x0EF3C8A6UL, 0x890072D6UL, 0x28207682UL, 0xA9A9F7BEUL,
    0xBF32679DUL, 0xD45B5B75UL, 0xB353FD00UL, 0xCBB0E358UL,
    0x830F220AUL, 0x1F8FB214UL, 0xD372CF08UL, 0xCC3C4A13UL,
    0x8CF63166UL, 0x061C87BEUL, 0x88C98F88UL, 0x6062E397UL,
    0x47CF8E7AUL, 0xB6C85283UL, 0x3CC2ACFBUL, 0x3FC06976UL,
    0x4E8F0252UL, 0x64D8314DUL, 0xDA3870E3UL, 0x1E665459UL,
    0xC10908F0UL, 0x513021A5UL, 0x6C5B68B7UL, 0x822F8AA0UL,
    0x3007CD3EUL, 0x74719EEFUL, 0xDC872681UL, 0x073340D4UL,
    0x7E432FD9UL, 0x0C5EC241UL, 0x8809286CUL, 0xF592D891UL,
    0x08A930F6UL, 0x957EF305UL, 0xB7FBFFBDUL, 0xC266E96FUL,
    0x6FE4AC98UL, 0xB173ECC0UL, 0xBC60B42AUL, 0x953498DAUL,
    0xFBA1AE12UL, 0x2D4BD736UL, 0x0F25FAABUL, 0xA4F3FCEBUL,
    0xE2969123UL, 0x257F0C3DUL, 0x9348AF49UL, 0x361400BCUL,
    0xE8816F4AUL, 0x3814F200UL, 0xA3F94043UL, 0x9C7A54C2UL,
    0xBC704F57UL, 0xDA41E7F9UL, 0xC25AD33AUL, 0x54F4A084UL,
    0xB17F5505UL, 0x59357CBEUL, 0xEDBD15C8UL, 0x7F97C5ABUL,
    0xBA5AC7B5UL, 0xB6F6DEAFUL, 0x3A479C3AUL, 0x5302DA25UL,
    0x653D7E6AUL, 0x54268D49UL, 0x51A477EAUL, 0x5017D55BUL,
    0xD7D25D88UL, 0x44136C76UL, 0x0404A8C8UL, 0xB8E5A121UL,
    0xB81A928AUL, 0x60ED5869UL, 0x97C55B96UL, 0xEAEC991BUL,
    0x29935913UL, 0x01FDB7F1UL, 0x088E8DFAUL, 0x9AB6F6F5UL,
    0x3B4CBF9FUL, 0x4A5DE3ABUL, 0xE6051D35UL, 0xA0E1D855UL,
    0xD36B4CF1UL, 0xF544EDEBUL, 0xB0E93524UL, 0xBEBB8FBDUL,
    0xA2D762CFUL, 0x49C92F54UL, 0x38B5F331UL, 0x7128A454UL,
    0x48392905UL, 0xA65B1DB8UL, 0x851C97BDUL, 0xD675CF2FUL
},

{
    0x85E04019UL, 0x332BF567UL, 0x662DBFFFUL, 0xCFC65693UL,
    0x2A8D7F6FUL, 0xAB9BC912UL, 0xDE6008A1UL, 0x2028DA1FUL,
    0x0227BCE7UL, 0x4D642916UL, 0x18FAC300UL, 0x50F18B82UL,
    0x2CB2CB11UL, 0xB232E75CUL, 0x4B3695F2UL, 0xB28707DEUL,
    0xA05FBCF6UL, 0xCD4181E9UL, 0xE150210CUL, 0xE24EF1BDUL,
    0xB168C381UL, 0xFDE4E789UL, 0x5C79B0D8UL, 0x1E8BFD43UL,
    0x4D495001UL, 0x38BE4341UL, 0x913CEE1DUL, 0x92A79C3FUL,
    0x089766BEUL, 0xBAEEADF4UL, 0x1286BECFUL, 0xB6EACB19UL,
    0x2660C200UL, 0x7565BDE4UL, 0x64241F7AUL, 0x8248DCA9UL,
    0xC3B3AD66UL, 0x28136086UL, 0x0BD8DFA8UL, 0x356D1CF2UL,
    0x107789BEUL, 0xB3B2E9CEUL, 0x0502AA8FUL, 0x0BC0351EUL,
    0x166BF52AUL, 0xEB12FF82UL, 0xE3486911UL, 0xD34D7516UL,
    0x4E7B3AFFUL, 0x5F43671BUL, 0x9CF6E037UL, 0x4981AC83UL,
    0x334266CEUL, 0x8C9341B7UL, 0xD0D854C0UL, 0xCB3A6C88UL,
    0x47BC2829UL, 0x4725BA37UL, 0xA66AD22BUL, 0x7AD61F1EUL,
    0x0C5CBAFAUL, 0x4437F107UL, 0xB6E79962UL, 0x42D2D816UL,
    0x0A961288UL, 0xE1A5C06EUL, 0x13749E67UL, 0x72FC081AUL,
    0xB1D139F7UL, 0xF9583745UL, 0xCF19DF58UL, 0xBEC3F756UL,
    0xC06EBA30UL, 0x07211B24UL, 0x45C28829UL, 0xC95E317FUL,
    0xBC8EC511UL, 0x38BC46E9UL, 0xC6E6FA14UL, 0xBAE8584AUL,
    0xAD4EBC46UL, 0x468F508BUL, 0x7829435FUL, 0xF124183BUL,
    0x821DBA9FUL, 0xAFF60FF4UL, 0xEA2C4E6DUL, 0x16E39264UL,
    0x92544A8BUL, 0x009B4FC3UL, 0xABA68CEDUL, 0x9AC96F78UL,
    0x06A5B79AUL, 0xB2856E6EUL, 0x1AEC3CA9UL, 0xBE838688UL,
    0x0E0804E9UL, 0x55F1BE56UL, 0xE7E5363BUL, 0xB3A1F25DUL,
    0xF7DEBB85UL, 0x61FE033CUL, 0x16746233UL, 0x3C034C28UL,
    0xDA6D0C74UL, 0x79AAC56CUL, 0x3CE4E1ADUL, 0x51F0C802UL,
    0x98F8F35AUL, 0x1626A49FUL, 0xEED82B29UL, 0x1D382FE3UL,
    0x0C4FB99AUL, 0xBB325778UL, 0x3EC6D97BUL, 0x6E77A6A9UL,
    0xCB658B5CUL, 0xD45230C7UL, 0x2BD1408BUL, 0x60C03EB7UL,
    0xB9068D78UL, 0xA33754F4UL, 0xF430C87DUL, 0xC8A71302UL,
    0xB96D8C32UL, 0xEBD4E7BEUL, 0xBE8B9D2DUL, 0x7979FB06UL,
    0xE7225308UL, 0x8B75CF77UL, 0x11EF8DA4UL, 0xE083C858UL,
    0x8D6B786FUL, 0x5A6317A6UL, 0xFA5CF7A0UL, 0x5DDA0033UL,
    0xF28EBFB0UL, 0xF5B9C310UL, 0xA0EAC280UL, 0x08B9767AUL,
    0xA3D9D2B0UL, 0x79D34217UL, 0x021A718DUL, 0x9AC6336AUL,
    0x2711FD60UL, 0x438050E3UL, 0x069908A8UL, 0x3D7FEDC4UL,
    0x826D2BEFUL, 0x4EEB8476UL, 0x488DCF25UL, 0x36C9D566UL,
    0x28E74E41UL, 0xC2610ACAUL, 0x3D49A9CFUL, 0xBAE3B9DFUL,
    0xB65F8DE6UL, 0x92AEAF64UL, 0x3AC7D5E6UL, 0x9EA80509UL,
    0xF22B017DUL, 0xA4173F70UL, 0xDD1E16C3UL, 0x15E0D7F9UL,
    0x50B1B887UL, 0x2B9F4FD5UL, 0x625ABA82UL, 0x6A017962UL,
    0x2EC01B9CUL, 0x15488AA9UL, 0xD716E740UL, 0x40055A2CUL,
    0x93D29A22UL, 0xE32DBF9AUL, 0x058745B9UL, 0x3453DC1EUL,
    0xD699296EUL, 0x496CFF6FUL, 0x1C9F4986UL, 0xDFE2ED07UL,
    0xB87242D1UL, 0x19DE7EAEUL, 0x053E561AUL, 0x15AD6F8CUL,
    0x66626C1CUL, 0x7154C24CUL, 0xEA082B2AUL, 0x93EB2939UL,
    0x17DCB0F0UL, 0x58D4F2AEUL, 0x9EA294FBUL, 0x52CF564CUL,
    0x9883FE66UL, 0x2EC40581UL, 0x763953C3UL, 0x01D6692EUL,
    0xD3A0C108UL, 0xA1E7160EUL, 0xE4F2DFA6UL, 0x693ED285UL,
    0x74904698UL, 0x4C2B0EDDUL, 0x4F757656UL, 0x5D393378UL,
    0xA132234FUL, 0x3D321C5DUL, 0xC3F5E194UL, 0x4B269301UL,
    0xC79F022FUL, 0x3C997E7EUL, 0x5E4F9504UL, 0x3FFAFBBDUL,
    0x76F7AD0EUL, 0x296693F4UL, 0x3D1FCE6FUL, 0xC61E45BEUL,
    0xD3B5AB34UL, 0xF72BF9B7UL, 0x1B0434C0UL, 0x4E72B567UL,
    0x5592A33DUL, 0xB5229301UL, 0xCFD2A87FUL, 0x60AEB767UL,
    0x1814386BUL, 0x30BCC33DUL, 0x38A0C07DUL, 0xFD1606F2UL,
    0xC363519BUL, 0x589DD390UL, 0x5479F8E6UL, 0x1CB8D647UL,
    0x97FD61A9UL, 0xEA7759F4UL, 0x2D57539DUL, 0x569A58CFUL,
    0xE84E63ADUL, 0x462E1B78UL, 0x6580F87EUL, 0xF3817914UL,
    0x91DA55F4UL, 0x40A230F3UL, 0xD1988F35UL, 0xB6E318D2UL,
    0x3FFA50BCUL, 0x3D40F021UL, 0xC3C0BDAEUL, 0x4958C24CUL,
    0x518F36B2UL, 0x84B1D370UL, 0x0FEDCE83UL, 0x878DDADAUL,
    0xF2A279C7UL, 0x94E01BE8UL, 0x90716F4BUL, 0x954B8AA3UL
},

{
    0xE216300DUL, 0xBBDDFFFCUL, 0xA7EBDABDUL, 0x35648095UL,
    0x7789F8B7UL, 0xE6C1121BUL, 0x0E241600UL, 0x052CE8B5UL,
    0x11A9CFB0UL, 0xE5952F11UL, 0xECE7990AUL, 0x9386D174UL,
    0x2A42931CUL, 0x76E38111UL, 0xB12DEF3AUL, 0x37DDDDFCUL,
    0xDE9ADEB1UL, 0x0A0CC32CUL, 0xBE197029UL, 0x84A00940UL,
    0xBB243A0FUL, 0xB4D137CFUL, 0xB44E79F0UL, 0x049EEDFDUL,
    0x0B15A15DUL, 0x480D3168UL, 0x8BBBDE5AUL, 0x669DED42UL,
    0xC7ECE831UL, 0x3F8F95E7UL, 0x72DF191BUL, 0x7580330DUL,
    0x94074251UL, 0x5C7DCDFAUL, 0xABBE6D63UL, 0xAA402164UL,
    0xB301D40AUL, 0x02E7D1CAUL, 0x53571DAEUL, 0x7A3182A2UL,
    0x12A8DDECUL, 0xFDAA335DUL, 0x176F43E8UL, 0x71FB46D4UL,
    0x38129022UL, 0xCE949AD4UL, 0xB84769ADUL, 0x965BD862UL,
    0x82F3D055UL, 0x66FB9767UL, 0x15B80B4EUL, 0x1D5B47A0UL,
    0x4CFDE06FUL, 0xC28EC4B8UL, 0x57E8726EUL, 0x647A78FCUL,
    0x99865D44UL, 0x608BD593UL, 0x6C200E03UL, 0x39DC5FF6UL,
    0x5D0B00A3UL, 0xAE63AFF2UL, 0x7E8BD632UL, 0x70108C0CUL,
    0xBBD35049UL, 0x2998DF04UL, 0x980CF42AUL, 0x9B6DF491UL,
    0x9E7EDD53UL, 0x06918548UL, 0x58CB7E07UL, 0x3B74EF2EUL,
    0x522FFFB1UL, 0xD24708CCUL, 0x1C7E27CDUL, 0xA4EB215BUL,
    0x3CF1D2E2UL, 0x19B47A38UL, 0x424F7618UL, 0x35856039UL,
    0x9D17DEE7UL, 0x27EB35E6UL, 0xC9AFF67BUL, 0x36BAF5B8UL,
    0x09C467CDUL, 0xC18910B1UL, 0xE11DBF7BUL, 0x06CD1AF8UL,
    0x7170C608UL, 0x2D5E3354UL, 0xD4DE495AUL, 0x64C6D006UL,
    0xBCC0C62CUL, 0x3DD00DB3UL, 0x708F8F34UL, 0x77D51B42UL,
    0x264F620FUL, 0x24B8D2BFUL, 0x15C1B79EUL, 0x46A52564UL,
    0xF8D7E54EUL, 0x3E378160UL, 0x7895CDA5UL, 0x859C15A5UL,
    0xE6459788UL, 0xC37BC75FUL, 0xDB07BA0CUL, 0x0676A3ABUL,
    0x7F229B1EUL, 0x31842E7BUL, 0x24259FD7UL, 0xF8BEF472UL,
    0x835FFCB8UL, 0x6DF4C1F2UL, 0x96F5B195UL, 0xFD0AF0FCUL,
    0xB0FE134CUL, 0xE2506D3DUL, 0x4F9B12EAUL, 0xF215F225UL,
    0xA223736FUL, 0x9FB4C428UL, 0x25D04979UL, 0x34C713F8UL,
    0xC4618187UL, 0xEA7A6E98UL, 0x7CD16EFCUL, 0x1436876CUL,
    0xF1544107UL, 0xBEDEEE14UL, 0x56E9AF27UL, 0xA04AA441UL,
    0x3CF7C899UL, 0x92ECBAE6UL, 0xDD67016DUL, 0x151682EBUL,
    0xA842EEDFUL, 0xFDBA60B4UL, 0xF1907B75UL, 0x20E3030FUL,
    0x24D8C29EUL, 0xE139673BUL, 0xEFA63FB8UL, 0x71873054UL,
    0xB6F2CF3BUL, 0x9F326442UL, 0xCB15A4CCUL, 0xB01A4504UL,
    0xF1E47D8DUL, 0x844A1BE5UL, 0xBAE7DFDCUL, 0x42CBDA70UL,
    0xCD7DAE0AUL, 0x57E85B7AUL, 0xD53F5AF6UL, 0x20CF4D8CUL,
    0xCEA4D428UL, 0x79D130A4UL, 0x3486EBFBUL, 0x33D3CDDCUL,
    0x77853B53UL, 0x37EFFCB5UL, 0xC5068778UL, 0xE580B3E6UL,
    0x4E68B8F4UL, 0xC5C8B37EUL, 0x0D809EA2UL, 0x398FEB7CUL,
    0x132A4F94UL, 0x43B7950EUL, 0x2FEE7D1CUL, 0x223613BDUL,
    0xDD06CAA2UL, 0x37DF932BUL, 0xC4248289UL, 0xACF3EBC3UL,
    0x5715F6B7UL, 0xEF3478DDUL, 0xF267616FUL, 0xC148CBE4UL,
    0x9052815EUL, 0x5E410FABUL, 0xB48A2465UL, 0x2EDA7FA4UL,
    0xE87B40E4UL, 0xE98EA084UL, 0x5889E9E1UL, 0xEFD390FCUL,
    0xDD07D35BUL, 0xDB485694UL, 0x38D7E5B2UL, 0x57720101UL,
    0x730EDEBCUL, 0x5B643113UL, 0x94917E4FUL, 0x503C2FBAUL,
    0x646F1282UL, 0x7523D24AUL, 0xE0779695UL, 0xF9C17A8FUL,
    0x7A5B2121UL, 0xD187B896UL, 0x29263A4DUL, 0xBA510CDFUL,
    0x81F47C9FUL, 0xAD1163EDUL, 0xEA7B5965UL, 0x1A00726EUL,
    0x11403092UL, 0x00DA6D77UL, 0x4A0CDD61UL, 0xAD1F4603UL,
    0x605BDFB0UL, 0x9EEDC364UL, 0x22EBE6A8UL, 0xCEE7D28AUL,
    0xA0E736A0UL, 0x5564A6B9UL, 0x10853209UL, 0xC7EB8F37UL,
    0x2DE705CAUL, 0x8951570FUL, 0xDF09822BUL, 0xBD691A6CUL,
    0xAA12E4F2UL, 0x87451C0FUL, 0xE0F6A27AUL, 0x3ADA4819UL,
    0x4CF1764FUL, 0x0D771C2BUL, 0x67CDB156UL, 0x350D8384UL,
    0x5938FA0FUL, 0x42399EF3UL, 0x36997B07UL, 0x0E84093DUL,
    0x4AA93E61UL, 0x8360D87BUL, 0x1FA98B0CUL, 0x1149382CUL,
    0xE97625A5UL, 0x0614D1B7UL, 0x0E25244BUL, 0x0C768347UL,
    0x589E8D82UL, 0x0D2059D1UL, 0xA466BB1EUL, 0xF8DA0A82UL,
    0x04F19130UL, 0xBA6E4EC0UL, 0x99265164UL, 0x1EE7230DUL,
    0x50B2AD80UL, 0xEAEE6801UL, 0x8DB2A283UL, 0xEA8BF59EUL
}};
#pragma endregion

#pragma region "DES constants"
__constant__ word32 Spbox[8][64] = {
{
0x01010400,0x00000000,0x00010000,0x01010404, 0x01010004,0x00010404,0x00000004,0x00010000,
0x00000400,0x01010400,0x01010404,0x00000400, 0x01000404,0x01010004,0x01000000,0x00000004,
0x00000404,0x01000400,0x01000400,0x00010400, 0x00010400,0x01010000,0x01010000,0x01000404,
0x00010004,0x01000004,0x01000004,0x00010004, 0x00000000,0x00000404,0x00010404,0x01000000,
0x00010000,0x01010404,0x00000004,0x01010000, 0x01010400,0x01000000,0x01000000,0x00000400,
0x01010004,0x00010000,0x00010400,0x01000004, 0x00000400,0x00000004,0x01000404,0x00010404,
0x01010404,0x00010004,0x01010000,0x01000404, 0x01000004,0x00000404,0x00010404,0x01010400,
0x00000404,0x01000400,0x01000400,0x00000000, 0x00010004,0x00010400,0x00000000,0x01010004},
{
0x80108020,0x80008000,0x00008000,0x00108020, 0x00100000,0x00000020,0x80100020,0x80008020,
0x80000020,0x80108020,0x80108000,0x80000000, 0x80008000,0x00100000,0x00000020,0x80100020,
0x00108000,0x00100020,0x80008020,0x00000000, 0x80000000,0x00008000,0x00108020,0x80100000,
0x00100020,0x80000020,0x00000000,0x00108000, 0x00008020,0x80108000,0x80100000,0x00008020,
0x00000000,0x00108020,0x80100020,0x00100000, 0x80008020,0x80100000,0x80108000,0x00008000,
0x80100000,0x80008000,0x00000020,0x80108020, 0x00108020,0x00000020,0x00008000,0x80000000,
0x00008020,0x80108000,0x00100000,0x80000020, 0x00100020,0x80008020,0x80000020,0x00100020,
0x00108000,0x00000000,0x80008000,0x00008020, 0x80000000,0x80100020,0x80108020,0x00108000},
{
0x00000208,0x08020200,0x00000000,0x08020008, 0x08000200,0x00000000,0x00020208,0x08000200,
0x00020008,0x08000008,0x08000008,0x00020000, 0x08020208,0x00020008,0x08020000,0x00000208,
0x08000000,0x00000008,0x08020200,0x00000200, 0x00020200,0x08020000,0x08020008,0x00020208,
0x08000208,0x00020200,0x00020000,0x08000208, 0x00000008,0x08020208,0x00000200,0x08000000,
0x08020200,0x08000000,0x00020008,0x00000208, 0x00020000,0x08020200,0x08000200,0x00000000,
0x00000200,0x00020008,0x08020208,0x08000200, 0x08000008,0x00000200,0x00000000,0x08020008,
0x08000208,0x00020000,0x08000000,0x08020208, 0x00000008,0x00020208,0x00020200,0x08000008,
0x08020000,0x08000208,0x00000208,0x08020000, 0x00020208,0x00000008,0x08020008,0x00020200},
{
0x00802001,0x00002081,0x00002081,0x00000080, 0x00802080,0x00800081,0x00800001,0x00002001,
0x00000000,0x00802000,0x00802000,0x00802081, 0x00000081,0x00000000,0x00800080,0x00800001,
0x00000001,0x00002000,0x00800000,0x00802001, 0x00000080,0x00800000,0x00002001,0x00002080,
0x00800081,0x00000001,0x00002080,0x00800080, 0x00002000,0x00802080,0x00802081,0x00000081,
0x00800080,0x00800001,0x00802000,0x00802081, 0x00000081,0x00000000,0x00000000,0x00802000,
0x00002080,0x00800080,0x00800081,0x00000001, 0x00802001,0x00002081,0x00002081,0x00000080,
0x00802081,0x00000081,0x00000001,0x00002000, 0x00800001,0x00002001,0x00802080,0x00800081,
0x00002001,0x00002080,0x00800000,0x00802001, 0x00000080,0x00800000,0x00002000,0x00802080},
{
0x00000100,0x02080100,0x02080000,0x42000100, 0x00080000,0x00000100,0x40000000,0x02080000,
0x40080100,0x00080000,0x02000100,0x40080100, 0x42000100,0x42080000,0x00080100,0x40000000,
0x02000000,0x40080000,0x40080000,0x00000000, 0x40000100,0x42080100,0x42080100,0x02000100,
0x42080000,0x40000100,0x00000000,0x42000000, 0x02080100,0x02000000,0x42000000,0x00080100,
0x00080000,0x42000100,0x00000100,0x02000000, 0x40000000,0x02080000,0x42000100,0x40080100,
0x02000100,0x40000000,0x42080000,0x02080100, 0x40080100,0x00000100,0x02000000,0x42080000,
0x42080100,0x00080100,0x42000000,0x42080100, 0x02080000,0x00000000,0x40080000,0x42000000,
0x00080100,0x02000100,0x40000100,0x00080000, 0x00000000,0x40080000,0x02080100,0x40000100},
{
0x20000010,0x20400000,0x00004000,0x20404010, 0x20400000,0x00000010,0x20404010,0x00400000,
0x20004000,0x00404010,0x00400000,0x20000010, 0x00400010,0x20004000,0x20000000,0x00004010,
0x00000000,0x00400010,0x20004010,0x00004000, 0x00404000,0x20004010,0x00000010,0x20400010,
0x20400010,0x00000000,0x00404010,0x20404000, 0x00004010,0x00404000,0x20404000,0x20000000,
0x20004000,0x00000010,0x20400010,0x00404000, 0x20404010,0x00400000,0x00004010,0x20000010,
0x00400000,0x20004000,0x20000000,0x00004010, 0x20000010,0x20404010,0x00404000,0x20400000,
0x00404010,0x20404000,0x00000000,0x20400010, 0x00000010,0x00004000,0x20400000,0x00404010,
0x00004000,0x00400010,0x20004010,0x00000000, 0x20404000,0x20000000,0x00400010,0x20004010},
{
0x00200000,0x04200002,0x04000802,0x00000000, 0x00000800,0x04000802,0x00200802,0x04200800,
0x04200802,0x00200000,0x00000000,0x04000002, 0x00000002,0x04000000,0x04200002,0x00000802,
0x04000800,0x00200802,0x00200002,0x04000800, 0x04000002,0x04200000,0x04200800,0x00200002,
0x04200000,0x00000800,0x00000802,0x04200802, 0x00200800,0x00000002,0x04000000,0x00200800,
0x04000000,0x00200800,0x00200000,0x04000802, 0x04000802,0x04200002,0x04200002,0x00000002,
0x00200002,0x04000000,0x04000800,0x00200000, 0x04200800,0x00000802,0x00200802,0x04200800,
0x00000802,0x04000002,0x04200802,0x04200000, 0x00200800,0x00000000,0x00000002,0x04200802,
0x00000000,0x00200802,0x04200000,0x00000800, 0x04000002,0x04000800,0x00000800,0x00200002},
{
0x10001040,0x00001000,0x00040000,0x10041040, 0x10000000,0x10001040,0x00000040,0x10000000,
0x00040040,0x10040000,0x10041040,0x00041000, 0x10041000,0x00041040,0x00001000,0x00000040,
0x10040000,0x10000040,0x10001000,0x00001040, 0x00041000,0x00040040,0x10040040,0x10041000,
0x00001040,0x00000000,0x00000000,0x10040040, 0x10000040,0x10001000,0x00041040,0x00040000,
0x00041040,0x00040000,0x10041000,0x00001000, 0x00000040,0x10040040,0x00001000,0x00041040,
0x10001000,0x00000040,0x10000040,0x10040000, 0x10040040,0x10000000,0x00040000,0x10001040,
0x00000000,0x10041040,0x00040040,0x10000040, 0x10040000,0x10001000,0x10001040,0x00000000,
0x10041040,0x00041000,0x00041000,0x00001040, 0x00001040,0x00040040,0x10000000,0x10041000}
};
#pragma endregion

#pragma region "constants"
__constant__ const byte PADDING[64] = {
    0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

// Cipher configuration
__constant__ const Cipher Ciphers[] =
{
//						Block Size	Key Size	Key Schedule Size
//	  ID				(Bytes)		(Bytes)		(Bytes)
    { AES,		16,			32,			AES_KS				},
    { SERPENT,	16,			32,			140*4				},
    { TWOFISH,	16,			32,			TWOFISH_KS			},
    { BLOWFISH,	8,			56,			sizeof (BF_KEY)		},	// Deprecated/legacy
    { CAST,		8,			16,			sizeof (CAST_KEY)	},	// Deprecated/legacy
    { TRIPLEDES,8,			8*3,		sizeof (TDES_KEY)	},	// Deprecated/legacy
    { 0,		0,			0,			0					}
};

__constant__ const uint_64t  i512[80] =
{
    li_64(6a09e667f3bcc908), li_64(bb67ae8584caa73b),
    li_64(3c6ef372fe94f82b), li_64(a54ff53a5f1d36f1),
    li_64(510e527fade682d1), li_64(9b05688c2b3e6c1f),
    li_64(1f83d9abfb41bd6b), li_64(5be0cd19137e2179)
};

/* SHA384/SHA512 mixing data    */

__constant__ const uint_64t  k512[80] =
{
    li_64(428a2f98d728ae22), li_64(7137449123ef65cd),
    li_64(b5c0fbcfec4d3b2f), li_64(e9b5dba58189dbbc),
    li_64(3956c25bf348b538), li_64(59f111f1b605d019),
    li_64(923f82a4af194f9b), li_64(ab1c5ed5da6d8118),
    li_64(d807aa98a3030242), li_64(12835b0145706fbe),
    li_64(243185be4ee4b28c), li_64(550c7dc3d5ffb4e2),
    li_64(72be5d74f27b896f), li_64(80deb1fe3b1696b1),
    li_64(9bdc06a725c71235), li_64(c19bf174cf692694),
    li_64(e49b69c19ef14ad2), li_64(efbe4786384f25e3),
    li_64(0fc19dc68b8cd5b5), li_64(240ca1cc77ac9c65),
    li_64(2de92c6f592b0275), li_64(4a7484aa6ea6e483),
    li_64(5cb0a9dcbd41fbd4), li_64(76f988da831153b5),
    li_64(983e5152ee66dfab), li_64(a831c66d2db43210),
    li_64(b00327c898fb213f), li_64(bf597fc7beef0ee4),
    li_64(c6e00bf33da88fc2), li_64(d5a79147930aa725),
    li_64(06ca6351e003826f), li_64(142929670a0e6e70),
    li_64(27b70a8546d22ffc), li_64(2e1b21385c26c926),
    li_64(4d2c6dfc5ac42aed), li_64(53380d139d95b3df),
    li_64(650a73548baf63de), li_64(766a0abb3c77b2a8),
    li_64(81c2c92e47edaee6), li_64(92722c851482353b),
    li_64(a2bfe8a14cf10364), li_64(a81a664bbc423001),
    li_64(c24b8b70d0f89791), li_64(c76c51a30654be30),
    li_64(d192e819d6ef5218), li_64(d69906245565a910),
    li_64(f40e35855771202a), li_64(106aa07032bbd1b8),
    li_64(19a4c116b8d2d0c8), li_64(1e376c085141ab53),
    li_64(2748774cdf8eeb99), li_64(34b0bcb5e19b48a8),
    li_64(391c0cb3c5c95a63), li_64(4ed8aa4ae3418acb),
    li_64(5b9cca4f7763e373), li_64(682e6ff3d6b2b8a3),
    li_64(748f82ee5defb2fc), li_64(78a5636f43172f60),
    li_64(84c87814a1f0ab72), li_64(8cc702081a6439ec),
    li_64(90befffa23631e28), li_64(a4506cebde82bde9),
    li_64(bef9a3f7b2c67915), li_64(c67178f2e372532b),
    li_64(ca273eceea26619c), li_64(d186b8c721c0c207),
    li_64(eada7dd6cde0eb1e), li_64(f57d4f7fee6ed178),
    li_64(06f067aa72176fba), li_64(0a637dc5a2c898a6),
    li_64(113f9804bef90dae), li_64(1b710b35131c471b),
    li_64(28db77f523047d84), li_64(32caab7b40c72493),
    li_64(3c9ebe0a15c9bebc), li_64(431d67c49c100d4c),
    li_64(4cc5d4becb3e42b6), li_64(597f299cfc657e2a),
    li_64(5fcb6fab3ad6faec), li_64(6c44198c4a475817)
};

__constant__ const mode(32t) gf_poly[2] = { 0, 0xe1000000 };
__constant__ const mode(32t) gf_poly64[2] = { 0, 0xd8000000 };

__constant__ const u16 gft_le[256] = gf_dat(xp);
__constant__ const u16 gft_le64[256] = gf_dat(xp64);

#define t_dec(m,n) t_##m##n
#define sb_data(w) {\
    w(0x63), w(0x7c), w(0x77), w(0x7b), w(0xf2), w(0x6b), w(0x6f), w(0xc5),\
    w(0x30), w(0x01), w(0x67), w(0x2b), w(0xfe), w(0xd7), w(0xab), w(0x76),\
    w(0xca), w(0x82), w(0xc9), w(0x7d), w(0xfa), w(0x59), w(0x47), w(0xf0),\
    w(0xad), w(0xd4), w(0xa2), w(0xaf), w(0x9c), w(0xa4), w(0x72), w(0xc0),\
    w(0xb7), w(0xfd), w(0x93), w(0x26), w(0x36), w(0x3f), w(0xf7), w(0xcc),\
    w(0x34), w(0xa5), w(0xe5), w(0xf1), w(0x71), w(0xd8), w(0x31), w(0x15),\
    w(0x04), w(0xc7), w(0x23), w(0xc3), w(0x18), w(0x96), w(0x05), w(0x9a),\
    w(0x07), w(0x12), w(0x80), w(0xe2), w(0xeb), w(0x27), w(0xb2), w(0x75),\
    w(0x09), w(0x83), w(0x2c), w(0x1a), w(0x1b), w(0x6e), w(0x5a), w(0xa0),\
    w(0x52), w(0x3b), w(0xd6), w(0xb3), w(0x29), w(0xe3), w(0x2f), w(0x84),\
    w(0x53), w(0xd1), w(0x00), w(0xed), w(0x20), w(0xfc), w(0xb1), w(0x5b),\
    w(0x6a), w(0xcb), w(0xbe), w(0x39), w(0x4a), w(0x4c), w(0x58), w(0xcf),\
    w(0xd0), w(0xef), w(0xaa), w(0xfb), w(0x43), w(0x4d), w(0x33), w(0x85),\
    w(0x45), w(0xf9), w(0x02), w(0x7f), w(0x50), w(0x3c), w(0x9f), w(0xa8),\
    w(0x51), w(0xa3), w(0x40), w(0x8f), w(0x92), w(0x9d), w(0x38), w(0xf5),\
    w(0xbc), w(0xb6), w(0xda), w(0x21), w(0x10), w(0xff), w(0xf3), w(0xd2),\
    w(0xcd), w(0x0c), w(0x13), w(0xec), w(0x5f), w(0x97), w(0x44), w(0x17),\
    w(0xc4), w(0xa7), w(0x7e), w(0x3d), w(0x64), w(0x5d), w(0x19), w(0x73),\
    w(0x60), w(0x81), w(0x4f), w(0xdc), w(0x22), w(0x2a), w(0x90), w(0x88),\
    w(0x46), w(0xee), w(0xb8), w(0x14), w(0xde), w(0x5e), w(0x0b), w(0xdb),\
    w(0xe0), w(0x32), w(0x3a), w(0x0a), w(0x49), w(0x06), w(0x24), w(0x5c),\
    w(0xc2), w(0xd3), w(0xac), w(0x62), w(0x91), w(0x95), w(0xe4), w(0x79),\
    w(0xe7), w(0xc8), w(0x37), w(0x6d), w(0x8d), w(0xd5), w(0x4e), w(0xa9),\
    w(0x6c), w(0x56), w(0xf4), w(0xea), w(0x65), w(0x7a), w(0xae), w(0x08),\
    w(0xba), w(0x78), w(0x25), w(0x2e), w(0x1c), w(0xa6), w(0xb4), w(0xc6),\
    w(0xe8), w(0xdd), w(0x74), w(0x1f), w(0x4b), w(0xbd), w(0x8b), w(0x8a),\
    w(0x70), w(0x3e), w(0xb5), w(0x66), w(0x48), w(0x03), w(0xf6), w(0x0e),\
    w(0x61), w(0x35), w(0x57), w(0xb9), w(0x86), w(0xc1), w(0x1d), w(0x9e),\
    w(0xe1), w(0xf8), w(0x98), w(0x11), w(0x69), w(0xd9), w(0x8e), w(0x94),\
    w(0x9b), w(0x1e), w(0x87), w(0xe9), w(0xce), w(0x55), w(0x28), w(0xdf),\
    w(0x8c), w(0xa1), w(0x89), w(0x0d), w(0xbf), w(0xe6), w(0x42), w(0x68),\
    w(0x41), w(0x99), w(0x2d), w(0x0f), w(0xb0), w(0x54), w(0xbb), w(0x16) }

#define mm_data(w) {\
    w(0x00), w(0x01), w(0x02), w(0x03), w(0x04), w(0x05), w(0x06), w(0x07),\
    w(0x08), w(0x09), w(0x0a), w(0x0b), w(0x0c), w(0x0d), w(0x0e), w(0x0f),\
    w(0x10), w(0x11), w(0x12), w(0x13), w(0x14), w(0x15), w(0x16), w(0x17),\
    w(0x18), w(0x19), w(0x1a), w(0x1b), w(0x1c), w(0x1d), w(0x1e), w(0x1f),\
    w(0x20), w(0x21), w(0x22), w(0x23), w(0x24), w(0x25), w(0x26), w(0x27),\
    w(0x28), w(0x29), w(0x2a), w(0x2b), w(0x2c), w(0x2d), w(0x2e), w(0x2f),\
    w(0x30), w(0x31), w(0x32), w(0x33), w(0x34), w(0x35), w(0x36), w(0x37),\
    w(0x38), w(0x39), w(0x3a), w(0x3b), w(0x3c), w(0x3d), w(0x3e), w(0x3f),\
    w(0x40), w(0x41), w(0x42), w(0x43), w(0x44), w(0x45), w(0x46), w(0x47),\
    w(0x48), w(0x49), w(0x4a), w(0x4b), w(0x4c), w(0x4d), w(0x4e), w(0x4f),\
    w(0x50), w(0x51), w(0x52), w(0x53), w(0x54), w(0x55), w(0x56), w(0x57),\
    w(0x58), w(0x59), w(0x5a), w(0x5b), w(0x5c), w(0x5d), w(0x5e), w(0x5f),\
    w(0x60), w(0x61), w(0x62), w(0x63), w(0x64), w(0x65), w(0x66), w(0x67),\
    w(0x68), w(0x69), w(0x6a), w(0x6b), w(0x6c), w(0x6d), w(0x6e), w(0x6f),\
    w(0x70), w(0x71), w(0x72), w(0x73), w(0x74), w(0x75), w(0x76), w(0x77),\
    w(0x78), w(0x79), w(0x7a), w(0x7b), w(0x7c), w(0x7d), w(0x7e), w(0x7f),\
    w(0x80), w(0x81), w(0x82), w(0x83), w(0x84), w(0x85), w(0x86), w(0x87),\
    w(0x88), w(0x89), w(0x8a), w(0x8b), w(0x8c), w(0x8d), w(0x8e), w(0x8f),\
    w(0x90), w(0x91), w(0x92), w(0x93), w(0x94), w(0x95), w(0x96), w(0x97),\
    w(0x98), w(0x99), w(0x9a), w(0x9b), w(0x9c), w(0x9d), w(0x9e), w(0x9f),\
    w(0xa0), w(0xa1), w(0xa2), w(0xa3), w(0xa4), w(0xa5), w(0xa6), w(0xa7),\
    w(0xa8), w(0xa9), w(0xaa), w(0xab), w(0xac), w(0xad), w(0xae), w(0xaf),\
    w(0xb0), w(0xb1), w(0xb2), w(0xb3), w(0xb4), w(0xb5), w(0xb6), w(0xb7),\
    w(0xb8), w(0xb9), w(0xba), w(0xbb), w(0xbc), w(0xbd), w(0xbe), w(0xbf),\
    w(0xc0), w(0xc1), w(0xc2), w(0xc3), w(0xc4), w(0xc5), w(0xc6), w(0xc7),\
    w(0xc8), w(0xc9), w(0xca), w(0xcb), w(0xcc), w(0xcd), w(0xce), w(0xcf),\
    w(0xd0), w(0xd1), w(0xd2), w(0xd3), w(0xd4), w(0xd5), w(0xd6), w(0xd7),\
    w(0xd8), w(0xd9), w(0xda), w(0xdb), w(0xdc), w(0xdd), w(0xde), w(0xdf),\
    w(0xe0), w(0xe1), w(0xe2), w(0xe3), w(0xe4), w(0xe5), w(0xe6), w(0xe7),\
    w(0xe8), w(0xe9), w(0xea), w(0xeb), w(0xec), w(0xed), w(0xee), w(0xef),\
    w(0xf0), w(0xf1), w(0xf2), w(0xf3), w(0xf4), w(0xf5), w(0xf6), w(0xf7),\
    w(0xf8), w(0xf9), w(0xfa), w(0xfb), w(0xfc), w(0xfd), w(0xfe), w(0xff) }

#define isb_data(w) {\
    w(0x52), w(0x09), w(0x6a), w(0xd5), w(0x30), w(0x36), w(0xa5), w(0x38),\
    w(0xbf), w(0x40), w(0xa3), w(0x9e), w(0x81), w(0xf3), w(0xd7), w(0xfb),\
    w(0x7c), w(0xe3), w(0x39), w(0x82), w(0x9b), w(0x2f), w(0xff), w(0x87),\
    w(0x34), w(0x8e), w(0x43), w(0x44), w(0xc4), w(0xde), w(0xe9), w(0xcb),\
    w(0x54), w(0x7b), w(0x94), w(0x32), w(0xa6), w(0xc2), w(0x23), w(0x3d),\
    w(0xee), w(0x4c), w(0x95), w(0x0b), w(0x42), w(0xfa), w(0xc3), w(0x4e),\
    w(0x08), w(0x2e), w(0xa1), w(0x66), w(0x28), w(0xd9), w(0x24), w(0xb2),\
    w(0x76), w(0x5b), w(0xa2), w(0x49), w(0x6d), w(0x8b), w(0xd1), w(0x25),\
    w(0x72), w(0xf8), w(0xf6), w(0x64), w(0x86), w(0x68), w(0x98), w(0x16),\
    w(0xd4), w(0xa4), w(0x5c), w(0xcc), w(0x5d), w(0x65), w(0xb6), w(0x92),\
    w(0x6c), w(0x70), w(0x48), w(0x50), w(0xfd), w(0xed), w(0xb9), w(0xda),\
    w(0x5e), w(0x15), w(0x46), w(0x57), w(0xa7), w(0x8d), w(0x9d), w(0x84),\
    w(0x90), w(0xd8), w(0xab), w(0x00), w(0x8c), w(0xbc), w(0xd3), w(0x0a),\
    w(0xf7), w(0xe4), w(0x58), w(0x05), w(0xb8), w(0xb3), w(0x45), w(0x06),\
    w(0xd0), w(0x2c), w(0x1e), w(0x8f), w(0xca), w(0x3f), w(0x0f), w(0x02),\
    w(0xc1), w(0xaf), w(0xbd), w(0x03), w(0x01), w(0x13), w(0x8a), w(0x6b),\
    w(0x3a), w(0x91), w(0x11), w(0x41), w(0x4f), w(0x67), w(0xdc), w(0xea),\
    w(0x97), w(0xf2), w(0xcf), w(0xce), w(0xf0), w(0xb4), w(0xe6), w(0x73),\
    w(0x96), w(0xac), w(0x74), w(0x22), w(0xe7), w(0xad), w(0x35), w(0x85),\
    w(0xe2), w(0xf9), w(0x37), w(0xe8), w(0x1c), w(0x75), w(0xdf), w(0x6e),\
    w(0x47), w(0xf1), w(0x1a), w(0x71), w(0x1d), w(0x29), w(0xc5), w(0x89),\
    w(0x6f), w(0xb7), w(0x62), w(0x0e), w(0xaa), w(0x18), w(0xbe), w(0x1b),\
    w(0xfc), w(0x56), w(0x3e), w(0x4b), w(0xc6), w(0xd2), w(0x79), w(0x20),\
    w(0x9a), w(0xdb), w(0xc0), w(0xfe), w(0x78), w(0xcd), w(0x5a), w(0xf4),\
    w(0x1f), w(0xdd), w(0xa8), w(0x33), w(0x88), w(0x07), w(0xc7), w(0x31),\
    w(0xb1), w(0x12), w(0x10), w(0x59), w(0x27), w(0x80), w(0xec), w(0x5f),\
    w(0x60), w(0x51), w(0x7f), w(0xa9), w(0x19), w(0xb5), w(0x4a), w(0x0d),\
    w(0x2d), w(0xe5), w(0x7a), w(0x9f), w(0x93), w(0xc9), w(0x9c), w(0xef),\
    w(0xa0), w(0xe0), w(0x3b), w(0x4d), w(0xae), w(0x2a), w(0xf5), w(0xb0),\
    w(0xc8), w(0xeb), w(0xbb), w(0x3c), w(0x83), w(0x53), w(0x99), w(0x61),\
    w(0x17), w(0x2b), w(0x04), w(0x7e), w(0xba), w(0x77), w(0xd6), w(0x26),\
    w(0xe1), w(0x69), w(0x14), w(0x63), w(0x55), w(0x21), w(0x0c), w(0x7d) }

#define rc_data(w) {\
    w(0x01), w(0x02), w(0x04), w(0x08), w(0x10),w(0x20), w(0x40), w(0x80),\
    w(0x1b), w(0x36) }

#define bytes2word(b0, b1, b2, b3)  \
        (((uint_32t)(b3) << 24) | ((uint_32t)(b2) << 16) | ((uint_32t)(b1) << 8) | (b0))

#define w0(p)   bytes2word(p, 0, 0, 0)
#define w1(p)   bytes2word(0, p, 0, 0)
#define w2(p)   bytes2word(0, 0, p, 0)
#define w3(p)   bytes2word(0, 0, 0, p)

#define WPOLY   0x011b

#define f2(x)   ((x<<1) ^ (((x>>7) & 1) * WPOLY))
#define f4(x)   ((x<<2) ^ (((x>>6) & 1) * WPOLY) ^ (((x>>6) & 2) * WPOLY))
#define f8(x)   ((x<<3) ^ (((x>>5) & 1) * WPOLY) ^ (((x>>5) & 2) * WPOLY) \
                        ^ (((x>>5) & 4) * WPOLY))
#define f3(x)   (f2(x) ^ x)
#define f9(x)   (f8(x) ^ x)
#define fb(x)   (f8(x) ^ f2(x) ^ x)
#define fd(x)   (f8(x) ^ f4(x) ^ x)
#define fe(x)   (f8(x) ^ f4(x) ^ f2(x))

#define v0(p)   bytes2word(fe(p), f9(p), fd(p), fb(p))
#define v1(p)   bytes2word(fb(p), fe(p), f9(p), fd(p))
#define v2(p)   bytes2word(fd(p), fb(p), fe(p), f9(p))
#define v3(p)   bytes2word(f9(p), fd(p), fb(p), fe(p))

#define u0(p)   bytes2word(f2(p), p, p, f3(p))
#define u1(p)   bytes2word(f3(p), f2(p), p, p)
#define u2(p)   bytes2word(p, f3(p), f2(p), p)
#define u3(p)   bytes2word(p, p, f3(p), f2(p))

#define d_4(t,n,b,e,f,g,h) __constant__ ALIGN(32) const t n[4][256] = { b(e), b(f), b(g), b(h) }

d_4(uint_32t, t_dec(f,n), sb_data, u0, u1, u2, u3);
d_4(uint_32t, t_dec(f,l), sb_data, w0, w1, w2, w3);
d_4(uint_32t, t_dec(i,m), mm_data, v0, v1, v2, v3);
d_4(uint_32t, t_dec(i,n), isb_data, v0, v1, v2, v3);
d_4(uint_32t, t_dec(i,l), isb_data, w0, w1, w2, w3);
__constant__ ALIGN(32) const uint_32t t_dec(r,c)[RC_LENGTH] = rc_data(w0);

/* finite field arithmetic for GF(2**8) with the modular    */
/* polynomial x^8 + x^6 + x^5 + x^3 + 1 (0x169)             */

#define G_M 0x0169

__constant__ const u1byte  tab_5b[4] = { 0, G_M >> 2, G_M >> 1, (G_M >> 1) ^ (G_M >> 2) };
__constant__ const u1byte  tab_ef[4] = { 0, (G_M >> 1) ^ (G_M >> 2), G_M >> 1, G_M >> 2 };

__constant__ const u1byte ror4[16] = { 0, 8, 1, 9, 2, 10, 3, 11, 4, 12, 5, 13, 6, 14, 7, 15 };
__constant__ const u1byte ashx[16] = { 0, 9, 2, 11, 4, 13, 6, 15, 8, 1, 10, 3, 12, 5, 14, 7 };

__constant__ const u1byte qt0[2][16] = 
{   { 8, 1, 7, 13, 6, 15, 3, 2, 0, 11, 5, 9, 14, 12, 10, 4 },
    { 2, 8, 11, 13, 15, 7, 6, 14, 3, 1, 9, 4, 0, 10, 12, 5 }
};

__constant__ const u1byte qt1[2][16] =
{   { 14, 12, 11, 8, 1, 2, 3, 5, 15, 4, 10, 6, 7, 0, 9, 13 }, 
    { 1, 14, 2, 11, 4, 12, 3, 7, 6, 13, 10, 5, 15, 9, 0, 8 }
};

__constant__ const u1byte qt2[2][16] = 
{   { 11, 10, 5, 14, 6, 13, 9, 0, 12, 8, 15, 3, 2, 4, 7, 1 },
    { 4, 12, 7, 5, 1, 6, 9, 10, 0, 14, 13, 8, 2, 11, 3, 15 }
};

__constant__ const u1byte qt3[2][16] = 
{   { 13, 7, 15, 4, 1, 2, 6, 14, 9, 11, 3, 0, 8, 5, 12, 10 },
    { 11, 9, 5, 1, 12, 3, 13, 14, 6, 4, 7, 15, 2, 0, 8, 10 }
};

/* permuted choice table (key) */
__constant__ const byte pc1[] = {
       57, 49, 41, 33, 25, 17,  9,
        1, 58, 50, 42, 34, 26, 18,
       10,  2, 59, 51, 43, 35, 27,
       19, 11,  3, 60, 52, 44, 36,

       63, 55, 47, 39, 31, 23, 15,
        7, 62, 54, 46, 38, 30, 22,
       14,  6, 61, 53, 45, 37, 29,
       21, 13,  5, 28, 20, 12,  4
};

/* number left rotations of pc1 */
__constant__ const byte totrot[] = {
       1,2,4,6,8,10,12,14,15,17,19,21,23,25,27,28
};

/* permuted choice key (table) */
__constant__ const byte pc2[] = {
       14, 17, 11, 24,  1,  5,
        3, 28, 15,  6, 21, 10,
       23, 19, 12,  4, 26,  8,
       16,  7, 27, 20, 13,  2,
       41, 52, 31, 37, 47, 55,
       30, 40, 51, 45, 33, 48,
       44, 49, 39, 56, 34, 53,
       46, 42, 50, 36, 29, 32
};

/* End of DES-defined tables */

/* bit 0 is left-most in byte */
__constant__ const int bytebit[] = {
       0200,0100,040,020,010,04,02,01
};
#pragma endregion

#endif
