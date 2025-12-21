#include "magic.h"
#include <stdio.h>

const char* MagicCardToString(MagicCard m)
{
    switch (m)
    {
    case MAGIC_PAIR_UPGRADE:
        return "Hand Score Upgrade: Pair +3 (Permanent)";
    case MAGIC_SUIT_CHANGE:
        return "Suit Change: Change a card suit once per round (Permanent)";
    default:
        return "Unknown Magic";
    }
}


void DrawTwoMagicCards(MagicCard *a, MagicCard *b)
{
    // 目前只有 2 種卡：最小合格版就抽兩張不同的（剛好都會出現）
    *a = MAGIC_PAIR_UPGRADE;
    *b = MAGIC_SUIT_CHANGE;
}

void ApplyMagicCard(MagicState *ms, MagicCard m)
{
    if (!ms) return;

    switch (m)
    {
    case MAGIC_PAIR_UPGRADE:
        ms->pairBonus += 3.0;
        break;
    case MAGIC_SUIT_CHANGE:
        ms->hasSuitChange = 1;
        break;
    default:
        break;
    }
}

void OfferMagicChoice(MagicState *ms)
{
    MagicCard a, b;
    DrawTwoMagicCards(&a, &b);

    printf("\n🎁 通關獎勵：抽到兩張 Magic Card（二選一）\n");
    printf("1) %s\n", MagicCardToString(a));
    printf("2) %s\n", MagicCardToString(b));
    printf("請輸入 1 或 2：");

    int choice = 0;
    scanf("%d", &choice);

    if (choice == 2)
    {
        ApplyMagicCard(ms, b);
        printf("你選擇了：%s\n", MagicCardToString(b));
    }
    else
    {
        ApplyMagicCard(ms, a);
        printf("你選擇了：%s\n", MagicCardToString(a));
    }

    printf("（目前魔法狀態）Pair Bonus：+%.1f，Suit Change：%s\n",
           ms->pairBonus, ms->hasSuitChange ? "有" : "無");
}

void UseSuitChangeIfWanted(Hand *player, const MagicState *ms)
{
    if (!player || !ms) return;
    if (!ms->hasSuitChange) return;

    int use = 0;
    printf("\n你有 Suit Change 能力：要使用嗎？(1=是, 0=否)：");
    scanf("%d", &use);
    if (use != 1) return;

    int idx = -1;
    int suitInt = -1;

    printf("要改哪一張？請輸入索引 0~6：");
    scanf("%d", &idx);

    if (idx < 0 || idx >= player->size || IsEmptyCard(player->cards[idx]))
    {
        printf("索引不合法或是空格，取消本次 Suit Change。\n");
        return;
    }

    printf("改成哪個花色？(0=♣, 1=♦, 2=♥, 3=♠)：");
    scanf("%d", &suitInt);

    if (suitInt < 0 || suitInt > 3)
    {
        printf("花色不合法，取消本次 Suit Change。\n");
        return;
    }

    player->cards[idx].suit = (Suit)suitInt;
    printf("✅ 已修改：[%d] 變成 ", idx);
    PrintCard(&player->cards[idx]);
    printf("\n");
}
int EarnGoldFromScore(double gained)
{
    // Gold = gained * 10
    double x = gained * 10.0;
    if (x < 0) x = 0;
    return (int)(x + 0.5); // 四捨五入避免浮點誤差
}

void PrintShopItems(const MagicState *ms)
{
    printf("\n======= 🛒 SHOP =======\n");
    printf("目前 Gold：%d\n", ms->gold);
    printf("你目前效果：Pair Bonus +%.1f | Suit Change %s | Redraw %s\n",
           ms->pairBonus,
           ms->hasSuitChange ? "有" : "無",
           ms->redrawPerLevel ? "有(每關一次)" : "無");

    printf("\n商品列表：\n");
    printf("1) Pair Upgrade (+1.0)   價格：120 Gold\n");
    printf("2) Suit Change(永久)     價格：150 Gold\n");
    printf("3) Redraw(每關一次)      價格：200 Gold\n");
    printf("0) 離開商店\n");
}

int BuyItem(MagicState *ms, int itemId)
{
    int price = 0;

    switch (itemId)
    {
    case 1: price = 120; break;
    case 2: price = 150; break;
    case 3: price = 200; break;
    default: return 0;
    }

    if (ms->gold < price)
    {
        printf("❌ Gold 不足（需要 %d，目前 %d）。\n", price, ms->gold);
        return 0;
    }

    ms->gold -= price;

    switch (itemId)
    {
    case 1:
        ms->pairBonus += 1.0;
        printf("✅ 購買成功：Pair Bonus +1.0（目前 +%.1f）\n", ms->pairBonus);
        break;
    case 2:
        ms->hasSuitChange = 1;
        printf("✅ 購買成功：Suit Change 已解鎖（永久）\n");
        break;
    case 3:
        ms->redrawPerLevel = 1;
        printf("✅ 購買成功：Redraw 已解鎖（每關一次）\n");
        break;
    }

    printf("剩餘 Gold：%d\n", ms->gold);
    return 1;
}

void ShopMenu(MagicState *ms)
{
    if (!ms) return;

    while (1)
    {
        PrintShopItems(ms);

        int choice = -1;
        printf("\n請輸入要購買的商品編號：");
        scanf("%d", &choice);

        if (choice == 0)
        {
            printf("離開商店。\n");
            break;
        }

        if (!BuyItem(ms, choice))
        {
            printf("（購買失敗或無此商品）\n");
        }
    }
}
