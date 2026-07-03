# 00 - Harness 快速診斷（2026-07-03，由 Fable 5 session 建立）

本檔是整套制度的依據。後面每份文件的規則都對應這裡的某個問題。
讀者：未來在此環境工作的任何 Claude session（預設 Sonnet 等級）。

## 環境事實（已實測，不要重新推導）

- 執行環境：Claude Code on the web 的 remote container（Linux），repo 每次 session 重新 clone，container 會被回收。
- **沒有 commit + push 的任何檔案，session 結束即消失**。`~/.claude/` 下的東西也不會跨 session 保留。
- 制度檔案唯一能存活的位置是 repo 本身：`CLAUDE.md`、`.claude/docs/`、`.claude/agents/`、`.claude/skills/` 都會隨 clone 載入（官方文件確認：repo 內的 `.claude/agents|skills|commands` 在 web 環境會載入；`~/.claude/` 使用者層級的不會）。
- 專案：C 語言撲克 roguelike（類 Balatro），共約 1800 行。目標平台是 **Windows**（raylib UI + mingw），但本容器是 Linux。
- GitHub 操作只能用 `mcp__github__*` 工具，沒有 `gh` CLI。

## 前三名問題與修法

### 第 1 名：零制度——沒有 CLAUDE.md，每個 session 從零重新摸索

**症狀**：repo 唯一的 commit 是「Add files via upload」。沒有任何檔案告訴新 session：這是什麼專案、怎麼建置、什麼能在這裡驗證、什麼不能。每個 session 都要重新 ls、重新讀原始碼、重新發現「喔原來這是 Windows 專案」。以本次 session 實測，光是重建這些事實就花了約 10 次工具呼叫。

**修法**（已執行）：建立 `CLAUDE.md`（精簡路由）+ `.claude/docs/`（詳細規則）。CLAUDE.md 只放「每個 session 都需要」的硬事實與路由；細節放獨立檔案，需要時才讀，省 token。

### 第 2 名：Repo 裡 5.4 MB 二進位檔 + `prompt.txt` 假文件

**症狀**：
- `game.exe`、`game_ui.exe`、5 個 `.dll`、`mingw64.exe` 共約 5.4 MB 被版控追蹤。任何 `git add -A` 都會把重編的二進位再 commit 一次；粗心的 Grep/Read 會撞上它們。
- `prompt.txt`（24 KB）看起來像文件，**其實是 `ui_raylib.c` 某個舊版本的原始碼快照**。弱模型被指示「先讀專案文件」時會整份讀進 context（約 7000 token），更糟的是把裡面的舊程式碼當成現況。

**修法**：
- CLAUDE.md 明文：禁止 Read 任何 `.exe`/`.dll`；`prompt.txt` 是歷史快照、僅供考古、禁止當作現行程式碼引用。
- 永遠用 `git add <具體檔名>`，禁止 `git add -A` / `git add .`。
- （需使用者決定，未執行）建議日後把二進位移出版控：加 `.gitignore` + `git rm --cached *.exe *.dll`。在使用者同意前不要動。

### 第 3 名：驗證斷層——Windows 目標程式在 Linux 容器裡，弱模型會假裝測過或亂繞路

**症狀**：`game_ui.exe` 在這裡跑不起來，raylib 標頭也不存在。弱模型面對「改完請測試」有兩種典型失敗：(a) 沒跑就宣稱「已測試通過」；(b) 燒大量 token 嘗試裝 raylib / wine（網路政策下多半失敗）。

**修法**（已實測可行，工具已入庫）：
- 邏輯層四個模組（`cards.c`、`hand_eval.c`、`score.c`、`magic.c`）在 Linux 用 gcc 編譯**零錯誤**，可直接寫單元測試驗證。
- CLI 版遊戲可用 `tools/winstub/windows.h`（幾行 stub）在 Linux 建置並實跑：
  `gcc -std=c11 -O2 -Itools/winstub main.c cards.c hand_eval.c score.c magic.c -o /tmp/game_cli`
- `ui_raylib.c` 在本容器**無法建置也無法執行**，只能人工 code review。凡是動到 UI 的改動，回報時必須明寫「UI 未實測，需使用者在 Windows 上跑 game_ui.exe 確認」，不得宣稱測試通過。
- 詳細驗證判準見 `.claude/docs/20-judgment.md` 的「完成的定義」。

## 次要問題（一句話帶過）

- `ui_raylib.c` 866 行是最大原始檔：改它之前先用 Grep 定位函式再做局部 Read，不要整檔讀進主對話。
- 主對話容易被大量檔案內容灌爆：大範圍讀取/掃描一律派 subagent，見 `.claude/docs/10-delegation.md`。
- push 紀律：每完成一個獨立變更就 commit + push；container 隨時可能被回收。
