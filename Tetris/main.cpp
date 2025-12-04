#include <iostream>
#include <conio.h>
#include <windows.h>
using namespace std;
#define H 20
#define W 15
char board[H][W] = {} ;
char blocks[][4][4] = {
        {{' ','I',' ',' '},
         {' ','I',' ',' '},
         {' ','I',' ',' '},
         {' ','I',' ',' '}},
        {{' ',' ',' ',' '},
         {' ','O','O',' '},
         {' ','O','O',' '},
         {' ',' ',' ',' '}},
        {{' ',' ',' ',' '},
         {' ','T',' ',' '},
         {'T','T','T',' '},
         {' ',' ',' ',' '}},
        {{' ',' ',' ',' '},
         {' ','S','S',' '},
         {'S','S',' ',' '},
         {' ',' ',' ',' '}},
        {{' ',' ',' ',' '},
         {'Z','Z',' ',' '},
         {' ','Z','Z',' '},
         {' ',' ',' ',' '}},
        {{' ',' ',' ',' '},
         {'J',' ',' ',' '},
         {'J','J','J',' '},
         {' ',' ',' ',' '}},
        {{' ',' ',' ',' '},
         {' ',' ','L',' '},
         {'L','L','L',' '},
         {' ',' ',' ',' '}}
};

const int tick = 50; // 20 fps
int speed = 1000;
int currentSpeed = 0;
int level = 0;
int x=4,y=0,b=1;


void gotoxy(int x, int y) {
    COORD c = {x, y};
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), c);
}
void boardDelBlock(){
    for (int i = 0 ; i < 4 ; i++)
        for (int j = 0 ; j < 4 ; j++)
            if (blocks[b][i][j] != ' ' && y+j < H)
                board[y+i][x+j] = ' ';
}
void block2Board(){
    for (int i = 0 ; i < 4 ; i++)
        for (int j = 0 ; j < 4 ; j++)
            if (blocks[b][i][j] != ' ' )
                board[y+i][x+j] = blocks[b][i][j];
}
void initBoard(){
    for (int i = 0 ; i < H ; i++)
        for (int j = 0 ; j < W ; j++)
            if ((i==H-1) || (j==0) || (j == W-1)) board[i][j] = '#';
            else board[i][j] = ' ';
}
void draw() {
    gotoxy(0, 0);
    for (int i = 0; i < H; i++, cout <<"\n") {
        for (int j = 0; j < W; j++) {
            if (board[i][j] == ' ') {
                cout << "  ";
            }
            else {

                unsigned char c = 219;
                cout << c << c;
            }
        }
    }
}
bool canMove(int dx, int dy){
    for (int i = 0 ; i < 4 ; i++)
        for (int j = 0 ; j < 4 ; j++)
            if (blocks[b][i][j] != ' '){
                int tx = x + j + dx;
                int ty = y + i + dy;
                if ( tx<1 || tx >= W-1 || ty >= H-1) return false;
                if ( board[ty][tx] != ' ') return false;
            }
    return true;
}

void increaseSpeed(int percent) {
    if (speed > 50)
        speed = speed * (100 - percent) / 100;
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
            for (int k = i; k > 0; k--) {
                for (int j = 1; j < W - 1; j++) {
                    board[k][j] = board[k - 1][j];
                }
            }
            for (int j = 1; j < W - 1; j++) board[0][j] = ' ';

            i++;

            draw();
            // _sleep(100);
            increaseSpeed(10);
            level += 1;
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
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++)
            temp[j][3 - i] = blocks[b][i][j];
    }
    if (canRotate(temp)) {
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++)
                blocks[b][i][j] = temp[i][j];
        }
    }
}

bool canFall(){
    bool fall = false;
    currentSpeed += tick;
    if (currentSpeed >= speed){
        currentSpeed -= speed;
        fall = true;
    }
    return fall;
}

int main()
{
    SetConsoleOutputCP(437);
    srand(time(0));
    b = rand() % 7;
    system("cls");
    initBoard();
    while (1){
        boardDelBlock();

        if ((GetAsyncKeyState('A') & 0x8000) && canMove(-1,0)) x--;
        if ((GetAsyncKeyState('D') & 0x8000) && canMove(1,0)) x++;
        if ((GetAsyncKeyState('X') & 0x8000) && canMove(0,1)) y++;
        if (GetAsyncKeyState('W') & 0x8000) rotateBlock();
        if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) break;

        if (canMove(0,1) && canFall()){
            y++;
        }
        else if (!canMove(0,1)){
            block2Board();
            removeLine();
            x = 5; y = 0; b = rand() % 7;
        }
        block2Board();
        draw();
        _sleep(tick);
    }
    return 0;
}
