#ifndef CARDS_H
#define CARDS_H

#include <stdio.h>
#include <stdlib.h>

// -------------------- 型別定義 --------------------

typedef enum
{
    SUIT_CLUBS,    // ♣ 0
    SUIT_DIAMONDS, // ♦ 1
    SUIT_HEARTS,   // ♥ 2
    SUIT_SPADES    // ♠ 3
} Suit;

typedef struct
{
    Suit suit;
    int rank; // 1~13；rank=0 代表空格
} Card;

typedef struct
{
    Card *cards;
    int size; // 牌堆剩幾張
} Deck;

typedef struct
{
    Card *cards;   // 固定 7 格
    int size;      // 固定 7
    int capacity;  // 固定 7
} Hand;

// -------------------- 顯示 --------------------
const char *SuitToString(Suit s);
const char *RankToString(int rank);
void PrintCard(const Card *c);
void PrintHand(const Hand *hand, const char *name);

// -------------------- 空格工具 --------------------
int IsEmptyCard(Card c);
Card MakeEmptyCard(void);
int HandHasAnyCard(const Hand *hand);

// -------------------- Deck --------------------
Deck *CreateDeck(void);
void DestroyDeck(Deck *deck);
void ShuffleDeck(Deck *deck);
int DealCard(Deck *deck, Card *outCard);

// -------------------- Hand（固定 7 格） --------------------
Hand *CreateHand(int fixedSlots);
void DestroyHand(Hand *hand);
void DrawUntilFull(Deck *deck, Hand *hand, int targetSize);
void RemoveSelectedCardsFromHand(Hand *hand, int indices[], int count);

#endif
