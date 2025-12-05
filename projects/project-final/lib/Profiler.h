#ifndef PROFILER_H
#define PROFILER_H
// helper for debugging

#include "Helper.h"
#include <string>
#include <unordered_map>
#include <vector>

class FrameProfiler
{
public:
    explicit FrameProfiler(bool enabled, double logIntervalSec = 0.5)
        : mEnabled(enabled),
          mLogInterval(logIntervalSec)
    {
        if (!mEnabled)
        {
            return;
        }
        mStartTime = GetTime();
        mLastMark = mStartTime;
        mSegments.reserve(8);
    }

    void mark(const char *label, const char *category = "default")
    {
        if (!mEnabled)
        {
            return;
        }
        const double now = GetTime();
        const double ms = (now - mLastMark) * 1000.0;
        const char *safeLabel = label ? label : "unknown";
        const char *safeCategory = category ? category : "default";
        mSegments.push_back({safeLabel, safeCategory, ms});
        mCategoryTotals[safeCategory] += ms;
        mLastMark = now;
    }

    void logSummary()
    {
        if (!mEnabled || mSegments.empty())
        {
            return;
        }

        const double now = GetTime();
        if (sLastLogTime > 0.0 && (now - sLastLogTime) < mLogInterval)
        {
            return;
        }
        sLastLogTime = now;

        const double totalMs = (now - mStartTime) * 1000.0;
        const Segment *maxSeg = nullptr;
        for (const Segment &seg : mSegments)
        {
            if (!maxSeg || seg.ms > maxSeg->ms)
            {
                maxSeg = &seg;
            }
        }

        std::string segSummary;
        segSummary.reserve(mSegments.size() * 16);
        for (size_t i = 0; i < mSegments.size(); ++i)
        {
            if (i > 0)
            {
                segSummary.push_back(' ');
            }
            char buf[32];
            std::snprintf(buf, sizeof(buf), "%.2fms", mSegments[i].ms);
            segSummary.append(mSegments[i].label);
            segSummary.push_back('=');
            segSummary.append(buf);
        }

        std::string catSummary;
        catSummary.reserve(mCategoryTotals.size() * 16);
        size_t catIdx = 0;
        for (const auto &entry : mCategoryTotals)
        {
            if (catIdx++ > 0)
            {
                catSummary.push_back(' ');
            }
            char buf[32];
            std::snprintf(buf, sizeof(buf), "%.2fms", entry.second);
            catSummary.append(entry.first);
            catSummary.push_back('=');
            catSummary.append(buf);
        }

        const char *maxLabel = (maxSeg && maxSeg->label) ? maxSeg->label : "-";
        const double maxMs = maxSeg ? maxSeg->ms : 0.0;
        LOG_DEBUG(TextFormat("Frame profile total=%.2fms max=%s(%.2fms) segments=%s categories=%s",
                             totalMs,
                             maxLabel,
                             maxMs,
                             segSummary.c_str(),
                             catSummary.c_str()));
    }

private:
    struct Segment
    {
        const char *label;
        const char *category;
        double ms;
    };

    bool mEnabled = false;
    double mStartTime = 0.0;
    double mLastMark = 0.0;
    double mLogInterval = 0.5;
    std::vector<Segment> mSegments;
    std::unordered_map<std::string, double> mCategoryTotals;
    static inline double sLastLogTime = 0.0;
};

#endif // PROFILER_H
