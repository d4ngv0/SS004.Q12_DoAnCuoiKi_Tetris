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

// Hình dáng 7 loại gạch Tetris chuẩn
char blocks[7][4][4] = {
    {{' ','I',' ',' '}, {' ','I',' ',' '}, {' ','I',' ',' '}, {' ','I',' ',' '}}, // I
    {{' ',' ',' ',' '}, {' ','O','O',' '}, {' ','O','O',' '}, {' ',' ',' ',' '}}, // O
    {{' ',' ',' ',' '}, {' ','T',' ',' '}, {'T','T','T',' '}, {' ',' ',' ',' '}}, // T
    {{' ',' ',' ',' '}, {' ','S','S',' '}, {'S','S',' ',' '}, {' ',' ',' ',' '}}, // S
    {{' ',' ',' ',' '}, {'Z','Z',' ',' '}, {' ','Z','Z',' '}, {' ',' ',' ',' '}}, // Z
    {{' ',' ',' ',' '}, {'J',' ',' ',' '}, {'J','J','J',' '}, {' ',' ',' ',' '}}, // J
    {{' ',' ',' ',' '}, {' ',' ','L',' '}, {'L','L','L',' '}, {' ',' ',' ',' '}}  // L
};

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