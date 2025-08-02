//这个程序是网上找的，现在找不到链接了，如果有人知道，可发邮箱maxnb666@qq.com告诉我，我会加上源码出处，谢谢！
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <ncurses.h>

int const ROW = 4;
int const COL = 4;
int game[ROW][COL] = {0};

//上下左右
int const UP = 1;
int const DOWN = 2;
int const LEFT = 3;
int const RIGHT = 4;

//游戏所处的状态
int const GAME_OVER = 1;
int const GAME_WIN = 2;
int const GAME_CONTINUE = 3;

enum GameNum
{
    Game_2 = 2,
    Game_4 = 4,
    Game_8 = 8,
    Game_16 = 16,
    Game_32 = 32,
    Game_64 = 64,
    Game_128 = 128,
    Game_256 = 256,
    Game_512 = 512,
    Game_1024 = 1024,
    Game_2048 = 2048,
};

//打印所得的数组
void Print()
{
    clear(); // 清屏
    printw("**********************  2048  ******************\n");
    printw("*****************  By Tanzf (Intern) ******************\n\n");
    for (int i = 0; i < ROW; ++i)
    {
        printw("---------------------------------\n");
        for (int j = 0; j < COL; ++j)
        {
            if (game[i][j] == 0)
            {
                printw("|   \t");
            }
            else
            {
                printw("|   %d\t", game[i][j]);
            }
        }
        printw("|\n");
    }
    printw("---------------------------------\n");
    refresh(); // 刷新屏幕
}

//生成数字
bool CreateNumber()
{
    int x = -1;
    int y = -1;
    int times = 0;
    int maxTimes = ROW * COL;
    //三分之二的概率生成2，三分之一的概率生成4
    int whitch = rand() % 3;
    do
    {
        x = rand() % ROW;
        y = rand() % COL;
        ++times;
    } while (game[x][y] != 0 && times <= maxTimes);

    //说明格子已经满了
    if (times >= maxTimes)
    {
        return false;
    }
    else
    {
        GameNum num;
        if (whitch == 0)
        {
            num = Game_4;
        }
        else
        {
            num = Game_2;
        }
        game[x][y] = num;
    }

    return true;
}

void Process(int direction)
{
    switch (direction)
    {
    case UP:
        //最上面一行不动
        for (int row = 1; row < ROW; ++row)
        {
            for (int crow = row; crow >= 1; --crow)
            {
                for (int col = 0; col < COL; ++col)
                {
                    //上一个格子为空
                    if (game[crow - 1][col] == 0)
                    {
                        game[crow - 1][col] = game[crow][col];
                        game[crow][col] = 0;
                    }
                    else
                    {
                        //合并
                        if (game[crow - 1][col] == game[crow][col])
                        {
                            game[crow - 1][col] *= 2;
                            game[crow][col] = 0;
                        }
                    }
                }
            }
        }
        break;
    case DOWN:
        //最下面一行不动
        for (int row = ROW - 2; row >= 0; --row)
        {
            for (int crow = row; crow < ROW - 1; ++crow)
            {
                for (int col = 0; col < COL; ++col)
                {
                    //上一个格子为空
                    if (game[crow + 1][col] == 0)
                    {
                        game[crow + 1][col] = game[crow][col];
                        game[crow][col] = 0;
                    }
                    else
                    {
                        //合并
                        if (game[crow + 1][col] == game[crow][col])
                        {
                            game[crow + 1][col] *= 2;
                            game[crow][col] = 0;
                        }
                    }
                }
            }
        }
        break;
    case LEFT:
        //最左边一列不动
        for (int col = 1; col < COL; ++col)
        {
            for (int ccol = col; ccol >= 1; --ccol)
            {
                for (int row = 0; row < ROW; ++row)
                {
                    //上一个格子为空
                    if (game[row][ccol - 1] == 0)
                    {
                        game[row][ccol - 1] = game[row][ccol];
                        game[row][ccol] = 0;
                    }
                    else
                    {
                        //合并
                        if (game[row][ccol - 1] == game[row][ccol])
                        {
                            game[row][ccol - 1] *= 2;
                            game[row][ccol] = 0;
                        }
                    }
                }
            }
        }
        break;
    case RIGHT:
        //最右边一列不动
        for (int col = COL - 2; col >= 0; --col)
        {
            for (int ccol = col; ccol <= COL - 2; ++ccol)
            {
                for (int row = 0; row < ROW; ++row)
                {
                    //上一个格子为空
                    if (game[row][ccol + 1] == 0)
                    {
                        game[row][ccol + 1] = game[row][ccol];
                        game[row][ccol] = 0;
                    }
                    else
                    {
                        //合并
                        if (game[row][ccol + 1] == game[row][ccol])
                        {
                            game[row][ccol + 1] *= 2;
                            game[row][ccol] = 0;
                        }
                    }
                }
            }
        }
        break;
    }
}

//处理输入输出，返回上下左右
int Input()
{
    int ch = getch();
    switch (ch)
    {
    case KEY_UP:
        return UP;
    case KEY_DOWN:
        return DOWN;
    case KEY_LEFT:
        return LEFT;
    case KEY_RIGHT:
        return RIGHT;
    default:
        return 0;
    }
}

//判断游戏状态
int Judge()
{
    //赢得游戏
    for (int i = 0; i < ROW; ++i)
    {
        for (int j = 0; j < COL; ++j)
        {
            if (game[i][j] == 2048)
            {
                return GAME_WIN;
            }
        }
    }

    //横向检查
    for (int i = 0; i < ROW; ++i)
    {
        for (int j = 0; j < COL - 1; ++j)
        {
            if (!game[i][j] || (game[i][j] == game[i][j + 1]))
            {
                return GAME_CONTINUE;
            }
        }
    }
    //纵向检查
    for (int j = 0; j < COL; ++j)
    {
        for (int i = 0; i < ROW - 1; ++i)
        {
            if (!game[i][j] || (game[i][j] == game[i + 1][j]))
            {
                return GAME_CONTINUE;
            }
        }
    }

    //不符合上述两种状况，游戏结束
    return GAME_OVER;
}

int main()
{
    //初始化ncurses
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);

    //设置一个随机数种子
    srand((unsigned int)time(0));
    CreateNumber();
    CreateNumber();
    Print();
    int direction = 0;
    int gameState = -1;
    while (true)
    {
        direction = Input();

        gameState = Judge();
        if (direction && gameState == GAME_CONTINUE)
        {
            Process(direction);
            CreateNumber();
            Print();
        }
        else if (gameState == GAME_WIN)
        {
            Print();
            printw("You Win!\n");
            break;
        }
        else if (gameState == GAME_OVER)
        {
            Print();
            printw("You lose!\n");
            break;
        }
    }

    //结束ncurses
    endwin();
    return 0;
}