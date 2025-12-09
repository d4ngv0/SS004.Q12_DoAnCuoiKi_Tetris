#include <iostream>
#include <vector>
#include <windows.h>
#include <ctime>
#include <conio.h>

using namespace std;

// --- Cấu hình Game ---
#define H 20
#define W 12  // Giảm chiều rộng một chút để vừa 2 bảng
#define TICK 50

// Hình dáng các khối gạch
char blocks[7][4][4] = {
    {{' ','I',' ',' '}, {' ','I',' ',' '}, {' ','I',' ',' '}, {' ','I',' ',' '}}, // I
    {{' ',' ',' ',' '}, {' ','O','O',' '}, {' ','O','O',' '}, {' ',' ',' ',' '}}, // O
    {{' ',' ',' ',' '}, {' ','T',' ',' '}, {'T','T','T',' '}, {' ',' ',' ',' '}}, // T
    {{' ',' ',' ',' '}, {' ','S','S',' '}, {'S','S',' ',' '}, {' ',' ',' ',' '}}, // S
    {{' ',' ',' ',' '}, {'Z','Z',' ',' '}, {' ','Z','Z',' '}, {' ',' ',' ',' '}}, // Z
    {{' ',' ',' ',' '}, {'J',' ',' ',' '}, {'J','J','J',' '}, {' ',' ',' ',' '}}, // J
    {{' ',' ',' ',' '}, {' ',' ','L',' '}, {'L','L','L',' '}, {' ',' ',' ',' '}}  // L
};

void gotoxy(int x, int y) {
    COORD c = {(short)x, (short)y};
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), c);
}

// --- Class TetrisGame (Dùng cho cả Người và AI) ---
class TetrisGame {
private:
    char board[H][W];
    int x, y, type;      // Tọa độ và loại gạch hiện tại
    int rotation;        // Trạng thái xoay (0-3)
    int score;
    int speed;
    int currentSpeed;
    int offsetX;         // Vị trí vẽ bảng (để vẽ 2 bảng song song)
    bool isAI;           // Đánh dấu đây là bot hay người
    bool gameOver;
    
    // Biến dùng riêng cho AI
    int aiTargetX;
    int aiTargetRot;
    int aiMoveTimer;

public:
    char currentBlock[4][4];

    TetrisGame(int offset, bool aiMode) {
        offsetX = offset;
        isAI = aiMode;
        reset();
    }

    void reset() {
        for (int i = 0; i < H; i++)
            for (int j = 0; j < W; j++)
                if (i == H - 1 || j == 0 || j == W - 1) board[i][j] = '#';
                else board[i][j] = ' ';
        
        score = 0;
        speed = 1000;
        currentSpeed = 0;
        gameOver = false;
        spawnBlock();
    }

    void spawnBlock() {
        x = W / 2 - 2;
        y = 0;
        type = rand() % 7;
        rotation = 0;
        
        // Copy block gốc vào currentBlock
        for(int i=0; i<4; i++)
            for(int j=0; j<4; j++)
                currentBlock[i][j] = blocks[type][i][j];

        if (!canMove(0, 0)) gameOver = true;

        // Nếu là AI, tính toán nước đi ngay khi gạch mới sinh ra
        if (isAI && !gameOver) {
            findBestMove();
            aiMoveTimer = 0;
        }
    }

    // Kiểm tra va chạm
    bool canMove(int dx, int dy, char customBlock[4][4] = nullptr) {
        char (*blk)[4] = (customBlock == nullptr) ? currentBlock : customBlock;
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                if (blk[i][j] != ' ') {
                    int tx = x + j + dx;
                    int ty = y + i + dy;
                    if (tx < 0 || tx >= W || ty >= H) return false;
                    if (board[ty][tx] != ' ') return false;
                }
            }
        }
        return true;
    }

    void rotate() {
        char temp[4][4];
        for (int i = 0; i < 4; i++)
            for (int j = 0; j < 4; j++)
                temp[j][3 - i] = currentBlock[i][j];

        // Kiểm tra xem xoay xong có bị kẹt không?
        // Lưu tạm block cũ để check
        char savedBlock[4][4];
        memcpy(savedBlock, currentBlock, sizeof(currentBlock));
        memcpy(currentBlock, temp, sizeof(temp)); // Thử xoay

        if (!canMove(0, 0)) {
            memcpy(currentBlock, savedBlock, sizeof(savedBlock)); // Hoàn tác nếu kẹt
        } else {
            rotation = (rotation + 1) % 4;
        }
    }

    // Helper cho AI: Xoay block giả lập mà không check va chạm bàn cờ thực tế
    void rotateSimulate(char blk[4][4]) {
        char temp[4][4];
        for (int i = 0; i < 4; i++)
            for (int j = 0; j < 4; j++)
                temp[j][3 - i] = blk[i][j];
        memcpy(blk, temp, sizeof(temp));
    }

    void updateLogic() {
        if (gameOver) return;

        // --- Logic AI Tự động di chuyển ---
        if (isAI) {
            aiMoveTimer += TICK;
            if (aiMoveTimer > 100) { // Tốc độ di chuyển ngang của AI
                if (rotation != aiTargetRot) rotate();
                else if (x < aiTargetX) if(canMove(1,0)) x++;
                else if (x > aiTargetX) if(canMove(-1,0)) x--;
                aiMoveTimer = 0;
            }
        }

        // --- Logic Rơi tự do ---
        currentSpeed += TICK;
        if (currentSpeed >= speed || (isAI && currentSpeed >= 100)) { // AI rơi nhanh hơn chút
            if (canMove(0, 1)) {
                y++;
            } else {
                lockBlock();
                checkLines();
                spawnBlock();
            }
            currentSpeed = 0;
        }
    }

    // Input cho người chơi
    void handleInput() {
        if (isAI || gameOver) return;
        
        if ((GetAsyncKeyState('A') & 0x8000) && canMove(-1, 0)) x--;
        if ((GetAsyncKeyState('D') & 0x8000) && canMove(1, 0)) x++;
        if ((GetAsyncKeyState('X') & 0x8000) && canMove(0, 1)) y++;
        if (GetAsyncKeyState('W') & 0x8000) { 
            rotate(); 
            Sleep(150); // Delay tránh xoay quá nhanh
        } 
    }

    void lockBlock() {
        for (int i = 0; i < 4; i++)
            for (int j = 0; j < 4; j++)
                if (currentBlock[i][j] != ' ')
                    board[y + i][x + j] = currentBlock[i][j];
    }

    void checkLines() {
        int lines = 0;
        for (int i = H - 2; i > 0; i--) {
            bool full = true;
            for (int j = 1; j < W - 1; j++)
                if (board[i][j] == ' ') { full = false; break; }
            
            if (full) {
                for (int k = i; k > 0; k--)
                    for (int j = 1; j < W - 1; j++)
                        board[k][j] = board[k - 1][j];
                for (int j = 1; j < W - 1; j++) board[0][j] = ' ';
                i++; // Kiểm tra lại dòng vừa rơi xuống
                lines++;
            }
        }
        if(lines > 0) score += lines * 100;
    }

    void draw() {
        // Vẽ tiêu đề
        gotoxy(offsetX, 0); 
        cout << (isAI ? "=== BOT AI ===" : "=== PLAYER ===");
        gotoxy(offsetX, 1); cout << "Score: " << score;

        // Vẽ Board
        for (int i = 0; i < H; i++) {
            gotoxy(offsetX, i + 3);
            for (int j = 0; j < W; j++) {
                // Kiểm tra xem có phải gạch đang rơi không
                bool isFallingBlock = false;
                if (!gameOver) {
                    if (j >= x && j < x + 4 && i >= y && i < y + 4) {
                        if (currentBlock[i - y][j - x] != ' ') {
                            cout << "[]";
                            isFallingBlock = true;
                        }
                    }
                }
                
                if (!isFallingBlock) {
                    if (board[i][j] == '#') cout << (char)178 << (char)178; // Tường
                    else if (board[i][j] == ' ') cout << " .";
                    else cout << "[]"; // Gạch đã cố định
                }
            }
        }
        if (gameOver) {
            gotoxy(offsetX + 2, H / 2 + 3);
            cout << "GAME OVER";
        }
    }

    // --- AI ALGORITHM (HEURISTICS) ---
    // Hàm đánh giá điểm số của bàn cờ (Càng cao càng tốt)
    double evaluateGrid(const char tempBoard[H][W]) {
        double aggregateHeight = 0;
        int completeLines = 0;
        int holes = 0;
        int bumpiness = 0;

        int colHeight[W] = {0};

        // 1. Tính chiều cao các cột
        for (int j = 1; j < W - 1; j++) {
            for (int i = 1; i < H - 1; i++) {
                if (tempBoard[i][j] != ' ') {
                    colHeight[j] = (H - 1) - i;
                    aggregateHeight += colHeight[j];
                    break;
                }
            }
        }

        // 2. Đếm số dòng hoàn thành
        for (int i = 1; i < H - 1; i++) {
            bool full = true;
            for (int j = 1; j < W - 1; j++)
                if (tempBoard[i][j] == ' ') { full = false; break; }
            if (full) completeLines++;
        }

        // 3. Đếm Holes (Lỗ hổng - ô trống bị chặn bên trên)
        for (int j = 1; j < W - 1; j++) {
            bool blockFound = false;
            for (int i = 1; i < H - 1; i++) {
                if (tempBoard[i][j] != ' ') blockFound = true;
                else if (blockFound && tempBoard[i][j] == ' ') holes++;
            }
        }

        // 4. Tính Bumpiness (Độ lồi lõm)
        for (int j = 1; j < W - 2; j++) {
            bumpiness += abs(colHeight[j] - colHeight[j + 1]);
        }

        // Công thức Heuristic phổ biến cho Tetris
        return (-0.51 * aggregateHeight) + (0.76 * completeLines) - (0.36 * holes) - (0.18 * bumpiness);
    }

    void findBestMove() {
        double bestScore = -999999.0;
        aiTargetX = x;
        aiTargetRot = 0;

        char simBlock[4][4];
        
        // Thử tất cả 4 góc xoay
        for (int r = 0; r < 4; r++) {
            // Tạo block xoay giả lập
            memcpy(simBlock, blocks[type], sizeof(simBlock));
            for(int k=0; k<r; k++) rotateSimulate(simBlock); 

            // Thử tất cả các cột có thể
            for (int tx = -2; tx < W; tx++) {
                // 1. Kiểm tra xem vị trí ngang này có hợp lệ không
                int tempY = y;
                // Hàm check va chạm local (phải viết lại dạng đơn giản cho loop này)
                bool validPos = true;
                for(int i=0; i<4; i++)
                    for(int j=0; j<4; j++)
                        if(simBlock[i][j] != ' ') {
                            if (tx + j < 1 || tx + j >= W - 1) validPos = false; // Chạm tường
                        }
                if (!validPos) continue;

                // 2. Tìm vị trí rơi xuống đáy (Drop)
                while (true) {
                    bool collision = false;
                    for(int i=0; i<4; i++) {
                        for(int j=0; j<4; j++) {
                            if(simBlock[i][j] != ' ') {
                                if (tempY + i + 1 >= H - 1 || board[tempY + i + 1][tx + j] != ' ') 
                                    collision = true;
                            }
                        }
                    }
                    if (collision) break;
                    tempY++;
                }

                // 3. Tạo bàn cờ giả định sau khi thả
                char tempBoard[H][W];
                memcpy(tempBoard, board, sizeof(board));
                for(int i=0; i<4; i++)
                    for(int j=0; j<4; j++)
                        if(simBlock[i][j] != ' ')
                            if(tempY + i < H) tempBoard[tempY + i][tx + j] = '#'; // Đánh dấu block

                // 4. Chấm điểm bàn cờ này
                double currentScore = evaluateGrid(tempBoard);

                if (currentScore > bestScore) {
                    bestScore = currentScore;
                    aiTargetX = tx;
                    aiTargetRot = r;
                }
            }
        }
    }
};

int main() {
    SetConsoleOutputCP(437);
    srand(time(0));
    
    // Tạo 2 instance game: Player (trái) và Bot (phải)
    TetrisGame player(2, false); 
    TetrisGame bot(W * 2 + 8, true); // Offset sang phải để vẽ

    system("cls");

    while (true) {
        // Xử lý Input Player
        player.handleInput();
        if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) break;

        // Cập nhật Logic
        player.updateLogic();
        bot.updateLogic();

        // Vẽ màn hình
        player.draw();
        bot.draw();

        Sleep(TICK);
    }
    return 0;
}