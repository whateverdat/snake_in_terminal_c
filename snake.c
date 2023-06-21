#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <stdbool.h>
#include <ncurses.h>

struct Food
{
    int y;
    int x;
};

struct Snake // Linked list
{
    int y;
    int x;
    struct Snake *next;
};

const int RowW = 64;
const int ColH = 16;

struct Snake *InitHead() // Create head node at a random position, called at start once
{
    struct Snake *head = calloc(1, sizeof(struct Snake)); // Callocing this, because the garbage value interferes with
    head->y = rand() % (RowW - 2) + 1;                    // the while statement below which ensures that spawn point is not
    while (head->x < ColH / 2)                            // near top edge but still random--since starting direction is up
        head->x = rand() % (ColH - 2) + 1;
    head->next = NULL;

    return head;
}

void CalculateFood(struct Snake **Snake, struct Food **Food) // Create food at random position
{                                                            // called at start and everytime
    struct Snake *snakeHead = *Snake;                        // the food is eaten
    struct Food *food = *Food;
    int x, y;

    x = rand() % (ColH - 4) + 2; // To ensure that the food will not spawn on the edge, add 2 to the lowest possible
    y = rand() % (RowW - 4) + 2; // number:0 & set the highest possible to 4 below map range--because 2 will be added

    while (snakeHead->next != NULL)
    {
        if (snakeHead->x == x && snakeHead->y == y) // Make sure that randomly
        {                                           // selected food location
            snakeHead = *Snake;                     // is not a snake location
            x = rand() % (ColH - 4) + 2;
            y = rand() % (RowW - 4) + 2;
            continue;
        }
        snakeHead = snakeHead->next;
    }

    food->x = x;
    food->y = y;
}

bool IsFoodEaten(struct Snake **Snake, struct Food **Food) // Check if head location is in the food location
{                                                          // runs in every refresh
    struct Snake *head = *Snake;
    struct Food *food = *Food;
    if (food->x == head->x && food->y == head->y)
        return true;
    return false;
}

bool DetectCollision(struct Snake **head) // Check for game over logic, collided with wall or tail
{
    struct Snake *snakeHead = *head;
    struct Snake *current = *head;

    if (snakeHead->x == 0 || snakeHead->y == 0 || snakeHead->x == (ColH - 1) || snakeHead->y == (RowW - 1)) // Wall collision
        return true;

    while (current->next != NULL)
    {
        current = current->next;
        if (snakeHead->x == current->x && snakeHead->y == current->y) // Tail collision
            return true;
    }
    return false;
}

void MoveSnake(struct Snake **Head, bool isFoodEaten, char direction) // Main func to move the snake
{
    struct Snake *head = *Head;
    bool first = true;      // On first iteration, the head location
    int x, y, tempX, tempY; // is stored, then the snake is moved
    x = head->x;
    y = head->y;

    // Move the snake
    if (direction == 'w')
        head->x--;
    else if (direction == 's')
        head->x++;
    else if (direction == 'a')
        head->y--;
    else
        head->y++;
    // Traverse and move each node to the position of the one before it
    while (head->next != NULL)
    {
        head = head->next;

        tempX = head->x;
        tempY = head->y;

        head->x = x;
        head->y = y;

        x = tempX;
        y = tempY;
    }

    if (isFoodEaten) // If a food piece is eaten, then add an additional empty node to the end
    {
        struct Snake *newTail = malloc(sizeof(struct Snake));
        newTail->next = NULL;
        newTail->x = x;
        newTail->y = y;
        head->next = newTail;
    }
}

bool DrawSnake(int x, int y, struct Snake **Head) // To be used while rendering, traverse and check
{                                                 // if current location is where a snake piece is
    struct Snake *head = *Head;
    while (head != NULL)
    {
        if (head->x == x && head->y == y)
            return true;
        head = head->next;
    }
    return false;
}

bool DrawFood(int x, int y, struct Food **Food) // Draw food similar to DrawSnake();
{
    struct Food *food = *Food;
    if (food->x == x && food->y == y)
        return true;

    return false;
}

void Graphics(struct Snake **Head, struct Food **Food, int score) // Function that renders what's on the screen
{
    int row, col;
    // Nested for loops
    for (row = 0; row < RowW; row++)
        for (col = 0; col < ColH; col++)
            // Drawing snake
            if (DrawSnake(col, row, Head))
                mvprintw(col, row, "@");
            // Drawing food
            else if (DrawFood(col, row, Food))
                mvprintw(col, row, "$");
            // Drawing top border
            else if (col == 0)
                // On the last upper border piece, add score
                if (row == (RowW - 1))
                    mvprintw(col, row, "#      Score: %d", score);
                else
                    mvprintw(col, row, "#");
            // Right border
            else if (row == (RowW - 1))
                mvprintw(col, row, "#");
            // Left border
            else if (row == 0)
                mvprintw(col, row, "#");
            // Bottom border
            else if (col == (ColH - 1))
                mvprintw(col, row, "#");
            // Movable area
            else
                mvprintw(col, row, " ");
    refresh(); // Update
}

void FreeMemory(struct Snake **head, struct Food **food) // Free the allocated memory
{
    free(*food);
    struct Snake *snakeHead = *head;
    while (snakeHead->next != NULL)
    {
        struct Snake *temp = snakeHead;
        snakeHead = snakeHead->next;
        free(temp);
    }
    free(snakeHead);
}

char GetInvalidInput(char *input) // There are invalid turns
{                                 // as the snake cannot turn
    switch (*input)               // to the opposite of the
    {                             // direction it currently is facing
    case 'w':
        return 's';
    case 's':
        return 'w';
    case 'a':
        return 'd';
    case 'd':
        return 'a';
    }
}

void GetInput(char *input)
{
    char invalid = GetInvalidInput(input);
    char i = getch();
    if (i != ERR) // No input
    {
        if (i != invalid) // 180 degrees direction change
        {
            switch (i)
            {
            case 'w':
            case 'W': // case insensitive
                *input = 'w';
                break;
            case 's':
            case 'S':
                *input = 's';
                break;
            case 'a':
            case 'A':
                *input = 'a';
                break;
            case 'd':
            case 'D':
                *input = 'd';
                break;
            default:
                break;
            }
        }
    }
}

// void EndGameAnimation(struct Snake **head, struct Food **food)
// {

//     struct Snake *snakeHead = *head;
//     int i;
//     for (i = 0; i < 5; i++)
//     {
//         printf("\033[H\033[J");
//         sleep(1);
//         Graphics(&snakeHead, food, 0);
//         sleep(1);
//     }
// }

void Game()
{

    initscr();             // Init curses screen, allows placing chars into screen by index
    cbreak();              // Get user input immediately
    noecho();              // Hide user input chars in terminal
    nodelay(stdscr, TRUE); // Constantly ask for input, get ERR if none is provided
    clear();               // Clear terminal
    // Data declerations
    struct Snake *Head = InitHead();
    struct Food *Food = malloc(sizeof(struct Food));
    CalculateFood(&Head, &Food);
    // Game state declerations
    bool isFoodEaten = false;
    bool collision = false;
    // Game settings
    int delay = 100100;
    int score = 0;
    char direction = 'w';

    while (true)
    {
        Graphics(&Head, &Food, score); // Update screen to current state
        GetInput(&direction);
        isFoodEaten = IsFoodEaten(&Head, &Food);  // Check current state for
        MoveSnake(&Head, isFoodEaten, direction); // food eaten or collision
        if (isFoodEaten)
        {
            CalculateFood(&Head, &Food); // New food piece
            score++;
            delay = delay - 700; // Decrease delay, game speed up
            if (delay < 50400)   // Max speed
                delay = 50400;
        }
        else if (DetectCollision(&Head))
        {
            // EndGameAnimation(&Head, &Food);
            sleep(1);
            FreeMemory(&Head, &Food);
            endwin(); // Recover terminal state before curses
            printf("Your score is %d\n", score);
            break;
        }
        if (direction == 'a' || direction == 'd') // Because there is line spacing on the terminal,
        {                                         // I tried to adjust the speed while moving
            usleep(delay * 0.7);                  // horizontally, these very specific numbers
            continue;                             // (50400, 0.7, 700) are vague calculations--for my terminal settings
        }                                         // I am not very pleased with results nonetheless
        usleep(delay);
    }
}

void Menu()
{
    printf("This is a simple Snake clone for terminal, written in C.\nWASD to move. Press Enter to start, enter Q to quit: ");
    char c;
    do
    {
        c = getchar();
        if (c == 'q' || c == 'Q') // In case of user entering q, exit the
            exit(0);              // program, memory is already freed &
    } while (c != '\n');          // In case of enter, break the loop
}

int main(void)
{
    srand(time(NULL)); // Required to get different numbers everytime
    while (true)       // not sure if I have fully understood the idea
    {
        Menu();
        printf("\033[H\033[J"); // Clearthe  terminal between sessions so
        Game();                 // they won't pile up when the program exits
    }

    return 0;
}
