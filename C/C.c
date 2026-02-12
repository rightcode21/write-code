#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <windows.h>
#include <conio.h>
#include <string.h>

#define MAX_CARDS 52
#define ENTER 13

// ================= 구조체 =================
typedef struct {
    int shape; 
    int number; 
    int value;  
} Card;

typedef struct {
    char name[30];
    int hp;
    int max_hp;
    int gold;
    int attack_power;
    Card hand[10];
    int card_count;
    // -- 추가 인벤토리 --
    int potions;    // 체력 물약 개수
    int xray_specs; // 투시 안경 개수
} Entity;

// ================= 전역 변수 =================
Card deck[MAX_CARDS];
int deck_index = 0;
int is_xray_active = 0; // 투시 안경 활성화 여부

// ================= 화면 제어 및 연출 함수 =================
void gotoxy(int x, int y) {
    COORD pos = { x, y };
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
}

void setColor(int color) {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}

void hideCursor() {
    CONSOLE_CURSOR_INFO cursorInfo = { 0, };
    cursorInfo.dwSize = 1;
    cursorInfo.bVisible = FALSE;
    SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursorInfo);
}

// [연출] 타이핑 효과 함수
void typingPrint(const char* str, int speed) {
    for (int i = 0; str[i] != '\0'; i++) {
        printf("%c", str[i]);
        Sleep(speed);
    }
    printf("\n");
}

// [연출] 몬스터 아스키 아트 출력
void printMonsterArt(int x, int y, char* name) {
    setColor(12);
    if (strstr(name, "슬라임")) {
        gotoxy(x, y);     printf("       .-.       ");
        gotoxy(x, y + 1); printf("      (o.o)      ");
        gotoxy(x, y + 2); printf("       |=|       ");
        gotoxy(x, y + 3); printf("      __|__      ");
    }
    else if (strstr(name, "오크")) {
        gotoxy(x, y);     printf("      ,     ,    ");
        gotoxy(x, y + 1); printf("     (  o.o  )   ");
        gotoxy(x, y + 2); printf("     /(  _  )\\   ");
        gotoxy(x, y + 3); printf("      ^^   ^^    ");
    }
    else if (strstr(name, "드래곤")) { 
        setColor(13); 
        gotoxy(x, y);     printf("   <>=======()   ");
        gotoxy(x, y + 1); printf("  (/\\ _   _ /\\)  ");
        gotoxy(x, y + 2); printf("   \\_  ( )  _/   ");
        gotoxy(x, y + 3); printf("     / ^^^ \\     ");
    }
    setColor(7); 
}

// ================= 게임 로직 함수 =================
void initDeck() {
    int i, j = 0;
    for (i = 0; i < 4; i++) {
        for (j = 1; j <= 13; j++) {
            deck[i * 13 + (j - 1)].shape = i;
            deck[i * 13 + (j - 1)].number = j;
            if (j >= 11) deck[i * 13 + (j - 1)].value = 10;
            else deck[i * 13 + (j - 1)].value = j;
        }
    }
}

void shuffleDeck() {
    for (int i = 0; i < MAX_CARDS; i++) {
        int dest = rand() % MAX_CARDS;
        Card temp = deck[i];
        deck[i] = deck[dest];
        deck[dest] = temp;
    }
    deck_index = 0;
}

Card drawCard() {
    if (deck_index >= MAX_CARDS) shuffleDeck();
    return deck[deck_index++];
}

int calculateScore(Entity* e) {
    int score = 0;
    int ace_count = 0;
    for (int i = 0; i < e->card_count; i++) {
        score += e->hand[i].value;
        if (e->hand[i].number == 1) ace_count++;
    }
    while (ace_count > 0 && score <= 11) {
        score += 10;
        ace_count--;
    }
    return score;
}

// [핵심] 카드 그리기 (투시 안경 적용)
void printCard(int x, int y, Card c, int hidden) {
    gotoxy(x, y);     printf("┌───┐");
    gotoxy(x, y + 1);

    // 숨겨진 카드인데 투시 안경 켜져 있으면 -> 보라색
    if (hidden && is_xray_active) {
        setColor(13); 
        char shapeChar[3];
        if (c.shape == 0) sprintf(shapeChar, "♠");
        else if (c.shape == 1) sprintf(shapeChar, "◆");
        else if (c.shape == 2) sprintf(shapeChar, "♥");
        else sprintf(shapeChar, "♣");

        char numChar[3];
        if (c.number == 1) sprintf(numChar, "A");
        else if (c.number == 11) sprintf(numChar, "J");
        else if (c.number == 12) sprintf(numChar, "Q");
        else if (c.number == 13) sprintf(numChar, "K");
        else sprintf(numChar, "%d", c.number);

        printf("│%s%s│", shapeChar, numChar);
        setColor(7);
    }
    else if (hidden) {
        printf("│ ? │");
    }
    else {
        char shapeChar[3];
        if (c.shape == 0) sprintf(shapeChar, "♠");
        else if (c.shape == 1) sprintf(shapeChar, "◆");
        else if (c.shape == 2) sprintf(shapeChar, "♥");
        else sprintf(shapeChar, "♣");

        char numChar[3];
        if (c.number == 1) sprintf(numChar, "A");
        else if (c.number == 11) sprintf(numChar, "J");
        else if (c.number == 12) sprintf(numChar, "Q");
        else if (c.number == 13) sprintf(numChar, "K");
        else sprintf(numChar, "%d", c.number);

        printf("│%s%s│", shapeChar, numChar);
    }
    gotoxy(x, y + 2); printf("└───┘");
}

// 메인 전투 화면
void drawInterface(Entity p, Entity m, int turn) {
    system("cls");

    // 1. 몬스터 정보 & 아스키 아트
    setColor(14); 
    gotoxy(2, 1); printf("=== [ %s ] HP: %d / %d ===", m.name, m.hp, m.max_hp);
    setColor(7);
    printMonsterArt(5, 3, m.name); 

    // 2. 테이블
    gotoxy(2, 8); printf("------------ [ BLACKJACK TABLE ] ------------");

    // 3. 딜러 카드
    gotoxy(2, 10); printf("Monster Hand:");
    for (int i = 0; i < m.card_count; i++) {
        if (turn == 1 && i == 0) printCard(18 + (i * 6), 9, m.hand[i], 1); 
        else printCard(18 + (i * 6), 9, m.hand[i], 0);
    }

    // 4. 플레이어 카드
    gotoxy(2, 14); printf("Player Hand: ");
    for (int i = 0; i < p.card_count; i++) {
        printCard(18 + (i * 6), 13, p.hand[i], 0);
    }

    int pScore = calculateScore(&p);
    gotoxy(2, 17); printf("SCORE : %d", pScore);
    if (pScore > 21) { setColor(4); printf(" (BUST!)"); setColor(7); }

    // 5. 플레이어 정보
    gotoxy(2, 20); printf("=============================================");
    gotoxy(2, 21); printf("[ 용사 ] HP: %d | GOLD: %d", p.hp, p.gold);
    gotoxy(2, 22); printf("         물약: %d개 | 투시안경: %d개", p.potions, p.xray_specs);

    // 투시 안경 효과 표시
    if (is_xray_active) {
        setColor(11);
        gotoxy(40, 22); printf("<투시 ON!>");
        setColor(7);
    }
}

// 상점 시스템
void shop(Entity* p) {
    while (1) {
        system("cls");
        setColor(11);
        printf("\n\n");
        printf("      [ 던전 상점 ]\n");
        printf("      (o_o) < 어서오게!\n");
        printf("      /| |\\\n\n");
        setColor(7);
        printf("  보유 골드: %d G\n\n", p->gold);
        printf("  1. 체력 물약 (HP +30) [가격: 100 G]\n");
        printf("  2. 투시 안경 (1회용)  [가격: 200 G]\n");
        printf("  3. 던전으로 복귀\n\n");
        printf("  선택 >> ");

        char ch = _getch();
        if (ch == '1') {
            if (p->gold >= 100) {
                p->gold -= 100;
                p->potions++;
                printf("\n  >> 물약을 구매했습니다!");
            }
            else printf("\n  >> 돈이 부족합니다!");
        }
        else if (ch == '2') {
            if (p->gold >= 200) {
                p->gold -= 200;
                p->xray_specs++;
                printf("\n  >> 투시 안경을 구매했습니다!");
            }
            else printf("\n  >> 돈이 부족합니다!");
        }
        else if (ch == '3') break;
        Sleep(500);
    }
}

// 게임 규칙 설명 화면
void showTutorial() {
    system("cls");
    setColor(11); 
    printf("\n\n");
    printf("  ============================================================\n");
    printf("             ♣ 게임 규칙 가이드 (How to Play) ♣\n");
    printf("  ============================================================\n\n");
    setColor(7); 

    printf("  1. [ 목표 ]\n");
    printf("     - 딜러(몬스터)보다 내 카드 합계가 '21'에 가까우면 승리!\n");
    printf("     - 합계가 21을 넘어가면 '버스트(Bust)'로 즉시 패배합니다.\n\n");

    printf("  2. [ 카드 점수 ]\n");
    printf("     - 2~9: 숫자 그대로 점수 적용\n");
    printf("     - J, Q, K: '10'점으로 계산\n");
    printf("     - A (Ace): 상황에 따라 '1' 또는 '11'로 유리하게 계산\n\n");

    printf("  3. [ 행동 요령 ]\n");
    setColor(14); 
    printf("     - [1] Hit (히트):   카드를 한 장 더 받습니다. (점수 UP)\n");
    printf("     - [2] Stand (스탠드): 현재 점수로 멈추고 대결합니다.\n");
    setColor(7);
    printf("     - [3] 물약:         HP를 30 회복합니다.\n");
    printf("     - [4] 투시안경:     딜러의 뒤집힌 카드를 몰래 봅니다.\n\n");

    printf("  ============================================================\n");
    printf("  >> 준비가 되셨다면 아무 키나 눌러서 던전에 입장하세요.\n");

    _getch(); 
}

// ================= 메인 함수 =================
int main() {
    srand(time(NULL));
    initDeck();
    hideCursor();

    // 초기 설정
    Entity player = { "전설의 타짜", 100, 100, 300, 10, {0}, 0, 1, 0 }; 

    // 레벨 데이터 (슬라임 -> 오크 -> 드래곤)
    Entity monsters[3] = {
        { "던전 슬라임", 50, 50, 100, 5 },
        { "문지기 오크", 100, 100, 200, 10 },
        { "심연의 드래곤", 200, 200, 1000, 15 }
    };

    // 1. 오프닝 
    system("cls");
    gotoxy(5, 5); typingPrint("어두운 던전...", 50);
    gotoxy(5, 7); typingPrint("모든 무기가 통하지 않는 이곳에서...", 50);
    gotoxy(5, 9); typingPrint("오직 '블랙잭'만이 살 길이다.", 50);
    gotoxy(5, 13); printf(">> Press Any Key to Start");
    _getch();
    showTutorial();

    // 2. 던전 루프 (총 3층)
    for (int level = 0; level < 3; level++) {
        Entity monster = monsters[level];

        // 층 입장 멘트
        system("cls");
        printf("\n\n\n     === 지하 %d층 : %s 출현! ===\n", level + 1, monster.name);
        Sleep(1500);

        // 전투 루프
        while (player.hp > 0 && monster.hp > 0) {
            shuffleDeck();
            player.card_count = 0;
            monster.card_count = 0;
            is_xray_active = 0; 

            // 카드 분배
            player.hand[player.card_count++] = drawCard();
            monster.hand[monster.card_count++] = drawCard();
            player.hand[player.card_count++] = drawCard();
            monster.hand[monster.card_count++] = drawCard();

            int playing = 1;

            // 플레이어 턴
            while (playing) {
                drawInterface(player, monster, 1);

                if (calculateScore(&player) == 21) {
                    gotoxy(2, 24); setColor(14); printf(">> ★ BLACKJACK! ★"); setColor(7);
                    playing = 0; break;
                }

                gotoxy(2, 24); printf("[1] Hit  [2] Stand  [3] 물약사용  [4] 투시안경");
                char ch = _getch();

                if (ch == '1') { 
                    player.hand[player.card_count++] = drawCard();
                    if (calculateScore(&player) > 21) {
                        drawInterface(player, monster, 1);
                        playing = 0;
                    }
                }
                else if (ch == '2') { 
                    playing = 0;
                }
                else if (ch == '3') { 
                    if (player.potions > 0 && player.hp < player.max_hp) {
                        player.potions--;
                        player.hp += 50;
                        if (player.hp > player.max_hp) player.hp = player.max_hp;
                        gotoxy(2, 25); printf(">> 꿀꺽! 체력이 회복되었습니다.");
                        Sleep(800);
                    }
                }
                else if (ch == '4') { 
                    if (player.xray_specs > 0) {
                        player.xray_specs--;
                        is_xray_active = 1; 
                        gotoxy(2, 25); printf(">> 딜러의 패가 보입니다...!");
                        Sleep(800);
                    }
                }
            }

            // 몬스터 턴 & 결과
            drawInterface(player, monster, 0); 

            int pScore = calculateScore(&player);
            int mScore = calculateScore(&monster);

            if (pScore <= 21) {
                while (mScore < 17) {
                    gotoxy(2, 25); printf(">> 몬스터가 카드를 뽑습니다...");
                    Sleep(1000);
                    monster.hand[monster.card_count++] = drawCard();
                    mScore = calculateScore(&monster);
                    drawInterface(player, monster, 0);
                }
            }

            // 데미지 판정
            gotoxy(2, 24); printf("                                            ");
            gotoxy(2, 24);

            int dmg = 0;
            if (pScore > 21) {
                printf(">> 버스트! (데미지 %d)", monster.attack_power);
                player.hp -= monster.attack_power;
            }
            else if (mScore > 21) {
                dmg = (pScore) * 1.5; // 버스트 승리는 1.5배
                printf(">> 몬스터 버스트! (데미지 %d)", dmg);
                monster.hp -= dmg;
                player.gold += 50; 
            }
            else if (pScore > mScore) {
                dmg = (pScore - mScore) * 2; // 점수 차이 * 2배
                printf(">> 승리! (데미지 %d)", dmg);
                monster.hp -= dmg;
                player.gold += 50;
            }
            else if (pScore < mScore) {
                printf(">> 패배... (데미지 %d)", monster.attack_power);
                player.hp -= monster.attack_power;
            }
            else {
                printf(">> 무승부.");
            }

            Sleep(2000);
        }

        // 전투 종료 후 처리
        if (player.hp <= 0) {
            system("cls");
            setColor(4);
            gotoxy(10, 10); printf("::: GAME OVER :::");
            gotoxy(10, 12); printf("용사는 던전에서 쓰러졌습니다...");
            _getch();
            return 0;
        }
        else {
            // 몬스터 처치 성공
            system("cls");
            printf("\n\n   >> %s 처치 완료!\n", monster.name);
            printf("   >> 보상으로 %d 골드를 획득했습니다.\n", monster.gold);
            player.gold += monster.gold;

            if (level < 2) { 
                printf("   >> 상점으로 이동합니다 (Press Key)...");
                _getch();
                shop(&player);
                player.hp += 10; // 보너스 체력 회복
                if (player.hp > player.max_hp) player.hp = player.max_hp;
            }
        }
    }

    // 엔딩
    system("cls");
    setColor(14);
    gotoxy(5, 5); typingPrint("축하합니다!!!", 100);
    gotoxy(5, 7); typingPrint("던전의 드래곤을 물리치고", 100);
    gotoxy(5, 9); typingPrint("전설의 도박왕이 되셨습니다!", 100);
    gotoxy(5, 13); printf(">> Happy Ending <<");
    _getch();

    return 0;
}