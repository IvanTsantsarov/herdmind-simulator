#include "accel.h"
#include <stdio.h>   // for printf; remove or replace if not desired

// --------- ctor ----------
Accel::Accel()
    : mSec( 0.0f ),
    mAlphaDc( 0.0f ),
    mAlphaSm( 0.0f ),
    mAlphaNoise( 0.0f ),
    mLpDc( 0.0f ),
    mLpSm( 0.0f ),
    mNoiseEma( 0.0f ),
    mBand0( 0.0f ),
    mBand1( 0.0f ),
    mBand2( 0.0f ),
    mLastPeakSec( -1e9f ),
    mPeakCount( 0 ),
    mCpmCount( 0 ),
    mCpmHead( 0 ),
    mFlagPossibleAtony( false ),
    mFlagHypomotility( false ),
    mFlagHyperactivity( false ),
    mFsHz( FS_HZ ),
    mDtSec( DT_SEC ) {

    for ( int i = 0 ; i < CPM_BUF_SIZE ; ++i ) mCpmBuf[i] = NAN;
    updateAlphas();
}

// --------- public ----------
void Accel::setSampleRateHz( float fs ) {
    if ( fs <= 0.5f ) fs = 0.5f;
    if ( fs > 500.0f ) fs = 500.0f;
    mFsHz = fs;
    mDtSec = 1.0f / mFsHz;
    updateAlphas();
}

void Accel::addData( float ax, float ay, float az ) {
    // Advance time
    mSec += mDtSec;

    // Magnitude
    float mag = sqrtf( ax * ax + ay * ay + az * az );

    // Filters
    updateFilters( mag );

    // Noise tracker
    mNoiseEma += mAlphaNoise * ( fabsf( mBand0 ) - mNoiseEma );

    // Peak detection after we have at least 3 band samples (mBand2 valid after first 2 samples)
    tryDetectPeak();

    // Update flags each sample (cheap)
    float gap = ( mPeakCount == 0 ) ? mSec : ( mSec - mLastPeakSec );
    mFlagPossibleAtony = ( mPeakCount == 0 ) || ( gap > ATONY_GAP_SEC );

    float med = medianCpm();
    mFlagHypomotility  = ( !isnan( med ) ) && ( med < HYPOMOTILITY_CPM );
    mFlagHyperactivity = ( !isnan( med ) ) && ( med > HYPERACTIVITY_CPM );
}

void Accel::printSummary() {
    float med = medianCpm();
    printf( "----- SUMMARY -----\n" );
    printf( "Samples seconds: %.1f\n", mSec );
    printf( "Detected peaks: %d\n", mPeakCount );
    if ( isnan( med ) ) {
        printf( "Median CPM: NaN\n" );
    } else {
        printf( "Median CPM: %.3f\n", med );
    }
    printf( "Flags: %s%s%s\n",
           mFlagPossibleAtony ? "[possible_atony] " : "",
           mFlagHypomotility  ? "[hypomotility] "   : "",
           mFlagHyperactivity ? "[hyperactivity] "  : "" );
}

bool Accel::getPossibleAtony() const      { return mFlagPossibleAtony; }
bool Accel::getHypomotility() const       { return mFlagHypomotility; }
bool Accel::getHyperactivity() const      { return mFlagHyperactivity; }
float Accel::getMedianCpm() const         { return medianCpm(); }

// --------- private ----------
void Accel::updateAlphas() {
    // alpha = 1 - exp(-dt / tau)
    float ad = 1.0f - expf( -mDtSec / ( (LONG_DC_SEC  > 1e-3f) ? LONG_DC_SEC  : 1e-3f ) );
    float as = 1.0f - expf( -mDtSec / ( (SHORT_SMOOTH_SEC > 1e-3f) ? SHORT_SMOOTH_SEC : 1e-3f ) );
    float an = 1.0f - expf( -mDtSec / ( (NOISE_TAU_SEC > 1e-3f) ? NOISE_TAU_SEC : 1e-3f ) );
    // Clamp to stable range (0,1)
    mAlphaDc    = ( ad < 1e-6f ) ? 1e-6f : ( ad > 0.5f ? 0.5f : ad );
    mAlphaSm    = ( as < 1e-6f ) ? 1e-6f : ( as > 0.5f ? 0.5f : as );
    mAlphaNoise = ( an < 1e-6f ) ? 1e-6f : ( an > 0.5f ? 0.5f : an );
}

void Accel::updateFilters( float mag ) {
    // DC low-pass
    mLpDc += mAlphaDc * ( mag - mLpDc );
    float detr = mag - mLpDc;

    // Smoothing low-pass
    mLpSm += mAlphaSm * ( detr - mLpSm );
    float band = mLpSm;

    // Shift band history: 2 <- 1 <- 0 <- band
    mBand2 = mBand1;
    mBand1 = mBand0;
    mBand0 = band;
}

void Accel::tryDetectPeak() {
    // Need at least three points
    static int warm = 0;
    if ( warm < 2 ) { warm++; return; }

    // Adaptive threshold
    float noise = ( mNoiseEma > 1e-9f ) ? mNoiseEma : 1e-9f;
    float thr = ADAPT_K * noise;
    if ( thr < MIN_PEAK_PROM ) thr = MIN_PEAK_PROM;

    // Check if mBand1 is a local maximum above threshold and respects refractory
    if ( mBand1 > mBand2 && mBand1 > mBand0 && mBand1 > thr ) {
        float lastPeakAgo = mSec - mLastPeakSec;
        if ( lastPeakAgo >= PEAK_MIN_SEP_S ) {
            // Approximate the peak time as the middle sample timestamp:
            float peakSec = mSec - mDtSec; // mBand1 time
            onPeak( peakSec, mBand1 );
        }
    }
}

void Accel::onPeak( float peakSec, float /*peakAmp*/ ) {
    if ( mPeakCount > 0 ) {
        float dt = peakSec - mLastPeakSec;
        if ( dt > 0.0f && dt < 600.0f ) {
            float cpm = 60.0f / dt;
            // Ring buffer write
            mCpmBuf[mCpmHead] = cpm;
            mCpmHead = ( mCpmHead + 1 ) % CPM_BUF_SIZE;
            if ( mCpmCount < CPM_BUF_SIZE ) mCpmCount++;
        }
    }
    mLastPeakSec = peakSec;
    mPeakCount++;

    // Optional: print a short event line (comment out on bare metal)
    // printf( "PEAK t=%.3f s\n", peakSec );
}

float Accel::medianCpm() const {
    if ( mCpmCount < 5 ) {
        // Not enough contraction intervals to compute a reliable median
        return NAN;
    }

    // Simple running mean instead of median — cheaper and stable enough for small N
    float sum = 0.0f;
    for ( int i = 0 ; i < mCpmCount ; ++i ) {
        int idx = ( ( mCpmHead - mCpmCount ) + i );
        while ( idx < 0 ) idx += CPM_BUF_SIZE;
        idx %= CPM_BUF_SIZE;
        sum += mCpmBuf[idx];
    }
    return sum / (float)mCpmCount;
}

