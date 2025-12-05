#include <iostream>
#include <conio.h>
#include <windows.h>
#include <ctime>
using namespace std;

#define H 20
#define W 15

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
int speed = 1000;
int currentSpeed = 0;
int level = 0;
int score = 0; // Thêm điểm số
int x = 4, y = 0, b = 1;
bool isGameOver = false;

// --- Các hàm đồ họa & Logic ---
void gotoxy(int x, int y) {
    COORD c = {(short)x, (short)y};
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), c);
}

void initBoard() {
    for (int i = 0; i < H; i++)
        for (int j = 0; j < W; j++)
            if ((i == H - 1) || (j == 0) || (j == W - 1)) board[i][j] = '#';
            else board[i][j] = ' ';
}

// Hàm Reset toàn bộ dữ liệu để chơi lại
void resetGame() {
    initBoard();
    speed = 1000;
    currentSpeed = 0;
    level = 0;
    score = 0;
    x = 4; y = 0;
    b = rand() % 7;
    isGameOver = false;
    system("cls"); // Xóa màn hình console
}

void draw() {
    gotoxy(0, 0);
    for (int i = 0; i < H; i++, cout << "\n") {
        for (int j = 0; j < W; j++) {
            if (board[i][j] == ' ') cout << "  ";
            else if (board[i][j] == '#') cout << (unsigned char)219 << (unsigned char)219; // Tường
            else cout << (unsigned char)219 << (unsigned char)219; // Gạch
        }
    }
    // Hiển thị thông tin bên cạnh
    gotoxy(W * 2 + 2, 5); cout << "Level: " << level;
    gotoxy(W * 2 + 2, 6); cout << "Score: " << score;
    gotoxy(W * 2 + 2, 8); cout << "A,D: Move";
    gotoxy(W * 2 + 2, 9); cout << "W: Rotate";
    gotoxy(W * 2 + 2, 10); cout << "X: Down";
}

void showGameOverScreen() {
    gotoxy(W - 4, H / 2 - 2); cout << "=============";
    gotoxy(W - 4, H / 2 - 1); cout << "  GAME OVER  ";
    gotoxy(W - 4, H / 2);     cout << "=============";
    gotoxy(W - 4, H / 2 + 2); cout << "Score: " << score;
    gotoxy(W - 4, H / 2 + 4); cout << "Press 'R' to Replay";
    gotoxy(W - 4, H / 2 + 5); cout << "Press 'ESC' to Quit";
}

// Logic khối gạch (Giữ nguyên như cũ)
void boardDelBlock() {
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            if (blocks[b][i][j] != ' ' && y + i < H && x + j < W)
                board[y + i][x + j] = ' ';
}

void block2Board() {
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            if (blocks[b][i][j] != ' ')
                board[y + i][x + j] = blocks[b][i][j];
}

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

void increaseSpeed(int percent) {
    if (speed > 100) speed = speed * (100 - percent) / 100;
}

void removeLine() {
    for (int i = H - 2; i > 0; i--) {
        bool isFull = true;
        for (int j = 1; j < W - 1; j++) {
            if (board[i][j] == ' ') {
                isFull = false;
                break;
            }
        }
        if (isFull) {
            for (int k = i; k > 0; k--)
                for (int j = 1; j < W - 1; j++)
                    board[k][j] = board[k - 1][j];
            for (int j = 1; j < W - 1; j++) board[0][j] = ' ';
            i++;
            score += 100; // Cộng điểm
            increaseSpeed(5);
            level++;
        }
    }
}

bool canRotate(char temp[4][4]) {
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            if (temp[i][j] != ' ') {
                int tx = x + j;
                int ty = y + i;
                if (tx < 1 || tx >= W - 1 || ty >= H - 1) return false;
                if (board[ty][tx] != ' ') return false;
            }
        }
    }
    return true;
}

void rotateBlock() {
    char temp[4][4];
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            temp[j][3 - i] = blocks[b][i][j];
    
    if (canRotate(temp)) {
        for (int i = 0; i < 4; i++)
            for (int j = 0; j < 4; j++)
                blocks[b][i][j] = temp[i][j];
    }
}

bool canFall() {
    currentSpeed += tick;
    if (currentSpeed >= speed) {
        currentSpeed = 0; // Reset bộ đếm
        return true;
    }
    return false;
}

int main() {
    SetConsoleOutputCP(437);
    srand(time(0));
    
    // Vòng lặp chính của ứng dụng (Game App Loop)
    while (true) {
        resetGame(); // 1. Khởi tạo dữ liệu mới

        // Vòng lặp ván chơi (Gameplay Loop)
        while (!isGameOver) {
            boardDelBlock(); // Xóa vị trí cũ của gạch

            // Xử lý Input
            if ((GetAsyncKeyState('A') & 0x8000) && canMove(-1, 0)) x--;
            if ((GetAsyncKeyState('D') & 0x8000) && canMove(1, 0)) x++;
            if ((GetAsyncKeyState('X') & 0x8000) && canMove(0, 1)) y++;
            if ((GetAsyncKeyState('W') & 0x8000)) {
                rotateBlock(); 
                Sleep(100); // Delay nhỏ để tránh xoay quá nhanh
            }
            if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) {
                isGameOver = true; // Thoát game chủ động
            }

            // Xử lý rơi tự do
            if (canFall()) {
                if (canMove(0, 1)) {
                    y++;
                } else {
                    // Nếu không rơi được nữa -> Chạm đáy/Gạch khác
                    block2Board(); // Cố định gạch vào board
                    removeLine();  // Ăn điểm
                    
                    // --- SINH GẠCH MỚI VÀ KIỂM TRA THUA ---
                    x = 4; y = 0; b = rand() % 7;
                    
                    // Kiểm tra ngay tại chỗ sinh ra có bị kẹt không?
                    if (!canMove(0, 0)) { 
                        isGameOver = true;
                    }
                }
            }

            block2Board(); // Vẽ gạch vào vị trí mới
            draw();        // Render màn hình
            Sleep(tick);  // Giữ FPS ổn định
        }

        // --- Xử lý khi Game Over ---
        showGameOverScreen();
        
        // Đợi người chơi chọn: R để chơi lại, ESC để thoát hẳn
        while (true) {
            if (GetAsyncKeyState('R') & 0x8000) {
                break; // Thoát vòng lặp chờ -> Quay lại vòng lặp Game App -> resetGame()
            }
            if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) {
                return 0; // Thoát chương trình
            }
            Sleep(100);
        }
    }
    return 0;
}