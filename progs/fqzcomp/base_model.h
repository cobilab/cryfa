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

    st_t Stats[4];
};

template <typename st_t>
BASE_MODEL<st_t>::BASE_MODEL()
{
    reset();
}

template <typename st_t>
BASE_MODEL<st_t>::BASE_MODEL(int *start) {
    for (int i = 0; i < 4; i++) {
	Stats[i] =  start[i];
    }
}

template <typename st_t>
void BASE_MODEL<st_t>::reset() {
    for ( int i=0; i<4; i++ )
	Stats[i] = 3*STEP;
}

template <typename st_t>
void BASE_MODEL<st_t>::reset(int *start) {
    for (int i = 0; i < 4; i++) {
	Stats[i] =  start[i];
    }
}

template <typename st_t>
void BASE_MODEL<st_t>::rescaleRare()
{
    Stats[0] -= (Stats[0] >> 1);
    Stats[1] -= (Stats[1] >> 1);
    Stats[2] -= (Stats[2] >> 1);
    Stats[3] -= (Stats[3] >> 1);
}

#define Encode256 Encode
#define GetFreq256 GetFreq

/*
 * With anti-div 256 variant
 * fqz_comp -s8 -n2 -q3 -b < SRR013951_2.fastq > /dev/null
 * Names  786046582 ->   25759042 (0.033)
 * Bases 1384145212 ->  296378677 (0.214)
 * Quals 1384145212 ->  729437735 (0.527)
 * 
 * real    5m59.869s
 * user    9m38.740s
 * sys     0m7.920s
 *
 *
 * Without the anti-div code:
 * fqz_comp -s8 -n2 -q3 -b < SRR013951_2.fastq > /dev/null
 * Names  786046582 ->   25759042 (0.033)
 * Bases 1384145212 ->  296296256 (0.214)
 * Quals 1384145212 ->  729437735 (0.527)
 * 
 * real    6m20.240s
 * user    9m53.850s
 * sys     0m8.210s
 *
 * => about 3-5% faster. Worth it?
 */

template <typename st_t>
inline void BASE_MODEL<st_t>::encodeSymbol(RangeCoder *rc, uint sym) {
    int SummFreq = (Stats[0] + Stats[1]) + (Stats[2] + Stats[3]);
    if ( SummFreq>=WSIZ ) {
	rescaleRare();
	SummFreq = (Stats[0] + Stats[1]) + (Stats[2] + Stats[3]);
    }

    switch(sym) {
    case 0:
	rc->Encode256(0,                              Stats[0], SummFreq);
	Stats[0] += STEP; 
	break;
    case 1:
	rc->Encode256(Stats[0],                       Stats[1], SummFreq);
	Stats[1] += STEP;
	break;
    case 2:
	rc->Encode256(Stats[0] + Stats[1],            Stats[2], SummFreq);
	Stats[2] += STEP;
	break;
    case 3:
	rc->Encode256((Stats[0] + Stats[1]) + Stats[2], Stats[3], SummFreq);
	Stats[3] += STEP;
	break;
    }

    /*
     * Scary but marginally faster due to removing branching:
     *
     * uint32_t y = ((Stats[0]<<8)) |
     *              ((Stats[0] + Stats[1]) * 0x01010000) |
     *               (Stats[2]<<24);
     * rc->Encode((y&(0xff<<(sym<<3)))>>(sym<<3), Stats[sym], SummFreq);
     * Stats[sym] += STEP;
     */

    return;
}

template <typename st_t>
inline void BASE_MODEL<st_t>::updateSymbol(uint sym) {
    int SummFreq = (Stats[0] + Stats[1]) + (Stats[2] + Stats[3]);
    if ( SummFreq>=WSIZ ) {
	rescaleRare();
    }

    /* known symbol */
    Stats[sym] += STEP;            
}

/*
 * Returns the bias of the best symbol compared to all other symbols.
 * This is a measure of how well adapted this model thinks it is to the
 * incoming probabilities.
 */
template <typename st_t>
inline uint BASE_MODEL<st_t>::getTopSym(void) {
    return M4(Stats);
}

template <typename st_t>
inline uint BASE_MODEL<st_t>::getSummFreq(void) {
    int SummFreq = (Stats[0] + Stats[1]) + (Stats[2] + Stats[3]);
    return SummFreq;
}

template <typename st_t>
inline uint BASE_MODEL<st_t>::decodeSymbol(RangeCoder *rc) {
    int SummFreq = (Stats[0] + Stats[1]) + (Stats[2] + Stats[3]);
    if ( SummFreq>=WSIZ) {
	rescaleRare();
	SummFreq = (Stats[0] + Stats[1]) + (Stats[2] + Stats[3]);
    }

    uint count=rc->GetFreq256(SummFreq);
    uint HiCount=0;             

    st_t* p=Stats;
    if ((HiCount += *p) > count) {
	rc->Decode(0, *p, SummFreq);
	Stats[0] += STEP;
	return 0;
    }

    if ((HiCount += *++p) > count) {
	rc->Decode(HiCount-*p, *p, SummFreq);
	Stats[1] += STEP;
	return 1;
    }

    if ((HiCount += *++p) > count) {
	rc->Decode(HiCount-*p, *p, SummFreq);
	Stats[2] += STEP;
	return 2;
    }

    rc->Decode(HiCount, Stats[3], SummFreq);
    Stats[3] += STEP;

    return 3;
}
