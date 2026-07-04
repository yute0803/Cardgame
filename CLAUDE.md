# Cardgame — 給每個 Claude session 的入口檔

C 語言撲克 roguelike（類 Balatro）：抽牌、湊牌型得分、過關進商店買魔法卡。約 1800 行。

## 硬事實（先接受，不要重新驗證）

- 目標平台是 **Windows**（raylib UI + mingw）；你所在的容器是 **Linux**，`game_ui.exe` 在這裡跑不起來。
- 這是 remote ephemeral 環境：**沒 commit + push 的工作等於沒做**。每完成一個獨立變更就 commit + push 到你被指定的分支。
- 邏輯模組（`cards.c` `hand_eval.c` `score.c` `magic.c`）在 Linux 可編譯可測試。CLI 版遊戲可在本地實跑：
  `gcc -std=c11 -O2 -Itools/winstub main.c cards.c hand_eval.c score.c magic.c -o /tmp/game_cli`
- Windows UI 版的建置指令（僅供寫給使用者參考，本容器不能執行）：
  `gcc -std=c11 -O2 ui_raylib.c cards.c hand_eval.c score.c magic.c -o game_ui.exe -mwindows -lraylib -lopengl32 -lgdi32 -lwinmm`

## 禁令

1. 不要 Read 任何 `.exe` / `.dll`。
2. `prompt.txt` 是舊版原始碼快照，不是文件：不要讀它、不要引用它的內容當現況。現行程式碼以 `.c` / `.h` 檔為準。
3. 不要用 `git add -A` 或 `git add .`（會把重編的二進位再 commit 進去）；一律 `git add <具體檔名>`。
4. 任何會影響 UI 版行為的改動（不只改 `ui_raylib.c`，也包括改它會顯示或使用的邏輯與數值）都不得宣稱「已測試」——本容器測不了 UI。回報時明寫：「UI 未實測，請在 Windows 上跑 game_ui.exe 確認」。
5. 改 `CLAUDE.md`、`.claude/docs/`、`.claude/agents/` 任何制度檔前，先讀 `.claude/docs/40-maintenance.md`。

## 檔案地圖

| 檔案 | 內容 |
|---|---|
| `cards.c/.h` | 牌堆、手牌、抽牌洗牌 |
| `hand_eval.c/.h` | 牌型判定（對子/順子/同花…） |
| `score.c/.h` | 分數計算 |
| `magic.c/.h` | 魔法卡效果與商店 |
| `main.c` | CLI 版入口（terminal 文字介面） |
| `ui_raylib.c` | 圖形 UI 版入口（866 行，最大檔；先 Grep 定位再局部 Read） |
| `tools/winstub/` | Linux 建置 CLI 版用的 windows.h 替身 |
| `web/cardgame.html` | 手機網頁版（由 C 邏輯逐函式移植；規則改動時需同步更新並重發 artifact） |

## 路由：什麼情況讀哪份文件

只在符合條件時才讀，不要開場全部讀一遍：

- **要派 subagent、或任務需要大量讀檔/掃 repo/查網頁** → 讀 `.claude/docs/10-delegation.md`
- **不確定「算不算做完」「該不該問使用者」「要不要換方法」** → 讀 `.claude/docs/20-judgment.md`
- **要寫派工 prompt** → 抄 `.claude/docs/30-templates.md` 的模板
- **要修改制度檔案本身** → 讀 `.claude/docs/40-maintenance.md`
- **session 開頭想了解這套制度的來龍去脈** → 讀 `.claude/docs/50-letter.md`（非必要）
- 踩到新坑或學到新教訓 → 按 `.claude/docs/40-maintenance.md` 的格式寫進 `.claude/docs/LESSONS.md`

## 基本工作紀律

- 大量讀取、掃 repo、查網頁、批次改檔 → 派 subagent，主對話只收結論（見 10-delegation.md）。
- 改完邏輯層程式碼 → 至少做到：gcc 編譯零 error + 跑一次 `/tmp/game_cli` 冒煙測試。**這只是最低下限**；完整的「完成」判準（含 verifier 驗收與 push）在 `.claude/docs/20-judgment.md` 第 2 節，宣稱完成前必讀。
- 回報時區分三種狀態，不要混用：「已實測通過」／「已編譯但未實測」／「無法在本容器驗證」。
