#include "cards.h"

// -------------------- 顯示工具 --------------------

const char *SuitToString(Suit s)
{
    switch (s)
    {
    case SUIT_CLUBS:    return "♣";
    case SUIT_DIAMONDS: return "♦";
    case SUIT_HEARTS:   return "♥";
    case SUIT_SPADES:   return "♠";
    default:            return "?";
    }
}

const char *RankToString(int rank)
{
    switch (rank)
    {
    case 0:  return " ";
    case 1:  return "A";
    case 11: return "J";
    case 12: return "Q";
    case 13: return "K";
    default:
    {
        static char buf[3];
        snprintf(buf, sizeof(buf), "%d", rank);
        return buf;
    }
    }
}

void PrintCard(const Card *c)
{
    printf("%s%s", SuitToString(c->suit), RankToString(c->rank));
}

int IsEmptyCard(Card c)
{
    return c.rank == 0;
}

Card MakeEmptyCard(void)
{
    Card c;
    c.suit = SUIT_CLUBS;
    c.rank = 0;
    return c;
}

int HandHasAnyCard(const Hand *hand)
{
    for (int i = 0; i < hand->size; ++i)
        if (!IsEmptyCard(hand->cards[i])) return 1;
    return 0;
}

void PrintHand(const Hand *hand, const char *name)
{
    printf("%s 的手牌（固定 7 格）：\n", name);
    for (int i = 0; i < hand->size; ++i)
    {
        printf("[%d] ", i);
        if (IsEmptyCard(hand->cards[i]))
        {
            printf("[  ]\n");
        }
        else
        {
            PrintCard(&hand->cards[i]);
            printf("\n");
        }
    }
}

// -------------------- Deck --------------------

Deck *CreateDeck(void)
{
    Deck *deck = (Deck *)malloc(sizeof(Deck));
    if (!deck)
    {
        fprintf(stderr, "CreateDeck: malloc Deck 失敗\n");
        exit(1);
    }

    deck->size = 52;
    deck->cards = (Card *)malloc(sizeof(Card) * deck->size);
    if (!deck->cards)
    {
        fprintf(stderr, "CreateDeck: malloc cards 失敗\n");
        free(deck);
        exit(1);
    }

    int index = 0;
    for (int s = 0; s < 4; ++s)
    {
        for (int r = 1; r <= 13; ++r)
        {
            deck->cards[index].suit = (Suit)s;
            deck->cards[index].rank = r;
            index++;
        }
    }

    return deck;
}

void DestroyDeck(Deck *deck)
{
    if (!deck) return;
    free(deck->cards);
    free(deck);
}

void ShuffleDeck(Deck *deck)
{
    if (!deck || deck->size <= 1) return;

    for (int i = deck->size - 1; i > 0; --i)
    {
        int j = rand() % (i + 1);
        Card temp = deck->cards[i];
        deck->cards[i] = deck->cards[j];
        deck->cards[j] = temp;
    }
}

int DealCard(Deck *deck, Card *outCard)
{
    if (!deck || deck->size <= 0) return 0;

    deck->size--;
    *outCard = deck->cards[deck->size]; // ← 這行也沒問題
    return 1;
}

// -------------------- Hand（固定 7 格） --------------------

Hand *CreateHand(int fixedSlots)
{
    if (fixedSlots <= 0) fixedSlots = 7;

    Hand *hand = (Hand *)malloc(sizeof(Hand));
    if (!hand)
    {
        fprintf(stderr, "CreateHand: malloc Hand 失敗\n");
        exit(1);
    }

    hand->cards = (Card *)malloc(sizeof(Card) * fixedSlots);
    if (!hand->cards)
    {
        fprintf(stderr, "CreateHand: malloc cards 失敗\n");
        free(hand);
        exit(1);
    }

    hand->size = fixedSlots;
    hand->capacity = fixedSlots;

    for (int i = 0; i < hand->size; ++i)
        hand->cards[i] = MakeEmptyCard();

    return hand;
}

void DestroyHand(Hand *hand)
{
    if (!hand) return;
    free(hand->cards);
    free(hand);
}

void DrawUntilFull(Deck *deck, Hand *hand, int targetSize)
{
    if (!deck || !hand) return;
    if (targetSize > hand->size) targetSize = hand->size;

    for (int i = 0; i < targetSize; ++i)
    {
        if (IsEmptyCard(hand->cards[i]))
        {
            Card c;
            if (!DealCard(deck, &c)) return;
            hand->cards[i] = c;   // ← 這行是正確的（整張 copy）
        }
    }
}

void RemoveSelectedCardsFromHand(Hand *hand, int indices[], int count)
{
    if (!hand || count <= 0) return;

    for (int i = 0; i < count; ++i)
    {
        int idx = indices[i];
        if (idx < 0 || idx >= hand->size) continue;
        hand->cards[idx] = MakeEmptyCard();
    }
}
