#ifndef MAGIC_H
#define MAGIC_H

#include "cards.h"

typedef enum
{
    MAGIC_PAIR_UPGRADE = 0, // Pair +3（永久）
    MAGIC_SUIT_CHANGE  = 1  // 每回合可改一張牌的花色（永久能力）
} MagicCard;

typedef struct
{
    double pairBonus;     // Pair 額外加分（永久）
    int hasSuitChange;    // 是否擁有 Suit Change（永久）

    int gold;             // ★新增：Gold（跨關保留）
    int redrawPerLevel;   // ★新增：是否擁有「每關一次重抽」
} MagicState;

typedef struct
{
    int redrawUsed; // 這一關是否已使用重抽
} LevelState;

const char* MagicCardToString(MagicCard m);
void DrawTwoMagicCards(MagicCard *a, MagicCard *b);
void ApplyMagicCard(MagicState *ms, MagicCard m);
void OfferMagicChoice(MagicState *ms);
void UseSuitChangeIfWanted(Hand *player, const MagicState *ms);
// Shop / Gold
int EarnGoldFromScore(double gained);
void ShopMenu(MagicState *ms);
void PrintShopItems(const MagicState *ms);
int BuyItem(MagicState *ms, int itemId);


#endif
