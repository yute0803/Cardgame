#include "hand_eval.h"

void SwapCard(Card *a, Card *b)
{
    Card temp = *a;
    *a = *b;
    *b = temp;
}

void SortByRank(Card *cards, int count)
{
    for (int i = 0; i < count - 1; ++i)
    {
        for (int j = 0; j < count - 1 - i; ++j)
        {
            if (cards[j].rank > cards[j + 1].rank)
                SwapCard(&cards[j], &cards[j + 1]);
        }
    }
}

int IsFlush(Card *cards, int count)
{
    if (count <= 0) return 0;
    Suit s = cards[0].suit;
    for (int i = 1; i < count; ++i)
        if (cards[i].suit != s) return 0;
    return 1;
}

// 假設 cards 已排序
int IsStraight(Card *cards, int count)
{
    if (count < 2) return 0;

    int normal = 1;
    for (int i = 0; i < count - 1; ++i)
        if (cards[i + 1].rank != cards[i].rank + 1) { normal = 0; break; }
    if (normal) return 1;

    // A 也可當高牌：排序後為 1,10,11,12,13 這種「A 接到 K」的形狀
    if (cards[0].rank != 1 || cards[count - 1].rank != 13) return 0;
    for (int i = 1; i < count - 1; ++i)
        if (cards[i + 1].rank != cards[i].rank + 1) return 0;
    return cards[1].rank == 13 - (count - 2);
}

const char *HandTypeToString(HandType type)
{
    switch (type)
    {
    case HAND_SINGLE:         return "單張";
    case HAND_PAIR:           return "一對";
    case HAND_TRIPS:          return "三條";
    case HAND_STRAIGHT:       return "順子";
    case HAND_FLUSH:          return "同花";
    case HAND_FULL_HOUSE:     return "葫蘆";
    case HAND_FOUR_OF_A_KIND: return "四條";
    case HAND_STRAIGHT_FLUSH: return "同花順";
    default:                  return "無效牌型";
    }
}

// 依你規則：張數限制牌型
HandType EvaluateHand(Card *cards, int count)
{
    if (count <= 0 || count > 5) return HAND_INVALID;
    if (count == 1) return HAND_SINGLE;

    Card temp[5];
    for (int i = 0; i < count; ++i) temp[i] = cards[i];
    SortByRank(temp, count);

    int counts[5] = {0};
    for (int i = 0; i < count; ++i)
    {
        counts[i] = 0;
        for (int j = 0; j < count; ++j)
            if (temp[j].rank == temp[i].rank) counts[i]++;
    }

    int four = 0, three = 0, pairFlag = 0;
    for (int i = 0; i < count; ++i)
    {
        if (counts[i] == 4) four = 1;
        else if (counts[i] == 3) three = 1;
        else if (counts[i] == 2) pairFlag = 1;
    }

    int flush = IsFlush(temp, count);
    int straight = IsStraight(temp, count);

    if (count == 2) return pairFlag ? HAND_PAIR : HAND_INVALID;
    if (count == 3) return three ? HAND_TRIPS : HAND_INVALID;
    if (count == 4) return four ? HAND_FOUR_OF_A_KIND : HAND_INVALID;

    // count == 5
    if (straight && flush) return HAND_STRAIGHT_FLUSH;
    if (four)              return HAND_FOUR_OF_A_KIND;
    if (three && pairFlag) return HAND_FULL_HOUSE;
    if (flush)             return HAND_FLUSH;
    if (straight)          return HAND_STRAIGHT;
    return HAND_INVALID;
}
