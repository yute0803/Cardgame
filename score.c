#include "score.h"

double GetBaseScoreByLevel(int level, HandType type)
{
    switch (type)
    {
    case HAND_SINGLE:
        if (level == 1) return 1.0;
        if (level == 2) return 0.5;
        return 0.0; // level 3
    case HAND_PAIR:
        if (level == 2) return 4.0;
        return 2.0; // level 1 & 3
    case HAND_TRIPS:          return 4.0;
    case HAND_STRAIGHT:       return 5.0;
    case HAND_FLUSH:          return 6.0;
    case HAND_FULL_HOUSE:     return 8.0;
    case HAND_FOUR_OF_A_KIND: return 10.0;
    case HAND_STRAIGHT_FLUSH: return 12.0;
    default:                  return 0.0;
    }
}

double GetScore(int level, HandType type, const MagicState *ms)
{
    double base = GetBaseScoreByLevel(level, type);
    if (base <= 0.0) return 0.0;

    if (ms && type == HAND_PAIR)
        base += ms->pairBonus;

    return base;
}
void ResetChain(ChainState *cs)
{
    cs->lastType = HAND_INVALID;
    cs->chainCount = 0;
    cs->multiplier = 1.0;
}
static int IsChainSuccess(HandType prev, HandType cur)
{
    // 斷連條件先擋掉
    if (cur == HAND_INVALID || cur == HAND_SINGLE) return 0;

    // 規則 1：連續出對子
    if (prev == HAND_PAIR && cur == HAND_PAIR) return 1;

    // 規則 2：對子 → 葫蘆 → 鐵支
    if (prev == HAND_PAIR && cur == HAND_FULL_HOUSE) return 1;
    if (prev == HAND_FULL_HOUSE && cur == HAND_FOUR_OF_A_KIND) return 1;

    return 0;
}

double ApplyChainMultiplier(ChainState *cs, HandType current)
{
    if (!cs) return 1.0;

       // 第一手：建立起點
    if (cs->lastType == HAND_INVALID)
    {
        cs->lastType = current;
        cs->chainCount = 1;
        cs->multiplier = 1.0;
        return cs->multiplier;
    }

    // 檢查是否接技成功
    if (IsChainSuccess(cs->lastType, current))
    {
        cs->chainCount++;
        cs->multiplier *= 1.25;            // 每成功一次倍率乘上去（你可改 1.2/1.3）
        if (cs->multiplier > 3.0) cs->multiplier = 3.0; // 上限避免爆表
    }
    else
    {
        // 斷連：以這次作為新起點
        cs->chainCount = 1;
        cs->multiplier = 1.0;
    }

    cs->lastType = current;
    return cs->multiplier;
}


double GetScoreChained(int level, HandType type, const MagicState *ms, ChainState *cs)
{
    double base = GetScore(level, type, ms);   // base + magic
    if (base <= 0.0)
    {
        return 0.0;
    }

    double mul = cs ? ApplyChainMultiplier(cs, type) : 1.0;
    return base * mul;
}
