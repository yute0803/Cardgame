#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "raylib.h"

#include "cards.h"
#include "hand_eval.h"
#include "score.h"
#include "magic.h"

// -------------------- Config --------------------
static const double LEVEL_TARGETS[3] = {55.0, 60.0, 65.0};

// Shop prices
#define PRICE_REDRAW 50
#define PRICE_SUIT 80
#define PRICE_PAIR 300

// -------------------- UI State --------------------
typedef enum
{
    UI_TITLE = 0,
    UI_RULES,
    UI_PLAYING,
    UI_LOSE,
    UI_ADV_RULES,
    UI_MAGIC_CHOICE,
    UI_SHOP,
    UI_WIN
} UIState;

static const char *RankGlyph(int r)
{
    switch (r)
    {
    case 1:
        return "A";
    case 11:
        return "J";
    case 12:
        return "Q";
    case 13:
        return "K";
    default:
    {
        static char buf[4];
        snprintf(buf, sizeof(buf), "%d", r);
        return buf;
    }
    }
}

static Color LerpColor(Color a, Color b, float t)
{
    Color c;
    c.r = (unsigned char)(a.r + (b.r - a.r) * t);
    c.g = (unsigned char)(a.g + (b.g - a.g) * t);
    c.b = (unsigned char)(a.b + (b.b - a.b) * t);
    c.a = (unsigned char)(a.a + (b.a - a.a) * t);
    return c;
}

static void DrawGradientBG(int w, int h)
{
    Color top = (Color){250, 252, 255, 255};
    Color bot = (Color){235, 242, 250, 255};

    for (int y = 0; y < h; y++)
    {
        float t = (float)y / (float)(h - 1);
        DrawLine(0, y, w, y, LerpColor(top, bot, t));
    }
}

static Color SuitColor(Suit s)
{
    if (s == SUIT_HEARTS)
        return (Color){220, 60, 70, 255};
    if (s == SUIT_DIAMONDS)
        return (Color){200, 40, 40, 255};
    return (Color){35, 35, 40, 255};
}

static void DrawHeart(Vector2 c, float s, Color col)
{
    float r = s * 0.22f;

    Vector2 pts[6] = {
        {c.x, c.y - s * 0.18f},
        {c.x - s * 0.38f, c.y - s * 0.30f},
        {c.x - s * 0.48f, c.y + s * 0.02f},
        {c.x, c.y + s * 0.62f},
        {c.x + s * 0.48f, c.y + s * 0.02f},
        {c.x + s * 0.38f, c.y - s * 0.30f}};
    DrawTriangleFan(pts, 6, col);

    DrawCircleV((Vector2){c.x - s * 0.22f, c.y - s * 0.18f}, r, col);
    DrawCircleV((Vector2){c.x + s * 0.22f, c.y - s * 0.18f}, r, col);
}

static void DrawDiamond(Vector2 c, float s, Color col)
{

    float radius = s * 0.65f;
    DrawPoly(c, 4, radius, 90.0f, col);
}

static void DrawClub(Vector2 c, float s, Color col)
{
    float r = s * 0.26f;

    Vector2 top = (Vector2){c.x, c.y - r * 1.20f};
    Vector2 left = (Vector2){c.x - r, c.y + r * 0.15f};
    Vector2 right = (Vector2){c.x + r, c.y + r * 0.15f};

    DrawCircleV(top, r, col);
    DrawCircleV(left, r, col);
    DrawCircleV(right, r, col);

    Rectangle stem = (Rectangle){c.x - r * 0.33f, c.y + r * 0.62f, r * 0.66f, s * 0.62f};
    DrawRectangleRounded(stem, 0.4f, 8, col);
}

static void DrawSpade(Vector2 c, float s, Color col)
{
    float r = s * 0.26f;

    Vector2 left = (Vector2){c.x - r, c.y + r * 0.10f};
    Vector2 right = (Vector2){c.x + r, c.y + r * 0.10f};
    DrawCircleV(left, r, col);
    DrawCircleV(right, r, col);

    Vector2 p1 = (Vector2){c.x - s * 0.60f, c.y + r * 0.25f};
    Vector2 p2 = (Vector2){c.x + s * 0.60f, c.y + r * 0.25f};
    Vector2 p3 = (Vector2){c.x, c.y - s * 0.72f};
    DrawTriangle(p1, p2, p3, col);

    Rectangle stem = (Rectangle){c.x - r * 0.33f, c.y + r * 0.78f, r * 0.66f, s * 0.62f};
    DrawRectangleRounded(stem, 0.4f, 8, col);
}

static void DrawSuitIcon(Suit s, Vector2 center, float size, Color col)
{
    switch (s)
    {
    case SUIT_CLUBS:
        DrawClub(center, size, col);
        break;
    case SUIT_DIAMONDS:
        DrawDiamond(center, size, col);
        break;
    case SUIT_HEARTS:
        DrawHeart(center, size, col);
        break;
    case SUIT_SPADES:
        DrawSpade(center, size, col);
        break;
    default:
        DrawCircleV(center, size * 0.25f, col);
        break;
    }
}

static void DrawRoundedLinesThick(Rectangle r, float roundness, int segments, int thickness, Color color)
{
    if (thickness < 1)
        thickness = 1;

    for (int i = 0; i < thickness; i++)
    {
        Rectangle rr = (Rectangle){r.x + i, r.y + i, r.width - 2 * i, r.height - 2 * i};
        if (rr.width <= 0 || rr.height <= 0)
            break;
        DrawRectangleRoundedLines(rr, roundness, segments, color);
    }
}

static void DrawButton(Rectangle r, const char *text, int enabled, int hovered)
{
    Color bg = enabled ? (Color){245, 247, 250, 255} : (Color){230, 230, 230, 255};
    Color bd = enabled ? (Color){140, 150, 165, 255} : (Color){180, 180, 180, 255};
    Color tx = enabled ? (Color){25, 25, 30, 255} : (Color){140, 140, 140, 255};

    if (enabled && hovered)
        bg = (Color){255, 255, 255, 255};

    DrawRectangleRounded((Rectangle){r.x + 3, r.y + 4, r.width, r.height}, 0.25f, 10, (Color){0, 0, 0, 35});
    DrawRectangleRounded(r, 0.25f, 10, bg);
    DrawRoundedLinesThick(r, 0.25f, 10, 2, bd);

    int fs = 22;
    int tw = MeasureText(text, fs);
    DrawText(text, (int)(r.x + (r.width - tw) / 2), (int)(r.y + (r.height - fs) / 2), fs, tx);
}

static void DrawPill(int x, int y, const char *text)
{
    int fs = 20;
    int tw = MeasureText(text, fs);
    Rectangle r = (Rectangle){(float)x, (float)y, (float)(tw + 22), 34};
    DrawRectangleRounded(r, 0.5f, 12, (Color){255, 255, 255, 220});
    DrawRectangleRoundedLines(r, 0.5f, 12, (Color){170, 180, 195, 255});
    DrawText(text, x + 11, y + 7, fs, (Color){35, 35, 45, 255});
}

static void DrawCardUI(Rectangle r, Card c, int selected, int hovered)
{
    DrawRectangleRounded((Rectangle){r.x + 5, r.y + 6, r.width, r.height}, 0.12f, 12, (Color){0, 0, 0, 35});

    Color face = (Color){255, 255, 255, 255};
    Color border = (Color){160, 170, 185, 255};
    if (hovered)
        border = (Color){95, 120, 255, 255};
    if (selected)
        border = (Color){255, 85, 85, 255};

    DrawRectangleRounded(r, 0.12f, 12, face);
    DrawRoundedLinesThick(r, 0.12f, 12, 2, border);

    if (IsEmptyCard(c))
    {
        DrawText("EMPTY", (int)r.x + 42, (int)r.y + 105, 18, (Color){140, 140, 140, 255});
        return;
    }

    Color sc = SuitColor(c.suit);

    Vector2 iconPos = (Vector2){r.x + 34, r.y + 40};
    DrawSuitIcon(c.suit, iconPos, 30, sc);

    const char *rk = RankGlyph(c.rank);
    DrawText(rk, (int)r.x + 60, (int)r.y + 28, 30, (Color){30, 30, 35, 255});

    Vector2 bigIcon = (Vector2){r.x + r.width * 0.64f, r.y + r.height * 0.62f};
    DrawSuitIcon(c.suit, bigIcon, 70, (Color){sc.r, sc.g, sc.b, 255});
}

static void DrawSuitPickButton(Rectangle r, Suit suit, int hovered)
{
    Color bg = hovered ? (Color){255, 255, 255, 255} : (Color){245, 247, 250, 255};
    Color bd = (Color){140, 150, 165, 255};

    DrawRectangleRounded((Rectangle){r.x + 3, r.y + 4, r.width, r.height}, 0.25f, 10, (Color){0, 0, 0, 35});
    DrawRectangleRounded(r, 0.25f, 10, bg);
    DrawRoundedLinesThick(r, 0.25f, 10, 2, bd);

    Vector2 c = (Vector2){r.x + r.width / 2.0f, r.y + r.height / 2.0f};
    DrawSuitIcon(suit, c, 34, SuitColor(suit));
}

static void DrawVolumeUI(Rectangle slider, Rectangle iconBtn, float volume01, int muted)
{
    DrawRectangleRounded((Rectangle){slider.x - 10, slider.y - 10, slider.width + iconBtn.width + 26, slider.height + 20},
                         0.25f, 10, (Color){255, 255, 255, 200});
    DrawRectangleRoundedLines((Rectangle){slider.x - 10, slider.y - 10, slider.width + iconBtn.width + 26, slider.height + 20},
                              0.25f, 10, (Color){170, 180, 195, 255});

    DrawRectangleRounded(slider, 0.4f, 10, (Color){235, 235, 240, 255});
    DrawRectangleRoundedLines(slider, 0.4f, 10, (Color){170, 180, 195, 255});

    float fillW = slider.width * volume01;
    DrawRectangleRounded((Rectangle){slider.x, slider.y, fillW, slider.height}, 0.4f, 10, (Color){120, 140, 255, 255});

    float knobX = slider.x + fillW;
    if (knobX < slider.x)
        knobX = slider.x;
    if (knobX > slider.x + slider.width)
        knobX = slider.x + slider.width;
    DrawCircle((int)knobX, (int)(slider.y + slider.height / 2), 10, (Color){30, 30, 35, 255});

    Color bg = (Color){245, 247, 250, 255};
    DrawRectangleRounded(iconBtn, 0.25f, 10, bg);
    DrawRectangleRoundedLines(iconBtn, 0.25f, 10, (Color){170, 180, 195, 255});

    int cx = (int)(iconBtn.x + 18);
    int cy = (int)(iconBtn.y + iconBtn.height / 2);
    DrawRectangle(cx - 10, cy - 6, 6, 12, (Color){30, 30, 35, 255});
    DrawTriangle((Vector2){cx - 4, cy - 8}, (Vector2){cx + 8, cy - 2}, (Vector2){cx - 4, cy + 8}, (Color){30, 30, 35, 255});

    if (muted)
    {
        DrawLine((int)iconBtn.x + 10, (int)iconBtn.y + 10, (int)(iconBtn.x + iconBtn.width - 10), (int)(iconBtn.y + iconBtn.height - 10), (Color){220, 60, 70, 255});
        DrawLine((int)(iconBtn.x + iconBtn.width - 10), (int)iconBtn.y + 10, (int)iconBtn.x + 10, (int)(iconBtn.y + iconBtn.height - 10), (Color){220, 60, 70, 255});
    }
}

static void ResetLevel(
    double *score,
    int *round,
    int *redrawUsed,
    int selected[7],
    int *suitMode,
    int *suitPick,
    Deck **deck,
    Hand **hand)
{
    *score = 0.0;
    *round = 1;
    *redrawUsed = 0;
    *suitMode = 0;
    *suitPick = -1;
    ChainState chain;
    ResetChain(&chain);

    for (int i = 0; i < 7; i++)
        selected[i] = 0;

    DestroyHand(*hand);
    DestroyDeck(*deck);

    *deck = CreateDeck();
    ShuffleDeck(*deck);
    *hand = CreateHand(7);
    DrawUntilFull(*deck, *hand, 7);
}

static const char *MagicLabel(MagicCard m)
{
    switch (m)
    {
    case MAGIC_PAIR_UPGRADE:
        return "PAIR BONUS +3 (Permanent)";
    case MAGIC_SUIT_CHANGE:
        return "SUIT CHANGE (Once per round)";
    default:
        return "UNKNOWN";
    }
}

// -------------------- Main --------------------
int main(void)
{
    InitWindow(1200, 740, "Card Game - Final UI (Polished)");
    SetTargetFPS(60);
    srand((unsigned)time(NULL));

    Texture2D bgMain = LoadTexture("assets/bg_main.png");
    Texture2D moving = LoadTexture("assets/moving.png");

    InitAudioDevice();
    bool audioReady = IsAudioDeviceReady();

    const char *bgmPath = "assets/bgm.wav";
    Music bgm = (Music){0};
    bool bgmLoaded = false;

    const char *sfxPlayPath = "assets/play.wav";
    Sound sfxPlay = (Sound){0};
    bool sfxPlayLoaded = false;

    if (FileExists(bgmPath))
    {
        bgm = LoadMusicStream(bgmPath);
        PlayMusicStream(bgm);
        bgmLoaded = true;
    }

    if (audioReady && FileExists(sfxPlayPath))
    {
        sfxPlay = LoadSound(sfxPlayPath);
        sfxPlayLoaded = true;
    }

    float volume = 0.6f;
    int muted = 0;
    float prevVolume = volume;

    // ---------- GAME ----------

    MagicState ms = {0};
    ChainState chain;
    ResetChain(&chain);
    UIState ui = UI_TITLE;

    int level = 1;
    double target = LEVEL_TARGETS[level - 1];
    double score = 0.0;
    int round = 1;

    Deck *deck = CreateDeck();
    ShuffleDeck(deck);

    Hand *hand = CreateHand(7);
    DrawUntilFull(deck, hand, 7);

    int selected[7] = {0};
    int redrawUsed = 0;

    int suitMode = 0;
    int suitPick = -1;

    MagicCard magicA, magicB;
    int magicReady = 0;
    int nextLevel = 2;

    while (!WindowShouldClose())
    {
        Vector2 mouse = GetMousePosition();

        if (audioReady && bgmLoaded)
        {
            UpdateMusicStream(bgm);
            SetMusicVolume(bgm, muted ? 0.0f : volume);
        }

        Rectangle volSlider = (Rectangle){930, 20, 200, 18};
        Rectangle volIcon = (Rectangle){1140, 12, 48, 34};

        bool draggingSlider = IsMouseButtonDown(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(mouse, volSlider);
        if (draggingSlider)
        {
            float t = (mouse.x - volSlider.x) / volSlider.width;
            if (t < 0)
                t = 0;
            if (t > 1)
                t = 1;
            volume = t;
            if (volume > 0.001f)
            {
                prevVolume = volume;
                muted = 0;
            }
        }

        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(mouse, volIcon))
        {
            if (!muted)
                muted = 1;
            else
            {
                muted = 0;
                if (volume <= 0.001f)
                    volume = (prevVolume > 0.001f ? prevVolume : 0.6f);
            }
        }

        BeginDrawing();
        DrawGradientBG(GetScreenWidth(), GetScreenHeight());
        DrawTexture(bgMain, 0, 0, WHITE);
        static float bgOffset = 0;
        bgOffset += 0.05f;
        DrawTexture(moving, (int)bgOffset % bgMain.width, 0, WHITE);
        DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(),
                      (Color){255, 255, 255, 150});

        DrawVolumeUI(volSlider, volIcon, volume, muted);

        if (ui == UI_TITLE)
        {
            DrawText("CARD GAME", 420, 160, 64, (Color){25, 25, 35, 255});
            DrawText("Poker + Strategy", 470, 230, 24, (Color){80, 80, 100, 255});

            Rectangle btnStart = (Rectangle){460, 320, 280, 70};
            Rectangle btnRules = (Rectangle){460, 410, 280, 70};
            Rectangle btnAdv = (Rectangle){460, 500, 280, 70};
            int hStart = CheckCollisionPointRec(mouse, btnStart);
            int hRules = CheckCollisionPointRec(mouse, btnRules);
            int hAdv = CheckCollisionPointRec(mouse, btnAdv);
            DrawButton(btnStart, "START GAME", 1, hStart);
            DrawButton(btnRules, "HOW TO PLAY", 1, hRules);
            DrawButton(btnAdv, "ADVANCED RULES", 1, hAdv);
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
            {
                if (CheckCollisionPointRec(mouse, btnStart))
                {
                    ui = UI_PLAYING;
                }
                else if (CheckCollisionPointRec(mouse, btnRules))
                {
                    ui = UI_RULES;
                }
                else if (CheckCollisionPointRec(mouse, btnAdv))
                    ui = UI_ADV_RULES;
            }
        }
        else if (ui == UI_RULES)
        {
            DrawText("HOW TO PLAY", 420, 120, 48, (Color){25, 25, 35, 255});

            DrawText(
                "1. Select up to 5 cards\n"
                "\n"
                "2. Press PLAY to score hands\n"
                "\n"
                "3. Poker hands give points\n"
                "\n"
                "4. Earn gold to buy upgrades\n"
                "\n"
                "5. Reach target score to win\n"
                "\n"
                "6. We have a lot of hand type ,such as \n"
                "  Single 1 point,  Pair 2 points,  Straight 5 points, \n"
                "  Flush 6 points,  Full House 8 points,\n"
                "  Four-of-a-Kind 10 points,  Straight Flush 12 points\n",
                300, 220, 26, (Color){50, 50, 70, 255});

            Rectangle btnBack = (Rectangle){460, 650, 280, 70};
            int hBack = CheckCollisionPointRec(mouse, btnBack);
            DrawButton(btnBack, "BACK", 1, hBack);

            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) &&
                CheckCollisionPointRec(mouse, btnBack))
            {
                ui = UI_TITLE;
            }
        }
        else if (ui == UI_ADV_RULES)
        {
            DrawText("ADVANCED RULES", 380, 120, 48, (Color){25, 25, 35, 255});

            DrawText(
                "1. Chaining Score Multiplier\n"
                "\n"
                "   Playing specific hands after another\n"
                "   results in a score multiplier.\n"
                "\n"
                "   Valid chains in this build:\n"
                "   - Pair after Pair\n"
                "   - Full House after Pair\n"
                "   - Four-of-a-Kind after Pair\n"
                "\n"
                "   Each successful chain increases the multiplier\n"
                "   for the next score (up to a cap).\n",
                250, 240, 26, (Color){50, 50, 70, 255});

            Rectangle btnBack = (Rectangle){460, 650, 280, 70};
            int hBack = CheckCollisionPointRec(mouse, btnBack);
            DrawButton(btnBack, "BACK", 1, hBack);

            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(mouse, btnBack))
            {
                ui = UI_TITLE;
            }
        }

        // ================= PLAYING =================
        else if (ui == UI_PLAYING)
        {
            DrawPill(30, 20, TextFormat("Level %d", level));
            DrawPill(150, 20, TextFormat("Round %d", round));
            DrawPill(30, 60, TextFormat("Score %.1f / %.1f", score, target));
            DrawPill(30, 100, TextFormat("Gold %d", ms.gold));
            DrawPill(30, 140, TextFormat("Deck %d", deck->size));

            DrawPill(30, 190, TextFormat("Pair Bonus +%.1f", ms.pairBonus));
            DrawPill(230, 190, TextFormat("Suit Change %s", ms.hasSuitChange ? "YES" : "NO"));
            DrawPill(460, 190, TextFormat("Redraw %s", ms.redrawPerLevel ? "YES" : "NO"));

            Rectangle btnPlay = (Rectangle){30, 270, 170, 62};
            Rectangle btnRedraw = (Rectangle){220, 270, 170, 62};
            Rectangle btnSuit = (Rectangle){410, 270, 210, 62};

            int hPlay = CheckCollisionPointRec(mouse, btnPlay);
            int hRedraw = CheckCollisionPointRec(mouse, btnRedraw);
            int hSuit = CheckCollisionPointRec(mouse, btnSuit);

            DrawButton(btnPlay, "PLAY", 1, hPlay);
            DrawButton(btnRedraw, "REDRAW", (ms.redrawPerLevel && !redrawUsed), hRedraw);
            DrawButton(btnSuit, suitMode ? "SUIT MODE ON" : "SUIT CHANGE", ms.hasSuitChange, hSuit);

            DrawText("Hand", 30, 360, 28, (Color){25, 25, 35, 255});

            Rectangle cardR[7];
            for (int i = 0; i < 7; i++)
                cardR[i] = (Rectangle){30 + i * 165.0f, 400, 150, 240};

            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
            {
                for (int i = 0; i < 7; i++)
                {
                    if (CheckCollisionPointRec(mouse, cardR[i]) && !IsEmptyCard(hand->cards[i]))
                    {
                        if (suitMode)
                            suitPick = i;
                        else
                            selected[i] = !selected[i];
                    }
                }

                if (CheckCollisionPointRec(mouse, btnPlay))
                {
                    int idx[5], cnt = 0;
                    Card ch[5];

                    for (int i = 0; i < 7 && cnt < 5; i++)
                    {
                        if (selected[i] && !IsEmptyCard(hand->cards[i]))
                        {
                            idx[cnt] = i;
                            ch[cnt++] = hand->cards[i];
                        }
                    }

                    if (cnt > 0)
                    {
                        HandType ht = EvaluateHand(ch, cnt);
                        double g = GetScoreChained(level, ht, &ms, &chain);

                        if (audioReady && sfxPlayLoaded && !muted)
                        {

                            SetSoundVolume(sfxPlay, 0.9f * volume);
                            PlaySound(sfxPlay);
                        }

                        RemoveSelectedCardsFromHand(hand, idx, cnt);
                        DrawUntilFull(deck, hand, 7);

                        if (g > 0.0)
                        {
                            if (audioReady && sfxPlayLoaded)
                            {
                                SetSoundVolume(sfxPlay, muted ? 0.0f : volume);
                                PlaySound(sfxPlay);
                            }
                            ms.gold += EarnGoldFromScore(g);
                            score += g;
                        }

                        round++;
                        for (int i = 0; i < 7; i++)
                            selected[i] = 0;
                    }
                }

                if (CheckCollisionPointRec(mouse, btnRedraw) && ms.redrawPerLevel && !redrawUsed)
                {
                    for (int i = 0; i < 7; i++)
                        hand->cards[i] = MakeEmptyCard();
                    DrawUntilFull(deck, hand, 7);
                    redrawUsed = 1;
                    for (int i = 0; i < 7; i++)
                        selected[i] = 0;
                }

                if (CheckCollisionPointRec(mouse, btnSuit) && ms.hasSuitChange)
                {
                    suitMode = !suitMode;
                    suitPick = -1;
                }
            }

            for (int i = 0; i < 7; i++)
            {
                int hovered = CheckCollisionPointRec(mouse, cardR[i]);
                DrawCardUI(cardR[i], hand->cards[i], selected[i], hovered);
            }

            if (suitMode)
            {
                DrawText("Suit Change: click a card, then choose a suit", 660, 280, 20, (Color){40, 40, 55, 255});

                if (suitPick >= 0)
                {
                    DrawText(TextFormat("Selected card index: %d", suitPick), 660, 310, 20, (Color){40, 40, 55, 255});

                    Rectangle s0 = (Rectangle){660, 340, 70, 60};
                    Rectangle s1 = (Rectangle){740, 340, 70, 60};
                    Rectangle s2 = (Rectangle){820, 340, 70, 60};
                    Rectangle s3 = (Rectangle){900, 340, 70, 60};

                    int hs0 = CheckCollisionPointRec(mouse, s0);
                    int hs1 = CheckCollisionPointRec(mouse, s1);
                    int hs2 = CheckCollisionPointRec(mouse, s2);
                    int hs3 = CheckCollisionPointRec(mouse, s3);

                    DrawSuitPickButton(s0, SUIT_CLUBS, hs0);
                    DrawSuitPickButton(s1, SUIT_DIAMONDS, hs1);
                    DrawSuitPickButton(s2, SUIT_HEARTS, hs2);
                    DrawSuitPickButton(s3, SUIT_SPADES, hs3);

                    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
                    {
                        int picked = 0;
                        if (CheckCollisionPointRec(mouse, s0))
                        {
                            hand->cards[suitPick].suit = SUIT_CLUBS;
                            picked = 1;
                        }
                        else if (CheckCollisionPointRec(mouse, s1))
                        {
                            hand->cards[suitPick].suit = SUIT_DIAMONDS;
                            picked = 1;
                        }
                        else if (CheckCollisionPointRec(mouse, s2))
                        {
                            hand->cards[suitPick].suit = SUIT_HEARTS;
                            picked = 1;
                        }
                        else if (CheckCollisionPointRec(mouse, s3))
                        {
                            hand->cards[suitPick].suit = SUIT_SPADES;
                            picked = 1;
                        }

                        if (picked)
                        {
                            suitMode = 0;
                            suitPick = -1;
                        }
                    }
                }
            }

            if (score >= target)
            {
                if (level >= 3)
                    ui = UI_WIN;
                else
                {
                    ui = UI_MAGIC_CHOICE;
                    magicReady = 0;
                    nextLevel = level + 1;
                }
            }

            if (deck && hand && deck->size <= 0 && !HandHasAnyCard(hand) && score < target)
            {
                ui = UI_LOSE;
            }
        }
        else if (ui == UI_MAGIC_CHOICE)
        {
            if (!magicReady)
            {
                DrawTwoMagicCards(&magicA, &magicB);
                magicReady = 1;
            }

            DrawText("MAGIC CHOICE", 450, 120, 48, (Color){25, 25, 35, 255});
            DrawText("Pick ONE permanent upgrade", 455, 175, 22, (Color){60, 60, 80, 255});

            Rectangle a = (Rectangle){220, 260, 760, 90};
            Rectangle b = (Rectangle){220, 380, 760, 90};

            int ha = CheckCollisionPointRec(mouse, a);
            int hb = CheckCollisionPointRec(mouse, b);

            DrawButton(a, MagicLabel(magicA), 1, ha);
            DrawButton(b, MagicLabel(magicB), 1, hb);

            DrawPill(30, 20, TextFormat("Gold: %d", ms.gold));
            DrawPill(30, 60, TextFormat("Pair Bonus: +%.1f", ms.pairBonus));

            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
            {
                if (CheckCollisionPointRec(mouse, a))
                {
                    ApplyMagicCard(&ms, magicA);
                    ui = UI_SHOP;
                }
                else if (CheckCollisionPointRec(mouse, b))
                {
                    ApplyMagicCard(&ms, magicB);
                    ui = UI_SHOP;
                }
            }
        }
        else if (ui == UI_SHOP)
        {
            DrawText("SHOP", 540, 110, 56, (Color){25, 25, 35, 255});
            DrawText("Spend Gold to buy upgrades", 455, 175, 22, (Color){60, 60, 80, 255});

            DrawPill(30, 20, TextFormat("Gold: %d", ms.gold));
            DrawPill(30, 60, TextFormat("Pair Bonus: +%.1f", ms.pairBonus));
            DrawPill(30, 100, TextFormat("Suit Change: %s", ms.hasSuitChange ? "YES" : "NO"));
            DrawPill(30, 140, TextFormat("Redraw: %s", ms.redrawPerLevel ? "YES" : "NO"));

            Rectangle r1 = (Rectangle){260, 260, 720, 78};
            Rectangle r2 = (Rectangle){260, 360, 720, 78};
            Rectangle r3 = (Rectangle){260, 460, 720, 78};
            Rectangle r4 = (Rectangle){420, 590, 380, 76};

            int canR = (!ms.redrawPerLevel) && (ms.gold >= PRICE_REDRAW);
            int canS = (!ms.hasSuitChange) && (ms.gold >= PRICE_SUIT);
            int canP = (ms.gold >= PRICE_PAIR);

            int h1 = CheckCollisionPointRec(mouse, r1);
            int h2 = CheckCollisionPointRec(mouse, r2);
            int h3 = CheckCollisionPointRec(mouse, r3);
            int h4 = CheckCollisionPointRec(mouse, r4);

            char t1[128], t2[128], t3[128];
            snprintf(t1, sizeof(t1), "REDRAW (Once per level) - %dG%s", PRICE_REDRAW, ms.redrawPerLevel ? "  [OWNED]" : "");
            snprintf(t2, sizeof(t2), "SUIT CHANGE (Once per round) - %dG%s", PRICE_SUIT, ms.hasSuitChange ? "  [OWNED]" : "");
            snprintf(t3, sizeof(t3), "PAIR BONUS +3 - %dG  (Now +%.1f)", PRICE_PAIR, ms.pairBonus);

            DrawButton(r1, t1, canR, h1);
            DrawButton(r2, t2, canS, h2);
            DrawButton(r3, t3, canP, h3);
            DrawButton(r4, "CONTINUE", 1, h4);

            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
            {
                if (CheckCollisionPointRec(mouse, r1) && canR)
                {
                    ms.gold -= PRICE_REDRAW;
                    ms.redrawPerLevel = 1;
                }
                else if (CheckCollisionPointRec(mouse, r2) && canS)
                {
                    ms.gold -= PRICE_SUIT;
                    ms.hasSuitChange = 1;
                }
                else if (CheckCollisionPointRec(mouse, r3) && canP)
                {
                    ms.gold -= PRICE_PAIR;
                    ms.pairBonus += 3.0;
                }
                else if (CheckCollisionPointRec(mouse, r4))
                {
                    level = nextLevel;
                    target = LEVEL_TARGETS[level - 1];
                    ResetLevel(&score, &round, &redrawUsed, selected, &suitMode, &suitPick, &deck, &hand);
                    ResetChain(&chain);
                    ui = UI_PLAYING;
                }
            }
        }
        else if (ui == UI_WIN)
        {
            DrawText("YOU WIN!", 430, 240, 70, (Color){20, 130, 70, 255});
            DrawText(TextFormat("Final Gold: %d", ms.gold), 480, 330, 26, (Color){40, 40, 55, 255});
            DrawText("Close the window to exit.", 440, 380, 24, (Color){80, 80, 95, 255});
        }
        else if (ui == UI_LOSE)
        {
            DrawText("YOU LOSE", 430, 220, 70, (Color){200, 60, 70, 255});
            DrawText(TextFormat("Score %.1f / %.1f", score, target), 450, 310, 26, (Color){40, 40, 55, 255});
            DrawText("You ran out of cards.", 455, 350, 24, (Color){80, 80, 95, 255});

            Rectangle btnAgain = (Rectangle){460, 430, 280, 70};
            int hAgain = CheckCollisionPointRec(mouse, btnAgain);
            DrawButton(btnAgain, "PLAY AGAIN", 1, hAgain);

            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(mouse, btnAgain))
            {
                // Full restart
                level = 1;
                nextLevel = 2;
                target = LEVEL_TARGETS[0];
                ms = (MagicState){0};

                ResetLevel(&score, &round, &redrawUsed, selected, &suitMode, &suitPick, &deck, &hand);

                ui = UI_TITLE;
            }
        }

        EndDrawing();
    }

    // cleanup
    if (sfxPlayLoaded)
        UnloadSound(sfxPlay);
    if (bgmLoaded)
        UnloadMusicStream(bgm);
    if (audioReady)
        CloseAudioDevice();
    UnloadTexture(moving);
    UnloadTexture(bgMain);
    DestroyHand(hand);
    DestroyDeck(deck);
    CloseWindow();
    return 0;
}
