#ifndef HAND_EVAL_H
#define HAND_EVAL_H

#include "cards.h"

typedef enum
{
    HAND_INVALID = 0,
    HAND_SINGLE,
    HAND_PAIR,
    HAND_TRIPS,
    HAND_STRAIGHT,
    HAND_FLUSH,
    HAND_FULL_HOUSE,
    HAND_FOUR_OF_A_KIND,
    HAND_STRAIGHT_FLUSH
} HandType;

void SwapCard(Card *a, Card *b);
void SortByRank(Card *cards, int count);
int IsFlush(Card *cards, int count);
int IsStraight(Card *cards, int count);
const char *HandTypeToString(HandType type);
HandType EvaluateHand(Card *cards, int count);

#endif
