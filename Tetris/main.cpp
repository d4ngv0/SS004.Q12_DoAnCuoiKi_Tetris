#include <iostream>
#include <conio.h>
#include <windows.h>
#include <string>
#include <fstream>
#include <vector>
#include <stack>
#include <algorithm>
#include <cmath>
#include <ctime>

using namespace std;
// --- 1. CẤU HÌNH & ĐỊNH NGHĨA ---
#define H 20
#define W 15
#define SETTING_SOUND 3
#define SETTING_SPEED 1
#define SETTING_VOLUMN 2
#define SETTING_RESUME 0

// Định nghĩa màu sắc cơ bản và nâng cao
#define FOREGROUND_WHITE (FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE)
#define COLOR_WHITE 7
#define COLOR_GRAY  8
#define COLOR_BRIGHT_WHITE (FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY)
#define COLOR_RED (FOREGROUND_RED | FOREGROUND_INTENSITY)

// Hàm chọn màu: Tất cả gạch là màu TRẮNG SÁNG (Monochrome style)
int getBlockColor(char type) {
    if (type == '#') return COLOR_GRAY; // Tường màu xám
    return COLOR_BRIGHT_WHITE;          // Gạch màu trắng sáng
}

// --- 2. BIẾN TOÀN CỤC ---
char board[H][W] = {};
char blocks[][4][4] = {
        {{' ','I',' ',' '}, {' ','I',' ',' '}, {' ','I',' ',' '}, {' ','I',' ',' '}},
        {{' ',' ',' ',' '}, {' ','O','O',' '}, {' ','O','O',' '}, {' ',' ',' ',' '}},
        {{' ',' ',' ',' '}, {' ','T',' ',' '}, {'T','T','T',' '}, {' ',' ',' ',' '}},
        {{' ',' ',' ',' '}, {' ','S','S',' '}, {'S','S',' ',' '}, {' ',' ',' ',' '}},
        {{' ',' ',' ',' '}, {'Z','Z',' ',' '}, {' ','Z','Z',' '}, {' ',' ',' ',' '}},
        {{' ',' ',' ',' '}, {'J',' ',' ',' '}, {'J','J','J',' '}, {' ',' ',' ',' '}},
        {{' ',' ',' ',' '}, {' ',' ','L',' '}, {'L','L','L',' '}, {' ',' ',' ',' '}}
};

const int tick = 50;
string localDir = "scoreboards.txt";
string apiURL = "https://script.google.com/macros/s/AKfycbyBSofw1Jugm68awOHDcthLfNzTuGC_2rxkbTafpgLc3w1NIfnHKwvJmOfIC_0FEuoX/exec";
int speed = 1000;
int currentSpeed = 0;
int level = 1;
int score = 0;
int next_b=0;
bool menuTriggered = false;
int x = 4, y = 0, b = 1;
bool isGameOver = false;
enum Screen { GAMEPLAY, MENU, PAUSE };
Screen screenState = GAMEPLAY;
int resumeMenuIndex = 0;
const char* resumeMenuItems[] = {"Sound enabled", "Volume", "Fall speed", "Resume"};
const int resumeMenuCount = sizeof(resumeMenuItems) / sizeof(resumeMenuItems[0]);

// --- 3. HÀM HỖ TRỢ ---
void setColor(int color) {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}

struct Settings {
    int volumePercent = 50;
    bool soundEnabled = false;
    int fallSpeedPercent = 100;
};
Settings settings;

int randomInRange(int a, int b){
    return rand() % b + a;
}

void gotoxy(int x, int y) {
    COORD c = {(short)x, (short)y};
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), c);
}

void clearLine(int y, int width = 50) {
    gotoxy(0, y);
    for(int i = 0; i < width; i++) cout << " ";
}

// --- 4. CÁC HÀM LOGIC QUAN TRỌNG (Đưa lên trước draw để sửa lỗi) ---

// Xóa vị trí gạch cũ trên board logic
void boardDelBlock() {
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            if (blocks[b][i][j] != ' ' && y + i < H && x + j < W)
                board[y + i][x + j] = ' ';
}

// Ghi vị trí gạch mới vào board logic
void block2Board() {
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            if (blocks[b][i][j] != ' ')
                board[y + i][x + j] = blocks[b][i][j];
}

// Kiểm tra va chạm
bool canMove(int dx, int dy) {
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            if (blocks[b][i][j] != ' ') {
                int tx = x + j + dx;
                int ty = y + i + dy;
                if (tx < 1 || tx >= W - 1 || ty >= H - 1) return false;
                if (board[ty][tx] != ' ') return false;
            }
    return true;
}

// --- 5. BỘ HIỆU ỨNG (VFX) ---

// Hiệu ứng hạt nổ (Màu trắng)
void effectParticles(int px, int py) {
    int offsets[4][2] = {{-1, -1}, {1, -1}, {-1, 0}, {1, 0}};
    char particles[] = {'*', '.', '+', 'x'};

    setColor(COLOR_BRIGHT_WHITE);
    for(int i=0; i<4; i++) {
        int dX = px + offsets[i][0];
        int dY = py + offsets[i][1];
        if(dX > 0 && dX < W-1 && dY > 0 && dY < H-1) {
            gotoxy(dX * 2, dY);
            cout << particles[rand() % 4];
        }
    }
    Sleep(60);

    setColor(COLOR_WHITE);
    for(int i=0; i<4; i++) {
        int dX = px + offsets[i][0];
        int dY = py + offsets[i][1];
        if(dX > 0 && dX < W-1 && dY > 0 && dY < H-1) {
            gotoxy(dX * 2, dY);
            cout << " ";
        }
    }
}

// Hiệu ứng rung màn hình
void effectShake() {
    string cmd = "color 4F";
    system(cmd.c_str());
    Sleep(30);
    cmd = "color 07";
    system(cmd.c_str());
}

// Hiệu ứng chữ bay
void effectFloatingText(int row, string text) {
    int col = W * 2 + 1;
    for (int i = 0; i < 3; i++) {
        setColor(COLOR_BRIGHT_WHITE);
        gotoxy(col, row - i);
        cout << text;
        Sleep(50);
        gotoxy(col, row - i);
        cout << "        ";
    }
    setColor(COLOR_WHITE);
}

// Hiệu ứng xóa hàng
void effectLineClear(int row) {
    for (int k = 0; k < 3; k++) {
        gotoxy(0, row);
        setColor(BACKGROUND_RED | BACKGROUND_INTENSITY | FOREGROUND_WHITE);
        cout << "================";
        Sleep(40);
        gotoxy(0, row);
        setColor(COLOR_WHITE);
        cout << "                ";
        Sleep(40);
    }
    setColor(COLOR_WHITE);
}

// --- 6. HÀM VẼ (DRAW) ---
// Bây giờ draw() đã nhìn thấy boardDelBlock() ở trên -> Hết lỗi
void draw() {
    boardDelBlock(); // Dọn đường cho bóng ma

    // Logic Cảnh báo
    bool isDanger = false;
    for (int j = 1; j < W - 1; j++) {
        if (board[4][j] != ' ') {
            isDanger = true;
            break;
        }
    }

    // Logic Bóng ma (Ghost)
    int ghostY = y;
    while (true) {
        bool collide = false;
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                if (blocks[b][i][j] != ' ') {
                    int tx = x + j;
                    int ty = ghostY + i + 1;
                    if (tx < 1 || tx >= W - 1 || ty >= H - 1 || board[ty][tx] != ' ') {
                        collide = true;
                    }
                }
            }
        }
        if (collide) break;
        ghostY++;
    }

    gotoxy(0, 0);
    // Vẽ Board
    for (int i = 0; i < H; i++, cout << "\n") {
        for (int j = 0; j < W; j++) {
            // Tường
            if (board[i][j] == '#') {
                if (isDanger) setColor(COLOR_RED); // Đỏ khi nguy hiểm
                else setColor(COLOR_GRAY);         // Xám bình thường
                cout << (unsigned char)219 << (unsigned char)219;
                setColor(COLOR_WHITE);
            }
            // Khoảng trống + Bóng ma
            else if (board[i][j] == ' ') {
                bool isGhostPart = false;
                if (i >= ghostY && i < ghostY + 4 && j >= x && j < x + 4) {
                    if (blocks[b][i - ghostY][j - x] != ' ') {
                        isGhostPart = true;
                    }
                }
                if (isGhostPart) {
                    setColor(COLOR_GRAY); // Vẽ bóng ma màu xám
                    cout << "::";
                    setColor(COLOR_WHITE);
                } else {
                    cout << "  ";
                }
            }
            // Gạch đã xếp (Màu trắng sáng)
            else {
                setColor(COLOR_BRIGHT_WHITE);
                cout << (unsigned char)219 << (unsigned char)219;
                setColor(COLOR_WHITE);
            }
        }
    }

    // Vẽ gạch đang rơi (Active Block) đè lên tất cả
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            if (blocks[b][i][j] != ' ') {
                if (y + i < H - 1) {
                    gotoxy((x + j) * 2, y + i);
                    setColor(COLOR_BRIGHT_WHITE);
                    cout << (unsigned char)219 << (unsigned char)219;
                    setColor(COLOR_WHITE);
                }
            }
        }
    }

    // Vẽ UI
    int uiX = W * 2 + 4;
    setColor(COLOR_BRIGHT_WHITE);
    gotoxy(uiX, 4); cout << "Level: " << level;
    gotoxy(uiX, 5); cout << "Score: " << score;

    gotoxy(uiX, 8); cout << "--- NEXT ---";
    for(int i=0; i<4; i++) { gotoxy(uiX + 2, 10 + i); cout << "        "; }

    for (int i = 0; i < 4; i++) {
        gotoxy(uiX + 2, 10 + i);
        for (int j = 0; j < 4; j++) {
            if (blocks[next_b][i][j] != ' ') {
                setColor(COLOR_BRIGHT_WHITE);
                cout << (unsigned char)219 << (unsigned char)219;
                setColor(COLOR_WHITE);
            } else {
                cout << "  ";
            }
        }
    }

    setColor(COLOR_WHITE);
    gotoxy(uiX, 15); cout << "--- KEYS ---";
    gotoxy(uiX, 16); cout << " Left/Right : Move";
    gotoxy(uiX, 17); cout << " Z, C       : Rotate";
    gotoxy(uiX, 18); cout << " Down       : Soft drop";
    gotoxy(uiX, 19); cout << " Space      : Hard drop";
    gotoxy(uiX, 20); cout << " M          : Menu";
}

// --- 7. CÁC HÀM XỬ LÝ KHÁC ---

void drawResumeMenu() {
    for(int i = 0; i < 12; i++) { clearLine(i); }
    gotoxy(0,0);
    cout << "===== PAUSE MENU =====\n\n";
    for (int i = 0; i < resumeMenuCount; ++i) {
        if (i == resumeMenuIndex) cout << " > "; else cout << "   ";
        if (i == SETTING_RESUME) cout << "Resume game\n";
        else if (i == SETTING_VOLUMN) cout << "Volume: " << settings.volumePercent << "%\n";
        else if (i == SETTING_SPEED) cout << "Fall speed: " << settings.fallSpeedPercent << "%\n";
        else if (i == SETTING_SOUND) cout << "Sound enabled: " << (settings.soundEnabled ? "ON" : "OFF") << "\n";
    }
    cout << "\nUp/Down Arrow: Browse | Left/Right Arrow: Decrease/Increase | ESC: Resume" ;
}

void applyFallSpeed() {
    int base = 1000;
    int pct = settings.fallSpeedPercent;
    speed = base * pct / 100;
    if (speed < 50) speed = 50;
}

void handleResumeMenuInput() {
    bool action = false;
    if (GetAsyncKeyState(VK_UP) & 0x8000) {
        if (resumeMenuIndex == 0) resumeMenuIndex = resumeMenuCount;
        if (resumeMenuIndex > 0) resumeMenuIndex--;
        Sleep(200); action = true;
    }
    if (GetAsyncKeyState(VK_DOWN) & 0x8000) {
        if (resumeMenuIndex == resumeMenuCount - 1) resumeMenuIndex = -1;
        if (resumeMenuIndex < resumeMenuCount - 1) resumeMenuIndex++;
        Sleep(200); action = true;
    }
    if (GetAsyncKeyState(VK_LEFT) & 0x8000) {
        switch (resumeMenuIndex){
            case SETTING_SOUND: settings.soundEnabled = !settings.soundEnabled; break;
            case SETTING_SPEED: settings.fallSpeedPercent = max(50, settings.fallSpeedPercent - 5); applyFallSpeed(); break;
            case SETTING_VOLUMN: settings.volumePercent = max(0, settings.volumePercent - 5); break;
            case SETTING_RESUME: screenState = GAMEPLAY; system("cls"); break;
        }
        Sleep(120); action = true;
    }
    if (GetAsyncKeyState(VK_RIGHT) & 0x8000) {
        switch (resumeMenuIndex){
            case SETTING_SOUND: settings.soundEnabled = !settings.soundEnabled; break;
            case SETTING_SPEED: settings.fallSpeedPercent = min(200, settings.fallSpeedPercent + 5); applyFallSpeed(); break;
            case SETTING_VOLUMN: settings.volumePercent = min(100, settings.volumePercent + 5); break;
            case SETTING_RESUME: screenState = GAMEPLAY; system("cls"); break;
        }
        Sleep(120); action = true;
    }
    if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) {
        screenState = GAMEPLAY; system("cls"); Sleep(120);
    }
    if (action) drawResumeMenu();
}

void hideCursor() {
    HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO info;
    info.dwSize = 100;
    info.bVisible = FALSE;
    SetConsoleCursorInfo(consoleHandle, &info);
}

void initBoard() {
    for (int i = 0; i < H; i++)
        for (int j = 0; j < W; j++)
            if ((i == H - 1) || (j == 0) || (j == W - 1)) board[i][j] = '#';
            else board[i][j] = ' ';
}

void resetGame() {
    initBoard();
    speed = 1000;
    currentSpeed = 0;
    level = 1;
    score = 0;
    x = 4; y = 0;
    b = randomInRange(0, 7);
    next_b = randomInRange(0, 7);;
    isGameOver = false;
    system("cls");
}

void showGameOverScreen() {
    setColor(COLOR_RED);
    gotoxy(W - 4, H / 2 - 2); cout << "=============";
    gotoxy(W - 4, H / 2 - 1); cout << "  GAME OVER  ";
    gotoxy(W - 4, H / 2);     cout << "=============";
    setColor(COLOR_WHITE);
    gotoxy(W - 4, H / 2 + 2); cout << "Score: " << score;
    gotoxy(W - 4, H / 2 + 4); cout << "Press 'R' to Replay";
    gotoxy(W - 4, H / 2 + 5); cout << "Press 'ESC' to Quit";
}

void increaseSpeed(int percent) {
    if (speed > 100) speed = speed * (100 - percent) / 100;
}

void removeLine() {
    int linesCleared = 0;
    for (int i = H - 2; i > 0; i--) {
        bool isFull = true;
        for (int j = 1; j < W - 1; j++) {
            if (board[i][j] == ' ') {
                isFull = false;
                break;
            }
        }
        if (isFull) {
            effectLineClear(i);
            effectFloatingText(i, "NICE!");
            effectShake();
            for (int k = i; k > 0; k--)
                for (int j = 1; j < W - 1; j++)
                    board[k][j] = board[k - 1][j];
            for (int j = 1; j < W - 1; j++) board[0][j] = ' ';
            i++;
            linesCleared++;
        }
    }
    if (linesCleared > 0) {
        score += linesCleared * 100 * level;
        if (score >= level * 500) {
            level++;
            if (speed > 100) speed -= 100;
        }
    }
}

bool canRotate(char temp[4][4], int& offset) {
    bool isCollide = true;
    while (isCollide && (offset < 5 && offset > -5)){
        isCollide = false;
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                if (temp[i][j] != ' ') {
                    int tx = x + j + offset;
                    int ty = y + i;
                    if (tx < 1){ isCollide = true; offset++; break; }
                    if (tx > W - 2){ isCollide = true; offset--; break; }
                    if (ty >= H - 1){ return false; }
                    if (board[ty][tx] != ' '){ return false; }
                }
            }
            if (isCollide) break;
        }
    }
    return true;
}

void rotateBlockClock() {
    char temp[4][4];
    for (int i = 0; i < 4; i++) for (int j = 0; j < 4; j++) temp[j][3 - i] = blocks[b][i][j];
    int offset = 0;
    if (canRotate(temp, offset)) {
        for (int i = 0; i < 4; i++) for (int j = 0; j < 4; j++) blocks[b][i][j] = temp[i][j];
        x += offset;
    }
}

void rotateBlockCterClock() {
    char temp[4][4];
    for (int i = 0; i < 4; i++) for (int j = 0; j < 4; j++) temp[3-j][i] = blocks[b][i][j];
    int offset = 0;
    if (canRotate(temp, offset)) {
        for (int i = 0; i < 4; i++) for (int j = 0; j < 4; j++) blocks[b][i][j] = temp[i][j];
        x += offset;
    }
}

bool canFall() {
    currentSpeed += tick;
    if (currentSpeed >= speed) { currentSpeed = 0; return true; }
    return false;
}

void hardDrop(){
    while (canMove(0,1)) {
        y++;
        block2Board();
        boardDelBlock();
    }
    effectParticles(x + 1, y + 3);
    effectParticles(x + 2, y + 3);
    block2Board();
    removeLine();
    x = 5; y = 0; b = next_b; next_b = randomInRange(0, 7);
    currentSpeed = 0;
}

void debug(){ cout<<x<<" "<<y<<"\n"; }

int main()
{
    SetConsoleOutputCP(437);
    srand(time(0));
    hideCursor();

    while (true) {
        resetGame();
        while (!isGameOver) {
            // Menu
            if (GetAsyncKeyState('M') & 0x8000) {
                if(!menuTriggered) {
                    screenState = MENU;
                    resumeMenuIndex = SETTING_RESUME;
                    system("cls");
                    drawResumeMenu();
                    menuTriggered = true;
                }
            } else menuTriggered = false;

            if (screenState == MENU) {
                handleResumeMenuInput();
                continue;
            }

            // Gameplay
            boardDelBlock();

            if ((GetAsyncKeyState(VK_LEFT) & 0x8000) && canMove(-1, 0)) x--;
            if ((GetAsyncKeyState(VK_RIGHT) & 0x8000) && canMove(1, 0)) x++;
            if ((GetAsyncKeyState(VK_DOWN) & 0x8000) && canMove(0, 1)) y++;

            if (GetAsyncKeyState('C') & 0x8000){ rotateBlockClock(); Sleep(100); }
            if (GetAsyncKeyState('Z') & 0x8000) { rotateBlockCterClock(); Sleep(100); }
            if (GetAsyncKeyState(VK_SPACE) & 0x8000) { hardDrop(); Sleep(200); }
            if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) { isGameOver = true; }

            if (canFall()) {
                if (canMove(0, 1)) {
                    y++;
                } else {
                    block2Board();
                    removeLine();
                    b = next_b;
                    next_b = randomInRange(0, 7);
                    x = 4; y = 0;
                    if (!canMove(0, 0)) { isGameOver = true; }
                }
            }

            block2Board();
            draw();
            Sleep(tick);
        }

        showGameOverScreen();

        while (true) {
            if (GetAsyncKeyState('R') & 0x8000) {
                break;
            }
            if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) {
                return 0;
            }
            Sleep(100);
        }
    }
    return 0;
}
