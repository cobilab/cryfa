/*
 * A fixed alphabet encoder for symbols 0 to 5 inclusive.
 * We have no escape symbol or resorting of data. It simply
 * accumulates stats and encodes in proportion.
 *
 * The intention is to use this per column for encoding
 * bases that differ to the consensus.
 */

/*
 * Fast mode uses 8-bit counters. It's generally 40% faster or so, but less
 * accurate and so larger file size (maybe only 1% though unless deep data).
 */

// Small enough to not overflow uint16_t even after +STEP
#ifdef WSIZ
#  undef WSIZ
#endif

#define M4(a) ((a)[0]>(a)[1]?((a)[0]>(a)[2]?((a)[0]>(a)[3]?(a)[0]:(a)[3]):((a)[2]>(a)[3]?(a)[2]:(a)[3])):((a)[1]>(a)[2]?((a)[1]>(a)[3]?(a)[1]:(a)[3]):((a)[2]>(a)[3]?(a)[2]:(a)[3])))

template <typename st_t>
struct BASE_MODEL {
    enum { STEP = sizeof(st_t) == 1 ? 1 : 8 };
    enum { WSIZ = (1<<8*sizeof(st_t))-2*STEP };
    
    BASE_MODEL();
    BASE_MODEL(int *start);
    void reset();
    void reset(int *start);
    inline void encodeSymbol(RangeCoder *rc, uint sym);
    inline void updateSymbol(uint sym);
    inline uint decodeSymbol(RangeCoder *rc);
    inline uint getTopSym(void);
    inline uint getSummFreq(void);

protected:
    void   rescaleRare();

    /*
     * Stats is 4 8-bit values consisting of cumulative frequencies:
     *
     * f[0], f[0]+f[1], f[0]+f[1]+f[2], f[0]+f[1]+f[2]+f[3]
     *
     * We can then do operations on all 4 frequencies at once.
     * Note "Stats >> 24" replaces the old SummFreq and
     * "Stats - (Stats<<8)" converts back to separated f[0], f[1], f[2]
     * and f[3] frequencies.
     */
    uint32_t Stats;
};

template <typename st_t>
BASE_MODEL<st_t>::BASE_MODEL()
{
    reset();
}

template <typename st_t>
BASE_MODEL<st_t>::BASE_MODEL(int *start) {
    Stats = start[0] |
	((start[0] + start[1])<<8) |
	((start[0] + start[1] + start[2])<<16) |
	((start[0] + start[1] + start[2] + start[3])<<24);
}

template <typename st_t>
void BASE_MODEL<st_t>::reset() {
    Stats = (3) | ((6)<<8) | ((9)<<16) | ((12)<<24);
}

template <typename st_t>
void BASE_MODEL<st_t>::reset(int *start) {
    Stats = start[0] |
	((start[0] + start[1])<<8) |
	((start[0] + start[1] + start[2])<<16) |
	((start[0] + start[1] + start[2] + start[3])<<24);
}

template <typename st_t>
void BASE_MODEL<st_t>::rescaleRare()
{
    Stats -= (Stats<<8);
    Stats -= (Stats & 0xfefefefe)>>1;
    Stats += (Stats<<8) + (Stats<<16) + (Stats<<24);

    // Or approximating via ?
    // Stats -= (Stats & 0xfefefefe)>>1;
    // Stats += 0x03020100;
}

template <typename st_t>
inline void BASE_MODEL<st_t>::encodeSymbol(RangeCoder *rc, uint sym) {
    if (Stats>>24 >= WSIZ)
	rescaleRare();

    int s8 = sym<<3;
    rc->Encode((Stats & 0xff0000) >> (24-s8),
	       ((Stats - (Stats<<8)) >> s8) & 0xff,
	       Stats >> 24);
    Stats += 0x01010101 << s8;
}

template <typename st_t>
inline void BASE_MODEL<st_t>::updateSymbol(uint sym) {
    if (Stats>>24 >= WSIZ)
	rescaleRare();
    Stats += 0x01010101 << (sym<<3);
}

/*
 * Returns the bias of the best symbol compared to all other symbols.
 * This is a measure of how well adapted this model thinks it is to the
 * incoming probabilities.
 */
template <typename st_t>
inline uint BASE_MODEL<st_t>::getTopSym(void) {
    /* How to do this quickly? MMX? */
    unsigned char c[4];
    c[0] = (Stats>>0)  & 0xff;
    c[1] = (Stats>>8)  & 0xff;
    c[2] = (Stats>>16) & 0xff;
    c[3] = (Stats>>24) & 0xff;

    return M4(c);
}

template <typename st_t>
inline uint BASE_MODEL<st_t>::getSummFreq(void) {
    return Stats>>24;
}


/* FIXME: not working */

template <typename st_t>
inline uint BASE_MODEL<st_t>::decodeSymbol(RangeCoder *rc) {
    if (Stats>>24 >= WSIZ)
	rescaleRare();

    uint count=rc->GetFreq(Stats>>24);

    /* Maybe binary search this? */

    if ((Stats&0xff) > count) {
	rc->Decode(0,
		   Stats&0xff,
		   Stats>>24);
	Stats += 0x01010101;
	return 0;
    }

    if (((Stats>>8)&0xff) > count) {
	rc->Decode(Stats&0xff,
		   ((Stats - (Stats<<8)) >> 8) & 0xff,
		   Stats>>24);
	Stats += 0x01010100;
	return 1;
    }

    if (((Stats>>16)&0xff) > count) {
	rc->Decode((Stats>>8)&0xff,
		   ((Stats - (Stats<<8)) >> 16) & 0xff,
		   Stats>>24);
	Stats += 0x01010000;
	return 2;
    }

    rc->Decode((Stats>>16)&0xff,
	       ((Stats - (Stats<<8)) >> 24) & 0xff,
	       Stats>>24);
    Stats += 0x01000000;
    return 3;
}
