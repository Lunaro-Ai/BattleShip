#include <ctype.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>

// types of ships
typedef enum
{
    CARRIER = 5, BATTLESHIP = 4, CRUISER = 3, SUBMARINE = 2, DESTROYER = 1, NOSHIP = 0, DESTROYED = -1
}Ship;

// shot results
typedef enum
{
    HIT,MISS,UNFIREDSPOT
} ShotResults;



// Main functions for game loop
void Initialization();
void ShipPlacement();
char* acceptInput();
bool HandleShipPlacement(char* input, Ship shipType);
void DisplayWorld(Ship **genericShip,ShotResults **shots);
void UpdateState();
void TearDown();

// helper functions
char* shipToString(char enumIntValue);
int charToInt(char c);
void swap(int* num1, int* num2);
char* getValidShot();
bool isalphaRange(char c);
void swapBuffer(char *c, char *c2);


//Single player functions
void SetupSinglePlayer();
void CPUShipPlacement();
ShotResults GetSinglePlayerShot(int* cpuRow,int* cpuCol);
void TeardownSinglePlayer();



// Globals variables:
Ship **shipGrid; // Pointer to a pointer that points to ship enums
ShotResults **shotGrid; // Pointer to a pointer that points to shot enums


Ship **cpuGrid;
ShotResults **cpuShots;



int main()
{

    srand(time(NULL));
    Initialization();
    UpdateState();
    TearDown();
}


/**
 * Function that allocates memory into two grids
 * Prints instructions for ship placement and prints board
 * Also calls ShipPlacement
 */
void Initialization()
{


    shipGrid = malloc(10* sizeof(Ship*)); // shipGrid now holds 10 pointers that point to a ship enum
    shotGrid = malloc(10* sizeof(ShotResults*)); // shotGrid now holds 10 pointers that point to a shot enum

    if (shipGrid == NULL || shotGrid == NULL)
    {
        printf("Memory could not be allocated\n");
        exit(1);
    }
    for (int i = 0; i < 10; i++)
    {
        shipGrid[i] = malloc(10 * sizeof(Ship)); // Each of the 10 ship pointers now point to 10 ship enums
        shotGrid[i] = malloc(10 * sizeof(ShotResults)); // Each of the 10 ship pointers now point to 10 ship enums

        if (shipGrid[i] == NULL || shotGrid[i] == NULL)
        {
            printf("Memory could not be allocated\n");
            exit(1);
        }

        for (int j = 0; j < 10; j++)
        {
            shipGrid[i][j] = NOSHIP; // not ships yet
            shotGrid[i][j] = UNFIREDSPOT; // no shots yet // NOT USED YET
        }

    }
    printf("Place your move."
           " Letters:(A-J)"
           " Digits:(0-9)\n"
           "Horizontal placement: letter + Digit + Digit Ex: A04\n"
           "Vertical placement: letter + letter + Digit Ex: AE0\n");

    SetupSinglePlayer();
    DisplayWorld(shipGrid,shotGrid);
    ShipPlacement();

}






char* acceptInput()
{
    char static buffer[100]; // static array that keeps input alive after functions ends
    while (1)
    {

        // input validation if buffer is null
        if (fgets(buffer,sizeof(buffer),stdin) == NULL)
        {
            printf("No input detected...");
            continue;
        }
        if (isalpha(buffer[0]) && isdigit(buffer[1]) && isdigit(buffer[2]) && buffer[3] == '\n') // Horizontal inputs in the form LNN
        {
            return buffer;
        }

        if (isalpha(buffer[0]) && isalpha(buffer[1]) && isdigit(buffer[2]) && buffer[3] == '\n') // Vertical inputs in the form of LLN
        {
            return buffer;
        }

        printf("location not found in bounds\n"
                   "Enter a valid location: ");
    }

}


bool HandleShipPlacement(char* input, Ship shipType)
{
    int length = 0; // Length of the current ship to be placed


    // horizontal player move LNN
    if (isdigit(input[1]))
    {
        int row = toupper(input[0]) - 'A'; // Letters needs to be converted to indices
        int colStart = charToInt(input[1]); // Start column for the letter placement of the ship
        int colEnd = charToInt(input[2]); // End column for the letter placement of the ship


        // handles backwards inputs ex: a40 | fb9

        if (colStart >= colEnd)
        {
            swap(&colStart,&colEnd);
        }
        length = abs(colEnd - colStart) + 1; // correct length for current ship


        if (row < 0 || row > 9 || colStart < 0 || colStart > 9 || colEnd < 0 || colEnd > 9)  // Checks bounds for input
        {
            printf("Input not in bounds (A-J,0-9)\n");
            return false;
        }

        if (length != shipType) // If the length does not match ship type inform user and return false
        {
            printf("Error! length does not match ship type\n");
            return false;// Check this
        }


        // for loop to check ship overlap
        for (int col = colStart; col <= colEnd; col++)
        {
            if (shipGrid[row][col] != NOSHIP)
            {
                printf("Error overlap\n");
                return false;
            }
        }

        // for loop to place ship horizontally
        for (int col = colStart; col <= colEnd; col++)
        {
            shipGrid[row][col] = shipType;
        }
        return true;
    }

    // vertical move section
    else
    {

        // Turns both letter indices into array indices
        int rowStart = toupper(input[0]) - 'A';
        int rowEnd = toupper(input[1]) - 'A';
        int col = charToInt(input[2]); // converts char to int for column placement

        // handles backwards inputs ex: a40 | fb9
        if (rowStart >= rowEnd)
        {
            swap(&rowStart,&rowEnd);
        }

        length = abs(rowEnd - rowStart) + 1; // length for vertical placement of ship

        if (col < 0 || col > 9 || rowStart < 0 || rowStart > 9 || rowEnd < 0 || rowEnd > 9) // bounds checker
        {
            printf("Input not in bounds (A-J) | (0-9)\n");
            return false;
        }

        if (length != shipType) // If the length does not match ship type inform user and return false
        {
            printf("Error! length does not match ship type\n");
            return false;
        }


        // for loop to check ship overlap (vertical)
        for (int row = rowStart; row <= rowEnd; row++)
        {
            if (shipGrid[row][col] != NOSHIP)
            {
                printf("Error overlap\n");
                return false;
            }
        }


        // for loop to place ship vertically
        for (int row = rowStart; row <= rowEnd; row++)
        {
            shipGrid[row][col] = shipType;
        }
        return true;

    }
}

/**
 * Function to display ship grid
 */
void DisplayWorld(Ship **genericShip,ShotResults **shots)
{

    // column heading
    printf("[Ship board]\n");

    printf("   ");
    for (int i = 0; i < 10; i++)
    {
        printf(" %d",i);
    }
    printf("\n");

    // prints the rows for ship grid. Chain if else statements for correct type of ship
    for (int j = 0; j < 10; j++)
    {
        printf("%c   ",'A'+j);

        for (int k = 0; k < 10; k++)
        {
          if (genericShip[j][k] == CARRIER)
          {
              printf("C ");
          }
          else if (genericShip[j][k] == BATTLESHIP)
          {
              printf("B ");
          }
          else if (genericShip[j][k] == CRUISER)
          {
              printf("c ");
          }
          else if (genericShip[j][k] == SUBMARINE)
          {
              printf("S ");
          }
          else if (genericShip[j][k] == DESTROYER)
          {
              printf("D ");
          }
          else if (genericShip[j][k] == DESTROYED)
          {
              printf("X ");
          }
          else
          {
              printf(". ");
          }

        }
        printf("\n");
    }

    // column heading
    printf("[Shots board]\n");
    printf("   ");
    for (int i = 0; i < 10; i++)
    {
        printf(" %d",i);
    }
    printf("\n");


    // print human shot board
    for (int j = 0; j < 10; j++)
    {
        printf("%c   ",'A' + j);

        for (int k = 0; k < 10; k++)
        {
            if (shots[j][k] == HIT)
            {
                printf("X ");
            }
            else if (shots[j][k] == MISS)
            {
                printf("M ");
            }
            else
            {
                printf(". ");
            }
        }
        printf("\n");
    }
}


/**
 * Frees ship grids & shot grids pointers from memory
 */
void TearDown()
{
    for (int i = 0; i < 10; i++)
    {
        free(shipGrid[i]);
        free(shotGrid[i]);
    }
    free(shipGrid);
    free(shotGrid);

    shipGrid = NULL;
    shotGrid = NULL;

    TeardownSinglePlayer();


}




/**
 * Function that handles the mini-game loop.
 * Call acceptInput, pass input to updateState, then display world(grid)
 */
void ShipPlacement()
{

    Ship ships[] = {CARRIER,BATTLESHIP,CRUISER,SUBMARINE,DESTROYER,NOSHIP};
    int size  = sizeof(ships) / sizeof(ships[0]);


    for (int i = 0; i < size-1; i++)
    {
        printf("Enter placement for a %s of length %d: ",shipToString(ships[i]),ships[i]);
        char* validInput = acceptInput();

        while (!HandleShipPlacement(validInput,ships[i]))
        {
            printf("please enter a valid location for a %s of length %d: ",shipToString(ships[i]),ships[i]);
            validInput = acceptInput();
        }
        DisplayWorld(shipGrid,shotGrid);
    }
    printf("Ship placement complete...\n");


}


/**
 *
 * @param enumIntValue Integer value of enum that is encoded as a character value
 * @return a character array (String) to represent enum as a string
 */
char* shipToString(char enumIntValue)
{
    switch (enumIntValue)
    {
    case CARRIER:
        return "Carrier";
    case BATTLESHIP:
        return "BattleShip";
    case CRUISER:
        return "Cruiser";
    case SUBMARINE:
        return "Submarine";
    case DESTROYER:
        return "Destroyer";
    default:
        return "UNKNOWN";
    }
}

/**
 *
 * @param c character value
 * @return integer value of character
 */
int charToInt(char const c)
{
    return c - '0';
}


/**
 *  swap function that takes in pointers for two integers
 * @param num1
 * @param num2
 */
void swap(int* num1, int* num2)
{
    int temp = *num1;
    *num1 = *num2;
    *num2 = temp;
}


//Single player functions

/**
 * Functions creates memory for cpu boards then calls the ship functions
 */
void SetupSinglePlayer()
{

    cpuGrid = malloc(10 * sizeof(Ship*)); // shipGrid now holds 10 pointers that point to a ship enum
    cpuShots = malloc(10 * sizeof(ShotResults*)); // shotGrid now holds 10 pointers that point to a shot enum

    if (cpuGrid == NULL || cpuShots == NULL)
    {
        printf("Memory could not be allocated for cpu board\n");
        exit(1);
    }
    for (int i = 0; i < 10; i++)
    {
        cpuGrid[i] = malloc(10 * sizeof(Ship)); // Each of the 10 ship pointers now point to 10 ship enums
        cpuShots[i] = malloc(10 * sizeof(ShotResults)); // Each of the 10 ship pointers now point to 10 ship enums

        if (cpuGrid[i] == NULL || cpuShots[i] == NULL)
        {
            printf("Memory could not be allocated for cpu board\n");
            exit(1);
        }

        for (int j = 0; j < 10; j++)
        {
            cpuGrid[i][j] = NOSHIP; // not ships yet
            cpuShots[i][j] = UNFIREDSPOT; // no shots yet // NOT USED YET
        }
    }
    CPUShipPlacement();
}


/**
 * Make random inputs here then place cpu ships
 */
void CPUShipPlacement()
{

    Ship ships[] = {CARRIER,BATTLESHIP,CRUISER,SUBMARINE,DESTROYER,NOSHIP};
    //LNN:HOR | LLN:VER

    for (int i = 0; i < 5; i++)
    {
        int len = ships[i]; // current ship length
        bool placed = false; // flag var

        while (!placed)
        {
            bool horizontal = rand() % 2; // randomizes if  horizontal placement or vertical
            if (horizontal) // if horizontal branch
            {
                int row = rand() % 10; // randomizes row
                int colStart = rand() % 10; // randomizes start
                int colEnd =  len + colStart - 1; // len of ship plus the start of the col to match the len of the current ship
                if (colEnd > 9) continue;


                // for loop to check ship overlap
                bool overlap = false;
                for (int col = colStart; col <= colEnd; col++)
                {
                    if (cpuGrid[row][col] != NOSHIP)
                    {
                        overlap = true;
                        break;
                    }
                }
                if (overlap) continue;
                // loop to place ships
                for (int col = colStart; col <= colEnd; col++)
                {
                    cpuGrid[row][col] = ships[i];
                }

                placed = true;
            }

            // vertical branch
            else
            {
                int rowStart = rand() % 10; // randomizes row start
                int rowEnd = rowStart + len - 1; // cal the row end
                int col = rand() % 10; // randomizes col
                if (rowEnd > 9) continue;

                bool overLap2 = false;

                // loop checks for overlap
                for (int row = rowStart; row <= rowEnd ; row++)
                {
                    if (cpuGrid[row][col] != NOSHIP)
                    {
                        overLap2 = true;
                        break;
                    }
                }
                if (overLap2) continue;

                // loop places ships vertically
                for (int row = rowStart; row <= rowEnd ; row++)
                {
                    cpuGrid[row][col] = ships[i];
                }
                placed = true;
            }
        }
    }
}

/**
 * Frees cpu memory of ships
 */
void TeardownSinglePlayer()
{
    for (int i = 0; i < 10; i++)
    {
        free(cpuGrid[i]);
        free(cpuShots[i]);
    }

    free(cpuGrid);
    free(cpuShots);

    cpuGrid = NULL;
    cpuShots = NULL;
}

/** // THIS FUNCTION NEEDS WORK COME BACK TO THIS!!!!!!!!!!!
 *
 * @param row Row player wants to hit
 * @param col Col player wants to hit
 * @return hit or miss enum value
 */
ShotResults MakeSinglePlayerShot(int row,int col)
{
    if (cpuGrid[row][col] != NOSHIP)
    {
        cpuGrid[row][col] = DESTROYED;
        printf("YOU HIT CPU AT %c%d\n",row + 'A',col);
        return HIT;
    }
    printf("YOU MISSED CPU AT %c%d\n",row + 'A',col);
    return MISS;
}

/**
 * Function randomizes a cpu shot then updates ship grid if hit
 * @param cpuRow
 * @param cpuCol
 * @return
 */
ShotResults GetSinglePlayerShot(int* cpuRow,int* cpuCol)
{
    int row,col;
    do
    {
        row = rand() % 10;
        col = rand() % 10;
    } while (cpuShots[row][col] != UNFIREDSPOT);

    *cpuRow = row;
    *cpuCol = col;

    if (shipGrid[row][col] != NOSHIP)
    {
        shipGrid[row][col] = DESTROYED;
        printf("CPU HIT YOU AT %c%d\n",'A'+  row, col);
        return HIT;
    }
    else
    {
        printf("CPU MISSED YOU AT %c%d\n",'A' + row, col);
        return MISS;
    }
}

/**
 * Updates cpu tracking grid with hit or miss
 * @param row
 * @param col
 * @param results of the cpu shot
 */
void SinglePlayerResponse(int row,int col,ShotResults results)
{
    cpuShots[row][col] = results;
}


/**
 * Functions checks for any winners by looping through board for destroyed ships or no ships
 * @param grid Any ship grid
 * @return bool value
 */
bool SinglePlayerDidWin(Ship **grid)
{
    for (int i = 0; i < 10; i++)
    {
        for (int j = 0; j < 10; j++)
        {
            if (grid[i][j] != DESTROYED && grid[i][j] != NOSHIP)
            {
                return false;
            }
        }
    }

    return true;
}

/**
 * Function that plays the main game loop
 * Human player places ships & CPU. Human fires then cpu
 * First player to destroy all opponents ships wins
 */
void UpdateState()
{


    bool gameOver = SinglePlayerDidWin(cpuGrid); // flag for game loop
    while (!gameOver)
    {



        char* shot = getValidShot(); // Get a valid shot


        int row = toupper(shot[0]) - 'A'; // turn char indices into int indices
        int col = charToInt(shot[1]); // turn char to int indices


        // if statement checking for reused shots
        if (shotGrid[row][col] != UNFIREDSPOT)
        {
            printf("%c%d has already been fired at\n",row+'A',col);
            continue;
        }

        ShotResults results = MakeSinglePlayerShot(row,col); // save player shot results
        shotGrid[row][col] = results; // update my shot grid



        if (SinglePlayerDidWin(cpuGrid))
        {
            printf("Game over you win! Final board\n");
            DisplayWorld(shipGrid,shotGrid);

            gameOver = true;
        }
        if (gameOver) break;;



        // cpus turn
        int cpuRow = 0; // cpus row from which will save from GetSinglePlayerShot()
        int cpuCol = 0; // cpus column from which will save from GetSinglePlayerShot()
        ShotResults cpuResults = GetSinglePlayerShot(&cpuRow,&cpuCol);
        SinglePlayerResponse(cpuRow,cpuCol,cpuResults);


        // Check players shot grid for al destroyed ships
        if (SinglePlayerDidWin(shipGrid))
        {
            printf("Game over CPU wins Final board\n");
            DisplayWorld(shipGrid,shotGrid);
            gameOver = true;
        }

        if (gameOver) break;;

        DisplayWorld(shipGrid,shotGrid);
    }



}

/**
 * Function that returns a valid shoot input
 * @return A valid shot input
 */
char* getValidShot()
{
    static char buffer[10]; // Shot input buffer


    while (1)
    {

        printf("Enter a shot location (e.g A5): ");

        if (fgets(buffer,sizeof(buffer),stdin) == NULL)
        {
            printf("No input in buffer\n");
        }

        // if statement for backwards shot inputs (e.g. 5A)
        if (isdigit(buffer[0]) && isalphaRange(buffer[1]) && buffer[2] == '\n')
        {
            swapBuffer(&buffer[0],&buffer[1]);
        }

        // return valid input
        if (isalphaRange(buffer[0]) && isdigit(buffer[1]) && buffer[2] == '\n')
        {
            return buffer;
        }

        printf("Not a valid shot\n");
    }

}

/**
 * Boolean function for chars that are in range of A-J
 * @param c character
 * @return bool value
 */
bool isalphaRange(char c)
{
    char u = toupper(c);

    if (u >= 'A' && u <= 'J') return true;

    return false;
}

/**
 * Helper function to help backwards shot inputs
 * @param c first char
 * @param c2 Second char
 */
void swapBuffer(char *c, char *c2)
{
    char temp = *c;
    *c = *c2;
    *c2 = temp;
}




