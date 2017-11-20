#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <limits.h>
#include <ctype.h>
#include <assert.h>
#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include <math.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>

#include "sfh.h"

#define MAJOR_VERS 4
#define FORMAT_VERS 4
#define MINOR_VERS 5

/* Keep as a power of 2 */
//#define QMAX 128
#define QMAX 64

/* Maximum length of a SOLiD sequencer entry */
#define MAX_SOLID 1024

#ifdef PTHREADS
#  include <pthread.h>
#  include <utmpx.h>
#else
#  define sched_getcpu() -1
#endif

/* Debug timing. Only works in non-threaded mode */
//#define TIMING

/*
 * SSE support to allow use of memory prefetching. It's only minor, but
 * it all helps.
 *
 * With    on -s6 -q3 -b (40million lines)
 *    encode: 2m55.691s+0m8.813s
 *    decode: 3m19.492s+0m2.720s
 *
 * Without on -s6 -q3 -b
 *    encode: 2m57.115s+0m3.260s
 *    decode: 3m46.338s+0m2.856s
 */
#ifdef __SSE__
#   include <xmmintrin.h>
#else
#   define _mm_prefetch(a,b)
#endif

#define ABS(a)   ((a)>0?(a):-(a))
#define MIN(a,b) ((a)<(b)?(a):(b))
#define MAX(a,b) ((a)>(b)?(a):(b))

/* Range Coder:
 *
 * This is still using Eugene Shelwien's code from coders6c2.zip.
 * TODO: implement my own from scratch, although it's doubtful I'll
 * get something as efficient.
 */
//#include "../trunk/clrf.cdr"
#include "clr.cdr"
//#include "clrf256.cdr"
//#include "rc.h"

/*
 * Order 0 models, optimsed for various sizes of alphabet.
 * order0_coder is the original Dmitry Shkarin code, tweaked a bit to
 * make RangeCoder a parameter and to work as a template for adjusting
 * the alphabet size.
 *
 * Simple_model is my own implementation adhering to the same API as
 * Dmitry Shkarin's original ORDER_0_CODER model. It's here for
 * purposes of adhering to the SequenceSqueeze entry rules, but
 * unfortunately it's ~5% slower.  It differs in having no escape
 * symbol (which is not a hindrance for stationary probabilities) and
 * using a different frequency updating method.
 *
 * Base coder is my own model specialising in symbols 0, 1, 2, 3, with
 * dedicated code for handling N (it encodes whichever of 0, 1, 2, 3
 * is most common and returns that value).
 *
 * All of these models are used in large arrays to implement a context,
 * ie to turn them into a high order model.
 */
#ifdef ORIG_MODEL
#    define ORDER_0_CODER SIMPLE_MODEL
#    include "order0_coder.h"
#else
#    include "simple_model.h" // SIMPLE_MODEL
#endif

#include "base_model.h"       // BASE_MODEL
//#include "base_model2.h"       // BASE_MODEL

//#define PREHASHED

/*
 * Defining this will use 2 sequence models, a smaller 7-mer one and a
 * larger one defined by the -s parameter. For novel kmers we encode using
 * the smaller 7-mer (and then update the full kmer version).
 *
 * This improves compression early on in a stream while the larger model
 * learns, but has no impact after a while as the larger model fills up.
 *
 * It's about 15% slower overall, but saves ~1%, depending on size of input.
 * Only a worthwhile trade when going for maximum compression.
 *
 * NOTE: now specified by -s<num>+; eg -s6+ vs -s6
 */


#define BLK_SIZE 10000000
//#define BLK_SIZE 1000000

/* QBITS is the size of the quality context, 2x quals */
#define QBITS 12
#define QSIZE (1<<QBITS)

/*
 * fqz parameter block.
 */
typedef struct {
    int slevel;  		// -s level
    int qlevel;  		// -q level
    int nlevel;  		// -n level
    int both_strands;		// True if -b used
    int extreme_seq;		// True if -e used; 16-bit seq counters
    int multi_seq_model;	// True if -s<level>+; use 2 model sizes
    int qual_approx;		// 0 for lossless, >0 for lossy qual encoding
    int do_threads;		// Simple multi-threading enabled.
    int do_hash;		// Generate and test check sums.
    int SOLiD;			// A SOLiD data file
} fqz_params;

/*
 * The fqz class itself
 */
struct fqz {
public:
    fqz();

    /* Replace with an argument struct */
    fqz(fqz_params *p);
    ~fqz();

    int encode(int in_fd, int out_fd);
    int decode(int in_fd, int out_fd);

    /* Compression metrics */
    uint64_t base_in, base_out;
    uint64_t qual_in, qual_out;
    uint64_t name_in, name_out;

    /* Entry points for pthread calls; consider as internal */
    void compress_r1();
    void compress_r2();
    void compress_r3();

    void decompress_r1();
    void decompress_r2();
    void decompress_r3();

protected:
    /* --- Parameters passed into the constructor */
    int slevel, qlevel, nlevel;
    int both_strands;
    int extreme_seq;
    int multi_seq_model;
    int qual_approx; 
    int do_threads;
    int do_hash;
    int SOLiD;

    int L[256];          // Sequence table lookups ACGTN->0..4
    char solid_primer;
    char primer_qual;    // True is primer base has a dummy quality

    /* --- Buffers */
    // Input and output buffers; need to be size of BLK_SIZE
    char in_buf[BLK_SIZE];
    char out_buf[BLK_SIZE];
    int out_ind, in_ind; // indices into in_buf & out_buf.

    /* Block variables for the encoder to work on */
    /* FIXME: shrink these, or new[] them */
    int ns;
    int seq_len; // +ve if fixed per block, -ve if variable.
    char name_buf[BLK_SIZE]; 
    char seq_buf[BLK_SIZE/2];
    char qual_buf[BLK_SIZE/2];
    int name_len_a[BLK_SIZE/9];
    int seq_len_a[BLK_SIZE/9];
    char out0[BLK_SIZE]; // seq_len
    char out1[BLK_SIZE]; // name
    char out2[BLK_SIZE/2]; // seq
    char out3[BLK_SIZE/2]; // qual
    int sz0, sz1, sz2, sz3;
    char *in_buf0, *in_buf1, *in_buf2, *in_buf3;

    // Hashes for error detection.
    uint32_t chksum;

    /* --- Models */
    // Sequence length
    SIMPLE_MODEL<256> model_len1;
    SIMPLE_MODEL<256> model_len2;
    SIMPLE_MODEL<2> model_same_len;
    int last_len;

    void encode_len(RangeCoder *rc, int len);
    int  decode_len(RangeCoder *rc);


    // Names
    SIMPLE_MODEL<256> model_name_prefix[256];
    SIMPLE_MODEL<256> model_name_suffix[256];
    SIMPLE_MODEL<256> model_name_len[256];
    SIMPLE_MODEL<128> model_name_middle[8192];

#define MAX_TOK 1000
    // Name lvl 2
    SIMPLE_MODEL<10> model_name_type[MAX_TOK];
    SIMPLE_MODEL<256> model_name_alpha_len[MAX_TOK];
    SIMPLE_MODEL<256> model_name_alpha[MAX_TOK];
    SIMPLE_MODEL<256> model_name_zero[MAX_TOK];
    SIMPLE_MODEL<256> model_name_digit0[MAX_TOK];
    SIMPLE_MODEL<256> model_name_digit1[MAX_TOK];
    SIMPLE_MODEL<256> model_name_digit2[MAX_TOK];
    SIMPLE_MODEL<256> model_name_digit3[MAX_TOK];
    SIMPLE_MODEL<256> model_name_ddelta[MAX_TOK];
    SIMPLE_MODEL<256> model_name_char[MAX_TOK];

    char last_name[1024]; // Last name
    int last_name_len;    // Length of last name
    int last_p_len;       // Length of last common prefix
    int last_s_len;       // Length of last common suffix

    void encode_name(RangeCoder *rc, char *name, int len);
    int decode_name(RangeCoder *rc, char *name);

    void encode_name2(RangeCoder *rc, char *name, int len);
    int decode_name2(RangeCoder *rc, char *name);


    // Sequence
    int NS; // Number of bases of sequence context.
    BASE_MODEL<uint8_t> *model_seq8;
    BASE_MODEL<uint16_t> *model_seq16;

    //#define NS_MASK ((1<<(2*NS))-1)
#define SMALL_NS 7
#define SMALL_MASK ((1<<(2*SMALL_NS))-1)
    BASE_MODEL<uint8_t> model_seq_small[1<<(2*SMALL_NS)];

    void encode_seq8 (RangeCoder *rc, char *seq, int len);
    void encode_seq16(RangeCoder *rc, char *seq, int len);

    void decode_seq8 (RangeCoder *rc, char *seq, int len);
    void decode_seq16(RangeCoder *rc, char *seq, int len);


    // Quality
    SIMPLE_MODEL<QMAX> *model_qual;
#define SMALL_QMASK (QSIZE-1)

    void encode_qual(RangeCoder *rc, char *seq, char *qual, int len);
    void decode_qual(RangeCoder *rc, char *qual, int len);

    void encode_qual_lossy(RangeCoder *rc, char *qual, int len, int approx);


    /* --- Main functions for compressing and decompressing blocks */
    int fq_compress(char *in,  int in_len,
		    char *out, int *out_len,
		    char **in_end, int *nseqs);

    char *fq_decompress(char *in, int comp_len, int *uncomp_len);

    void load_hash_freqs(const char *fn);
};


/* -------------------------------------------------------------------------
 * Constructors and destructors
 */
fqz::fqz() {
    fqz(NULL);
}

fqz::fqz(fqz_params *p) {
    if (p) {
	slevel          = p->slevel;
	qlevel          = p->qlevel;
	nlevel          = p->nlevel;
	both_strands    = p->both_strands;
	extreme_seq     = p->extreme_seq;
	multi_seq_model = p->multi_seq_model;
	qual_approx     = p->qual_approx;
	do_threads      = p->do_threads;
	do_hash         = p->do_hash; // negligible slow down
	SOLiD           = p->SOLiD;
    } else {
	slevel = 3;
	qlevel = 2;
	nlevel = 1;
	both_strands = 0;
	extreme_seq = 0;
	multi_seq_model = 0;
	qual_approx = 0;
	do_threads = 1;
	do_hash = 1;
	SOLiD = 0;
    }

    /* ACGTN* */
    for (int i = 0; i < 256; i++)
	L[i] = 0;
    if (SOLiD) {
	L['0'] = 0;
	L['1'] = 1;
	L['2'] = 2;
	L['3'] = 3;
	solid_primer = 'x';
	primer_qual = 0;
    } else {
	L['A'] = L['a'] = 0;
	L['C'] = L['c'] = 1;
	L['G'] = L['g'] = 2;
	L['T'] = L['t'] = 3;
    }
    
    NS = 7 + slevel;
    if (extreme_seq) {
	model_seq8  = NULL;
	model_seq16 = new BASE_MODEL<uint16_t>[1<<(2*NS)];
    } else {
	model_seq8  = new BASE_MODEL<uint8_t>[1<<(2*NS)];
	model_seq16 = NULL;
    }

    int qsize = QSIZE;
    if (qlevel > 1) qsize *= 16;
    if (qlevel > 2) qsize *= 16;
    model_qual = new SIMPLE_MODEL<QMAX>[qsize];

#ifdef PREHASHED
    /* Helps on shallow data far more than deep data */
    load_hash_freqs("human_hash13_2strand");
    //load_hash_freqs("human_hash16");
#endif

    /* Name settings */
    memset(last_name, ' ', 1024);
    last_name_len = 0;
    last_p_len = 0;
    last_s_len = 0;

    /* Length settings */
    last_len = 0;

    name_in = name_out = 0;
    base_in = base_out = 0;
    qual_in = qual_out = 0;
}

fqz::~fqz() {
    if (model_seq8)  delete[] model_seq8;
    if (model_seq16) delete[] model_seq16;
    if (model_qual)  delete[] model_qual;
}

/*
 * Use a predefined hash table of sequence frequencies for priming the
 * model_seq contexts. If this turns out to be widely useful then
 * model_seq should be updated to allow this to work in one easy data read.
 *
 * Arguably we should also compute the stats once and then not update
 * during encoding.
 */
void fqz::load_hash_freqs(const char *fn) {
    unsigned char c4[4];
    int ctx = 0;
    FILE *fp = fopen(fn, "rb");

    fprintf(stderr, "Loading %s...", fn);
    fflush(stderr);

    if (!fp) {
	perror(fn);
	return;
    }

    while (1 == fread(c4, 4, 1, fp)) {
	int st[4];
	st[0] = c4[0] + 1;
	st[1] = c4[1] + 1;
	st[2] = c4[2] + 1;
	st[3] = c4[3] + 1;

	assert(ctx < (1<<(2*NS)));
	if (extreme_seq) {
	    model_seq16[ctx++].reset(st);
	} else {
	    model_seq8[ctx++].reset(st);
	}
    }

    fclose(fp);

    fprintf(stderr, "done\n");
}


/* -------------------------------------------------------------------------
 * Name model
 */
void fqz::encode_name(RangeCoder *rc, char *name, int len) {
    int p_len, s_len; // prefix and suffix length
    int i, j, k, last_char;
//    static char meta[] = {
//	0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0, /* 00-0f */
//	0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0, /* 10-1f */
//	1, 1, 1, 1, 1, 1, 1, 1,   1, 1, 1, 1, 1, 1, 1, 1, /* 20-2f */
//	0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 1, 1, 1, 1, 1, 1, /* 30-3f */
//	1, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0, /* 40-4f */
//	0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 1, 1, 1, 1, 1, /* 50-5f */
//	1, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0, /* 60-6f */
//	0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 1, 1, 1, 1, 1, /* 70-7f */
//    };

    _mm_prefetch((const char *)&model_name_prefix[last_p_len], _MM_HINT_T0);
    _mm_prefetch((const char *)&model_name_suffix[last_s_len], _MM_HINT_T0);
    _mm_prefetch((const char *)&model_name_len[last_name_len], _MM_HINT_T0);

    // Prefix
    for (i = 0; i < len && i < last_name_len; i++) {
	if (name[i] != last_name[i])
	    break;
    }
    p_len = i;

    // Suffix
    for (i = len-1, j= last_name_len-1; i >= 0 && j >= 0; i--, j--) {
	if (name[i] != last_name[j])
	    break;
    }
    s_len = len-1 - i;
    if (len - s_len - p_len < 0)
	s_len = len - p_len;

    model_name_prefix[last_p_len].   encodeSymbol(rc, p_len);
    model_name_suffix[last_s_len].   encodeSymbol(rc, s_len);
    model_name_len   [last_name_len].encodeSymbol(rc, len);

    last_p_len = p_len;
    last_s_len = s_len;

    int len2 = len - s_len, lc2 = p_len ? 1 : 0;
    for (i = j = p_len, k = 0; i < len2; i++, j++, k++) {
	last_char = ((last_name[j]-32)*2 + lc2 + k*64) % 8192;
	_mm_prefetch((const char *)&model_name_middle[last_char], _MM_HINT_T0);

	//last_char = ((last_name[j]-32)*2 + lc2 + (i+j)*32) % 8192;

	model_name_middle[last_char].encodeSymbol(rc, name[i] & 0x7f);

	//if (meta[name[i]]      && name[i] != last_name[j]) j++;
	//if (meta[last_name[j]] && name[i] != last_name[j]) j--;

	//if (meta[name[i]]) k = (k+3)>>2<<2;

	if (name[i] == ' ' && last_name[j] != ' ') j++;
        if (name[i] != ' ' && last_name[j] == ' ') j--;
	if (name[i] == ':' && last_name[j] != ':') j++;
	if (name[i] != ':' && last_name[j] == ':') j--;

	if (name[i] == ':' || name[i] == ' ') k = (k+3)>>2<<2;

	lc2 = name[i] == last_name[j];
    }

    memcpy(last_name, name, len);
    last_name_len = len;
}

int fqz::decode_name(RangeCoder *rc, char *name) {
    int p_len, s_len, len; // prefix and suffix length
    int i, j, k;
    int last_char;

    _mm_prefetch((const char *)&model_name_prefix[last_p_len], _MM_HINT_T0);
    _mm_prefetch((const char *)&model_name_prefix[last_p_len], _MM_HINT_T0);
    _mm_prefetch((const char *)&model_name_prefix[last_p_len], _MM_HINT_T0);

    p_len = model_name_prefix[last_p_len].   decodeSymbol(rc);
    s_len = model_name_suffix[last_s_len].   decodeSymbol(rc);
    len   = model_name_len   [last_name_len].decodeSymbol(rc);

    last_p_len = p_len;
    last_s_len = s_len;

    for (i = 0; i < p_len; i++)
	name[i] = last_name[i];

    //fprintf(stderr, "%d: p_len = %d, s_len = %d, len = %d last='%.*s'\n",
    //column, p_len, s_len, len, last_name_len, last);

    int len2 = len - s_len, lc2 = p_len ? 1 : 0;
    for (i = j = p_len, k = 0; i < len2; i++, j++, k++) {
	unsigned char c;

	last_char = ((last_name[j]-32)*2 + lc2 + k*64) % 8192;
	//last_char = (last_name[j] + j*64) % 8192;

	c = model_name_middle[last_char].decodeSymbol(rc);
	//c = 'x';
	name[i] = c;

	if (c == ' ' && last_name[j] != ' ') j++;
        if (c != ' ' && last_name[j] == ' ') j--;
	if (c == ':' && last_name[j] != ':') j++;
	if (c != ':' && last_name[j] == ':') j--;

	if (name[i] == ':' || name[i] == ' ') k = (k+3)>>2<<2;

	lc2 = c == last_name[j];
    }

    for (j = last_name_len-s_len; i < len; i++, j++)
	name[i] = last_name[j];

    memcpy(last_name, name, len);
    last_name_len = len;

    return len;
}

/* Level 2 */
enum name_type {N_UNK = 0, N_ALPHA, N_CHAR,
		N_ZERO, N_DIGITS, N_DDELTA, N_MATCH, N_END};

void fqz::encode_name2(RangeCoder *rc, char *name, int len) {
    int i, j, k;

    static int last_token_type[1024];
    static int last_token_int[1024];
    static int last_token_str[1024];

    //fprintf(stderr, "NAME: %.*s\n", len, name);

    int ntok = 0;
    for (i = j = 0, k = 0; i < len; i++, j++, k++) {
	/* Determine data type of this segment */
	int n_type = N_ALPHA;
	if (isalpha(name[i])) {
	    int s = i+1;
	    while (s < len && isalpha(name[s]))
		s++;
	    n_type = N_ALPHA;

	    if (last_token_type[ntok] == N_ALPHA) {
		if (s-i == last_token_int[ntok] &&
		    memcmp(&name[i], 
			   &last_name[last_token_str[ntok]],
			   s-i) == 0) {
		    //fprintf(stderr, "Tok %d (mat)\n", N_MATCH);
		    model_name_type[ntok].encodeSymbol(rc, N_MATCH);
		} else {
		    //fprintf(stderr, "Tok %d (alpha)\n", N_ALPHA);
		    model_name_type[ntok].encodeSymbol(rc, N_ALPHA);
		    model_name_alpha_len[ntok].encodeSymbol(rc, s-i);
		    for (int x = 0; x < s-i; x++) {
			model_name_alpha[ntok].encodeSymbol(rc, name[i+x]);
		    }
		}
	    } else {
		//fprintf(stderr, "Tok %d (alpha)\n", N_ALPHA);
		model_name_type[ntok].encodeSymbol(rc, N_ALPHA);
		model_name_alpha_len[ntok].encodeSymbol(rc, s-i);
		for (int x = 0; x < s-i; x++) {
		    model_name_alpha[ntok].encodeSymbol(rc, name[i+x]);
		}
	    }

	    last_token_int[ntok] = s-i;
	    last_token_str[ntok] = i;
	    last_token_type[ntok] = N_ALPHA;

	    i = s-1;
	} else if (name[i] == '0') {
	    int s = i, v;
	    while (s < len && name[s] == '0')
		s++;
	    v = s-i;
	    n_type = N_ZERO;

	    if (last_token_type[ntok] == N_ZERO) {
		if (last_token_int[ntok] == v) {
		    //fprintf(stderr, "Tok %d (mat)\n", N_MATCH);
		    model_name_type[ntok].encodeSymbol(rc, N_MATCH);
		} else {
		    //fprintf(stderr, "Tok %d (0)\n", N_ZERO);
		    model_name_type[ntok].encodeSymbol(rc, N_ZERO);
		    model_name_zero[ntok].encodeSymbol(rc, v);
		}
	    } else {
		//fprintf(stderr, "Tok %d (0)\n", N_ZERO);
		model_name_type[ntok].encodeSymbol(rc, N_ZERO);
		model_name_zero[ntok].encodeSymbol(rc, v);
	    }

	    last_token_int[ntok] = v;
	    last_token_type[ntok] = N_ZERO;

	    i = s-1;
	} else if (isdigit(name[i])) {
	    int s = i;
	    int v = 0;
	    int d = 0;
	    while (s < len && isdigit(name[s]) && v < (1<<27)) {
		v = v*10 + name[s] - '0';
		s++;
	    }
	    n_type = N_DIGITS;

	    if (last_token_type[ntok] == N_DIGITS) {
		if ((d = v - last_token_int[ntok]) == 0) {
		    //fprintf(stderr, "Tok %d (mat)\n", N_MATCH);
		    model_name_type[ntok].encodeSymbol(rc, N_MATCH);
		} else if (d < 256 && d > 0) {
		    //fprintf(stderr, "Tok %d (delta)\n", N_DDELTA);
		    model_name_type[ntok].encodeSymbol(rc, N_DDELTA);
		    model_name_ddelta[ntok].encodeSymbol(rc, d);
		} else {
		    //fprintf(stderr, "Tok %d (dig)\n", N_DIGITS);
		    model_name_type[ntok].encodeSymbol(rc, N_DIGITS);
		    model_name_digit0[ntok].encodeSymbol(rc, (v>> 0) & 0xff);
		    model_name_digit1[ntok].encodeSymbol(rc, (v>> 8) & 0xff);
		    model_name_digit2[ntok].encodeSymbol(rc, (v>>16) & 0xff);
		    model_name_digit3[ntok].encodeSymbol(rc, (v>>24) & 0xff);
		}
	    } else {
		//fprintf(stderr, "Tok %d (dig)\n", N_DIGITS);
		model_name_type[ntok].encodeSymbol(rc, N_DIGITS);
		model_name_digit0[ntok].encodeSymbol(rc, (v>> 0) & 0xff);
		model_name_digit1[ntok].encodeSymbol(rc, (v>> 8) & 0xff);
		model_name_digit2[ntok].encodeSymbol(rc, (v>>16) & 0xff);
		model_name_digit3[ntok].encodeSymbol(rc, (v>>24) & 0xff);
	    }

	    last_token_int[ntok] = v;
	    last_token_type[ntok] = N_DIGITS;

	    i = s-1;
	} else {
	    n_type = N_CHAR;

	    if (last_token_type[ntok] == N_CHAR) {
		if (name[i] == last_token_int[ntok]) {
		    //fprintf(stderr, "Tok %d (mat)\n", N_MATCH);
		    model_name_type[ntok].encodeSymbol(rc, N_MATCH);
		} else {
		    //fprintf(stderr, "Tok %d (chr)\n", N_CHAR);
		    model_name_type[ntok].encodeSymbol(rc, N_CHAR);
		    model_name_char[ntok].encodeSymbol(rc, name[i]);
		}
	    } else {
		//fprintf(stderr, "Tok %d (chr)\n", N_CHAR);
		model_name_type[ntok].encodeSymbol(rc, N_CHAR);
		model_name_char[ntok].encodeSymbol(rc, name[i]);
	    }

	    last_token_int[ntok] = name[i];
	    last_token_type[ntok] = N_CHAR;
	}

	ntok++;
    }
    //fprintf(stderr, "Tok %d (end)\n", N_END);
    model_name_type[ntok].encodeSymbol(rc, N_END);
    
    memcpy(last_name, name, len);
    last_name_len = len;
}

int fqz::decode_name2(RangeCoder *rc, char *name) {
    enum name_type tok;
    int ntok = 0, i = 0, v;

    static int last_token_type[1024];
    static int last_token_int[1024];
    static int last_token_str[1024];

    for (;;) {
	tok = (enum name_type)model_name_type[ntok].decodeSymbol(rc);
	//fprintf(stderr, "tok=%d, last type=%d int=%d str=%d\n",
	//	tok, last_token_type[ntok], last_token_int[ntok],
	//	last_token_str[ntok]);
	if (tok == N_END)
	    break;

	switch (tok) {
	    /* Str delta too? */
	case N_ALPHA:
	    v = model_name_alpha_len[ntok].decodeSymbol(rc);
	    last_token_int[ntok] = v; // len
	    last_token_str[ntok] = i;
	    for (int x = 0; x < v; x++)
		// also per 'x'; per pos in name? */
		name[i++] = model_name_alpha[ntok].decodeSymbol(rc);
	    last_token_type[ntok] = N_ALPHA;
	    break;

	case N_CHAR:
	    v = model_name_char[ntok].decodeSymbol(rc);
	    name[i++] = v;
	    last_token_int[ntok] = v;
	    last_token_type[ntok] = N_CHAR;
	    break;

	case N_ZERO:
	    v = model_name_zero[ntok].decodeSymbol(rc);
	    last_token_int[ntok] = v;
	    for (int x = 0; x < v; x++)
		name[i++] = '0';
	    last_token_type[ntok] = N_ZERO;
	    break;

	case N_DIGITS: {
	    char rev[100];
	    int ri = 0, v0, v1, v2, v3;

	    v0 = model_name_digit0[ntok].decodeSymbol(rc);
	    v1 = model_name_digit1[ntok].decodeSymbol(rc);
	    v2 = model_name_digit2[ntok].decodeSymbol(rc);
	    v3 = model_name_digit3[ntok].decodeSymbol(rc);
	    v = v0 + (v1<<8) + (v2<<16) + (v3<<24);
	    last_token_int[ntok] = v;
	    while (v > 0) {
		rev[ri++] = '0' + (v%10);
		v /= 10;
	    }
	    while (ri > 0)
		name[i++] = rev[--ri];
	    last_token_type[ntok] = N_DIGITS;
	    break;
	}

	case N_DDELTA: {
	    char rev[100];
	    int ri = 0;

	    v = model_name_ddelta[ntok].decodeSymbol(rc);
	    v += last_token_int[ntok];
	    last_token_int[ntok] = v;
	    while (v > 0) {
		rev[ri++] = '0' + (v%10);
		v /= 10;
	    }
	    while (ri > 0)
		name[i++] = rev[--ri];
	    last_token_type[ntok] = N_DIGITS;
	    break;
	}

	case N_MATCH:
	    switch (last_token_type[ntok]) {
	    case N_CHAR:
		name[i++] = last_token_int[ntok];
		break;

	    case N_ALPHA:
		v = last_token_int[ntok];
		for (int x = 0; x < v; x++)
		    name[i++] = last_name[last_token_str[ntok]+x];
		last_token_str[ntok] = i-v;
		break;

	    case N_ZERO:
		v = last_token_int[ntok];
		for (int x = 0; x < v; x++)
		    name[i++] = '0';
		break;

	    case N_DIGITS: {
		char rev[100];
		int ri = 0;
		v = last_token_int[ntok];

		while (v > 0) {
		    rev[ri++] = '0' + (v%10);
		    v /= 10;
		}
		while (ri > 0)
		    name[i++] = rev[--ri];
		break;
	    }
	    }
	    break;

	default:
	    fprintf(stderr, "Unexpected name token %d\n", tok);
	    return -1;
	}

	ntok++;
    }

    name[i] = '\0';
    memcpy(last_name, name, i);
    last_name_len = i;

    //fprintf(stderr, "%s\n", name);

    return i;
}

/* -------------------------------------------------------------------------
 * Sequence length model
 */
void fqz::encode_len(RangeCoder *rc, int len) {
    if (len != last_len) {
	model_same_len.encodeSymbol(rc, 0);
	model_len1.encodeSymbol(rc, len & 0xff);
	model_len2.encodeSymbol(rc, (len >> 8) & 0xff);
    } else {
	model_same_len.encodeSymbol(rc, 1);
    }
}

int fqz::decode_len(RangeCoder *rc) {
    if (model_same_len.decodeSymbol(rc)) {
	return last_len;
    } else {
	int l1 = model_len1.decodeSymbol(rc);
	int l2 = model_len2.decodeSymbol(rc);
	last_len = l1 + (l2 << 8);
	return last_len;
    }
}


/* -------------------------------------------------------------------------
 * Sequence model
 *
 * We have 8-bit and 16-bit accumulator versions.
 * The 8-bit one is lower memory and somtimes slightly faster, but
 * marginally less optimal in compression ratios (within 1%).
 */
void fqz::encode_seq8(RangeCoder *rc, char * seq, int len) {
    int last, last2;
    int bc[4] = {(3-0) << (2*NS-2),
		 (3-1) << (2*NS-2),
		 (3-2) << (2*NS-2),
		 (3-3) << (2*NS-2)};
    const int NS_MASK = ((1<<(2*NS))-1);

    /* Corresponds to a 12-mer word that doesn't occur in human genome. */
    last  = 0x007616c7 & NS_MASK;
    last2 = (0x2c6b62ff >> (32 - 2*NS)) & NS_MASK;
    
    _mm_prefetch((const char *)&model_seq8[last], _MM_HINT_T0);

    if (multi_seq_model) {
	for (int i = 0; i < len; i++) {
	    unsigned int l2 = (last << 2) & NS_MASK;
	    _mm_prefetch((const char *)&model_seq8[l2+0], _MM_HINT_T0);
	    //_mm_prefetch((const char *)&model_seq8[l2+3], _MM_HINT_T0);
		
	    unsigned char  b = L[(unsigned char)seq[i]];

	    /* Works OK on small files to rapidly train, but no gain on
	     * large ones
	     */
	    if ((model_seq8     [last             ].getTopSym() *
		 model_seq_small[last & SMALL_MASK].getSummFreq()) >
		(model_seq_small[last & SMALL_MASK].getTopSym() *
		 model_seq8     [last             ].getSummFreq())) {
		model_seq8[last].encodeSymbol(rc, b);
		//model_seq_small[last & SMALL_MASK].updateSymbol(b);
	    } else {
		model_seq_small[last & SMALL_MASK].encodeSymbol(rc, b);
		model_seq8[last].updateSymbol(b);
	    }

	    last = (last*4 + b) & NS_MASK;

	    /*
	     * On deep data hashing both strands works well. On shallow data
	     * it adds a significant CPU hit for very minimal gains (at best
	     * 0.5% smaller seq portion).
	     * Eg: -6=>513049628, -6b=>510382143, -8b=501978520 (Seq only)
	     *
	     * Pre-seeding the hash table with double-stranded human genome
	     * hashes seems like a faster starting point and will help more
	     * for shallow data too. However even this isn't as significant
	     * as it sounds.
	     * -5=>516624591, -5h=>514002730
	     */
	    if (both_strands) {
		int b2 = last2 & 3;
		last2 = last2/4 + ((3-b) << (2*NS-2));
		_mm_prefetch((const char *)&model_seq8[last2], _MM_HINT_T0);
		model_seq8[last2].updateSymbol(b2);
	    }
	}
    } else {
	if (both_strands) {
	    unsigned l2 = last2;
	    for (int i = 0; i < len && i < 128; i++) {
		unsigned char  b = L[(unsigned char)seq[i]];
		l2 = l2/4 + bc[b];
		_mm_prefetch((const char *)&model_seq8[l2], _MM_HINT_T0);
	    }

	    for (int i = 0; i < len; i++) {
		unsigned int l2 = (last << 2) & NS_MASK;
		_mm_prefetch((const char *)&model_seq8[l2+0], _MM_HINT_T0);
		
		unsigned char  b = L[(unsigned char)seq[i]];
		model_seq8[last].encodeSymbol(rc, b);

		last = (last*4 + b) & NS_MASK;

		int b2 = last2 & 3;
		last2 = last2/4 + bc[b];
		model_seq8[last2].updateSymbol(b2);
	    }
	} else {
	    for (int i = 0; i < len; i++) {
		unsigned int l2 = (last << 2) & NS_MASK;
		_mm_prefetch((const char *)&model_seq8[l2+0], _MM_HINT_T0);
		
		unsigned char  b = L[(unsigned char)seq[i]];
		model_seq8[last].encodeSymbol(rc, b);

		last = ((last<<2) + b) & NS_MASK;
	    }
	}
    }
}

void fqz::encode_seq16(RangeCoder *rc, char *seq, int len) {
    int last, last2;

    const int NS_MASK = ((1<<(2*NS))-1);

    /* Corresponds to a 12-mer word that doesn't occur in human genome. */
    last  = 0x7616c7 & NS_MASK;
    last2 = (0x2c6b62ff >> (32 - 2*NS)) & NS_MASK;

    for (int i = 0; i < len; i++) {
	unsigned char  b = L[(unsigned char)seq[i]];
	model_seq16[last].encodeSymbol(rc, b);
	last = (last*4 + b) & NS_MASK;
	//volatile int p = *(int *)&model_seq16[last];
	_mm_prefetch((const char *)&model_seq16[last], _MM_HINT_T0);

	if (both_strands) {
	    int b2 = last2 & 3;
	    last2 = last2/4 + ((3-b) << (2*NS-2));
	    _mm_prefetch((const char *)&model_seq16[last2], _MM_HINT_T0);
	    model_seq16[last2].updateSymbol(b2);
	}
    }
}

void fqz::decode_seq8(RangeCoder *rc, char *seq, int len) {
    int last, last2;
    const char *dec = SOLiD ? "0123." : "ACGTN";

    const int NS_MASK = ((1<<(2*NS))-1);

    /*
     * We can't do the same prefetch loop here as we don't know what the
     * data is yet, so we can't predict the memory addresses in model[]
     * that we're going to access.
     *
     * However we can guess it'll be one of 4 base calls, so populate the
     * cache with all 4 choices so by the time we get there it'll have been
     * loaded.
     */

    last  = 0x7616c7 & NS_MASK;
    last2 = (0x2c6b62ff >> (32 - 2*NS)) & NS_MASK;

    if (multi_seq_model) {
	for (int i = 0; i < len; i++) {
	    unsigned char b;
	    unsigned int m = (last<<2) & NS_MASK;
	    _mm_prefetch((const char *)&model_seq8[m+0], _MM_HINT_T0);
	    //_mm_prefetch((const char *)&model_seq8[m+3], _MM_HINT_T0);
	    
	    m &= SMALL_MASK;
	    _mm_prefetch((const char *)&model_seq_small[m+0], _MM_HINT_T0);
	    //_mm_prefetch((const char *)&model_seq_small[m+3], _MM_HINT_T0);

	    if ((model_seq8     [last             ].getTopSym() *
		 model_seq_small[last & SMALL_MASK].getSummFreq()) >
		(model_seq_small[last & SMALL_MASK].getTopSym() *
		 model_seq8     [last             ].getSummFreq())) {
		b = model_seq8[last].decodeSymbol(rc);
		//model_seq_small[last & SMALL_MASK].updateSymbol(b);
	    } else {
		b = model_seq_small[last & SMALL_MASK].decodeSymbol(rc);
		model_seq8[last].updateSymbol(b);
	    }
	    *seq++ = dec[b];
	    last = (last*4 + b) & NS_MASK;

	    _mm_prefetch((const char *)&model_seq8[last], _MM_HINT_T0);

	    if (both_strands) {
		int b2 = last2 & 3;
		last2 = last2/4 + ((3-b) << (2*NS-2));
		_mm_prefetch((const char *)&model_seq8[last2], _MM_HINT_T0);
		model_seq8[last2].updateSymbol(b2);
	    }
	}
    } else {
	if (both_strands) {
	    for (int i = 0; i < len; i++) {	
		unsigned char b;
		unsigned int m = (last<<2) & NS_MASK;
		int b2;

		/* Get next set loaded */
		_mm_prefetch((const char *)&model_seq8[m+0], _MM_HINT_T0);
		//_mm_prefetch((const char *)&model_seq8[m+3], _MM_HINT_T0);

		b = model_seq8[last].decodeSymbol(rc);

		*seq++ = dec[b];
		last = (last*4 + b) & NS_MASK;

		b2 = last2 & 3;
		last2 = last2/4 + ((3-b) << (2*NS-2));
		_mm_prefetch((const char *)&model_seq8[last2], _MM_HINT_T0);
		model_seq8[last2].updateSymbol(b2);
	    }
	} else {
	    for (int i = 0; i < len; i++) {	
		unsigned char b;
		unsigned int m = (last<<2) & NS_MASK;

		/* Get next set loaded */
		_mm_prefetch((const char *)&model_seq8[m+0], _MM_HINT_T0);
		//_mm_prefetch((const char *)&model_seq8[m+3], _MM_HINT_T0);

		b = model_seq8[last].decodeSymbol(rc);

		*seq++ = dec[b];
		last = (last*4 + b) & NS_MASK;
	    }
	}
    }
}

void fqz::decode_seq16(RangeCoder *rc, char *seq, int len) {
    int last, last2;
    const char *dec = SOLiD ? "0123." : "ACGTN";

    const int NS_MASK = ((1<<(2*NS))-1);

    last  = 0x7616c7 & NS_MASK;
    last2 = (0x2c6b62ff >> (32 - 2*NS)) & NS_MASK;
    for (int i = 0; i < len; i++) {
	unsigned char b = model_seq16[last].decodeSymbol(rc);
	*seq++ = dec[b];
	last = (last*4 + b) & NS_MASK;
	_mm_prefetch((const char *)&model_seq16[last], _MM_HINT_T0);

	if (both_strands) {
	    int b2 = last2 & 3;
	    last2 = last2/4 + ((3-b) << (2*NS-2));
	    _mm_prefetch((const char *)&model_seq16[last2], _MM_HINT_T0);
	    model_seq16[last2].updateSymbol(b2);
	}
    }
}


/* -------------------------------------------------------------------------
 * Quality model
 */

#if 0
void fqz::encode_qual(RangeCoder *rc, char *seq, char *qual, int len) {
    unsigned int last = 0;
    int delta = 5;
    int i, len2 = len;
    int q1 = 0, q2 = 0;
    unsigned int X[1024];

    /* Removing "Killer Bees" */
    while (len2 > 0 && qual[len2-1] == '#')
	len2--;

    /* Prefetching & context caching. Only minor speed improvement. */
    for (i = 0; i < len2; i++) {
	unsigned char q = (qual[i] - '!') & (QMAX-1);

	X[i] = last;
	_mm_prefetch((const char *)&model_qual[last], _MM_HINT_T0);
	_mm_prefetch(64+(const char *)&model_qual[last], _MM_HINT_T0);

	// previous 2-3 bytes
	if (QBITS == 12) {
	    last = ((MAX(q1, q2)<<6) + q) & ((1<<QBITS)-1);
	} else {
	    last = ((last << 6) + q) & ((1<<QBITS)-1);
	}

	if (qlevel > 1) {
	    last  += (q1==q2) << QBITS;
	    // delta saves 3-4%, but adds 14% cpu
	    delta += (q1>q)*(q1-q);
	    last  += (MIN(7*8, delta)&0xf8) << (QBITS-2);
	}

	if (qlevel > 2)
	    last += (MIN(i+15,127)&(15<<3))<<(QBITS+1);     // i>>3

	q2 = q1; q1 = q;
    }
    X[i] = last;

    /* The actual encoding */
    for (i = 0; i < len2; i++) {
	unsigned char q = (qual[i] - '!') & (QMAX-1);
	model_qual[X[i]].encodeSymbol(rc, q);
    }

    if (len != len2) {
	model_qual[X[i]].encodeSymbol(rc, QMAX-1); /* terminator */
    }
}
#else
void fqz::encode_qual(RangeCoder *rc, char *seq, char *qual, int len) {
    unsigned int last = 0;
    int delta = 5;
    int i, len2 = len;
    int q1 = 0, q2 = 0;

    /* Removing "Killer Bees" */
    while (len2 > 0 && qual[len2-1] == '#')
	len2--;

    for (i = 0; i < len2; i++) {
	unsigned char q = (qual[i] - '!') & (QMAX-1);

#ifdef MULTI_QUAL_MODEL
	if (model_qual[last].bias() > model_qual[last & SMALL_QMASK].bias()) {
	    model_qual[last].encodeSymbol(rc, q);
	} else {
	    model_qual[last & SMALL_QMASK].encodeSymbol(rc, q);
	    model_qual[last].updateSymbol(q);
	}
#else
	model_qual[last].encodeSymbol(rc, q);
#endif

	// previous 2-3 bytes
	if (QBITS == 12) {
	    last = ((MAX(q1, q2)<<6) + q) & ((1<<QBITS)-1);
	} else {
	    last = ((last << 6) + q) & ((1<<QBITS)-1);
	}

	if (qlevel > 1) {
	    last  += (q1==q2) << QBITS;
	    // delta saves 3-4%, but adds 14% cpu
	    delta += (q1>q)*(q1-q);
	    last  += (MIN(7*8, delta)&0xf8) << (QBITS-2);
	}

	if (qlevel > 2)
	    //last += (MIN(i+7,127)&(7<<4))<<(QBITS);   // i>>4
	    last += (MIN(i+15,127)&(15<<3))<<(QBITS+1);     // i>>3
	    //last += (MIN(i+31,127)&(31<<2))<<(QBITS+2); // i>>2

	_mm_prefetch((const char *)&model_qual[last], _MM_HINT_T0);
	q2 = q1; q1 = q;

	assert(last < (QSIZE*16));
    }

    if (len != len2)
	model_qual[last].encodeSymbol(rc, QMAX-1); /* terminator */
}
#endif

/*
 * This attempts to encode qualities within a fixed distance, provided it
 * does appear to genuingly improve compression.
 *
 * Raw qual = 7722157 2.00 bpb
 * +/- 1    = 4092457 1.06 bpb
 * +/- 2    = 2853429 .739 bpb
 * +/- 3    = 2043044
 * (after first 1000 reads)
 * +/- 1    = 4758744 1.23 bpb
 * +/- 2    = 3354701 .869 bpb
 * +/- 3    = 2534989
 */

void fqz::encode_qual_lossy(RangeCoder *rc, char *qual, int len, int approx) {
    unsigned int last = 0;
    int delta = 5;
    int i, len2 = len;
    int q1 = 0, q2 = 0;

    //static int counter = 0;
    //counter++;

#if 1
    while (len2 > 0 && qual[len2-1] == '#')
	len2--;
#endif

    for (i = 0; i < len2; i++) {
	unsigned char q = (qual[i] - '!') & (QMAX-1);

#if 0	
	/* Simulation of illumina binning */
	if (q < 1)
	    model_qual[last].encodeSymbol(&rc, q);
	else if (q <= 9)
	    q = model_qual[last].encodeNearSymbol(&rc, 5, 4);
	else if (q <= 19)
	    q = model_qual[last].encodeNearSymbol(&rc, 15, 4);
	else if (q <= 24)
	    q = model_qual[last].encodeNearSymbol(&rc, 22, 2);
	else if (q <= 29)
	    q = model_qual[last].encodeNearSymbol(&rc, 27, 2);
	else if (q <= 34)
	    q = model_qual[last].encodeNearSymbol(&rc, 32, 2);
	else if (q <= 39)
	    q = model_qual[last].encodeNearSymbol(&rc, 37, 2);
	else if (q <= 59)
	    q = model_qual[last].encodeNearSymbol(&rc, 41, 1);
	else 
	    model_qual[last].encodeSymbol(&rc, q);

#else
	if (/* counter > 1000 && */ q > 1+approx && q < QMAX-1-approx) 
	    q = model_qual[last].encodeNearSymbol(rc, q, approx);
	else
	    model_qual[last].encodeSymbol(rc, q);
#endif
	    
	if (QBITS == 12) {
	    last = ((MAX(q1, q2)<<6) + q) & ((1<<QBITS)-1);
	} else {
	    last = ((last << 6) + q) & ((1<<QBITS)-1);
	}

	if (qlevel > 1) {
	    last  += (q1==q2) << QBITS;
	    delta += (q1>q)*(q1-q);
	    last  += (MIN(7*8, delta)&0xf8) << (QBITS-2);
	}

	if (qlevel > 2)
	    //last += (MIN(i+7,127)&(7<<4))<<(QBITS);   // i>>4
	    last += (MIN(i+15,127)&(15<<3))<<(QBITS+1);     // i>>3
	    //last += (MIN(i+31,127)&(31<<2))<<(QBITS+2); // i>>2

	_mm_prefetch((const char *)&model_qual[last], _MM_HINT_T0);
	q2 = q1; q1 = q;
    }

    if (len != len2)
	model_qual[last].encodeSymbol(rc, QMAX-1); /* terminator */
}

void fqz::decode_qual(RangeCoder *rc, char *qual, int len) {
    unsigned int last = 0;
    int i;
    int delta = 5;
    int q1 = 0, q2 = 0;

    for (i = 0; i < len; i++) {
	unsigned char q = model_qual[last].decodeSymbol(rc);

	if (q == QMAX-1) {
	    while (i < len)
		qual[i++] = '#';
	} else {
	    qual[i] = q + '!';
	
	    if (QBITS == 12) {
		last = ((MAX(q1, q2)<<6) + q) & ((1<<QBITS)-1);
	    } else {
		last = ((last << 6) + q) & ((1<<QBITS)-1);
	    }

	    if (qlevel > 1) {
		last  += (q1==q2) << QBITS;
		delta += (q1>q)*(q1-q);
		last  += (MIN(7*8, delta)&0xf8) << (QBITS-2);
	    }

	    if (qlevel > 2)
		//last += (MIN(i+7,127)&(7<<4))<<(QBITS);   // i>>4
		last += (MIN(i+15,127)&(15<<3))<<(QBITS+1);     // i>>3
		//last += (MIN(i+31,127)&(31<<2))<<(QBITS+2); // i>>2

	    _mm_prefetch((const char *)&model_qual[last], _MM_HINT_T0);
	    q2 = q1; q1 = q;
	}
    }
}


/* --------------------------------------------------------------------------
 * Compression functions.
 */


#ifdef TIMING
static long c1 = 0, c2 = 0, c3 = 0;
#endif

/* pthread enty points */
static void *fq_compress_r1(void *v) {
    //fprintf(stderr, "r1 start on %d\n", sched_getcpu());
    ((fqz *)v)->compress_r1();
    //fprintf(stderr, "r1 end\n");
    return NULL;
}

static void *fq_compress_r2(void *v) {
    //fprintf(stderr, "r2 start on %d\n", sched_getcpu());
    ((fqz *)v)->compress_r2();
    //fprintf(stderr, "r2 end\n");
    return NULL;
}

static void *fq_compress_r3(void *v) {
    //fprintf(stderr, "r3 start on %d\n", sched_getcpu());
    ((fqz *)v)->compress_r3();
    //fprintf(stderr, "r3 end\n");
    return NULL;
}

/* Sequence length & name */
void fqz::compress_r1() {
    char *name_p = name_buf;
    RangeCoder rc;

#ifdef TIMING
    clock_t c = clock();
#endif

    rc.output(out1);
    rc.StartEncode();
    if (nlevel == 1) {
	for (int i = 0; i < ns; i++) {
	    encode_name(&rc, name_p, name_len_a[i]);
	    name_p += name_len_a[i];
	}
    } else {
	for (int i = 0; i < ns; i++) {
	    encode_name2(&rc, name_p, name_len_a[i]);
	    name_p += name_len_a[i];
	}
    }
    rc.FinishEncode();

    sz1 = rc.size_out();
    name_in  += name_p - name_buf;
    name_out += sz1;

#ifdef TIMING
    c1 += clock() - c;
#endif
}

/* Sequence itself */
void fqz::compress_r2() {
    char *seq_p  = seq_buf;
    RangeCoder rc;

#ifdef TIMING
    clock_t c = clock();
#endif

    rc.output(out2);
    rc.StartEncode();
    for (int i = 0; i < ns; i++) {
	if (extreme_seq)
	    encode_seq16(&rc, seq_p, seq_len_a[i]);
	else
	    encode_seq8(&rc, seq_p, seq_len_a[i]);
	seq_p  += seq_len_a[i];
    }
    rc.FinishEncode();

    sz2 = rc.size_out();
    base_in  += seq_p - seq_buf;
    base_out += sz2;

#ifdef TIMING
    c2 += clock() - c;
#endif
}

/* Quality values */
void fqz::compress_r3() {
    char *qual_p = qual_buf;
    char *seq_p = seq_buf;
    RangeCoder rc;

#ifdef TIMING
    clock_t c = clock();
#endif

    rc.output(out3);
    rc.StartEncode();
    for (int i = 0; i < ns; i++) {
	if (qual_approx) {
	    encode_qual_lossy(&rc, qual_p, seq_len_a[i], qual_approx);
	} else {
	    encode_qual(&rc, seq_p, qual_p, seq_len_a[i]);
	}
	qual_p += seq_len_a[i];
	seq_p  += seq_len_a[i];
    }
    rc.FinishEncode();

    sz3 = rc.size_out();
    qual_in  += qual_p - qual_buf;
    qual_out += sz3;

#ifdef TIMING
    c3 += clock() - c;
#endif
}

/*
 * Reads from in[0..in_len-1] and writes a compressed copy to out, setting
 * *out_len to the returned size. The caller needs to ensure out is large
 * enough.
 *
 * We can only compress entire sequences and in[] may end in a partial
 * sequence. Hence *in_end is set to a pointer to the end of the processed
 * buffer.
 *
 * Also returns *nseqs filled out.
 *
 * Returns total compressed length on success, with data in out[]
 *        -1 on failure
 */
int fqz::fq_compress(char *in,  int in_len,
		     char *out, int *out_len,
		     char **in_end, int *nseqs) {
    int end = 0, end_hash = 0;
    int i, j, k;
    //static char not_nl[256];
    static int not_nl[256];

    char *name_p = name_buf;
    char *seq_p  = seq_buf;
    char *qual_p = qual_buf;

    ns = 0;

    for (i = 0; i < 256; i++)
	not_nl[i] = 1;
    not_nl['\r'] = not_nl['\n'] = 0;

    /* Parse and separate into name, seq, qual buffers */
    seq_len = 0;

    for (i = k = 0; i < in_len; ) {
	char *name, *seq, *qual;

	/* Name */
	if (in[i] != '@')
	    return -1;

	name = &in[i+1];
	j = i;
	in[k++] = in[i];
	i++;
	//while (i < in_len && in[i] != '\n' && in[i] != '\r')
	while (i < in_len && not_nl[(uc)in[i]])
	    in[k++] = *name_p++ = in[i++];
	name_len_a[ns] = i-j-1;

	if (in[i] == '\r') i++;
	in[k++] = in[i];
	if (++i >= in_len)
	    break;

	/* Sequence */
	if (SOLiD) i++;
	seq = seq_p;
	//for (j = i; i < in_len && in[i] != '\n' && in[i] != '\r'; i++)
	for (j = i; i < in_len && not_nl[(uc)in[i]]; i++)
	    in[k++] = *seq_p++ = in[i];
	seq_len_a[ns] = i-j;
	
	if (in[i] == '\r') i++;
	in[k++] = in[i];
	if (++i >= in_len)
	    break;

	/* +name, assume to be identical to @name */
	if (in[i] != '+')
	    return -1;
	in[k++] = in[i];

	//for (; i < in_len && in[i] != '\n' && in[i] != '\r'; i++)
	for (; i < in_len && not_nl[(uc)in[i]]; i++)
	    ;
	if (in[i] == '\r') i++;
	in[k++] = in[i];
	if (++i >= in_len)
	    break;

	/* Quality */
	qual = &in[i];
	if (SOLiD) {
	    int old_i = i;
	    /* Check qual and seq len matches. SOLiD format varies. */
	    //for (j = i; i < in_len && in[i] != '\n' && in[i] != '\r'; i++)
	    for (j = i; i < in_len && not_nl[(uc)in[i]]; i++)
		;
	    if (i-j == seq_len_a[ns]+1) {
		primer_qual = 1;
		old_i++;
	    } else if (i-j == seq_len_a[ns]) {
		primer_qual = 0;
	    } else {
		if (i >= in_len) {
		    i++;
		    break;
		}

		fprintf(stderr, "Seq %d: unexpected length of quality "
			"string\n", ns);
		return -1;
	    }
	    //for (j = 0, i = old_i; i < in_len && in[i] != '\n' && in[i] != '\r'; i++, j++) {
	    for (j = 0, i = old_i; i < in_len && not_nl[(uc)in[i]]; i++, j++) {
		if (seq[j] == '.') in[i] = '!'; // Ensure N is qual 0
		if (in[i] == '!' && seq[j] != '.') in[i] = '"';
		*qual_p++ = in[i];
	    }
	} else {
	    static int is_N[256]={
	        1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1, /*  0 */
		1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1, /* 10 */
		1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1, /* 20 */
		1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1, /* 30 */
		1, 0, 1, 0,  1, 1, 1, 0,  1, 1, 1, 1,  1, 1, 1, 1, /* 40 */
		1, 1, 1, 1,  0, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1, /* 50 */
		1, 0, 1, 0,  1, 1, 1, 0,  1, 1, 1, 1,  1, 1, 1, 1, /* 60 */
		1, 1, 1, 1,  0, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1, /* 70 */
		1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1, /* 80 */
		1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1, /* 90 */
		1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1, /* A0 */
		1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1, /* B0 */
		1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1, /* C10 */
		1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1, /* D0 */
		1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1, /* E0 */
		1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1, /* F0 */
	    };
	    //for (j = 0; i < in_len && in[i] != '\n' && in[i] != '\r'; i++, j++) {
	    for (j = 0; i < in_len && not_nl[(uc)in[i]]; i++, j++) {
	        if (is_N[(unsigned char)seq[j]]) in[i] = '!'; // Ensure N is qual 0
		if (in[i] == '!' && !is_N[(unsigned char)seq[j]]) in[i] = '"';
		in[k++] = *qual_p++ = in[i];
	    }
	}

	if (in[i] == '\r') i++;
	in[k++] = in[i];
	if (++i > in_len)
	    break;

	end = i; end_hash = k;

	if (seq_len == 0)
	    seq_len = seq_len_a[ns];
	else if (seq_len != seq_len_a[ns])
	    seq_len = -1;

	ns++;
    }

    *in_end = in+end;
    *nseqs = ns;

    /* Note: must be after seq==N qual editing code above */
    //fprintf(stderr, "%d %d\n", end, end_hash);
    //fprintf(stderr, "%.*s\n", end_hash, in);
    chksum = (do_hash && !qual_approx) ? sfhash((uc *)in, end_hash) : 0;

    /* Encode seq len, we have a dependency on this for seq/qual */
    //fprintf(stderr, "-----\n");
    RangeCoder rc;
    rc.output(out0);
    rc.StartEncode();
    for (int i = 0; i < ns; i++) {
	//fprintf(stderr, "Encode %d: %d\n", i, seq_len_a[i]);
	encode_len(&rc, seq_len_a[i]);
    }
    rc.FinishEncode();
    sz0 = rc.size_out();

#if 0
    /* Encode the 3 buffers in parallel */
#ifdef PTHREADS
    if (do_threads) {
	pthread_t t1, t2, t3;
	pthread_create(&t1, NULL, fq_compress_r1, (void*)this);
	pthread_create(&t2, NULL, fq_compress_r2, (void*)this); 
	pthread_create(&t3, NULL, fq_compress_r3, (void*)this);

	pthread_join(t1, NULL);
	pthread_join(t2, NULL);
	pthread_join(t3, NULL);
    } else {
	compress_r1();
	compress_r2();
	compress_r3();
    }
#else
    compress_r1();
    compress_r2();
    compress_r3();
#endif
#endif

    //fprintf(stderr, "hashes %08x %08x %08x\n", name_hash, seq_hash, qual_hash);
    
    /* Concatenate compressed output into a single block */
    char *out_p = out;
    *out_p++ = (chksum >>  0) & 0xff;
    *out_p++ = (chksum >>  8) & 0xff;
    *out_p++ = (chksum >> 16) & 0xff;
    *out_p++ = (chksum >> 24) & 0xff;

    *out_p++ = (end >>  0) & 0xff;  /* Uncompressed size */
    *out_p++ = (end >>  8) & 0xff;
    *out_p++ = (end >> 16) & 0xff;
    *out_p++ = (end >> 24) & 0xff;

    *out_p++ = (ns  >>  0) & 0xff;  /* Number of sequences */
    *out_p++ = (ns  >>  8) & 0xff;
    *out_p++ = (ns  >> 16) & 0xff;
    *out_p++ = (ns  >> 24) & 0xff;

    *out_p++ = (sz0 >>  0) & 0xff;  /* Size of 4 range-coder blocks */
    *out_p++ = (sz0 >>  8) & 0xff;
    *out_p++ = (sz0 >> 16) & 0xff;
    *out_p++ = (sz0 >> 24) & 0xff;

    *out_p++ = (sz1 >>  0) & 0xff;
    *out_p++ = (sz1 >>  8) & 0xff;
    *out_p++ = (sz1 >> 16) & 0xff;
    *out_p++ = (sz1 >> 24) & 0xff;

    *out_p++ = (sz2 >>  0) & 0xff;
    *out_p++ = (sz2 >>  8) & 0xff;
    *out_p++ = (sz2 >> 16) & 0xff;
    *out_p++ = (sz2 >> 24) & 0xff;

    *out_p++ = (sz3 >>  0) & 0xff;
    *out_p++ = (sz3 >>  8) & 0xff;
    *out_p++ = (sz3 >> 16) & 0xff;
    *out_p++ = (sz3 >> 24) & 0xff;

    memcpy(out_p, out0, sz0); out_p += sz0;
    memcpy(out_p, out1, sz1); out_p += sz1;
    memcpy(out_p, out2, sz2); out_p += sz2;
    memcpy(out_p, out3, sz3); out_p += sz3;

    *out_len = out_p - out;

    return *out_len;
}

/*
 * A blocking read that refuses to return truncated reads.
 */
static size_t xread(int fd, char *buf, size_t count) {
    ssize_t len, tlen;

    tlen = 0;
    do {
	len = read(fd, buf, count);
	if (len == -1) {
	    if (errno == EINTR)
		continue;
	    return -1;
	}

	if (len == 0)
	    return tlen;

	buf   += len;
	count -= len;
	tlen  += len;
    } while (count);

    return tlen;
}


/*
 * Encode an entire stream
 *
 * Returns 0 on success
 *        -1 on failure
 */
int fqz::encode(int in_fd, int out_fd) {
    int sz, blk_start = 0;
    size_t total_sz = 0;
    int first_block = 1;

    /*
     * Parse one block at a time. Blocks may not terminate on exact fastq
     * boundaries, so we need to know where we ended processing and move
     * that partial fastq entry back to the start for the next block.
     *
     * We write out the block size too so we can decompress block at a time.
     * This may also permits a level of parallellism in the future.
     */
    while ((sz = xread(in_fd, &in_buf[blk_start], BLK_SIZE - blk_start)) > 0) {
	char *comp, *in_end = NULL;
	int comp_len = 0, nseqs = 0;
	
	if (-1 == fq_compress(in_buf, sz + blk_start,
			      out_buf+4, &comp_len,
			      &in_end, &nseqs)) {
	    fprintf(stderr, "Failure to parse and/or compress.\n");
	    return -1;
	}
	comp = out_buf;

	if (comp == NULL) {
	    fprintf(stderr, "Abort: encode_data returned NULL\n");
	    return -1;
	}

	if (first_block) {
	    first_block = 0;
	    if (SOLiD) {
		/* Find primer base */
		int i = 0, qlen, slen;
		while (i < sz && in_buf[i] != '\n') i++;
		if (1 != write(out_fd, in_buf+i+1, 1)) {
		    fprintf(stderr, "Abort: write failed\n");
		    return -1;
		}

		/* Check if quality string has dummy qual for this too */
		slen = ++i;
		while (i < sz && in_buf[i] != '\n' && in_buf[i] != '\r') i++; // seq;
		slen = i++-slen;
		while (i < sz && in_buf[i] != '\n' && in_buf[i] != '\r') i++; // "+" line
		qlen = ++i;
		while (i < sz && in_buf[i] != '\n' && in_buf[i] != '\r') i++; // qual;
		qlen = i++-qlen;

		char c = qlen == slen;
		if (1 != write(out_fd, &c, 1)) {
		    fprintf(stderr, "Abort: write failed\n");
		    return -1;
		}
	    }
	}

	out_buf[0] = (comp_len >>  0) & 0xff;
	out_buf[1] = (comp_len >>  8) & 0xff;
	out_buf[2] = (comp_len >> 16) & 0xff;
	out_buf[3] = (comp_len >> 24) & 0xff;
	comp_len += 4;

	if (comp_len != write(out_fd, out_buf, comp_len)) {
	    fprintf(stderr, "Abort: truncated write.\n");
	    return -1;
	}

	total_sz += comp_len;

	/* We maybe ended on a partial fastq entry, so start from there */
	memmove(in_buf, in_end, (sz + blk_start) - (in_end - in_buf));
	blk_start = (sz + blk_start) - (in_end - in_buf);
    }

    return 0;
}


/* --------------------------------------------------------------------------
 * Decompression functions.
 */
#define DECODE_INT(a) ((a)[0] + ((a)[1]<<8) + ((a)[2]<<16) + ((a)[3]<<24))

/* pthread enty points */
static void *fq_decompress_r1(void *v) {
    //fprintf(stderr, "r1 start on %d\n", sched_getcpu());
    ((fqz *)v)->decompress_r1();
    //fprintf(stderr, "r1 end\n");
    return NULL;
}

static void *fq_decompress_r2(void *v) {
    //fprintf(stderr, "r2 start on %d\n", sched_getcpu());
    ((fqz *)v)->decompress_r2();
    //fprintf(stderr, "r2 end\n");
    return NULL;
}

static void *fq_decompress_r3(void *v) {
    //fprintf(stderr, "r3 start on %d\n", sched_getcpu());
    ((fqz *)v)->decompress_r3();
    //fprintf(stderr, "r3 end\n");
    return NULL;
}

void fqz::decompress_r1(void) {
    RangeCoder rc;
    rc.input(in_buf1);
    rc.StartDecode();

    char *name_p = name_buf;
    if (nlevel == 1) {
	for (int i = 0; i < ns; i++) {
	    *name_p++ = '@';
	    name_p += decode_name(&rc, name_p);
	    *name_p++ = '\n';
	}
    } else {
	for (int i = 0; i < ns; i++) {
	    *name_p++ = '@';
	    name_p += decode_name2(&rc, name_p);
	    *name_p++ = '\n';
	}
    }
    rc.FinishDecode();
}

void fqz::decompress_r2(void) {
    RangeCoder rc;
    rc.input(in_buf2);
    rc.StartDecode();

    char *seq_p = seq_buf;
    for (int i = 0; i < ns; i++) {
	if (extreme_seq)
	    decode_seq16(&rc, seq_p, seq_len_a[i]);
	else
	    decode_seq8(&rc, seq_p, seq_len_a[i]);
	seq_p += seq_len_a[i];
    }
    rc.FinishDecode();
}

void fqz::decompress_r3(void) {
    RangeCoder rc;
    rc.input(in_buf3);
    rc.StartDecode();

    char *qual_p = qual_buf;
    for (int i = 0; i < ns; i++) {
	decode_qual(&rc, qual_p, seq_len_a[i]);
	qual_p += seq_len_a[i];
    }
    rc.FinishDecode();
}

/* Decompress a single block */
char *fqz::fq_decompress(char *in, int comp_len, int *out_len) {
    char *name_p, *seq_p, *qual_p;

    uint32_t chk    = DECODE_INT((unsigned char *)(in));
    //uint32_t ulen   = DECODE_INT((unsigned char *)in+4);
    uint32_t nseqs  = DECODE_INT((unsigned char *)(in+8));
    uint32_t sz0    = DECODE_INT((unsigned char *)(in+12));
    uint32_t sz1    = DECODE_INT((unsigned char *)(in+16));
    uint32_t sz2    = DECODE_INT((unsigned char *)(in+20));
    uint32_t sz3    = DECODE_INT((unsigned char *)(in+24));

    in += 28;
    ns = nseqs;

    /* Use ulen for allocating decoding buffers */

//    fprintf(stderr, "%d -> %d\n", comp_len, ulen);
//    fprintf(stderr, "   ns=%d, sz={%d, %d, %d, %d}\n",
//	    nseqs, sz0, sz1, sz2, sz3);

    in_buf0 = in; in += sz0;
    in_buf1 = in; in += sz1;
    in_buf2 = in; in += sz2;
    in_buf3 = in; in += sz3;

    RangeCoder rc0;
    rc0.input(in_buf0);
    rc0.StartDecode();

    for (int i = 0; i < ns; i++)
	seq_len_a[i] = decode_len(&rc0);
    rc0.FinishDecode();

#ifdef PTHREADS
    if (do_threads && qlevel <= 3) {
	/* -q4 adds dependency between seq[] and qual[] */
	pthread_t t1, t2, t3;
	pthread_create(&t1, NULL, fq_decompress_r1, (void*)this);
	pthread_create(&t2, NULL, fq_decompress_r2, (void*)this); 
	pthread_create(&t3, NULL, fq_decompress_r3, (void*)this);

	pthread_join(t1, NULL);
	pthread_join(t2, NULL);
	pthread_join(t3, NULL);
    } else {
	decompress_r1();
	decompress_r2();
	decompress_r3();
    }
#else
    decompress_r1();
    decompress_r2();
    decompress_r3();
#endif

    //fprintf(stderr, "hashes %08x %08x %08x\n", name_hash, seq_hash, qual_hash);

    /* Stick together the arrays into out_buf */
    out_ind = 0;
    name_p = name_buf;
    seq_p = seq_buf;
    qual_p = qual_buf;
    if (SOLiD) {
	for (int i = 0; i < ns; i++) {
	    /* name */
	    while ((out_buf[out_ind++] = *name_p++) != '\n')
		;

	    /* seq */
	    out_buf[out_ind++] = solid_primer;
	    for (int j = 0; j < seq_len_a[i]; j++)
		out_buf[out_ind++] = *seq_p++;
	    out_buf[out_ind++] = '\n';
	    out_buf[out_ind++] = '+';
	    out_buf[out_ind++] = '\n';

	    /* qual */
	    if (primer_qual)
		out_buf[out_ind++] = '!';
	    for (int j = 0; j < seq_len_a[i]; j++) {
		if ((out_buf[out_ind++] = *qual_p++) == '!') {
		    out_buf[out_ind-4-seq_len_a[i] - primer_qual] = '.';
		}
	    }
	    out_buf[out_ind++] = '\n';
	}
    } else {
	for (int i = 0; i < ns; i++) {
	    /* name */
	    while ((out_buf[out_ind++] = *name_p++) != '\n')
		;

	    /* seq */
	    for (int j = 0; j < seq_len_a[i]; j++)
		out_buf[out_ind++] = *seq_p++;
	    out_buf[out_ind++] = '\n';
	    out_buf[out_ind++] = '+';
	    out_buf[out_ind++] = '\n';

	    /* qual */
	    for (int j = 0; j < seq_len_a[i]; j++) {
		if ((out_buf[out_ind++] = *qual_p++) == '!') {
		    out_buf[out_ind-4-seq_len_a[i]] = 'N';
		}
	    }
	    out_buf[out_ind++] = '\n';
	}
    }

    chksum = do_hash ? sfhash((uc *)out_buf, out_ind) : 0;
    if (do_hash && chk && chksum != chk) {
	fprintf(stderr, "Mismatching checksums. Aborting. Rerun with -X to ignore this error.\n");
	return NULL;
    }

    *out_len = out_ind;
    return out_buf;
}

/*
 * Decode an entire stream
 *
 * Returns 0 on success
 *        -1 on failure
 */
int fqz::decode(int in_fd, int out_fd) {
    unsigned char len_buf[4];

    if (SOLiD) {
	if (1 != read(in_fd, &solid_primer, 1))
	    return -1;
	if (1 != read(in_fd, &primer_qual, 1))
	    return -1;
    }
    /*
     * Parse one block at a time. Blocks may not terminate on exact fastq
     * boundaries, so we need to know where we ended processing and move
     * that partial fastq entry back to the start for the next block.
     *
     * We write out the block size too so we can decompress block at a time.
     * This may also permits a level of parallellism in the future.
     */
    while (4 == read(in_fd, len_buf, 4)) {
	int32_t comp_len =
	    (len_buf[0] <<  0) + 
	    (len_buf[1] <<  8) +
	    (len_buf[2] << 16) +
	    (len_buf[3] << 24);
	char *uncomp_buf;
	int   uncomp_len, rem_len = comp_len, in_off = 0;

	//fprintf(stderr, "Block of length %d\n", comp_len);

	do {
	    errno = 0;
	    int tmp_len = read(in_fd, in_buf+in_off, rem_len);
	    if (errno == EINTR && tmp_len == -1)
		continue;

	    if (tmp_len == -1) {
		fprintf(stderr, "Abort: read failed, %d.\n", errno);
		perror("foo");
		return -1;
	    }
	    if (tmp_len == 0) {
		fprintf(stderr, "Abort: truncated read, %d.\n", errno);
		return -1;
	    }
	    rem_len -= tmp_len;
	    in_off  += tmp_len;
	} while (rem_len);

	uncomp_buf = fq_decompress(in_buf, comp_len, &uncomp_len);

	if (uncomp_buf) {
	    if (uncomp_len != write(out_fd, uncomp_buf, uncomp_len)) {
		fprintf(stderr, "Abort: truncated write.\n");
		return -1;
	    }

	} else {
	    fprintf(stderr, "Failed to decompress block\n");
	    return -1;
	}
    }

    return 0;
}

/* --------------------------------------------------------------------------
 * Main program entry.
 */
static void usage(int err) {
    FILE *fp = err ? stderr : stdout;

    fprintf(fp, "fqz_comp v%d.%d. Author James Bonfield, 2012\n",
	    MAJOR_VERS, MINOR_VERS);
    fprintf(fp, "The range coder is derived from Eugene Shelwien.\n\n");

    fprintf(fp, "To compress:\n  fqz_comp [options] [input_file [output_file]]\n\n");
    fprintf(fp, "    -Q <num>       Perform lossy compression with all quality values\n");
    fprintf(fp, "                   being within 'num' distance from their original value.\n");
    fprintf(fp, "                   Default is lossless, i.e. \"-q 0\"\n\n");
    fprintf(fp, "    -s <level>     Sequence compression level. 1-9 [Def. 3]\n");
    fprintf(fp, "                   Specifying '+' on the end (eg -s5+) will use\n");
    fprintf(fp, "                   models of multiple sizes for improved compression.\n\n");
    fprintf(fp, "    -b             Use both strands in sequence hash table.\n\n");
    fprintf(fp, "    -e             Extra seq compression: 16-bit vs 8-bit counters.\n\n");
    fprintf(fp, "    -q <level>     Quality compression level.  1-3 [Def. 2]\n\n");
    fprintf(fp, "    -n <level>     Name compression level.  1-2 [Def. 2]\n\n");
    fprintf(fp, "    -P             Disable multi-threading\n\n");

    fprintf(fp, "    -X             Disable generation/verification of check sums\n\n");
    fprintf(fp, "    -S             SOLiD format\n\n");

    fprintf(fp, "To decompress:\n   fqz_comp -d < foo.fqz > foo.fastq\n");
    fprintf(fp, "or fqz_comp -d foo.fqz foo.fastq\n");

    exit(err);
}

int main(int argc, char **argv) {
    fqz *f;
    int decompress = 0;
    int opt;
    int in_fd = 0;
    int out_fd = 1;
    fqz_params p;

    /* Initialise and parse command line arguments */
    p.slevel = 3;
    p.qlevel = 2;
    p.nlevel = 2;
    p.both_strands = 0;
    p.extreme_seq = 0;
    p.multi_seq_model = 0;
    p.qual_approx = 0;
    p.do_threads = 1;
    p.do_hash = 1;
    p.SOLiD = 0;

    while ((opt = getopt(argc, argv, "hdQ:s:q:n:bePXS")) != -1) {
	switch (opt) {
	case 'h':
	    usage(0);

	case 'd':
	    decompress = 1;
	    break;

	case 'Q':
	    p.qual_approx = atoi(optarg);
	    break;

	case 'q':
	    p.qlevel = atoi(optarg);
	    if (p.qlevel < 1 || p.qlevel > 3)
		usage(1);
	    break;

	case 's': {
	    char *end;

	    p.slevel = strtol(optarg, &end, 10);
	    if (p.slevel < 1 || p.slevel > 9)
		usage(1);

	    if (*end == '+')
		p.multi_seq_model = 1;

	    break;
	}

	case 'n':
	    p.nlevel = atoi(optarg);
	    if (p.nlevel < 1 || p.nlevel > 2)
		usage(1);
	    break;

	case 'b':
	    p.both_strands = 1;
	    break;

	case 'e':
	    p.extreme_seq = 1;
	    break;

	case 'P':
	    p.do_threads = 0;
	    break;
	    
	case 'X':
	    p.do_hash = 0;
	    break;

	case 'S':
	    p.SOLiD = 1;
	    break;

	default:
	    usage(1);
	}
    }

    if (argc - optind > 2)
	usage(1);

    if (optind != argc) {
	if ((in_fd = open(argv[optind], O_RDONLY)) == -1) {
	    perror(argv[optind]);
	    exit(1);
	}
	optind++;
    }

    if (optind != argc) {
	out_fd = open(argv[optind], O_RDWR | O_CREAT | O_TRUNC, 0666);
	if (out_fd == -1) {
	    perror(argv[optind]);
	    exit(1);
	}
	optind++;
    }

    if (decompress) {
	unsigned char magic[8];

	/* Check magic number */
	if (8 != read(in_fd, magic, 8)) {
	    fprintf(stderr, "Abort: truncated read.\n");
	    return 1;
	}
	if (memcmp(".fqz", magic, 4) != 0) {
	    fprintf(stderr, "Unrecognised file format.\n");
	    return 1;
	}
	if (magic[4] != MAJOR_VERS || magic[5] != FORMAT_VERS) {
	    fprintf(stderr, "Unsupported file format version %d.%d\n",
		    magic[4], magic[5]);
	    return 1;
	}
	
	p.slevel = magic[6] & 0x0f;
	p.qlevel = ((magic[6] >> 4) & 3);
	p.nlevel = (magic[6] >> 6);
	p.both_strands    = magic[7] & 1;
	p.extreme_seq     = magic[7] & 2;
	p.multi_seq_model = magic[7] & 4;
	p.SOLiD           = magic[7] & 8;
	if (p.slevel > 9 || p.slevel < 1) {
	    fprintf(stderr, "Unexpected quality compression level %d\n",
		    p.slevel);
	    return 1;
	}
	if (p.qlevel > 3 || p.qlevel < 1) {
	    fprintf(stderr, "Unexpected sequence compression level %d\n",
		    p.qlevel);
	    return 1;
	}
	if (p.nlevel > 2 || p.nlevel < 1) {
	    fprintf(stderr, "Unexpected sequence compression level %d\n",
		    p.qlevel);
	    return 1;
	}

	f = new fqz(&p);
	return f->decode(in_fd, out_fd) ? 1 : 0;

    } else {
	int level = p.slevel | (p.qlevel << 4) | (p.nlevel << 6);
	int flags = p.both_strands
	    + p.extreme_seq*2
	    + p.multi_seq_model*4
	    + p.SOLiD*8;
	int r;
	unsigned char magic[8] = {'.', 'f', 'q', 'z',
				  MAJOR_VERS,
				  FORMAT_VERS,
				  level,
				  flags,
	};

	if (8 != write(out_fd, magic, 8)) {
	    fprintf(stderr, "Abort: truncated write.\n");
	    return 1;
	}

	f = new fqz(&p);
	r = f->encode(in_fd, out_fd);

#ifdef TIMING
	fprintf(stderr, "Names %10"PRId64" -> %10"PRId64" (%0.3f) in %.2fs\n",
		f->name_in, f->name_out, (double)f->name_out / f->name_in,
		(double)c1 / CLOCKS_PER_SEC);
	fprintf(stderr, "Bases %10"PRId64" -> %10"PRId64" (%0.3f) in %.2fs\n",
		f->base_in, f->base_out, (double)f->base_out / f->base_in,
		(double)c2 / CLOCKS_PER_SEC);
	fprintf(stderr, "Quals %10"PRId64" -> %10"PRId64" (%0.3f) in %.2fs\n",
		f->qual_in, f->qual_out, (double)f->qual_out / f->qual_in,
		(double)c3 / CLOCKS_PER_SEC);
#else
	fprintf(stderr, "Names %10"PRId64" -> %10"PRId64" (%0.3f)\n",
		f->name_in, f->name_out, (double)f->name_out / f->name_in);
	fprintf(stderr, "Bases %10"PRId64" -> %10"PRId64" (%0.3f)\n",
		f->base_in, f->base_out, (double)f->base_out / f->base_in);
	fprintf(stderr, "Quals %10"PRId64" -> %10"PRId64" (%0.3f)\n",
		f->qual_in, f->qual_out, (double)f->qual_out / f->qual_in);
#endif

	return r ? 1 : 0;
    }
}
