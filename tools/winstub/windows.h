/* Linux 用的最小 windows.h 替身，讓 main.c（CLI 版遊戲）能在本容器建置與實跑。
   用法： gcc -std=c11 -O2 -Itools/winstub main.c cards.c hand_eval.c score.c magic.c -o /tmp/game_cli
   注意：只 stub 了 main.c 實際用到的 API；若日後 main.c 新增 Windows API，在這裡補對應的空實作。 */
#pragma once
#include <unistd.h>
#define Sleep(ms) usleep((ms) * 1000)
#define CP_UTF8 65001
static inline void SetConsoleOutputCP(unsigned int cp) { (void)cp; }
static inline void SetConsoleCP(unsigned int cp) { (void)cp; }
