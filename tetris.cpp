#include <iostream>
#include <thread>
#include <vector>
#include <chrono>

#include <unistd.h>
#include <stdio.h>
#include <ncurses.h>

using namespace std;

int nFieldWidth = 12;
int nFieldHeight = 18;
int nScreenWidth = 12; // Console screen width
int nScreenHeight = 18; // Console screen height
int nBoxOffsetX = 4;
int nBoxOffsetY = 6;
wstring tetromino[7];
unsigned char *pField = nullptr;

int Rotate(int px, int py, int r)
{
    int rot = 0;
    switch(r % 4)
    {
        case 0: 
            rot = py * 4 + px; // 0 degrees
            break;
        case 1: 
            rot = 12 + py - (4 * px); // 90 degrees
            break;
        case 2: 
            rot = 15 - (4 * py) - px; // 180 degrees
            break;
        case 3: 
            rot = 3 - py + (4 * px); // 270 degrees
            break;
    }
    return rot;
}

bool DoesPieceFit(int ntromino, int nRotation, int nPosX, int nPosY)
{
    for (int px = 0; px < 4; px++)
        for(int py = 0; py < 4; py++)
        {
            // Get index into piece
            int pi = Rotate(px, py, nRotation);

            // Get index into field
            int fi = (nPosY + py) * nFieldWidth + (nPosX + px);

            if(nPosX + px >= 0 && nPosX + px < nFieldWidth && nPosY + py >= 0 && nPosY + py < nFieldHeight && tetromino[ntromino][pi] == L'X' && pField[fi] != 0)
            {
                return false; // fail on first hit
            }
        }
    return true;
}


int main()
{
    // Create screen buffer
    wchar_t *screen = new wchar_t[nScreenWidth*nScreenHeight];
    for(int i = 0; i < nScreenWidth*nScreenHeight; i++) 
        screen[i] = L' ';

    // Create screen window
    WINDOW *TetrisLand;
    int offsetx, offsety, ch;
    initscr();
    noecho();
    cbreak();
    timeout(50);
    keypad(stdscr, TRUE);
    refresh();
    offsetx = (80 - (nFieldWidth + nBoxOffsetX))/2;
    offsety = (24 - (nFieldHeight + nBoxOffsetY))/2;
    TetrisLand = newwin(nFieldHeight + nBoxOffsetY,nFieldWidth + nBoxOffsetX,offsety,offsetx);
    box(TetrisLand,0,0);
    refresh();

    // Create assets
    tetromino[0].append(L"..X.");
    tetromino[0].append(L"..X.");
    tetromino[0].append(L"..X.");
    tetromino[0].append(L"..X.");

    tetromino[1].append(L"..X.");
    tetromino[1].append(L"..X.");
    tetromino[1].append(L"...X");
    tetromino[1].append(L"...X");

    tetromino[2].append(L"..X.");
    tetromino[2].append(L"..X.");
    tetromino[2].append(L".X..");
    tetromino[2].append(L".X..");

    tetromino[3].append(L"....");
    tetromino[3].append(L".XX.");
    tetromino[3].append(L"..X.");
    tetromino[3].append(L"..X.");

    tetromino[4].append(L"....");
    tetromino[4].append(L"..XX"); 
    tetromino[4].append(L"..X.");
    tetromino[4].append(L"..X.");

    tetromino[5].append(L"..X.");
    tetromino[5].append(L".XX.");
    tetromino[5].append(L"..X.");
    tetromino[5].append(L"....");

    tetromino[6].append(L"....");
    tetromino[6].append(L".XX.");
    tetromino[6].append(L".XX.");
    tetromino[6].append(L"....");

    // Create empty playing field
    pField = new unsigned char[nFieldWidth*nFieldHeight]; 
    for (int x = 0; x < nFieldWidth; x++)
        for (int y = 0; y < nFieldHeight; y++)
            pField[y*nFieldWidth + x] = (x == 0 || x == nFieldWidth - 1 || y == nFieldHeight - 1) ? 9 : 0;

    // Game logic
    bool bGameOver = false;
    srand((unsigned int)time(NULL));
    int nCurrentPiece = rand() % 7;
    int nCurrentRotation = 0;
    int nCurrentX = nFieldWidth / 2;
    int nCurrentY = 0;
    int nSpeed = 20;
	int nSpeedCount = 0;
    int nPieceCount;
	bool bForceDown = false;
    int nScore = 0;
    string totalScore;
	vector<int> vLines;
 
    while ((ch = getch()) != 'x') {

        if(bGameOver != true) {

            // Game Timing 
            this_thread::sleep_for(std::chrono::milliseconds(50)); // Small Step = 1 Game Tick
            nSpeedCount++;
            bForceDown = (nSpeedCount == nSpeed);

            // Input
            enum direction { UP, DOWN, RIGHT, LEFT, ROTATE, NONE };
            int cur_dir;
            if(ch != ERR) {
                switch(ch) {
                    case KEY_UP:
                        cur_dir = UP;
                        break;
                    case KEY_DOWN:
                        cur_dir = DOWN;
                        break;
                    case KEY_RIGHT:
                        cur_dir = RIGHT;
                        break;
                    case KEY_LEFT:
                        cur_dir = LEFT;
                        break;
                    case L'z':
                        cur_dir = ROTATE;
                        break;
                    default:
                        break;
                }
            }

            // Game Logic ================

            // Handle player movement and rotation
            nCurrentX += ((cur_dir == RIGHT) && DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX + 1, nCurrentY)) ? 1 : 0;
            nCurrentX -= ((cur_dir == LEFT) && DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX - 1, nCurrentY)) ? 1 : 0;		
            nCurrentY += ((cur_dir == DOWN) && DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY + 1)) ? 1 : 0;
            nCurrentRotation += ((cur_dir == ROTATE) && DoesPieceFit(nCurrentPiece, nCurrentRotation + 1, nCurrentX, nCurrentY)) ? 1 : 0;
            cur_dir = NONE;

            // Force the piece down the playfield if it's time
            if (bForceDown)
            {
                nSpeedCount = 0; // reset speed count
                nPieceCount++; // add new piece
                if (nPieceCount % 50 == 0) // Update difficulty every 50 pieces
                    if (nSpeed >= 10) nSpeed--;
                
                // Test if piece can be moved down
                if (DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY + 1))
                    nCurrentY++; // It can, so do it!
                else
                {
                    // It can't! Lock the piece in place
                    for (int px = 0; px < 4; px++)
                        for (int py = 0; py < 4; py++)
                            if (tetromino[nCurrentPiece][Rotate(px, py, nCurrentRotation)] != L'.')
                                pField[(nCurrentY + py) * nFieldWidth + (nCurrentX + px)] = nCurrentPiece + 1;

                    // Check for lines
                    for (int py = 0; py < 4; py++)
                        if(nCurrentY + py < nFieldHeight - 1)
                        {
                            bool bLine = true;
                            for (int px = 1; px < nFieldWidth - 1; px++)
                                bLine &= (pField[(nCurrentY + py) * nFieldWidth + px]) != 0;

                            if (bLine)
                            {
                                // Remove Line, set to =
                                for (int px = 1; px < nFieldWidth - 1; px++)
                                    pField[(nCurrentY + py) * nFieldWidth + px] = 8;
                                vLines.push_back(nCurrentY + py);
                            }						
                        }

                    nScore += 25;
                    if(!vLines.empty())	nScore += (1 << vLines.size()) * 100;

                    // Pick New Piece
                    nCurrentX = nFieldWidth / 2;
                    nCurrentY = 0;
                    nCurrentRotation = 0;
                    nCurrentPiece = rand() % 7;

                    // If piece does not fit straight away, game over!
                    bGameOver = !DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY);
                }
            }

            // Render output

            // Draw playing field
            for (int x = 0; x < nFieldWidth; x++)
                for (int y = 0; y < nFieldHeight; y++)
                    screen[(y)*nScreenWidth + (x)] = L" ABCDEFG=#"[pField[y*nFieldWidth + x]];

            // Draw Current Piece
            for (int px = 0; px < 4; px++)
                for (int py = 0; py < 4; py++)
                    if (tetromino[nCurrentPiece][Rotate(px, py, nCurrentRotation)] != L'.')
                        screen[(nCurrentY + py)*nScreenWidth + (nCurrentX + px)] = nCurrentPiece + 65;

            // Animate Line Completion
            if (!vLines.empty())
            {
                // Display Frame (cheekily to draw lines)
                for (int x = 0; x < nFieldWidth; x++)
                    for (int y = 0; y < nFieldHeight; y++)
                        mvwaddch(TetrisLand,(y + nBoxOffsetY/2),(x + nBoxOffsetX/2),screen[(y)*nScreenWidth + (x)]);
                this_thread::sleep_for(std::chrono::milliseconds(400)); // Delay a bit

                for (auto &v : vLines)
                    for (int px = 1; px < nFieldWidth - 1; px++)
                    {
                        for (int py = v; py > 0; py--)
                            pField[py * nFieldWidth + px] = pField[(py - 1) * nFieldWidth + px];
                        pField[px] = 0;
                    }
                vLines.clear();
            }

            // Display Frame
            for (int x = 0; x < nFieldWidth; x++)
                for (int y = 0; y < nFieldHeight; y++)
                    mvwaddch(TetrisLand,(y + nBoxOffsetY/2),(x + nBoxOffsetX/2),screen[(y)*nScreenWidth + (x)]);

            // Draw Score
            totalScore = "SCORE = " + std::to_string(nScore);
            mvwprintw(TetrisLand,nFieldHeight + 4,2,totalScore.c_str());
            wrefresh(TetrisLand);
        } else {
            mvwprintw(TetrisLand,(nFieldHeight + nBoxOffsetY)/2 - 1,1,"* GAME  OVER *");
            mvwprintw(TetrisLand,(nFieldHeight + nBoxOffsetY)/2 + 1,2,"HIT X TO END");
            wrefresh(TetrisLand);
        }

    }
    // Delete Frame
    delwin(TetrisLand);
    endwin();

    return 0;
}