#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <cab202_graphics.h>
#include <cab202_timers.h>
// Insert other functions here, or include header files
#include <time.h>

#define WALL_CAPACITY 50
#define COORD_CAPACITY 400
#define CHEESE_CAPACITY 5
#define TRAP_CAPACITY 5
#define MIN(x, y) (((x) < (y)) ? (x) : (y))
#define MAX(x, y) (((x) > (y)) ? (x) : (y))

// Screen dimensions
int W, H;
// Variables that hold game information located in the status bar
bool game_over = false;
bool pause_game = false;
int game_score = 5;
int current_lives = 5;
int fireworks = 0;
int current_level = 1;
int coord_index_counter = 0;

// cheese_location stores the locations of the cheeses on screen, cheese is a counter for how many are on screen
int cheese_location[CHEESE_CAPACITY][3];
int cheese = 0;

// trap_location stores the locations of the cheeses on screen, traps is a counter for how many are on screen
int trap_location[TRAP_CAPACITY][3];
int traps = 0;

// Timer variables
int timer_m = 0;
int timer_s = 0;
int time_paused = 0;
int paused_time_holder = 0;

// Check if door has been drawn
bool door_exist = false;
int door_x;
int door_y;

// This double array contains the coordinates for where the walls will be drawn
double walls[WALL_CAPACITY][4];
// Not sure what to set this to lol
int wall_coords[COORD_CAPACITY][2];
double jerry_starting_location[2];
double jerry_x;
double jerry_y;
double jerry_spawn_x;
double jerry_spawn_y;
double tom_starting_location[2];
double tom_x;
double tom_y;
double tom_spawn_x;
double tom_spawn_y;
double dx;
double dy;

bool quit_game = false;
bool restart_game = false;

// initialise variables start & end for holding seconds elapsed. cheese_t and cheese_check to be used for 2 second intervals for cheese spawns
time_t start, end, paused, cheese_t, cheese_check, trap_t, trap_check;

void draw_file(FILE *stream)
{
    //  Counter intialised for the following while loop to insert wall coordinates into walls array using the counter number to insert at index
    int wall_index_counter = 0;
    //  While stream has not reach end of input:
    while (!feof(stream))
    {
        //  Declare a variable called command of type char to know what the incoming data will be
        char command;

        //  Declare four variables of type int, with names arg1, arg2, arg3, arg4.
        double arg1, arg2, arg3, arg4;

        // Read the incoming data
        double num = fscanf(stream, " %c %lf %lf %lf %lf", &command, &arg1, &arg2, &arg3, &arg4);

        //  If the number of items scanned is 3:
        if (num == 3)
        {
            if (command == 'J')
            {
                jerry_starting_location[0] = arg1;
                jerry_starting_location[1] = arg2;
            }
            else if (command == 'T')
            {
                tom_starting_location[0] = arg1;
                tom_starting_location[1] = arg2;
            }
        }
        //  Otherwise, if the number of items scanned is 5:
        else if (num == 5)
        {
            //  If the command is 'W', insert wall coordinates into walls array using the counter number as the index
            if (command == 'W')
            {
                walls[wall_index_counter][0] = arg1;
                walls[wall_index_counter][1] = arg2;
                walls[wall_index_counter][2] = arg3;
                walls[wall_index_counter][3] = arg4;
                wall_index_counter++;
            }
        }
    }
}

// Check if two things have collided and return true or false
bool collided(double x0, double y0, double x1, double y1)
{
    return round(x0) == round(x1) && round(y0) == round(y1);
}

// Store the x,y coordinates of the walls
void store_coords(int x1, int y1, int x2, int y2)
{
    if (x1 == x2)
    {
        int y_min = MIN(y1, y2);
        int y_max = MAX(y1, y2);

        for (int i = y_min; i <= y_max; i++)
        {
            wall_coords[coord_index_counter][0] = x1;
            wall_coords[coord_index_counter][1] = i;
            coord_index_counter++;
        }
    }
    else if (y1 == y2)
    {

        int x_min = MIN(x1, x2);
        int x_max = MAX(x1, x2);

        for (int i = x_min; i <= x_max; i++)
        {
            wall_coords[coord_index_counter][0] = i;
            wall_coords[coord_index_counter][1] = y1;
            coord_index_counter++;
        }
    }
}
// Setup game timer in status bar
void game_timer()
{
    // Check if game is not paused
    if (!pause_game)
    {
        // If game has been previously paused, add the paused time to the paused time holder and reset time_paused to 0
        if (time_paused != 0)
        {
            paused_time_holder += time_paused;
            time_paused = 0;
        }
        // Create variable end to store the address of time
        time(&end);

        // Store the difference between end and start to begin incrementing the game timer
        int elapsed = difftime(end, start);

        // Store the value of elapsed into timer_s to begin incrementing the timer
        timer_s = elapsed - paused_time_holder;
        // Once the timer has reached 60, increment one minute (timer_m++), and re-initialise start to 0 to do the following (00:59 -> 01:00)
        if (timer_s == 60)
        {
            time(&start);
            timer_m++;
        }
    }
    // If game is paused, create new timer 'paused', and increment paused time
    else
    {
        time(&paused);
        time_paused = difftime(paused, end);
    }
}

// Check if user has pressed 'p' for pausing the game
void pause_game_check(int ch)
{
    if (ch == 'p')
    {
        if (pause_game == false)
        {
            pause_game = true;
        }
        else
        {
            pause_game = false;
        }
    }
}
// Draw the status bar information
void draw_screen_ui()
{
    game_timer();

    // Write the status bar information on screen
    draw_formatted(0, 0, "Student number: n10395792   Score: %d   Current Lives: %d   Player: J  Time: %02d:%02d", game_score, current_lives, timer_m, timer_s);
    draw_formatted(0, 2, "Cheese: %d            Traps: %d            Fireworks: %d            Level: %d", cheese, traps, fireworks, current_level);
    // Status bar bottom wall
    draw_line(0, 3, W - 1, 3, '*');
}

// Draw the level
void draw_level()
{
    // Loop over the walls array to draw the walls in the level
    for (int i = 0; i < 50; i++)
    {
        int wall_x1 = walls[i][0] * W;
        int wall_x2 = walls[i][2] * W;
        int wall_y1 = 5 + (walls[i][1] * (H - 6));
        int wall_y2 = 5 + (walls[i][3] * (H - 6));

        // If walls[i] has stored coords, draw_line will draw the wall to the screen
        if (walls[i][0] != 0)
        {
            draw_line(wall_x1, wall_y1, wall_x2, wall_y2, '*');
        }
    }
}
// Setup Jerry's position. Refactor later
void setup_jerry()
{
    if (jerry_starting_location[0] == 0 && jerry_starting_location[1] == 0)
    {
        jerry_starting_location[0] = 1;
        jerry_starting_location[1] = 4;

        jerry_x = jerry_starting_location[0];
        jerry_y = jerry_starting_location[1];

        jerry_spawn_x = jerry_x;
        jerry_spawn_y = jerry_y;
    }
    else if (jerry_starting_location[1] == 0)
    {
        jerry_starting_location[1] = 4;

        jerry_x = jerry_starting_location[0] * (W - 1);
        jerry_y = jerry_starting_location[1];

        jerry_spawn_x = jerry_x;
        jerry_spawn_y = jerry_y;
    }
    else if (jerry_starting_location[0] == 0)
    {
        jerry_starting_location[0] = 1;

        jerry_x = jerry_starting_location[0];
        jerry_y = jerry_starting_location[1] * (H - 1);

        jerry_spawn_x = jerry_x;
        jerry_spawn_y = jerry_y;
    }
    else
    {
        jerry_x = jerry_starting_location[0] * (W - 1);
        jerry_y = jerry_starting_location[1] * (H - 1);

        jerry_spawn_x = jerry_x;
        jerry_spawn_y = jerry_y;
    }
}
// Setup Tom's position. Refactor later
void setup_tom()
{
    if (tom_starting_location[0] == 0 && tom_starting_location[1] == 0)
    {
        tom_starting_location[0] = 1;
        tom_starting_location[1] = 4;

        tom_x = tom_starting_location[0];
        tom_y = tom_starting_location[1];

        tom_spawn_x = tom_x;
        tom_spawn_y = tom_y;
    }
    else if (tom_starting_location[1] == 0)
    {
        tom_starting_location[1] = 4;

        tom_x = tom_starting_location[0] * (W - 1);
        tom_y = tom_starting_location[1];

        tom_spawn_x = tom_x;
        tom_spawn_y = tom_y;
    }
    else if (tom_starting_location[0] == 0)
    {
        tom_starting_location[0] = 1;

        tom_x = tom_starting_location[0];
        tom_y = tom_starting_location[1] * (H - 1);

        tom_spawn_x = tom_x;
        tom_spawn_y = tom_y;
    }
    else
    {
        tom_x = tom_starting_location[0] * (W - 1);
        tom_y = tom_starting_location[1] * (H - 1);

        tom_spawn_x = tom_x;
        tom_spawn_y = tom_y;
    }
}

// Draw Jerry to the screen
void draw_jerry()
{
    draw_char(jerry_x, jerry_y, 'J');
}

// Draw Tom to the screen
void draw_tom()
{
    draw_char(round(tom_x), round(tom_y), 'T');
}

// Wall collision check for Tom & Jerry. Uses integer concatenation to match x & y values e.g. x = 16, y = 6 > x_y = 166
bool wall_collision_check(int x, int y)
{

    int a = 1;
    int x_plus_y;
    while (y >= a)
    {
        a *= 10;
        x_plus_y = x * a + y;
    }

    for (int i = 0; i < 400; i++)
    {
        if (wall_coords[i][0] != 0)
        {
            a = 1;
            int wall_x = wall_coords[i][0];
            int wall_y = wall_coords[i][1];
            int wall_x_plus_y;

            while (wall_y >= a)
            {
                a *= 10;
                wall_x_plus_y = wall_x * a + wall_y;
            }
            // To ensure the concatenated int can't be made from other numbers, cross-check if both x coord's match each other
            if (x_plus_y == wall_x_plus_y && x == wall_x)
            {
                return true;
            }
        }
    }
    return false;
}

// Check if an item won't appear on top of a cheese
bool cheese_collision_check(int x, int y)
{
    for (int i = 0; i < 5; i++)
    {
        if (x == cheese_location[i][0] && y == cheese_location[i][1])
        {
            return true;
        }
    }
    return false;
}

// Check if an item won't appear on top of a trap
bool trap_collision_check(int x, int y)
{
    for (int i = 0; i < 5; i++)
    {
        if (x == trap_location[i][0] && y == trap_location[i][1])
        {
            return true;
        }
    }
    return false;
}

// Update jerry based on where he moves and check for collision with walls & objects
void update_jerry(int ch)
{
    if (ch == 'a' && jerry_x > 1)
    {
        if (!wall_collision_check(jerry_x - 1, jerry_y))
        {
            jerry_x--;
        }
    }
    else if (ch == 'd' && jerry_x < W - 2)
    {

        if (!wall_collision_check(jerry_x + 1, jerry_y))
        {
            jerry_x++;
        }
    }
    else if (ch == 's' && jerry_y < H - 2)
    {
        if (!wall_collision_check(jerry_x, jerry_y + 1))
        {
            jerry_y++;
        }
    }
    else if (ch == 'w' && jerry_y > 4)
    {
        if (!wall_collision_check(jerry_x, jerry_y - 1))
        {
            jerry_y--;
        }
    }
}

// Update Tom based on where he moves and check for collision with walls & objects. Moves in random direction on collision
void update_tom()
{
    dx = 0.075;
    dy = 0.075;
    srand(time(NULL));
    int random_direction = rand() % 3;

    if (tom_x > jerry_x)
    {
        if (!wall_collision_check(tom_x - 1, tom_y))
        {
            tom_x -= dx;
        }
        else
        {
            if (random_direction == 0)
            {
                if (!wall_collision_check(tom_x, tom_y + 1))
                {
                    tom_y += dy * 3;
                }
            }
            else if (random_direction == 1)
            {
                if (!wall_collision_check(tom_x, tom_y - 1))
                {
                    tom_y -= dy * 3;
                }
            }
            else
            {
                if (!wall_collision_check(tom_x + 1, tom_y))
                {
                    tom_x += dx * 3;
                }
            }
        }
    }
    else if (tom_x < jerry_x)
    {
        if (!wall_collision_check(tom_x + 1, tom_y))
        {
            tom_x += dx;
        }
        else
        {
            if (random_direction == 0)
            {
                if (!wall_collision_check(tom_x, tom_y + 1))
                {
                    tom_y++;
                }
            }
            else
            {
                if (!wall_collision_check(tom_x, tom_y - 1))
                {
                    tom_y--;
                }
            }
        }
    }

    if (tom_y > jerry_y)
    {
        if (!wall_collision_check(tom_x, tom_y - 1))
        {
            tom_y -= dy;
        }
        else
        {
            if (random_direction == 0)
            {
                if (!wall_collision_check(tom_x + 1, tom_y))
                {
                    tom_x++;
                }
            }
            else
            {
                if (!wall_collision_check(tom_x - 1, tom_y))
                {
                    tom_x--;
                }
            }
        }
    }
    else if (tom_y < jerry_y)
    {
        if (!wall_collision_check(tom_x, tom_y + 1))
        {
            tom_y += dy;
        }
        else
        {
            if (random_direction == 0)
            {
                if (!wall_collision_check(tom_x + 1, tom_y))
                {
                    tom_x++;
                }
            }
            else
            {
                if (!wall_collision_check(tom_x - 1, tom_y))
                {
                    tom_x--;
                }
            }
        }
    }
    if (collided(tom_x, tom_y, jerry_x, jerry_y))
    {
        jerry_x = jerry_spawn_x;
        jerry_y = jerry_spawn_y;

        tom_x = tom_spawn_x;
        tom_y = tom_spawn_y;

        // current_lives--;
        // if (current_lives == 0)
        // {
        //     game_over = true;
        // }
    }
}

// Setup the cheese locations and store them in the cheese_location array with a boolean of false
// The boolean will let the program know if a cheese has been spawned or not
void setup_cheese()
{
    srand(time(NULL));
    for (int i = 0; i < 5; i++)
    {
        int cheese_x = 1 + rand() % (W - 2);
        int cheese_y = 3 + rand() % (H - 4);
        if (!wall_collision_check(cheese_x, cheese_y) && !cheese_collision_check(cheese_x, cheese_y))
        {
            cheese_location[i][0] = cheese_x;
            cheese_location[i][1] = cheese_y;
            cheese_location[i][2] = false;
        }
    }
}

// Draw the cheese to the screen and check if Jerry has collided with it
void draw_cheese()
{
    for (int i = 0; i < 5; i++)
    {
        if (cheese_location[i][2] == true)
        {
            draw_char(cheese_location[i][0], cheese_location[i][1], 'C');
            if (collided(jerry_x, jerry_y, cheese_location[i][0], cheese_location[i][1]))
            {
                cheese--;
                cheese_location[i][0] = 0;
                cheese_location[i][1] = 0;
                cheese_location[i][2] = false;
                game_score++;
                // Reset the cheese_check clock to begin intervals at 0 again in case all 5 have spawned
                time(&cheese_check);
            }
        }
    }
}

// Update cheese based on a few conditions
void update_cheese()
{
    if (!pause_game)
    {
        // Timer to do a check for 2 second intervals
        time(&cheese_t);
        int elapsed = difftime(cheese_t, cheese_check);

        // If 2 seconds has passed, do a loop and check for the first cheese in the loop that hasn't spawned, and spawn it
        if (elapsed >= 2)
        {
            for (int i = 0; i < 5; i++)
            {

                if (cheese_location[i][2] == false)
                {
                    cheese_location[i][2] = true;
                    cheese++;
                    time(&cheese_check);
                    break;
                }
            }
        }
        srand(time(NULL));

        // If a cheese has been eaten by Jerry, re-create the cheese with new coordinates and set to false
        for (int i = 0; i < 5; i++)
        {
            int cheese_x = 1 + rand() % (W - 2);
            int cheese_y = 3 + rand() % (H - 4);
            if (cheese_location[i][0] == 0 && cheese_location[i][1] == 0)
            {
                if (!wall_collision_check(cheese_x, cheese_y) && !cheese_collision_check(cheese_x, cheese_y) && !trap_collision_check(cheese_x, cheese_y))
                {
                    cheese_location[i][0] = cheese_x;
                    cheese_location[i][1] = cheese_y;
                    cheese_location[i][2] = false;
                }
            }
        }
    }
    else
    {
        time(&cheese_check);
    }
}

// Draw a trap to the screen and check if Jerry has collided with it
void draw_trap()
{
    for (int i = 0; i < 5; i++)
    {
        if (trap_location[i][2] == true)
        {
            draw_char(trap_location[i][0], trap_location[i][1], 'M');
            if (collided(jerry_x, jerry_y, trap_location[i][0], trap_location[i][1]))
            {
                traps--;
                // current_lives--;
                // if (current_lives == 0)
                // {
                //     game_over = true;
                // }
                trap_location[i][0] = 0;
                trap_location[i][1] = 0;
                trap_location[i][2] = false;
                // Reset the cheese_check clock to begin intervals at 0 again in case all 5 have spawned
                time(&trap_check);
            }
        }
    }
}

// Update trap based on a few conditions
void update_trap()
{
    if (!pause_game)
    {
        // Timer to do a check for 3 second intervals
        time(&trap_t);
        int elapsed = difftime(trap_t, trap_check);

        // If 3 seconds has passed, do a loop and check for the first trap in the loop that hasn't spawned, and spawn it
        if (elapsed >= 3)
        {
            for (int i = 0; i < 5; i++)
            {

                if (!cheese_collision_check(tom_x, tom_y) && !collided(tom_x, tom_y, door_x, door_y) && !trap_collision_check(tom_x, tom_y))
                {
                    if (trap_location[i][2] == false)
                    {
                        trap_location[i][0] = tom_x;
                        trap_location[i][1] = tom_y;
                        trap_location[i][2] = true;
                        traps++;
                        time(&trap_check);
                        break;
                    }
                }
            }
        }
    }
    else
    {
        time(&trap_check);
    }
}

// Draw the door once Jerry has a score of 5 or more
void draw_door()
{
    if (game_score >= 5)
    {
        if (door_exist == false)
        {
            srand(time(NULL));
            int door_x_check = 1 + rand() % (W - 2);
            int door_y_check = 3 + rand() % (H - 4);

            if (!wall_collision_check(door_x_check, door_y_check) && !cheese_collision_check(door_x_check, door_y_check) && !trap_collision_check(door_x_check, door_y_check))
            {
                door_x = door_x_check;
                door_y = door_y_check;
                door_exist = true;
            }
        }
        else
        {
            draw_char(door_x, door_y, 'X');

            if (collided(door_x, door_y, jerry_x, jerry_y))
            {
                // game_over = true;
            }
        }
    }
}

// Store the coordinates of the walls by passing to store_coords function the draw_line coordinates of walls
void setup_wall_collision()
{
    for (int i = 0; i < 50; i++)
    {
        int wall_x1 = walls[i][0] * W;
        int wall_x2 = walls[i][2] * W;
        int wall_y1 = 5 + (walls[i][1] * (H - 6));
        int wall_y2 = 5 + (walls[i][3] * (H - 6));
        if (walls[i][0] != 0)
        {
            store_coords(wall_x1, wall_y1, wall_x2, wall_y2);
        }
    }
}

// Draw everything
void draw_all()
{
    clear_screen();
    draw_level();
    draw_jerry();
    draw_tom();
    draw_trap();
    draw_cheese();
    draw_door();
    draw_screen_ui();
    show_screen();
}

// Game Reset
void reset()
{
    game_over = false;
    pause_game = false;
    game_score = 5;
    current_lives = 5;
    fireworks = 0;
    current_level = 1;
    coord_index_counter = 0;
    cheese = 0;
    traps = 0;
    timer_m = 0;
    timer_s = 0;
    time_paused = 0;
    paused_time_holder = 0;
    quit_game = false;
    restart_game = false;
    setup_jerry();
    setup_tom();
    setup_wall_collision();
    setup_cheese();
}

void gameover()
{
    while (game_over == true)
    {
        clear_screen();
        draw_string(screen_width() / 4 - 5, screen_height() / 2, "Game over! Would you like to play again or quit?");
        draw_string(screen_width() / 4 - 5, screen_height() / 2 + 1, "R for restart and Q for quit");
        show_screen();
        int userinput = get_char();
        if (userinput == 'r')
        {
            reset();
            game_over = false;
            break;
        }
        if (userinput == 'q')
        {
            quit_game = true;
            break;
        }
    }
}

// Setup the game
void setup()
{
    // Insert setup logic here
    // Screen border instructions
    // Initialise timer called start to compare to a timer called end in game_timer() for incrementing the game time
    time(&start);
    // Initialise timer called cheese_check to compare to a timer called cheese_t in draw_cheese() for spawning cheese at 2 second intervals
    time(&cheese_check);
    // Initialise timer called trap_check to compare to a timer called trap_t in setup_trap() for spawning traps at 3 second intervals
    time(&trap_check);
    H = screen_height(); // Default Cygwin height = 24
    W = screen_width();  // Default Cygwin width = 80
}

void loop()
{
    // Insert loop logic here.
    int key = get_char();
    update_cheese();
    update_jerry(key);
    update_tom();
    update_trap();
    pause_game_check(key);
}

int main(int argc, char *argv[])
{
    for (int i = 1; i < argc; i++)
    {
        FILE *stream = fopen(argv[i], "r");
        if (stream != NULL)
        {
            draw_file(stream);
            fclose(stream);
        }
    }
    setup_screen();
    setup();

    while (quit_game == false)
    {
        loop();
        draw_all();
        // USED TO DEGBUG AND FORCE A GAME OVER TO TEST RESET
        if (get_char() == '1')
        {
            game_over = true;
        }

        gameover();

        timer_pause(50);
    }

    return 0;
}