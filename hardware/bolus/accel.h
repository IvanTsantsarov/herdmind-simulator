#pragma once
#include <stdint.h>
#include <math.h>

// ======================= CONSTANTS =======================
#define FS_HZ                10.0f        // sampling rate (Hz). tune for your MCU loop
#define DT_SEC               (1.0f / FS_HZ)

#define SHORT_SMOOTH_SEC     5.0f         // smoothing LP time constant (sec)
#define LONG_DC_SEC          180.0f       // DC LP time constant (sec) — IIR, no big buffer
#define NOISE_TAU_SEC        120.0f       // noise EMA time constant (sec)

#define PEAK_MIN_SEP_S       20.0f        // min time between peaks (sec)
#define MIN_PEAK_PROM        0.005f       // minimal amplitude (units of accel magnitude)
#define ADAPT_K              2.0f         // threshold multiplier

#define ATONY_GAP_SEC        1800.0f      // 30 min without a peak
#define HYPOMOTILITY_CPM     0.8f
#define HYPERACTIVITY_CPM    3.5f

#define CPM_BUF_SIZE         33           // must be odd to take median; ~132 bytes RAM
// ========================================================

class Accel {
public:
    Accel();

    // Feed one accelerometer sample (called at ~FS_HZ)
    void addData( float ax, float ay, float az );

    // Optional: adjust sampling rate at runtime (resets internal alphas)
    void setSampleRateHz( float fs );

    // Quick text summary (uses printf; you can stub it out on bare metal)
    void printSummary();

    // Health-flag getters
    bool getPossibleAtony() const;
    bool getHypomotility() const;
    bool getHyperactivity() const;

    // Optional: expose current median CPM (NaN if not enough data yet)
    float getMedianCpm() const;

private:
    // --------- Internal helpers (no dynamic memory) ---------
    void updateAlphas();
    void updateFilters( float mag );
    void tryDetectPeak();
    void onPeak( float peakSec, float peakAmp );
    float medianCpm() const;


    // --------- Members (m + Capitalized, no underscores) ---------
    // Timestamp derived from sample count and DT
    float mSec;

    // Filters (first-order IIRs, constant memory)
    float mAlphaDc;
    float mAlphaSm;
    float mAlphaNoise;

    float mLpDc;        // long DC low-pass state
    float mLpSm;        // short smoothing low-pass state
    float mNoiseEma;    // EMA of |band|

    // Last three band samples for 3-point peak detection
    float mBand0;       // newest
    float mBand1;       // middle (candidate peak)
    float mBand2;       // oldest

    // Peak timing
    float mLastPeakSec;
    int   mPeakCount;

    // CPM ring buffer (for flags)
    float mCpmBuf[CPM_BUF_SIZE];
    int   mCpmCount;
    int   mCpmHead;     // next write index

    // Flags
    bool  mFlagPossibleAtony;
    bool  mFlagHypomotility;
    bool  mFlagHyperactivity;

    // Cached parameters
    float mFsHz;
    float mDtSec;
};
