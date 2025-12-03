#pragma once

/* For deflate.cpp */

#define LENGTH_CODES 29
/* number of length codes, not counting the special END_BLOCK code */

#define LITERALS  256
/* number of literal bytes 0..255 */

#define L_CODES (LITERALS+1+LENGTH_CODES)
/* number of Literal or Length codes, including the END_BLOCK code */

#define D_CODES   30
/* number of distance codes */

#define BL_CODES  19
/* number of codes used to transfer the bit lengths */

#define HEAP_SIZE (2*L_CODES+1)
/* maximum heap size */

#define MAX_BITS 15
/* All codes must not exceed MAX_BITS bits */

#define INIT_STATE    42
#define BUSY_STATE   113
#define FINISH_STATE 666
/* Stream status */

/* Maximum value for windowBits in deflateInit2 and inflateInit2.
 * WARNING: reducing MAX_WBITS makes minigzip unable to extract .gz files
 * created by gzip. (Files created by minigzip can still be extracted by
 * gzip.)
 */
#ifndef MAX_WBITS
#define MAX_WBITS   15 /* 32K LZ77 window */
#endif

#ifndef DEF_WBITS
#define DEF_WBITS MAX_WBITS
#endif
/* default windowBits for decompression. MAX_WBITS is for compression only */

#define MAX_MEM_LEVEL 9
/* Maximum value for memLevel in deflateInit2 */

#if MAX_MEM_LEVEL >= 8
#  define DEF_MEM_LEVEL 8
#else
#  define DEF_MEM_LEVEL  MAX_MEM_LEVEL
#endif
/* default memLevel */

#define STORED_BLOCK 0
#define STATIC_TREES 1
#define DYN_TREES    2
/* The three kinds of block type */

#define MIN_MATCH  3
#define MAX_MATCH  258
/* The minimum and maximum match lengths */

#define PRESET_DICT 0x20 /* preset dictionary flag in zlib header */

#define MANY 1440

#define BMAX 15         /* maximum bit length of any code */

typedef unsigned char  uch;
typedef uch FAR uchf;
typedef unsigned short ush;
typedef ush FAR ushf;
typedef unsigned long  ulg;
typedef ush Pos;
typedef Pos FAR Posf;
typedef unsigned IPos;

typedef unsigned char Byte;  /* 8 bits */
typedef unsigned int uInt;  /* 16 bits or more */
typedef unsigned long uLong; /* 32 bits or more */
typedef Byte FAR Bytef;
typedef char FAR charf;
typedef int FAR intf;
typedef uInt FAR uIntf;
typedef uLong FAR uLongf;
typedef Byte FAR *voidpf;
typedef Byte *voidp;

typedef uLong (*check_func)(uLong check, const Bytef *buf, uInt len);

struct deflate_internal_state;
struct inflate_internal_state;

typedef struct z_stream {
    Bytef    *next_in;  /* next input byte */
    uInt     avail_in;  /* number of bytes available at next_in */
    uLong    total_in;  /* total nb of input bytes read so far */

    Bytef    *next_out; /* next output byte should be put there */
    uInt     avail_out; /* remaining free space at next_out */
    uLong    total_out; /* total nb of bytes output so far */

    char     *msg;      /* last error message, NULL if no error */
    struct deflate_internal_state FAR *stateD; /* not visible by applications */
	struct inflate_internal_state FAR *stateI;


    voidpf     opaque;  /* private data object passed to malloc and free */

    int     data_type;  /* best guess about the data type: ascii or binary */
    uLong   adler;      /* adler32 value of the uncompressed data */
    uLong   reserved;   /* reserved for future use */
} FAR *z_streamp;

typedef struct inflate_huft_s {
  union {
    struct {
      Byte Exop;        /* number of extra bits or operation */
      Byte Bits;        /* number of bits in this code or subcode */
    } what;
    uInt pad;           /* pad structure to a power of 2 (4 bytes for */
  } word;               /*  16-bit, 8 bytes for 32-bit int's) */
  uInt base;            /* literal, length base, distance base,
                           or table offset */
} FAR inflate_huft;

typedef enum {
      IB_METHOD,   /* waiting for method byte */
      IB_FLAG,     /* waiting for flag byte */
      IB_DICT4,    /* four dictionary check bytes to go */
      IB_DICT3,    /* three dictionary check bytes to go */
      IB_DICT2,    /* two dictionary check bytes to go */
      IB_DICT1,    /* one dictionary check byte to go */
      IB_DICT0,    /* waiting for inflateSetDictionary */
      IB_BLOCKS,   /* decompressing blocks */
      IB_CHECK4,   /* four check bytes to go */
      IB_CHECK3,   /* three check bytes to go */
      IB_CHECK2,   /* two check bytes to go */
      IB_CHECK1,   /* one check byte to go */
      IB_DONE,     /* finished check, done */
      IB_BAD}      /* got an error--stay here */
inflate_mode;

typedef enum {
      IBM_TYPE,     /* get type bits (3, including end bit) */
      IBM_LENS,     /* get lengths for stored */
      IBM_STORED,   /* processing stored block */
      IBM_TABLE,    /* get table lengths */
      IBM_BTREE,    /* get bit lengths tree for a dynamic block */
      IBM_DTREE,    /* get length, distance trees for a dynamic block */
      IBM_CODES,    /* processing fixed or dynamic block */
      IBM_DRY,      /* output remaining window bytes */
      IBM_DONE,     /* finished last block, done */
      IBM_BAD}      /* got a data error--stuck here */
inflate_block_mode;

typedef enum {        /* waiting for "i:"=input, "o:"=output, "x:"=nothing */
      ICM_START,    /* x: set up for LEN */
      ICM_LEN,      /* i: get length/literal/eob next */
      ICM_LENEXT,   /* i: getting length extra (have base) */
      ICM_DIST,     /* i: get distance next */
      ICM_DISTEXT,  /* i: getting distance extra */
      ICM_COPY,     /* o: copying bytes in window, waiting for space */
      ICM_LIT,      /* o: got literal, waiting for output space */
      ICM_WASH,     /* o: got eob, possibly still output waiting */
      ICM_END,      /* x: got eob and all data flushed */
      ICM_BADCODE}  /* x: got error */
inflate_codes_mode;

/* inflate codes private state */
typedef struct inflate_codes_state {

  /* mode */
  inflate_codes_mode mode;      /* current inflate_codes mode */

  /* mode dependent information */
  uInt len;
  union {
    struct {
      inflate_huft *tree;       /* pointer into tree */
      uInt need;                /* bits needed */
    } code;             /* if LEN or DIST, where in tree */
    uInt lit;           /* if LIT, literal */
    struct {
      uInt get;                 /* bits to get for extra */
      uInt dist;                /* distance back to copy from */
    } copy;             /* if EXT or COPY, where and how much */
  } sub;                /* submode */

  /* mode independent information */
  Byte lbits;           /* ltree bits decoded per branch */
  Byte dbits;           /* dtree bits decoder per branch */
  inflate_huft *ltree;          /* literal/length/eob tree */
  inflate_huft *dtree;          /* distance tree */

} FAR inflate_codes_statef;

/* constants */
#define Z_NO_FLUSH      0
#define Z_PARTIAL_FLUSH 1 /* will be removed, use Z_SYNC_FLUSH instead */
#define Z_SYNC_FLUSH    2
#define Z_FULL_FLUSH    3
#define Z_FINISH        4
/* Allowed flush values; see deflate() below for details */

#define Z_OK            0
#define Z_STREAM_END    1
#define Z_NEED_DICT     2
#define Z_ERRNO        (-1)
#define Z_STREAM_ERROR (-2)
#define Z_DATA_ERROR   (-3)
#define Z_MEM_ERROR    (-4)
#define Z_BUF_ERROR    (-5)
/* Return codes for the compression/decompression functions. Negative
 * values are errors, positive values are used for special but normal events.
 */

#define Z_NO_COMPRESSION         0
#define Z_BEST_SPEED             1
#define Z_BEST_COMPRESSION       9
#define Z_DEFAULT_COMPRESSION  (-1)
/* compression levels */

#define Z_FILTERED            1
#define Z_HUFFMAN_ONLY        2
#define Z_DEFAULT_STRATEGY    0
/* compression strategy; see deflateInit2() below for details */

#define Z_BINARY   0
#define Z_ASCII    1
#define Z_UNKNOWN  2
/* Possible values of the data_type field */

#define Z_DEFLATED   8
/* The deflate compression method (the only one supported in this version) */

#define Z_NULL  0  /* for initializing zalloc, zfree, opaque */

uLong compressBound(uLong sourceLen);

/* basic functions */
int inflate(z_streamp strm, int flush);
int inflateEnd(z_streamp strm);
int inflateSetDictionary(z_streamp strm, const Bytef *dictionary, uInt dictLength);
int inflateSync(z_streamp strm);
int inflateReset(z_streamp strm);
int deflate(z_streamp strm, int flush);
int deflateEnd(z_streamp strm);
int deflateSetDictionary(z_streamp strm, const Bytef *dictionary, uInt dictLength);
int deflateCopy(z_streamp dest, z_streamp source);
int deflateReset(z_streamp strm);
int deflateParams(z_streamp strm, int level, int strategy);

/* utility functions */
#ifdef __cplusplus
extern "C" {
#endif

int compress(Bytef *dest, uLongf *destLen, const Bytef *source, uLong sourceLen);
int compress2(Bytef *dest, uLongf *destLen, const Bytef *source, uLong sourceLen, int level);
int uncompress(Bytef *dest, uLongf *destLen, const Bytef *source, uLong sourceLen);

#ifdef __cplusplus
}
#endif

/* checksum functions */
uLong adler32(uLong adler, const Bytef *buf, uInt len);
uLong crc32(uLong crc, const Bytef *buf, uInt len);

int deflateInit_(z_streamp strm, int level, int stream_size);
int inflateInit_(z_streamp strm, int stream_size);
int deflateInit2_(z_streamp strm, int  level, int  method, int windowBits, int memLevel, int strategy, int stream_size);
int inflateInit2_(z_streamp strm, int  windowBits, int stream_size);
#define deflateInit(strm, level) deflateInit_((strm), (level), sizeof(z_stream))
#define inflateInit(strm) inflateInit_((strm), sizeof(z_stream))
#define deflateInit2(strm, level, method, windowBits, memLevel, strategy) deflateInit2_((strm),(level),(method),(windowBits),(memLevel), (strategy), sizeof(z_stream))
#define inflateInit2(strm, windowBits) inflateInit2_((strm), (windowBits), sizeof(z_stream))
#define put_byte(s, c) {s->pending_buf[s->pending++] = (c);}
#define MIN_LOOKAHEAD (MAX_MATCH+MIN_MATCH+1)
#define MAX_DIST(s)  ((s)->w_size-MIN_LOOKAHEAD)

struct inflate_blocks_state;
typedef struct inflate_blocks_state FAR inflate_blocks_statef;

inflate_blocks_statef * inflate_blocks_new(z_streamp z, check_func c, uInt w);

int inflate_blocks(inflate_blocks_statef *, z_streamp, int);

void inflate_blocks_reset(inflate_blocks_statef *, z_streamp , uLongf *);

int inflate_blocks_free(inflate_blocks_statef *, z_streamp);

void inflate_set_dictionary(inflate_blocks_statef *s, const Bytef *d, uInt n);

int inflate_blocks_sync_point(inflate_blocks_statef *s);

int inflate_flush(inflate_blocks_statef *s, z_streamp z, int r);

int inflate_trees_bits(
    uIntf *,                    /* 19 code lengths */
    uIntf *,                    /* bits tree desired/actual depth */
    inflate_huft * FAR *,       /* bits tree result */
    inflate_huft *,             /* space for trees */
    z_streamp);                /* for messages */

int inflate_trees_dynamic(
    uInt,                       /* number of literal/length codes */
    uInt,                       /* number of distance codes */
    uIntf *,                    /* that many (total) code lengths */
    uIntf *,                    /* literal desired/actual bit depth */
    uIntf *,                    /* distance desired/actual bit depth */
    inflate_huft * FAR *,       /* literal/length tree result */
    inflate_huft * FAR *,       /* distance tree result */
    inflate_huft *,             /* space for trees */
    z_streamp);                /* for messages */