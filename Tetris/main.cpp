#include <iostream>
#include <vector>
#include <windows.h>
#include <ctime>
#include <conio.h>

using namespace std;

// --- CẤU HÌNH GAME ---
#define H 20      // Chiều cao
#define W 12      // Chiều rộng
#define TICK 50   // Tốc độ refresh game (ms)

#include <string>
#include <fstream>
#include <vector>
#include <stack>
#include <algorithm>
#include <cmath>
#include <ctime>
#include <mmsystem.h>    // Cho PlaySound
#pragma comment(lib, "winmm.lib") // Liên kết thư viện

using namespace std;
// --- 1. CẤU HÌNH & ĐỊNH NGHĨA ---
#define H 20
#define W 15
#define SETTING_RESUME 0
#define SETTING_SPEED 1
#define SETTING_VOLUMN 2
#define SETTING_RESUME 0
#define SETTING_SOUND 3
#define SETTING_BACK_TO_MAIN 4

#define GAMEMODE_CLASSIC 0
#define GAMEMODE_INVISIBLE 1

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



#define COLOR_RESET_M     "\033[0m"
#define COLOR_BLUE_BG_M   "\033[44m"
#define COLOR_YELLOW_M    "\033[93m"
#define COLOR_CYAN_M      "\033[96m"
#define COLOR_WHITE_M     "\033[97m"
// --- Khai báo biến toàn cục ---
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

const int tick = 50; // 20 fps
string name = "";
string localDir = "scoreboards.txt";
string apiURL = "https://script.google.com/macros/s/AKfycbzrrxtmpe1Zwt5As_nnBE46XlxWE0NlE2EAkmW3YD6lHmEwVzLWgoqfLcyi-gfNBnya/exec";
int speed = 1000;
int currentSpeed = 0;
int level = 1;
int score = 0;
int next_b=0;
bool menuTriggered = false;
int x = 4, y = 0, b = 1;
bool isGameOver = false;

enum Mode { CLASSIC, INVISIBLE };
int gameModeIndex = 0;
const int gameModeCount = 2;
Mode gameMode = CLASSIC;
int resumeMenuIndex = 0;
int settingIndex = 0;
enum Screen { MAINMENU, GAMEPLAY, PREPARE, GAMEMODE, MENU, SETTINGS, PAUSE, SAVE, SUBMIT, SUBMITTING };
Screen screenState = MAINMENU;
const char* mainMenuItems[] = {"START GAME", "SETTINGS", "EXIT"};
const int mainMenuCount = 3;
const char* resumeMenuItems[] = {"Resume game", "Fall speed", "Volume", "Sound enabled", "Back to Main Menu"};
const char* settingItems[] = { "Fall speed", "Volume", "Sound enabled","Back to Main Menu"};
const int resumeMenuCount = sizeof(resumeMenuItems) / sizeof(resumeMenuItems[0]);
const int settingMenuCount = sizeof(settingItems) / sizeof(settingItems[0]);
int mainMenuIndex = 0;
bool isInGame = false; // False = Ở Main Menu, True = Đang trong ván chơi

char currentBlock[4][4];// them block hien

// --- 3. HÀM HỖ TRỢ ---
void setColor(int color) {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}
// --- Các hàm đồ họa & Logic ---
void copyTemplateToCurrent(int idx) {
    for (int i = 0; i < 4; ++i)
        for (int j = 0; j < 4; ++j)
            currentBlock[i][j] = blocks[idx][i][j];
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

// Hàm di chuyển con trỏ console (tránh dùng system("cls") gây giật màn hình)
void gotoxy(int x, int y) {
    COORD c = {(short)x, (short)y};
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), c);
}

// Ẩn con trỏ nhấp nháy
void hideCursor() {
    HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO info;
    info.dwSize = 100;
    info.bVisible = FALSE;
    SetConsoleCursorInfo(consoleHandle, &info);
}

// --- LỚP XỬ LÝ GAME RIÊNG BIỆT ---
class TetrisPlayer {
private:
    char board[H][W];
    int x, y, type;       // Tọa độ và loại gạch hiện tại
    int rotation;         // Góc xoay
    int score;
    int timer;            // Bộ đếm thời gian để gạch rơi
    int speed;            // Tốc độ rơi (càng nhỏ càng nhanh)
    int offsetX;          // Vị trí vẽ trên màn hình (để chia đôi màn hình)
    int playerID;         // 1 hoặc 2
    
public:
    bool gameOver;
    char currentBlock[4][4];

    TetrisPlayer(int offset, int id) {
        offsetX = offset;
        playerID = id;
        setup();
    }

    void setup() {
        gameOver = false;
        score = 0;
        timer = 0;
        speed = 20; // Tốc độ rơi mặc định

        // 1. Xóa sạch bàn cờ TRƯỚC KHI sinh gạch
        for (int i = 0; i < H; i++) {
            for (int j = 0; j < W; j++) {
                if (i == H - 1 || j == 0 || j == W - 1) 
                    board[i][j] = '#'; // Tường
                else 
                    board[i][j] = ' '; // Khoảng trống
            }
        }

        // 2. Sau đó mới sinh gạch đầu tiên
        spawnBlock();
    }

    void spawnBlock() {
        x = W / 2 - 2;
        y = 0;
        type = rand() % 7;
        rotation = 0;
        
        // Copy hình dáng gạch vào biến tạm
        for(int i=0; i<4; i++)
            for(int j=0; j<4; j++)
                currentBlock[i][j] = blocks[type][i][j];

        // Kiểm tra ngay khi vừa sinh ra có bị kẹt không (Game Over check)
        if (!checkCollision(0, 0)) {
            gameOver = true;
        }
    }

    // Kiểm tra va chạm (trả về true nếu đi được, false nếu đụng)
    bool checkCollision(int dx, int dy) {
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                if (currentBlock[i][j] != ' ') {
                    int newX = x + j + dx;
                    int newY = y + i + dy;
                    
                    // Ra khỏi biên hoặc đụng block đã đóng băng
                    if (newX < 0 || newX >= W || newY >= H || board[newY][newX] != ' ') 
                        return false;
                }
            }
        }
        return true;
    }

    void rotate() {
        char temp[4][4];
        // Xoay ma trận 4x4
        for (int i = 0; i < 4; i++)
            for (int j = 0; j < 4; j++)
                temp[j][3 - i] = currentBlock[i][j];

        // Lưu trạng thái cũ để restore nếu xoay bị kẹt
        char savedBlock[4][4];
        memcpy(savedBlock, currentBlock, sizeof(currentBlock));
        memcpy(currentBlock, temp, sizeof(temp));

        if (!checkCollision(0, 0)) {
            memcpy(currentBlock, savedBlock, sizeof(savedBlock)); // Hoàn tác
        }
    }

    // Xử lý logic rơi và ăn điểm
    void update() {
        if (gameOver) return;

        timer++;
        if (timer >= speed) {
            if (checkCollision(0, 1)) {
                y++;
            } else {
                lockBlock();
                checkLines();
                spawnBlock();
            }
            timer = 0;
void clearLine(int y, int width = 50) {
    gotoxy(0, y);
    for(int i = 0; i < width; i++) cout << " ";
}

// --- 4. CÁC HÀM LOGIC QUAN TRỌNG (Đưa lên trước draw để sửa lỗi) ---

// Xóa vị trí gạch cũ trên board logic
void boardDelBlock() {
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            if (currentBlock[i][j] != ' ' && y + i < H && x + j < W)
                board[y + i][x + j] = ' ';
}

// Ghi vị trí gạch mới vào board logic
void block2Board() {
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            if (currentBlock[i][j] != ' ')
                board[y + i][x + j] = currentBlock[i][j];
}

// Kiểm tra va chạm
bool canMove(int dx, int dy) {
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            if (currentBlock[i][j] != ' ') {
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
            cout << particles[randomInRange(0,4)];
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
        gotoxy(2, row);
        setColor(BACKGROUND_RED | BACKGROUND_INTENSITY | FOREGROUND_WHITE);
        for (int i = 1; i < W-1; i++) cout << "==";
        Sleep(40);
        gotoxy(2, row);
        setColor(COLOR_WHITE);
        for (int i = 1; i < W-1; i++) cout << "  ";
        Sleep(40);
    }
    setColor(COLOR_WHITE);
}

// --- 6. HÀM VẼ (DRAW) ---
void drawUI(){
    int uiX = W * 2 + 4;

    gotoxy(uiX, 4); cout << "Level: " << level;
    gotoxy(uiX, 5); cout << "Score: " << score;


    gotoxy(uiX, 8); cout << "--- NEXT ---";

    for(int i=0; i<4; i++) {
        gotoxy(uiX + 2, 10 + i); cout << "        ";
    }

    for (int i = 0; i < 4; i++) {
        gotoxy(uiX + 2, 10 + i);
        for (int j = 0; j < 4; j++) {
            if (blocks[next_b][i][j] != ' ') {
                cout << (unsigned char)219 << (unsigned char)219;
            } else {
                cout << "  ";
            }
        }
    }

    gotoxy(uiX, 15); cout << "--- KEYS ---";
    gotoxy(uiX, 16); cout << " Left, Right Arrow : Move";
    gotoxy(uiX, 17); cout << " Z, C              : Rotate";
    gotoxy(uiX, 18); cout << " Down Arrow        : Soft drop";
    gotoxy(uiX, 19); cout << " SPACE             : Hard drop";
    gotoxy(uiX, 20); cout << " M                 : Menu";
}
void drawBlock(){
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            if (currentBlock[i][j] != ' ') {
                if (y + i < H - 1) {
                    gotoxy((x + j) * 2, y + i);
                    setColor(COLOR_BRIGHT_WHITE);
                    cout << (unsigned char)219 << (unsigned char)219;
                    setColor(COLOR_WHITE);
                }
            }
        }
    }
}
void drawBoardNGhost(){
    // Logic Cảnh báo
    bool isDanger = false;
    if (gameMode != INVISIBLE){
        for (int j = 1; j < W - 1; j++) {
            if (board[4][j] != ' ') {
                isDanger = true;
                break;
            }
        }
    }

    // Logic Bóng ma (Ghost)
    int ghostY = y;
    while (true) {
        bool collide = false;
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                if (currentBlock[i][j] != ' ') {
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
                    if (currentBlock[i - ghostY][j - x] != ' ') {
                        isGhostPart = true;
                    }
                }
                if (gameMode != INVISIBLE && isGhostPart) {
                    setColor(COLOR_GRAY); // Vẽ bóng ma màu xám
                    cout << "::";
                    setColor(COLOR_WHITE);
                } else {
                    cout << "  ";
                }
            }
            // Gạch đã xếp (Màu trắng sáng)
            else {
                if (gameMode != INVISIBLE){
                    setColor(COLOR_BRIGHT_WHITE);
                    cout << (unsigned char)219 << (unsigned char)219;
                    setColor(COLOR_WHITE);
                } else {
                    cout <<"  ";
                }

            }
        }
    }
}
void draw() {
    boardDelBlock(); // Dọn đường cho bóng ma
    drawBoardNGhost();

    // Vẽ gạch đang rơi (Active Block) đè lên tất cả
    drawBlock();
    drawUI();
}
// Hàm lấy kích thước Console thực tế
void getConsoleSize(int &width, int &height) {
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    width = csbi.srWindow.Right - csbi.srWindow.Left + 1;
    height = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
}

// Hàm vẽ màn hình chính
void drawMainMenu() {
    system("cls");

    // Lấy kích thước console thực tế
    int consoleWidth = 80;
    int consoleHeight = 25;
    getConsoleSize(consoleWidth, consoleHeight);

    // Kích thước menu mới: Rộng hơn để tạo padding, Cao hơn để tạo spacing
    const int menuWidth = 40;
    const int menuInternalSpacing = 1; // Khoảng cách dòng giữa các item menu

    // Tiêu đề và nội dung
    string title = "TETRIS GAME";
    const char* items[] = {"START GAME", "SETTINGS", "EXIT"};
    int nItems = sizeof(items) / sizeof(items[0]);
    string hint = "Up Arrow/Down Arrow = Move; SPACE/Left Arrow/Right Arrow = Select";

    // --- Tính toán Chiều Cao Khung Viền Menu (menuHeight) ---
    // Tiêu đề (1) + Dòng trống (1) + Items (nItems) + Spacing (nItems-1) + Dòng trống (1)
    const int menuContentHeight = 1 + 1 + nItems + (nItems - 1) * menuInternalSpacing + 1;
    const int menuHeight = menuContentHeight + 2; // +2 cho viền trên/dưới

    // --- Tính toán vị trí (Căn giữa hoàn toàn) ---
    int startY = (consoleHeight - menuHeight) / 2;
    int startX = (consoleWidth - menuWidth) / 2;

    // Đảm bảo không bị tràn màn hình
    if (startY < 0) startY = 0;
    if (startX < 0) startX = 0;

    // --- Vẽ Khung Viền (Box) ---
    // Ký tự Unicode cho Box Drawing (cần SetConsoleOutputCP(437) ở main)
    const char TL = (char)201, TR = (char)187; // Top Left/Right
    const char BL = (char)200, BR = (char)188; // Bottom Left/Right
    const char HZ = (char)205, VT = (char)186; // Horizontal/Vertical

    // Vẽ 4 góc và viền ngang/dọc
    gotoxy(startX, startY); cout << TL;
    gotoxy(startX + menuWidth - 1, startY); cout << TR;
    gotoxy(startX, startY + menuHeight - 1); cout << BL;
    gotoxy(startX + menuWidth - 1, startY + menuHeight - 1); cout << BR;

    for (int i = 1; i < menuWidth - 1; i++) {
        gotoxy(startX + i, startY); cout << HZ;
        gotoxy(startX + i, startY + menuHeight - 1); cout << HZ;
    }

    for (int i = 1; i < menuHeight - 1; i++) {
        gotoxy(startX, startY + i); cout << VT;
        gotoxy(startX + menuWidth - 1, startY + i); cout << VT;
    }

    // --- Vẽ Tiêu đề (Căn giữa ngang trong khung) ---
    int titleX = startX + (menuWidth - title.size()) / 2;
    gotoxy(titleX, startY + 1);
    cout << COLOR_YELLOW_M << title << COLOR_RESET_M;

    // --- Vẽ Menu Items (Căn giữa ngang trong khung) ---
    int longestItemLength = 0;
    for (int i = 0; i < nItems; ++i) {
        longestItemLength = max(longestItemLength, (int)strlen(items[i]));
    }

    // Vị trí X bắt đầu để các item thẳng hàng (đã tính padding rộng hơn)
    // Căn giữa dựa trên độ dài chữ, trừ đi một chút cho mũi tên để nhìn cân đối hơn
    int itemStartX = startX + (menuWidth - longestItemLength) / 2 - 2;

    int currentY = startY + 3; // Bắt đầu sau Tiêu đề và 1 dòng trống

    for (int i = 0; i < nItems; i++) {
        gotoxy(itemStartX, currentY);

        if (i == mainMenuIndex) {
            cout << COLOR_CYAN_M << " > " << items[i] << COLOR_RESET_M;
        } else {
            cout << COLOR_WHITE_M << "   " << items[i] << COLOR_RESET_M;
        }

        currentY += (1 + menuInternalSpacing); // Tăng Y lên 1 dòng item + khoảng cách
    }

    // --- Vẽ Hướng dẫn Phím (Căn giữa ngang bên dưới khung) ---
    int hintX = (consoleWidth - hint.size()) / 2;
    gotoxy(hintX, startY + menuHeight + 1);
    cout << COLOR_BLUE_BG_M << COLOR_WHITE_M << hint << COLOR_RESET_M;
}

// Hàm vẽ màn hình chính
void drawGameModeMenu() {
    system("cls");

    // Lấy kích thước console thực tế
    int consoleWidth = 80;
    int consoleHeight = 25;
    getConsoleSize(consoleWidth, consoleHeight);

    // Kích thước menu mới: Rộng hơn để tạo padding, Cao hơn để tạo spacing
    const int menuWidth = 40;
    const int menuInternalSpacing = 1; // Khoảng cách dòng giữa các item menu

    // Tiêu đề và nội dung
    string title = "TETRIS GAME";
    const char* items[] = {"CLASSIC", "INVISIBLE"};
    int nItems = sizeof(items) / sizeof(items[0]);
    string hint = "Up Arrow/Down Arrow = Move; SPACE/Left Arrow/Right Arrow = Select";

    // --- Tính toán Chiều Cao Khung Viền Menu (menuHeight) ---
    // Tiêu đề (1) + Dòng trống (1) + Items (nItems) + Spacing (nItems-1) + Dòng trống (1)
    const int menuContentHeight = 1 + 1 + nItems + (nItems - 1) * menuInternalSpacing + 1;
    const int menuHeight = menuContentHeight + 2; // +2 cho viền trên/dưới

    // --- Tính toán vị trí (Căn giữa hoàn toàn) ---
    int startY = (consoleHeight - menuHeight) / 2;
    int startX = (consoleWidth - menuWidth) / 2;

    // Đảm bảo không bị tràn màn hình
    if (startY < 0) startY = 0;
    if (startX < 0) startX = 0;

    // --- Vẽ Khung Viền (Box) ---
    // Ký tự Unicode cho Box Drawing (cần SetConsoleOutputCP(437) ở main)
    const char TL = (char)201, TR = (char)187; // Top Left/Right
    const char BL = (char)200, BR = (char)188; // Bottom Left/Right
    const char HZ = (char)205, VT = (char)186; // Horizontal/Vertical

    // Vẽ 4 góc và viền ngang/dọc
    gotoxy(startX, startY); cout << TL;
    gotoxy(startX + menuWidth - 1, startY); cout << TR;
    gotoxy(startX, startY + menuHeight - 1); cout << BL;
    gotoxy(startX + menuWidth - 1, startY + menuHeight - 1); cout << BR;

    for (int i = 1; i < menuWidth - 1; i++) {
        gotoxy(startX + i, startY); cout << HZ;
        gotoxy(startX + i, startY + menuHeight - 1); cout << HZ;
    }

    for (int i = 1; i < menuHeight - 1; i++) {
        gotoxy(startX, startY + i); cout << VT;
        gotoxy(startX + menuWidth - 1, startY + i); cout << VT;
    }

    // --- Vẽ Tiêu đề (Căn giữa ngang trong khung) ---
    int titleX = startX + (menuWidth - title.size()) / 2;
    gotoxy(titleX, startY + 1);
    cout << COLOR_YELLOW_M << title << COLOR_RESET_M;

    // --- Vẽ Menu Items (Căn giữa ngang trong khung) ---
    int longestItemLength = 0;
    for (int i = 0; i < nItems; ++i) {
        longestItemLength = max(longestItemLength, (int)strlen(items[i]));
    }

    // Vị trí X bắt đầu để các item thẳng hàng (đã tính padding rộng hơn)
    // Căn giữa dựa trên độ dài chữ, trừ đi một chút cho mũi tên để nhìn cân đối hơn
    int itemStartX = startX + (menuWidth - longestItemLength) / 2 - 2;

    int currentY = startY + 3; // Bắt đầu sau Tiêu đề và 1 dòng trống

    for (int i = 0; i < nItems; i++) {
        gotoxy(itemStartX, currentY);

        if (i == gameModeIndex) {
            cout << COLOR_CYAN_M << " > " << items[i] << COLOR_RESET_M;
        } else {
            cout << COLOR_WHITE_M << "   " << items[i] << COLOR_RESET_M;
        }

        currentY += (1 + menuInternalSpacing); // Tăng Y lên 1 dòng item + khoảng cách
    }

    // --- Vẽ Hướng dẫn Phím (Căn giữa ngang bên dưới khung) ---
    int hintX = (consoleWidth - hint.size()) / 2;
    gotoxy(hintX, startY + menuHeight + 1);
    cout << COLOR_BLUE_BG_M << COLOR_WHITE_M << hint << COLOR_RESET_M;
}


// Hàm chạy nhạc nền
void updateBackgroundMusic() {
    if (settings.soundEnabled) {
        PlaySound(TEXT("bgm.wav"), NULL, SND_FILENAME | SND_LOOP | SND_ASYNC);
    } else {
        PlaySound(NULL, 0, 0); // Dừng nhạc
    }
}

// Hàm điều chỉnh âm lượng
void setMusicVolume(int percent) {
    if (percent < 0) percent = 0;
    if (percent > 100) percent = 100;

    DWORD volume = (DWORD)(0xFFFF * percent / 100); // 0-FFFF
    DWORD volumeAllChannels = (volume & 0xFFFF) | (volume << 16); // Kênh trái/phải
    waveOutSetVolume(NULL, volumeAllChannels); // Điều chỉnh volume toàn hệ thống
}

//Hàm vẽ menu console
//void drawResumeMenu() {
//    // Clear vùng menu (10 dòng đầu)
//    for(int i = 0; i < 30; i++) {
//        clearLine(i);
//    }
//
//    // Vẽ UI
//    int uiX = W * 2 + 4;
//    setColor(COLOR_BRIGHT_WHITE);
//    gotoxy(uiX, 4); cout << "Level: " << level;
//    gotoxy(uiX, 5); cout << "Score: " << score;
//
//    gotoxy(uiX, 8); cout << "--- NEXT ---";
//    for(int i=0; i<4; i++) { gotoxy(uiX + 2, 10 + i); cout << "        "; }
//
//    for (int i = 0; i < 4; i++) {
//        gotoxy(uiX + 2, 10 + i);
//        for (int j = 0; j < 4; j++) {
//            if (blocks[next_b][i][j] != ' ') {
//                setColor(COLOR_BRIGHT_WHITE);
//                cout << (unsigned char)219 << (unsigned char)219;
//                setColor(COLOR_WHITE);
//            } else {
//                cout << "  ";
//            }
//        }
//    }
//
//    setColor(COLOR_WHITE);
//    gotoxy(uiX, 15); cout << "--- KEYS ---";
//    gotoxy(uiX, 16); cout << " Left/Right : Move";
//    gotoxy(uiX, 17); cout << " Z/C        : Rotate";
//    gotoxy(uiX, 18); cout << " Down       : Soft drop";
//    gotoxy(uiX, 19); cout << " Space      : Hard drop";
//    gotoxy(uiX, 20); cout << " M          : Menu";
//}



void drawResumeMenu() {
    system("cls");
    for(int i = 0; i < 12; i++) { clearLine(i); }
    gotoxy(0,0);
    cout << "===== PAUSE MENU =====\n\n";
    for (int i = 0; i < resumeMenuCount; ++i) {
        if (i == resumeMenuIndex) cout << " > "; else cout << "   ";
        switch (i){
            case SETTING_RESUME:{
                cout << "Resume game\n";
                break;
            }
            case SETTING_VOLUMN:{
                cout << "Volume: " << settings.volumePercent << "%\n";
                break;
            }
            case SETTING_SPEED:{
                cout << "Fall speed: " << settings.fallSpeedPercent << "%\n";
                break;
            }
            case SETTING_SOUND:{
                cout << "Sound enabled: " << (settings.soundEnabled ? "ON" : "OFF") << "\n";
                break;
            }
            case SETTING_BACK_TO_MAIN:{
                cout << "Back to main menu\n";
                break;
            }
        }
    }
    cout << "\nUp/Down Arrow: Browse | Left/Right Arrow: Decrease/Increase/Select | ESC: Resume" ;
}

void drawSettingMenu() {
    system("cls");
    for(int i = 0; i < 12; i++) { clearLine(i); }
    gotoxy(0,0);
    cout << "===== SETTINGS =====\n\n";
    for (int i = 1; i < settingMenuCount+1; ++i) {
        if (i == settingIndex) cout << " > ";
        else cout << "   ";

        switch (i){
            case SETTING_BACK_TO_MAIN:{
                cout << "Back to main menu\n";
                break;
            }
            case SETTING_VOLUMN:{
                cout << "Volume: " << settings.volumePercent << "%\n";
                break;
            }
            case SETTING_SPEED:{
                cout << "Fall speed: " << settings.fallSpeedPercent << "%\n";
                break;
            }
            case SETTING_SOUND:{
                cout << "Sound enabled: " << (settings.soundEnabled ? "ON" : "OFF") << "\n";
                break;
            }
        }
    }
    cout << "\nUp/Down Arrow: Browse | Left/Right Arrow: Decrease/Increase/Select | ESC: Resume" ;
}
// --- 7. CÁC HÀM XỬ LÝ KHÁC ---

void applyFallSpeed() {
    int base = 1000;
    int pct = settings.fallSpeedPercent;
    speed = base * pct / 100;
    if (speed < 50) speed = 50;
}

void handleResumeMenuInput() {
    // Di chuyển chọn bằng mũi tên lên/xuống
    bool action = false;
    if (GetAsyncKeyState(VK_UP) & 0x8000) {
        resumeMenuIndex = (resumeMenuIndex+resumeMenuCount-1)%resumeMenuCount;
        Sleep(200); action = true;
    }
    if (GetAsyncKeyState(VK_DOWN) & 0x8000) {
        resumeMenuIndex = (resumeMenuIndex+1)%resumeMenuCount;
        Sleep(200); action = true;
    }
    if (GetAsyncKeyState(VK_LEFT) & 0x8000) {
        switch (resumeMenuIndex){
            case SETTING_SOUND:{
                settings.soundEnabled = !settings.soundEnabled;
                updateBackgroundMusic();
                break;
            }
            case SETTING_SPEED:{
                settings.fallSpeedPercent = max(50, settings.fallSpeedPercent - 5);
                applyFallSpeed();
                break;
            }
            case SETTING_VOLUMN:{
                settings.volumePercent = max(0, settings.volumePercent - 5);
                setMusicVolume(settings.volumePercent); // cập nhật volume
                break;
            }
            case SETTING_RESUME:{
                screenState = GAMEPLAY; // Nếu đang chơi thì tiếp tục
                system("cls");
                break;
            }
            case SETTING_BACK_TO_MAIN:{
                isGameOver = true; // Đặt cờ thua để thoát vòng lặp game
                isInGame = false; // Đánh dấu đã thoát game
                screenState = MAINMENU; // Chuyển trạng thái về Menu chính
                system("cls");
                break;
            }
        }
        Sleep(120); action = true;
    }
    if (GetAsyncKeyState(VK_RIGHT) & 0x8000) {
        switch (resumeMenuIndex){
            case SETTING_SOUND:{
                settings.soundEnabled = !settings.soundEnabled;
                updateBackgroundMusic();
                break;
            }
            case SETTING_SPEED:{
                settings.fallSpeedPercent = min(200, settings.fallSpeedPercent + 5);
                applyFallSpeed();
                break;
            }
            case SETTING_VOLUMN:{
                settings.volumePercent = min(100, settings.volumePercent + 5);
                setMusicVolume(settings.volumePercent); // cập nhật volume
                break;
            }
            case SETTING_RESUME:{
                if (isInGame) {
                    screenState = GAMEPLAY;
                } else {
                    screenState = MAINMENU;
                }
                system("cls");
                break;
            }
            case SETTING_BACK_TO_MAIN:{
                isGameOver = true;
                isInGame = false;
                screenState = MAINMENU;
                system("cls");
                break;
            }
        }
        Sleep(120); action = true;
    }
    if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) {
        screenState = GAMEPLAY; // Nếu đang chơi thì tiếp tục
        system("cls");
        Sleep(120);
    }
    if (action) drawResumeMenu();
}
void handleSettingInput() {
    // Di chuyển chọn bằng mũi tên lên/xuống
    bool action = false;
    if (GetAsyncKeyState(VK_UP) & 0x8000) {
        settingIndex = (settingIndex+settingMenuCount-2)%settingMenuCount+1;
        Sleep(200); action = true;
    }
    if (GetAsyncKeyState(VK_DOWN) & 0x8000) {
        settingIndex = (settingIndex)%settingMenuCount+1;
        Sleep(200); action = true;
    }
    if (GetAsyncKeyState(VK_LEFT) & 0x8000) {
        switch (settingIndex){
            case SETTING_SOUND:{
                settings.soundEnabled = !settings.soundEnabled;
                updateBackgroundMusic();
                break;
            }
            case SETTING_SPEED:{
                settings.fallSpeedPercent = max(50, settings.fallSpeedPercent - 5);
                applyFallSpeed();
                break;
            }
            case SETTING_VOLUMN:{
                settings.volumePercent = max(0, settings.volumePercent - 5);
                setMusicVolume(settings.volumePercent); // cập nhật volume
                break;
            }
            case SETTING_BACK_TO_MAIN:{
                isGameOver = true; // Đặt cờ thua để thoát vòng lặp game
                isInGame = false; // Đánh dấu đã thoát game
                screenState = MAINMENU; // Chuyển trạng thái về Menu chính
                system("cls");
                break;
            }
        }
        Sleep(120); action = true;
    }
    if (GetAsyncKeyState(VK_RIGHT) & 0x8000) {
        switch (settingIndex){
            case SETTING_SOUND:{
                settings.soundEnabled = !settings.soundEnabled;
                updateBackgroundMusic();
                break;
            }
            case SETTING_SPEED:{
                settings.fallSpeedPercent = min(200, settings.fallSpeedPercent + 5);
                applyFallSpeed();
                break;
            }
            case SETTING_VOLUMN:{
                settings.volumePercent = min(100, settings.volumePercent + 5);
                setMusicVolume(settings.volumePercent); // cập nhật volume
                break;
            }
            case SETTING_BACK_TO_MAIN:{
                isGameOver = true;
                isInGame = false;
                screenState = MAINMENU;
                system("cls");
                break;
            }
        }
        Sleep(120); action = true;
    }
    if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) {
        screenState = MAINMENU;
        system("cls");
        Sleep(120);
    }

    if (action){
        drawSettingMenu();
    }

}

void handleGameModeInput() {
    bool action = false;
    if (GetAsyncKeyState(VK_UP) & 0x8000) {
        gameModeIndex = (gameModeIndex+gameModeCount-1)%gameModeCount;
        Sleep(200); action = true;
    }
    if (GetAsyncKeyState(VK_DOWN) & 0x8000) {
        gameModeIndex = (gameModeIndex+1)%gameModeCount;
        Sleep(200); action = true;
    }
    if (GetAsyncKeyState(VK_LEFT) & 0x8000) {
        switch (gameModeIndex){
            case GAMEMODE_CLASSIC:{
                gameMode = CLASSIC;
                screenState = PREPARE;
                break;
            }
            case GAMEMODE_INVISIBLE:{
                gameMode = INVISIBLE;
                screenState = PREPARE;
                break;
            }
        }
        Sleep(120); action = true;
    }
    if (GetAsyncKeyState(VK_RIGHT) & 0x8000) {
        switch (gameModeIndex){
            case GAMEMODE_CLASSIC:{
                gameMode = CLASSIC;
                screenState = PREPARE;
                break;
            }
            case GAMEMODE_INVISIBLE:{
                gameMode = INVISIBLE;
                screenState = PREPARE;
                break;
            }
        }
        Sleep(120); action = true;
    }
    if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) {
        screenState = MAINMENU;
        system("cls");
        Sleep(120);
    }

    if (action){
        drawGameModeMenu();
    }

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
    next_b = randomInRange(0, 7);
    copyTemplateToCurrent(b);
    isGameOver = false;
    isInGame = true;
    system("cls"); // Xóa màn hình console
}

void gameSettingLoop() {
    settingIndex = 1;
    drawSettingMenu(); // Vẽ menu pause lần đầu

    while (screenState == SETTINGS) {
        // Xử lý input cho menu
        handleSettingInput();

        // Sleep nhỏ để tránh chiếm CPU
        Sleep(50);
    }

    // Khi thoát menu, xóa màn hình và quay lại gameplay
    system("cls");
}

void gameModeLoop() {
    gameModeIndex = 0;
    drawGameModeMenu();

    while (screenState == GAMEMODE) {
        handleGameModeInput();
        Sleep(50);
    }
    system("cls");
}

// Hàm xử lý input màn hình chính
void handleMainMenu() {
    static bool up_held = false;
    static bool down_held = false;
    static bool space_held = false;
    static bool left_held = false;
    static bool right_held = false;


    bool up_now = GetAsyncKeyState(VK_UP) & 0x8000;
    bool down_now = GetAsyncKeyState(VK_DOWN) & 0x8000;
    bool space_now = GetAsyncKeyState(VK_SPACE) & 0x8000;
    bool left_now = GetAsyncKeyState(VK_LEFT) & 0x8000;
    bool right_now = GetAsyncKeyState(VK_RIGHT) & 0x8000;

    // Di chuyển menu lên
    if (up_now && !up_held) {
        mainMenuIndex = (mainMenuIndex + mainMenuCount - 1) % mainMenuCount;
        drawMainMenu();
    }
    up_held = up_now;

    // Di chuyển menu xuống
    if (down_now && !down_held) {
        mainMenuIndex = (mainMenuIndex + 1) % mainMenuCount;
        drawMainMenu();
    }
    down_held = down_now;

    // Chọn menu
    if (space_now && !space_held || left_now && !left_held || right_now && !right_held) {
        switch(mainMenuIndex) {
            case 0: // SELECT GAME MODE
                system("cls");
                screenState = GAMEMODE;
                Sleep(100);
                gameModeLoop();
                resetGame();
                screenState = GAMEPLAY;
                break;
            case 1: // SETTINGS / MENU
                screenState = SETTINGS;
                system("cls");
                gameSettingLoop();
                drawMainMenu();
                break;
            case 2: // EXIT
                exit(0);
        }
    }
    space_held = space_now;
    right_held = right_now;
    left_held = left_now;
}

void mainMenuLoop() {
    isInGame = false;
    drawMainMenu(); // vẽ menu ngay khi vào

    while (screenState == MAINMENU) {
        handleMainMenu(); // xử lý input
        Sleep(20);
    }
}

//void handleGameInput(){
//    if ((GetAsyncKeyState(VK_LEFT) & 0x8000) && canMove(-1, 0)) x--;
//    if ((GetAsyncKeyState(VK_RIGHT) & 0x8000) && canMove(1, 0)) x++;
//    if ((GetAsyncKeyState(VK_DOWN) & 0x8000) && canMove(0, 1)) y++;
//    if (GetAsyncKeyState('C') & 0x8000){
//        rotateBlockClock();
//        _sleep(100); // Delay nhỏ để tránh xoay quá nhanh
//    }
//    if (GetAsyncKeyState('Z') & 0x8000) {
//        rotateBlockCterClock();
//        _sleep(100); // Delay nhỏ để tránh xoay quá nhanh
//    }
//    if (GetAsyncKeyState(VK_SPACE) & 0x8000) {
//        hardDrop();
//        _sleep(100);
//    }
//    if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) {
//        isGameOver = true; // Thoát game chủ động
//    }
//}

//void draw() {
//    gotoxy(0, 0);
//    for (int i = 0; i < H; i++, cout << "\n") {
//        for (int j = 0; j < W; j++) {
//            if (board[i][j] == ' ') cout << "  ";
//            else if (board[i][j] == '#') cout << (unsigned char)219 << (unsigned char)219; // Tường
//            else cout << (unsigned char)219 << (unsigned char)219; // Gạch
//        }
//    }
//    int uiX = W * 2 + 4;
//
//
//    gotoxy(uiX, 4); cout << "Level: " << level;
//    gotoxy(uiX, 5); cout << "Score: " << score;
//
//
//    gotoxy(uiX, 8); cout << "--- NEXT ---";
//
//    for(int i=0; i<4; i++) {
//        gotoxy(uiX + 2, 10 + i); cout << "        ";
//    }
//
//    for (int i = 0; i < 4; i++) {
//        gotoxy(uiX + 2, 10 + i);
//        for (int j = 0; j < 4; j++) {
//            if (blocks[next_b][i][j] != ' ') {
//                cout << (unsigned char)219 << (unsigned char)219;
//            } else {
//                cout << "  ";
//            }
//        }
//    }
//
//    gotoxy(uiX, 15); cout << "--- KEYS ---";
//    gotoxy(uiX, 16); cout << " Left, Right Arrow : Move";
//    gotoxy(uiX, 17); cout << " Z, C              : Rotate";
//    gotoxy(uiX, 18); cout << " Down Arrow        : Soft drop";
//    gotoxy(uiX, 19); cout << " SPACE             : Hard drop";
//    gotoxy(uiX, 20); cout << " M                 : Menu";
//}
string execCurl(string cmd) {
    string result;
    char buffer[256];

    FILE* pipe = _popen(cmd.c_str(), "r");
    if (!pipe) return "ERROR";

    while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
        result += buffer;
    }

    _pclose(pipe);
    return result;
}

void addScore2LocalLdb(string name, int score){
    ofstream ost;
    ost.open(localDir, ios::out | ios::app);
    ost<<name<<" "<<score<<"\n";
    ost.close();
}

void deleteLocalLdb(){
    ofstream ost(localDir);
    ost<<"";
    ost.close();
}

void addScore2GlobalLdb(string name, int score){
    string data = "\"name="+name+"&score="+to_string(score)+"\" ";
    try {
        string json = execCurl("curl -s -d " + data + apiURL);
    } catch (...){
        cout<<"Error! Can't push score to global leaderboard!\n";
    }
}

string get10GlobalLdb(int page){
    string json ="{}";
    try {
        json = execCurl("curl -L -s \"" + apiURL + "?page=" + to_string(page) + "\"");
    } catch (...){
        cout<<"Error! Can't get scores from global leaderboard!\n";
    }
    return json;
}

bool isNumber(string& s){
    for (char c : s){
        if (!isdigit(c)){
            return false;
        }
    }
    return true;
}

bool compareString(vector<string> a, vector<string> b){
    return stoi(a[1]) > stoi(b[1]);
}

vector<vector<string>> getLocalDatas(){
    string name, score;
    vector<vector<string>> datas;
    ifstream ifs(localDir);
    while (ifs>>name>>score){
        vector<string> line;
        line.push_back(name);
        if (isNumber(score)){
            line.push_back(score);
            datas.push_back(line);
        } else{
            continue;
        }
    }
    ifs.close();
    return datas;
}

string get10LocalLdb(){
    string data = "";
    vector<vector<string>> datas = getLocalDatas();
    sort(datas.begin(), datas.end(), compareString);
    for (int i = 0; i < (datas.size() < 10 ? datas.size() : 10); i++){
        data += datas[i][0] + " " + datas[i][1] + "\n";
    }
    return data;
}

vector<vector<string>> processJSON(string json){
    vector<vector<string>> datas;
    stack<char> st;
    int i = 1;
    st.push(json[0]);
    vector<string> line;
    string data = "";

    while (!st.empty()){
        if (json[i] == ']'){
            if (st.size() > 1){
                line.push_back(data);
                data = "";
                datas.push_back(line);
                line.clear();
            }
            st.pop();
        } else if (json[i] == '['){
            st.push(json[i]);
        } else if (json[i] == ','){
            if (st.size() > 1){
                line.push_back(data);
                data = "";
            }
        } else{
            data += json[i];
        }

        i++;
    }

    return datas;
}

void showGameOverScreen() {
    setColor(COLOR_RED);
    gotoxy(W - 4, H / 2 - 2); cout << "=============";
    gotoxy(W - 4, H / 2 - 1); cout << "  GAME OVER  ";
    gotoxy(W - 4, H / 2);     cout << "=============";
    gotoxy(W - 4, H / 2 + 1); cout << "                           ";
    gotoxy(W - 4, H / 2 + 2); cout << "Score: " << score;
    gotoxy(W - 4, H / 2 + 3); cout << "                           ";
    setColor(COLOR_WHITE);
    gotoxy(W - 4, H / 2 + 4); cout << "Press 'SPACE' to Replay    ";
    gotoxy(W - 4, H / 2 + 5); cout << "Press 'Z' to Save score    ";
    gotoxy(W - 4, H / 2 + 6); cout << "Press 'C' to Submit score  ";
    gotoxy(W - 4, H / 2 + 7); cout << "Press 'ESC' to Quit        ";
}

void showInputSaveLocalScore() {
    gotoxy(W - 4, H / 2 + 3); cout << "Input your name:       ";
    gotoxy(W - 4, H / 2 + 4); cout << "                           ";
    gotoxy(W - 4, H / 2 + 5); cout << "Press 'ENTER' to Save      ";
    gotoxy(W - 4, H / 2 + 6); cout << "                           ";
    gotoxy(W - 4, H / 2 + 7); cout << "                           ";
    gotoxy(W - 4, H / 2 + 4);
    FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));
    cin >> name;
    addScore2LocalLdb(name, score);
    screenState = GAMEPLAY;
    showGameOverScreen();
}

void showInputSubmitGlobalScore() {
    gotoxy(W - 4, H / 2 + 3); cout << "Input your name:       ";
    gotoxy(W - 4, H / 2 + 4); cout << "                           ";
    gotoxy(W - 4, H / 2 + 5); cout << "Press 'ENTER' to Submit    ";
    gotoxy(W - 4, H / 2 + 6); cout << "                           ";
    gotoxy(W - 4, H / 2 + 7); cout << "                           ";
    gotoxy(W - 4, H / 2 + 4);
    FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));
    cin >> name;
    name = "\""+name+"\"";
    gotoxy(W - 4, H / 2 + 6); cout << "Submitting your score, please wait...";
    screenState = SUBMITTING;
    addScore2GlobalLdb(name, score);
    screenState = GAMEPLAY;
    showGameOverScreen();
}


void increaseSpeed(int percent) {
    if (speed > 100) speed = speed * (100 - percent) / 100;
}

void removeLine() {
    int linesCleared = 0;
    string floatingText[4] = {"NICE!", "AMAZING!!", "EXCELLENT!!!", "TETRIS!!!!"};
    for (int i = H - 2; i > 0; i--) {
        bool isFull = true;
        for (int j = 1; j < W - 1; j++) {
            if (board[i][j] == ' ') {
                isFull = false;
                break;
            }
        }
        if (isFull) {
            effectLineClear(i-linesCleared);
            effectFloatingText(i-linesCleared, floatingText[linesCleared]);
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
            if (speed > 100) increaseSpeed(10);
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

    // Đóng băng gạch vào bàn cờ
    void lockBlock() {
        for (int i = 0; i < 4; i++)
            for (int j = 0; j < 4; j++)
                if (currentBlock[i][j] != ' ')
                    board[y + i][x + j] = currentBlock[i][j];
    }

    // Kiểm tra ăn dòng
    void checkLines() {
        int lines = 0;
        for (int i = H - 2; i > 0; i--) {
            bool full = true;
            for (int j = 1; j < W - 1; j++)
                if (board[i][j] == ' ') { full = false; break; }
            
            if (full) {
                // Dời các dòng trên xuống
                for (int k = i; k > 0; k--)
                    for (int j = 1; j < W - 1; j++)
                        board[k][j] = board[k - 1][j];
                // Dòng trên cùng thành trống
                for (int j = 1; j < W - 1; j++) board[0][j] = ' ';
                
                i++; // Kiểm tra lại dòng hiện tại (do dòng trên vừa tụt xuống)
                lines++;
            }
        }
        if (lines > 0) score += lines * 100;
void rotateBlockClock() {
    char temp[4][4];
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            temp[j][3 - i] = currentBlock[i][j];
    int offset = 0;
    if (canRotate(temp, offset)) {
        for (int i = 0; i < 4; i++)
            for (int j = 0; j < 4; j++)
                currentBlock[i][j] = temp[i][j];
        x += offset;
    }
}

void rotateBlockCterClock() {
    char temp[4][4];
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            temp[3-j][i] = currentBlock[i][j];
    int offset = 0;
    if (canRotate(temp, offset)) {
        for (int i = 0; i < 4; i++)
            for (int j = 0; j < 4; j++)
                currentBlock[i][j] = temp[i][j];
        x += offset;
    }

    // Xử lý phím bấm
    void input() {
        if (gameOver) return;

        // Player 1: WASD
        if (playerID == 1) {
            if ((GetAsyncKeyState('A') & 0x8000) && checkCollision(-1, 0)) x--;
            if ((GetAsyncKeyState('D') & 0x8000) && checkCollision(1, 0)) x++;
            if ((GetAsyncKeyState('S') & 0x8000) && checkCollision(0, 1)) y++;
            if ((GetAsyncKeyState('W') & 0x8000)) { rotate(); Sleep(100); } // Sleep nhỏ để tránh xoay quá nhanh
        }
        // Player 2: Mũi tên
        else {
            if ((GetAsyncKeyState(VK_LEFT) & 0x8000) && checkCollision(-1, 0)) x--;
            if ((GetAsyncKeyState(VK_RIGHT) & 0x8000) && checkCollision(1, 0)) x++;
            if ((GetAsyncKeyState(VK_DOWN) & 0x8000) && checkCollision(0, 1)) y++;
            if ((GetAsyncKeyState(VK_UP) & 0x8000)) { rotate(); Sleep(100); }
        }
    }

    void draw() {
        // Vẽ tiêu đề
        gotoxy(offsetX, 0); cout << "PLAYER " << playerID;
        gotoxy(offsetX, 1); cout << "Score: " << score;

        for (int i = 0; i < H; i++) {
            gotoxy(offsetX, i + 3);
            for (int j = 0; j < W; j++) {
                // Kiểm tra xem có phải là gạch đang rơi không
                bool isFalling = false;
                if (!gameOver && j >= x && j < x + 4 && i >= y && i < y + 4) {
                    if (currentBlock[i - y][j - x] != ' ') {
                        cout << "[]";
                        isFalling = true;
                    }
                }

                if (!isFalling) {
                    if (board[i][j] == '#') cout << (char)178 << (char)178; // Tường
                    else if (board[i][j] == ' ') cout << " ."; // Nền
                    else cout << "[]"; // Gạch đã đóng băng
                }
            }
        }
        
        if (gameOver) {
            gotoxy(offsetX + 2, H / 2 + 3);
            cout << "GAME OVER!";
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
    if (gameMode != INVISIBLE){
        effectParticles(x + 1, y + 3);
        effectParticles(x + 2, y + 3);
    }

    block2Board();
    removeLine();
    x = 5; y = 0; b = next_b; next_b = randomInRange(0, 7);

    copyTemplateToCurrent(b);
    if (!canMove(0, 0)) {
        isGameOver = true;
    }
  currentSpeed = 0;
}

void handleGameInput(){
    if ((GetAsyncKeyState(VK_LEFT) & 0x8000) && canMove(-1, 0)) x--;
    if ((GetAsyncKeyState(VK_RIGHT) & 0x8000) && canMove(1, 0)) x++;
    if ((GetAsyncKeyState(VK_DOWN) & 0x8000) && canMove(0, 1)) y++;

    if (GetAsyncKeyState('C') & 0x8000){ rotateBlockClock(); Sleep(100); }
    if (GetAsyncKeyState('Z') & 0x8000) { rotateBlockCterClock(); Sleep(100); }
    if (GetAsyncKeyState(VK_SPACE) & 0x8000) { hardDrop(); Sleep(200); }
    if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) { isGameOver = true; }

}

void generateNextBlock(){
    b = next_b;
    next_b = randomInRange(0, 7);
    copyTemplateToCurrent(b);
    x = 4; y = 0;
}

void debug(){
    ofstream ost("text.txt");
    for (int i = 0; i < H; i++){
        for (int j = 0; j < W; j++){
            ost<<board[i][j];
        }
        ost<<"\n";
    }
    ost.close();
}

int main()
{
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dwMode = 0;
    GetConsoleMode(hOut, &dwMode);
    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hOut, dwMode);
    SetConsoleOutputCP(437);
    srand(time(0));
    hideCursor();
    gameMode = INVISIBLE;

    drawMainMenu();

    updateBackgroundMusic();     // Bắt đầu nhạc nền
    setMusicVolume(settings.volumePercent); // Volume mặc định
    // Vòng lặp chính của ứng dụng (Game App Loop)
    while (true) {
        // ===== MAIN MENU =====
        if (screenState == MAINMENU){
            mainMenuLoop();
            continue;
        }
        // Vòng lặp ván chơi (Gameplay Loop)
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
                if (screenState == MAINMENU) break; // Thoát vòng lặp gameplay nếu chọn Back to Main Menu
                continue; // không xử lý gameplay khi đang ở menu
            }

            // Gameplay
            handleGameInput();
            boardDelBlock();

            if (canFall()) {
                if (canMove(0, 1)) {
                    y++;
                } else {
                    block2Board();
                    removeLine();
                    // --- SINH GẠCH MỚI VÀ KIỂM TRA THUA ---
                    generateNextBlock();
                    if (!canMove(0, 0)) { isGameOver = true; }
                }
            }
            block2Board();
            draw();
            debug();
            Sleep(tick);
        }

        if (screenState == MAINMENU) continue;
        // --- Xử lý khi Game Over ---
        showGameOverScreen();
        isInGame = false;
        // Đợi người chơi chọn: R để chơi lại, ESC để thoát hẳn
        while (true) {
            if (GetAsyncKeyState(VK_SPACE) & 0x8000) {
                break; // Thoát vòng lặp chờ -> Quay lại vòng lặp Game App -> resetGame()
            }
            if (GetAsyncKeyState('Z') & 0x8000){
                showInputSaveLocalScore();
                screenState = SAVE;
            }
            if (GetAsyncKeyState('C') & 0x8000){
                showInputSubmitGlobalScore();
                screenState = SUBMIT;
            }
            if (GetAsyncKeyState('R') & 0x8000) {
                screenState = GAMEPLAY;
                break; // Thoát vòng lặp chờ -> Quay lại vòng lặp Game App -> resetGame()
            }
            if (GetAsyncKeyState('M') & 0x8000) {
                screenState = MAINMENU;
                break; // Thoát vòng lặp chờ -> Quay lại vòng lặp Game App -> mainMenuLoop()
            }

            if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) {
                return 0;
            }

            Sleep(100);
        }
    }
};

int main() {
    SetConsoleOutputCP(437); // Hiển thị ký tự đặc biệt
    hideCursor();
    srand(time(0)); // Reset bộ sinh số ngẫu nhiên

    // Khởi tạo 2 người chơi ở 2 vị trí màn hình khác nhau
    // Player 1 ở tọa độ x=2, Player 2 ở tọa độ x=40
    TetrisPlayer p1(2, 1);
    TetrisPlayer p2(W * 2 + 10, 2); 

    system("cls");
    
    // Màn hình chờ
    gotoxy(15, 10); cout << "READY? PRESS ENTER TO START!";
    while(true) {
        if (GetAsyncKeyState(VK_RETURN) & 0x8000) break;
    }
    system("cls");

    while (true) {
        // 1. Nhận Input
        p1.input();
        p2.input();

        // 2. Xử lý Logic
        p1.update();
        p2.update();

        // 3. Vẽ ra màn hình
        p1.draw();
        p2.draw();
        
        // Thoát game
        if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) break;

        Sleep(TICK);
    }

    return 0;
}