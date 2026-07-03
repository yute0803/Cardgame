# 教訓日誌（格式見 40-maintenance.md 第 3 節；只增不刪，滿 40 條觸發精簡程序）

## 2026-07-03 Write 工具偶發 permission stream 錯誤
坑：Write 工具連續回報「Tool permission stream closed before response received」，重試同樣失敗（疑似 MCP 重連期間的暫時故障）。
解：改用 Bash heredoc（`cat > 檔案 << 'EOF'`）寫檔可正常運作。遇到同樣錯誤先重試一次，再失敗就換 heredoc。

## 2026-07-03 main.c 在 Linux 編譯卡 windows.h
坑：`fatal error: windows.h: No such file or directory`；main.c 只用到 Sleep 與 SetConsoleOutputCP/SetConsoleCP。
解：用 `-Itools/winstub` 引入 repo 內建 stub 即可建置並實跑 CLI 版。完整指令在 CLAUDE.md。
