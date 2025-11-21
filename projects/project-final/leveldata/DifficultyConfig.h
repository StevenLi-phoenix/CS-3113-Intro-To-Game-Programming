#ifndef DIFFICULTY_CONFIG_H
#define DIFFICULTY_CONFIG_H

#include "../constants.h"
#include <algorithm>

namespace difficulty
{
    inline constexpr int GOLD_REQUIRED[branch::DIFFICULTY_PRESET_COUNT] = {0, 3, 5, 8};
    inline constexpr int INITIAL_BRANCHES[branch::DIFFICULTY_PRESET_COUNT] = {
        branch::PRESET_INITIALS[0],
        branch::PRESET_INITIALS[1],
        branch::PRESET_INITIALS[2],
        branch::PRESET_INITIALS[3]
    };
    inline constexpr int BOX_REWARD[branch::DIFFICULTY_PRESET_COUNT] = {
        branch::PRESET_BOX_REWARDS[0],
        branch::PRESET_BOX_REWARDS[1],
        branch::PRESET_BOX_REWARDS[2],
        branch::PRESET_BOX_REWARDS[3]
    };
}

struct DifficultyState
{
    int index = 1;

    int goldRequirement() const
    {
        int clamped = std::clamp(index, 0, branch::DIFFICULTY_PRESET_COUNT - 1);
        return difficulty::GOLD_REQUIRED[clamped];
    }

    int initialBranches() const
    {
        int clamped = std::clamp(index, 0, branch::DIFFICULTY_PRESET_COUNT - 1);
        return difficulty::INITIAL_BRANCHES[clamped];
    }

    int boxReward() const
    {
        int clamped = std::clamp(index, 0, branch::DIFFICULTY_PRESET_COUNT - 1);
        return difficulty::BOX_REWARD[clamped];
    }
};

#endif // DIFFICULTY_CONFIG_H
