#ifndef SCORE_H
#define SCORE_H

#include "hand_eval.h"
#include "magic.h"

double GetBaseScoreByLevel(int level, HandType type);
double GetScore(int level, HandType type, const MagicState *ms);
typedef struct {
    HandType lastType;     // 上一次有效出牌的牌型
    int chainCount;        // 目前連鎖層數（成功幾次）
    double multiplier;     // 目前倍率（例如 1.0, 1.2, 1.44...）
} ChainState;
void ResetChain(ChainState *cs);
double ApplyChainMultiplier(ChainState *cs, HandType current);
double GetScoreChained(int level, HandType type, const MagicState *ms, ChainState *cs);

#endif
