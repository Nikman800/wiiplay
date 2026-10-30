#ifndef RP_PARTY_COMMON_DEBUG_GRAPH_H
#define RP_PARTY_COMMON_DEBUG_GRAPH_H
#include <Pack/types_pack.h>

//! @addtogroup rp_party
//! @{

/**
 * @brief Ring buffer of 600 float samples drawn as a debug graph
 * @details No vtable. Size 0x96C, verified (array stride of the new[] arrays made by
 * RP_allocDebugBuffers: 6 at 8043C664, 3 at 8043C668, 3 at 8043C66C).
 * @note Wii Play-only framework class; the name is a project convention.
 */
class RPDebugGraph {
public:
    RPDebugGraph() : mStart(0), mHead(0), mFull(false) {}

    void Clear() {
        mStart = 0;
        mHead = 0;
        mFull = false;
    }

    /**
     * @brief Stores the sample at mHead, wraps at 600, marks full when it meets mStart [H]
     */
    void Push(f32 sample) {
        mSamples[mHead] = sample;
        mHead = mHead + 1 < SAMPLE_NUM ? mHead + 1 : 0;
        mFull = mFull | (mHead == mStart);
    }

    int WrapCount(int count) const {
        return count < SAMPLE_NUM ? count : count - SAMPLE_NUM;
    }

    int Count() const {
        if (!mFull) {
            return WrapCount(mHead - mStart);
        }
        return SAMPLE_NUM;
    }

    //! @brief The sample pushed index pushes ago (0 is the newest)
    const f32& Recent(s32 index) const {
        return mSamples[mHead - 1 - index >= 0 ? mHead - 1 - index
                                               : mHead + SAMPLE_NUM - 1 - index];
    }

    //! @brief Recent() by value, with the index bound by reference (RP_drawDebugLogAndGraphs)
    f32 Sample(const s32& index) {
        s32 pos = mHead - 1 - index;
        s32 wrapped = mHead + SAMPLE_NUM - 1 - index;
        return mSamples[pos >= 0 ? pos : wrapped];
    }

private:
    static const int SAMPLE_NUM = 600;

    f32 mSamples[SAMPLE_NUM]; // at 0x0
    s32 mStart;               // at 0x960
    s32 mHead;                // at 0x964
    bool mFull;               // at 0x968
};

//! @}

#endif
