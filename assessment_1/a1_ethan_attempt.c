/*
*********************************************
*        Student Name: Ethan Houley         *
*             Subject: CAB202 Sem 2, 2019   *
*           File Name: a1_ethan_attemp.c    *
*          Start date: 28/8/2019            * 
*********************************************
*/
#include <cab202_graphics.h>
#include <cab202_timers.h>
#include <limits.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// *********************************************
// Globle Varaibles
// Character Information
bool PLAYINGASTOM;
double tomSpawn[10][2];
double tomLocation[2];
double jerrySpawn[10][2];
double jerryLocation[2];

// Screen and Level Map location and Map
int screenWidith, screenHeight;
char *fileLocation[10];
// int numberOfLine = 52, numberOfArgs;                    // number of lines from file input
double wallLocation[10][100][4]; // wallLocation[numberOfLine - 2, 4]; number of lines from file input

// Game States
bool GAMEOVER = false;
int LEVEL, LIVES, SCORE, CHEESE, MOUSETRAPS, WEAPONS, DELAY = 50;
double startTime;

// *********************************************
void MovementOfCharacter(double whichCharacter[], int direction)
{
    if (direction == 'w') // Up
    {
        whichCharacter[1]--;
    }
    if (direction == 's') // Down
    {
        whichCharacter[1]++;
    }
    if (direction == 'a') // Left
    {
        whichCharacter[0]--;
    }
    if (direction == 'd') // Right
    {
        whichCharacter[0]++;
    }
}

void QuitOrRestart(int ch)
{
    if (ch == 'r')
    {
        // Reset everythigng
    }
    if (ch == 'q')
    {
        exit(1);
    }
}

void UserInput(int ch)
{
    if (ch == 'w' || ch == 'a' || ch == 's' || ch == 'd')
    {
        if (PLAYINGASTOM == true)
        {
            MovementOfCharacter(tomLocation, ch);
        }
        if (PLAYINGASTOM == false)
        {
            MovementOfCharacter(jerryLocation, ch);
        }
    }
    QuitOrRestart(ch);
}

void Cheats() {}

int ReturnScore()
{
    return SCORE;
}

int ReturnLives()
{
    return LIVES;
}

char ReturnPlayer()
{
    if (PLAYINGASTOM == false)
    {
        return 'J';
    }
    else
    {
        return 'T';
    }
}

int ReturnCheese()
{
    return CHEESE;
}

int ReturnTraps()
{
    return MOUSETRAPS;
}

int ReturnFireworks()
{
    return WEAPONS;
}

int ReturnLevel()
{
    return LEVEL;
}

// TIME
int ReturnMinutes()
{
    return 0;
}

int ReturnSeconds()
{
    return 0;
}

// Screen Layout
void StatusBar()
{
    draw_line(0, 3, screenWidith, 3, '-');
    draw_formatted(0, 0, "Student Number: n00000000");
    draw_formatted((screenWidith / 10) * 3, 0, "Score: %i", ReturnScore());
    draw_formatted((screenWidith / 10) * 5, 0, "Lives: %i", ReturnLives());
    draw_formatted((screenWidith / 10) * 7, 0, "Player: %c", ReturnPlayer());
    draw_formatted((screenWidith / 10) * 9, 0, "Time: %02i:%02i", ReturnMinutes(), ReturnSeconds());
    draw_formatted(0, 2, "Cheese: %i", ReturnCheese());
    draw_formatted((screenWidith / 10) * 3, 2, "Traps: %i", ReturnTraps());
    draw_formatted((screenWidith / 10) * 5, 2, "Fireworks: %i", ReturnFireworks());
    draw_formatted((screenWidith / 10) * 7, 2, "Level: %i", ReturnLevel());
}

void DrawMap()
{
    for (int i = 0; i < 14; i++)
    {
        draw_line(wallLocation[1][i][0], wallLocation[1][i][1], wallLocation[1][i][2], wallLocation[1][i][3], '*');
        /* code */
    }
}

void DrawPlayer()
{
    draw_char(tomLocation[0], tomLocation[1], 'T');
    draw_char(jerryLocation[0], jerryLocation[1], 'J');
}

void DrawItems() {}

void DrawAll()
{
    clear_screen();
    StatusBar();
    DrawMap();
    DrawItems();
    DrawPlayer();
    show_screen();
}

void AdjustCharacterLocation(int index, char tORj)
{
    if (tORj == 't')
    {
        // X
        if (tomSpawn[index][0] >= screenWidith)
        {
            tomSpawn[index][0] = screenWidith - 1;
        }
        if (tomSpawn[index][0] == 0)
        {
            tomSpawn[index][0] = 1;
        }
        // Y
        if (tomSpawn[index][1] <= 3)
        {
            tomSpawn[index][1] = 4;
        }

        if (tomSpawn[index][1] >= screenHeight)
        {
            tomSpawn[index][1] = screenHeight - 1;
        }
    }
    else if (tORj == 'j')
    {
        // X
        if (jerrySpawn[index][0] >= screenWidith)
        {
            jerrySpawn[index][0] = screenWidith - 1;
        }
        if (jerrySpawn[index][0] == 0)
        {
            jerrySpawn[index][0] = 1;
        }
        // Y
        if (jerrySpawn[index][1] <= 3)
        {
            jerrySpawn[index][1] = 4;
        }

        if (jerrySpawn[index][1] >= screenHeight)
        {
            jerrySpawn[index][1] = screenHeight - 1;
        }
    }
}

void ReadFile(int index)
{
    FILE *stream = fopen(fileLocation[index], "r");
    // Read file
    int wallCount = 0;
    while (!feof(stream))
    {
        // Checks what letter.
        char startingLetter;
        double location[4];

        fscanf(stream, "%c %lf %lf %lf %lf", &startingLetter, &location[0], &location[1], &location[2], &location[3]);

        // Checks for number of items scanned
        if (startingLetter == 'W')
        {
            wallLocation[index][wallCount][0] = (screenWidith - 1) * location[0];
            wallLocation[index][wallCount][1] = 5 + (screenHeight - 6) * location[1];
            wallLocation[index][wallCount][2] = (screenWidith - 1) * location[2];
            wallLocation[index][wallCount][3] = 5 + (screenHeight - 6) * location[3];
            wallCount++;
        }
        if (startingLetter == 'T')
        {
            tomSpawn[index][0] = (screenWidith - 1) * location[0];
            tomSpawn[index][1] = (screenHeight - 1) * location[1];
            AdjustCharacterLocation(index, 't');
        }
        if (startingLetter == 'J')
        {
            jerrySpawn[index][0] = (screenWidith - 1) * location[0];
            jerrySpawn[index][1] = (screenHeight - 1) * location[1];
            AdjustCharacterLocation(index, 'j');
        }
    }

    fclose(stream);
}

void ReadCommandLineInput(int argc, char *argv[])
{
    for (int i = 1; i < argc; i++)
    {
        // Read Command Line Arguments
        fileLocation[i] = argv[i];
        ReadFile(i);
    }
}

void setup()
{
    // Insert setup logic here
    tomLocation[0] = tomSpawn[1][0];
    tomLocation[1] = tomSpawn[1][1];
    jerryLocation[0] = jerrySpawn[1][0];
    jerryLocation[1] = jerrySpawn[1][1];
}

void loop()
{
    // Insert loop logic here.
    clear_screen();
    DrawAll();
    UserInput(get_char());
    show_screen();
}

int main(int argc, char *argv[])
{
    setup_screen();
    get_screen_size(&screenWidith, &screenHeight);
    ReadCommandLineInput(argc, argv);
    setup();
    // DEBUG CODE FOR READ COMMAND LINE INPUT
    // while (1)
    // {
    //     clear_screen();
    //     draw_formatted(2, 1, "%lf wall x1 Location", wallLocation[1][0][0]);
    //     draw_formatted(2, 2, "%lf wall y1 Location", wallLocation[1][0][1]);
    //     draw_formatted(2, 3, "%lf wall x2 Location", wallLocation[1][0][2]);
    //     draw_formatted(2, 4, "%lf wall y2 Location", wallLocation[1][0][3]);

    //     draw_formatted(2, 10, "%lf Tom x Location", tomLocation[0]);
    //     draw_formatted(2, 11, "%lf Tom y Location", tomLocation[1]);
    //     draw_formatted(2, 12, "%lf Jerry x Location", jerryLocation[0]);
    //     draw_formatted(2, 13, "%lf Jerry y Location", jerryLocation[1]);
    //     show_screen();
    // }

    while (!GAMEOVER)
    {
        loop();
        timer_pause(DELAY);
    }
    return 0;
}