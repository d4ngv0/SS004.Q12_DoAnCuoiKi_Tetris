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
string localDir = "scoreboards.txt";
string apiURL = "https://script.google.com/macros/s/AKfycbyBSofw1Jugm68awOHDcthLfNzTuGC_2rxkbTafpgLc3w1NIfnHKwvJmOfIC_0FEuoX/exec";
int speed = 1000;
int currentSpeed = 0;
int level = 1;
int score = 0; // Thêm điểm số
int next_b=0;
int x = 4, y = 0, b = 1;
bool isGameOver = false;
enum Screen { GAMEPLAY, MENU, PAUSE };
Screen screenState = GAMEPLAY;
int menuIndex = 0; // mục đang chọn trong menu
const char* menuItems[] = {"Sound enabled", "Volume", "Fall speed", "Resume"};
const int menuCount = sizeof(menuItems) / sizeof(menuItems[0]);

// --- Các hàm đồ họa & Logic ---

struct Settings {
    int volumePercent = 50;   // 0 - 100 (giả lập, vì hiện chưa có phát âm)
    bool soundEnabled = false; // có/không dùng hiệu ứng âm thanh sau này
    int fallSpeedPercent = 100; // 50-200 (map vào 'speed' của game)
};
Settings settings;

// --- Hàm vẽ menu console ---
void drawMenu() {
    system("cls");
    cout << "===== MENU CHINH =====\n\n";
    for (int i = 0; i < menuCount; ++i) {
        // highlight mục đang chọn
        if (i == menuIndex) cout << " > ";
        else cout << "   ";

        if (i == 0) {
            cout << "Sound enabled: " << (settings.soundEnabled ? "ON" : "OFF") << "\n";
        } else if (i == 1) {
            cout << "Volume: " << settings.volumePercent << "%\n";
        } else if (i == 2) {
            cout << "Fall speed: " << settings.fallSpeedPercent << "%\n";
        } else if (i == 3) {
            cout << "Resume game\n";
        }
    }
    cout << "\n↑/↓: Chon muc | ←/→: Giam/Tang | Enter: Ap dung/Thuc thi | ESC/M: Thoat menu\n";
}

// --- Map fallSpeedPercent -> speed (ms giữa các lần rơi) ---
void applyFallSpeed() {
    // speed gốc của bạn là 1000 ms. Cho phép từ 50% (nhanh) đến 200% (chậm)
    int base = 1000;
    int pct = settings.fallSpeedPercent; // 50 - 200
    speed = base * pct / 100;
    if (speed < 50) speed = 50; // đừng quá nhanh để tránh “rơi liên tục”
}

void handleMenuInput() {
    // Di chuyển chọn bằng mũi tên lên/xuống
    if (GetAsyncKeyState(VK_UP) & 0x8000) {
        if (menuIndex > 0) menuIndex--;
        Sleep(120); // chống lặp quá nhanh
    }
    if (GetAsyncKeyState(VK_DOWN) & 0x8000) {
        if (menuIndex < menuCount - 1) menuIndex++;
        Sleep(120);
    }

    // Điều chỉnh giá trị bằng mũi tên trái/phải
    if (GetAsyncKeyState(VK_LEFT) & 0x8000) {
        if (menuIndex == 0) {
            settings.soundEnabled = !settings.soundEnabled;
        } else if (menuIndex == 1) {
            settings.volumePercent = max(0, settings.volumePercent - 5);
        } else if (menuIndex == 2) {
            settings.fallSpeedPercent = max(50, settings.fallSpeedPercent - 5);
            applyFallSpeed();
        }
        Sleep(120);
    }
    if (GetAsyncKeyState(VK_RIGHT) & 0x8000) {
        if (menuIndex == 0) {
            settings.soundEnabled = !settings.soundEnabled;
        } else if (menuIndex == 1) {
            settings.volumePercent = min(100, settings.volumePercent + 5);
        } else if (menuIndex == 2) {
            settings.fallSpeedPercent = min(200, settings.fallSpeedPercent + 5);
            applyFallSpeed();
        }
        Sleep(120);
    }

    // Enter: thực thi mục Resume
    if (GetAsyncKeyState(VK_RETURN) & 0x8000) {
        if (menuIndex == 3) {
            screenState = GAMEPLAY; // quay lại chơi
            system("cls");
        }
        Sleep(150);
    }

    // ESC hoặc M: thoát menu
    if ((GetAsyncKeyState(VK_ESCAPE) & 0x8000) || (GetAsyncKeyState('M') & 0x8000)) {
        screenState = GAMEPLAY;
        system("cls");
        Sleep(150);
    }
}

void gotoxy(int x, int y) {
    COORD c = {(short)x, (short)y};
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), c);
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

// Hàm Reset toàn bộ dữ liệu để chơi lại
void resetGame() {
    initBoard();
    speed = 1000;
    currentSpeed = 0;
    level = 1;
    score = 0;
    x = 4; y = 0;
    b = rand() % 7;
    next_b=rand() %7;
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
    gotoxy(uiX, 16); cout << "--- KEYS ---";
    gotoxy(uiX, 17); cout << " A, D : Move";
    gotoxy(uiX, 18); cout << " W    : Rotate";
    gotoxy(uiX, 19); cout << " S    : Drop";
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

void hardDrop(){
    while (canMove(0,1)) {
        y++;
        block2Board();
        boardDelBlock();
    }
    block2Board();
    removeLine();
    x = 5; y = 0; b = rand() % 7;
    currentSpeed = 0;
}

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
        string json = execCurl("curl -d " + data + apiURL);
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

void debug(vector<vector<string>> datas){
    for (int i = 0; i < datas.size(); i++){
        cout<<datas[i][0]<<" "<<datas[i][1]<<" "<<datas[i][2]<<"\n";
    }
}

int main()
{
    SetConsoleOutputCP(437);
    srand(time(0));
    hideCursor();

    // Vòng lặp chính của ứng dụng (Game App Loop)
    while (true) {
        resetGame(); // 1. Khởi tạo dữ liệu mới

        // Vòng lặp ván chơi (Gameplay Loop)
        while (!isGameOver) {
            // Phím mở menu
            if (GetAsyncKeyState('M') & 0x8000) {
                screenState = MENU;
                menuIndex = 0;
                drawMenu();
                Sleep(150);
            }

            if (screenState == MENU) {
                drawMenu();
                handleMenuInput();
                continue; // không xử lý gameplay khi đang ở menu
            }

            boardDelBlock(); // Xóa vị trí cũ của gạch

            // Xử lý Input
            if ((GetAsyncKeyState('A') & 0x8000) && canMove(-1, 0)) x--;
            if ((GetAsyncKeyState('D') & 0x8000) && canMove(1, 0)) x++;
            if ((GetAsyncKeyState('S') & 0x8000) && canMove(0, 1)) y++;
            if ((GetAsyncKeyState('W') & 0x8000)) {
                rotateBlock();
                Sleep(100); // Delay nhỏ để tránh xoay quá nhanh
            }
            if (GetAsyncKeyState(VK_SPACE) & 0x8000) {
                hardDrop();
                _sleep(200);
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
                    b=next_b;
                    next_b=rand() %7;

                    // --- SINH GẠCH MỚI VÀ KIỂM TRA THUA ---
                    x = 4; y = 0;

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
