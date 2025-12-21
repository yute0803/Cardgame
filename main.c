#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <windows.h>

#include "cards.h"
#include "hand_eval.h"
#include "score.h"
#include "magic.h"

// -------------------- 你可以改這裡：每關目標分數 --------------------
static const double LEVEL_TARGETS[3] = {10.0, 10.0, 10.0};

// Level 流程
static int PlayOneLevel(int level, double target, MagicState *ms);

int main(void)
{
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    srand((unsigned)time(NULL));

    MagicState ms;
    ms.pairBonus = 0.0;
    ms.hasSuitChange = 0;
    ms.gold = 0;
    ms.redrawPerLevel = 0;

    printf("=== Card Game (Level 1/2/3 + Magic Cards) ===\n");

    for (int level = 1; level <= 3; ++level)
    {
        double target = LEVEL_TARGETS[level - 1];
        int passed = PlayOneLevel(level, target, &ms);

        if (!passed)
        {
            printf("\n你在 Level %d 失敗，Game Over。\n", level);
            return 0;
        }

        printf("\n✅ Level %d 通關！\n", level);

        // 通關後（但不是最後一關）給 Magic Card 二選一 + 商店
        if (level < 3)
        {
            OfferMagicChoice(&ms);
            ShopMenu(&ms);

            printf("按 Enter 進入下一關...");
            getchar(); // 吃掉 scanf 留下的換行
            getchar();
        }
    }

    printf("\n🎉 恭喜！你已通過 Level 1~3 全部關卡！\n");
    printf("最終魔法效果：Pair Bonus = +%.1f，Suit Change = %s\n",
           ms.pairBonus, ms.hasSuitChange ? "有" : "無");
    printf("最終 Gold：%d\n", ms.gold);

    return 0;
}

static int PlayOneLevel(int level, double target, MagicState *ms)
{
    printf("\n=============================\n");
    printf("【Level %d】 目標分數：%.1f\n", level, target);
    printf("（Magic）Pair Bonus：+%.1f，Suit Change：%s，Gold：%d，Redraw：%s\n",
           ms->pairBonus,
           ms->hasSuitChange ? "有" : "無",
           ms->gold,
           ms->redrawPerLevel ? "有(每關一次)" : "無");
    printf("=============================\n");

    Deck *deck = CreateDeck();
    ShuffleDeck(deck);

    Hand *player = CreateHand(7);
    DrawUntilFull(deck, player, 7);

    LevelState ls;
    ls.redrawUsed = 0;

    double score = 0.0;
    int round = 1;

    while (1)
    {
        printf("\n----- 回合 %d (Level %d) -----\n", round, level);
        PrintHand(player, "玩家");
        printf("牌堆剩餘張數：%d\n", deck->size);
        printf("目前分數：%.1f / %.1f\n", score, target);

        // 通關
        if (score >= target)
        {
            printf("🎯 已達標！Level %d 通關。\n", level);
            DestroyHand(player);
            DestroyDeck(deck);
            return 1;
        }

        // 失敗：沒牌了
        if (deck->size == 0 && !HandHasAnyCard(player))
        {
            printf("牌堆沒牌、手牌也空了，且未達標。\n");
            DestroyHand(player);
            DestroyDeck(deck);
            return 0;
        }

        // Suit Change（每回合最多一次）
        UseSuitChangeIfWanted(player, ms);

        int selectCount = 0;

        if (ms->redrawPerLevel && !ls.redrawUsed)
            printf("\n你想出幾張牌？(1~5，0 放棄，9=使用重抽)：");
        else
            printf("\n你想出幾張牌？(1~5，0 放棄本關)：");

        scanf("%d", &selectCount);

        // 重抽
        if (selectCount == 9)
        {
            if (!ms->redrawPerLevel)
            {
                printf("你尚未購買重抽能力。\n");
                continue;
            }
            if (ls.redrawUsed)
            {
                printf("本關已使用過重抽了。\n");
                continue;
            }

            for (int i = 0; i < player->size; ++i)
                player->cards[i] = MakeEmptyCard();

            DrawUntilFull(deck, player, 7);

            ls.redrawUsed = 1;
            printf("✅ 已使用重抽：手牌已重新抽滿。\n");
            continue;
        }

        // 放棄本關
        if (selectCount == 0)
        {
            printf("你選擇放棄本關。\n");
            DestroyHand(player);
            DestroyDeck(deck);
            return 0;
        }

        if (selectCount < 1 || selectCount > 5)
        {
            printf("張數不合法。\n");
            continue;
        }

        // 檢查手上可用牌數是否足夠
        int available = 0;
        for (int i = 0; i < player->size; ++i)
            if (!IsEmptyCard(player->cards[i])) available++;

        if (available < selectCount)
        {
            printf("可用牌不夠 %d 張（目前可用 %d 張）。\n", selectCount, available);
            continue;
        }

        int indices[5] = {0};
        Card chosen[5];

        printf("請輸入你要出的牌的索引 (0~6，用空白分隔)：\n");
        for (int i = 0; i < selectCount; ++i)
            scanf("%d", &indices[i]);

        // 驗證：範圍、空格、重複
        int invalidInput = 0;

        for (int i = 0; i < selectCount; ++i)
        {
            int idx = indices[i];
            if (idx < 0 || idx >= player->size) { invalidInput = 1; break; }
            if (IsEmptyCard(player->cards[idx])) { invalidInput = 1; break; }
        }

        if (!invalidInput)
        {
            for (int i = 0; i < selectCount; ++i)
            {
                for (int j = i + 1; j < selectCount; ++j)
                {
                    if (indices[i] == indices[j]) { invalidInput = 1; break; }
                }
                if (invalidInput) break;
            }
        }

        if (invalidInput)
        {
            printf("索引不合法（超出範圍/選到空格/重複）。請重來。\n");
            continue;
        }

        for (int i = 0; i < selectCount; ++i)
            chosen[i] = player->cards[indices[i]];

        HandType ht = EvaluateHand(chosen, selectCount);
        printf("你出的牌型是：%s\n", HandTypeToString(ht));

        double gained = GetScore(level, ht, ms);

        // 無效牌型：0 分但消耗牌
        if (gained <= 0.0)
        {
            printf("無效牌型，本回合 0 分，但牌會被消耗。\n");
            RemoveSelectedCardsFromHand(player, indices, selectCount);
            DrawUntilFull(deck, player, 7);
            round++;
            continue;
        }

        // 有效牌型：加分 + Gold + 消耗牌 + 補牌
        printf("本回合加分：%.1f\n", gained);

        int earned = EarnGoldFromScore(gained);
        ms->gold += earned;
        printf("本回合獲得 Gold：%d（目前 Gold：%d）\n", earned, ms->gold);

        score += gained;

        RemoveSelectedCardsFromHand(player, indices, selectCount);
        DrawUntilFull(deck, player, 7);

        round++;
    }
}
