
/**
 * File: life.cpp
 * --------------
 * Implements the Game of Life.
 */

#include <iostream>  // for cout
using namespace std;

#include "console.h" // required of all files that contain the main function
#include "simpio.h"  // for getLine
#include "gevents.h" // for mouse event detection
#include "strlib.h"

#include "life-constants.h"  // for kMaxAge
#include "life-graphics.h"   // for class LifeDisplay

// Prototypes
static void welcome();
static void initializeGrid(Grid<int> & newGrid, LifeDisplay & display);
static void promptUserForFile(ifstream & infile, string prompt); // Inspired by Eric Roberts
static void fillInGrid (ifstream & infile, Grid<int> & unfilledGrid, LifeDisplay & unfilledDisplay);
static void determineDimensions(ifstream & infile, Grid<int> & unsizedGrid, LifeDisplay & unsizedDisplay);
static void placeFirstCells(ifstream & infile, Grid<int> & lifelessGrid, LifeDisplay & lifelessDisplay);
static void fillGridAndDisplay (Grid<int> & filledBoard, LifeDisplay & filledDisplay);
static void runSimulation (Grid<int> & simulatedBoard, LifeDisplay & simulatedDisplay); // Inspired by Julie Zewinski
static void advanceBoard(Grid<int> & previousBoard, LifeDisplay & previousDisplay, bool & advanceHasSimulationEnded);
static void createNextBoard(Grid<int> & priorBoard , bool & nextHasSimulationEnded);
static bool howIsCellDoing (int rows, int cols, Grid<int> priorBoard, Grid<int> & newBoard, bool & specialCaseHasSimulationEnded);
static int determineNumNeighbors (int rows, int cols, const Grid<int> neighborBoard);
static void runManualSimulation(Grid<int> & manualBoard, LifeDisplay & manualDisplay, bool & manualHasSimulationEnded);
static void runAutoSimulation(Grid<int> & autoBoard, LifeDisplay & autoDisplay, int pauseTime, bool & autoHasSimulationEnded);
static int askUserForPauseTime();
static bool askUserToPlayAgain();
static void goodbye();

/*
 * This is the main method of the program.
 *
 * The purpose of this program is to simulate the game of Conway's Game of Life.
 * The main method initializes a grid which will keep track of relevant data and
 * a display to show a visualization of the data inside the grid.
 *
 * The program will then enter a while loop which will continue until the user chooses
 * to break outside of it, and therefore end the program. The while loop is comprised
 * of three main steps. The first, 'initalize grid', builds a new grid and constructs
 * a new display based on a file that the user feeds into the program through the console.
 * The second, 'run simulation', runs the game until the game comes to a natural end
 * or the user forces the program to stop. The third step asks the user if they wish
 * to play again. If the user does not, the while loop is exited.
 *
 * The program then displays a message, through 'goodbye,' telling the user that
 * the program is over. After this, the program ends.
 */

int main() {
    Grid<int> lifeBoard;
    welcome();
    LifeDisplay display;
    display.setTitle("Game of Life");
    while (true) {
        initializeGrid(lifeBoard , display);
        runSimulation(lifeBoard, display);
        bool doesUserWantToPlayAgain = askUserToPlayAgain();
        if (!doesUserWantToPlayAgain) break;
    }
    goodbye();
    return 0;
}

/* Print out greeting for beginning of program. */
static void welcome() {
    cout << "Welcome to the game of Life, a simulation of the lifecycle of a bacteria colony." << endl;
    cout << "Cells live and die by the following rules:" << endl << endl;
    cout << "\tA cell with 1 or fewer neighbors dies of loneliness" << endl;
    cout << "\tLocations with 2 neighbors remain stable" << endl;
    cout << "\tLocations with 3 neighbors will spontaneously create life" << endl;
    cout << "\tLocations with 4 or more neighbors die of overcrowding" << endl << endl;
    cout << "In the animation, new cells are dark and fade to gray as they age." << endl << endl;
    getLine("Hit [enter] to continue....   ");
}

/* Prints out farewell for the program */
static void goodbye() {
    cout << endl;
    cout << "The program has ended." << endl;
}

/*
 * This method creates a blank ifstream, prompts the user to insert a file that
 * initalizes the infile, and then fills out the Grid data sturcture using the
 * contents of that file. When this is all completed, the file closes.
 */
static void initializeGrid(Grid<int> & lifeBoard, LifeDisplay & newDisplay){
    ifstream infile;
    promptUserForFile(infile, "Insert filename, then press enter: ");
    fillInGrid(infile, lifeBoard, newDisplay);
    infile.close();
}

/*
 * This method first initalizes the dimmensions of the Grid data structure, and then
 * fills in the initial data (from an ifstream) into that data structure.
 */
static void fillInGrid(ifstream & infile, Grid<int> & unfilledLifeGrid, LifeDisplay & unfilledDisplay) {
    determineDimensions(infile, unfilledLifeGrid, unfilledDisplay);
    placeFirstCells (infile, unfilledLifeGrid, unfilledDisplay);
}

/*
 * This method was heavily inspired by Eric Roberts. (See C++ textbook)
 *
 * The method prompts the user to insert a file's name through the console.
 * If the program is unable to open the file, the method tells the user to try again.
 * It will continue doing so until the user inputs a valid file name.
 */

static void promptUserForFile(ifstream & infile, string prompt){
    while (true)    {
        cout << prompt;
        string fileName;
        getline(cin, fileName);
        infile.open(fileName.c_str());
        if (!infile.fail()) break;
        infile.clear();
        cout << "Unable to open that file. Try again." << endl;
    }
}

/*
 * This method determines the dimmesnions of the Grid data structue from specified
 * numbers in an ifstream. This is done so by first skipping every line in the ifstream
 * that starts with an asterisk. (These are comments, and do not need to be included
 * in the data structure in anyway). Once this is done, it is assumed that the next
 * two lines in the ifstream will consist of a single number each. With this assumption,
 * the Grid is initalized to have the same number of rows as the first number in
 * the ifstream and the same number of columns as the second number in the ifstream.
 */

static void determineDimensions(ifstream & infile, Grid<int> & unsizedGrid, LifeDisplay & unsizedDisplay){
    string testLine;
    int rows;
    int columns;
    while (true){
        getline(infile, testLine);
        if (!(testLine.at(0) == '#')) break;
    }
    rows = stringToInteger(testLine);
    getline(infile, testLine);
    columns = stringToInteger(testLine);
    unsizedGrid.resize(rows, columns);
    unsizedDisplay.setDimensions(rows, columns);
}

/*
 * This method reads lines from an ifstream, one by one, and places the number '1'
 * in the proper spot on the grid every time the character 'X' appears in the ifstream.
 */

static void placeFirstCells (ifstream & infile, Grid<int> & lifelessGrid, LifeDisplay & lifelessDisplay){
    string lifeRowBlueprint;
    for (int i = 0; i < lifelessGrid.numRows(); i++){
        getline(infile, lifeRowBlueprint);
        for (int j = 0; j < lifelessGrid.numCols(); j++){
            if (lifeRowBlueprint.at(j) == 'X') {
                lifelessGrid[i][j] = 1;
                lifelessDisplay.drawCellAt(i, j, 1);
            }
        }
    }
}

/*
 * This method updates the display so that it shows an updated visualization of the
 * data within the Grid data structure to the user.
 */

static void fillGridAndDisplay (Grid<int> & filledBoard, LifeDisplay & filledDisplay){
    for (int i = 0; i < filledBoard.numRows(); i++){
        for (int j = 0; j < filledBoard.numCols(); j++){
            filledDisplay.drawCellAt(i, j, filledBoard[i][j]);
        }
    }
}

/*
 * This method discovers what speed the user wants the simulation to run at, and then
 * runs the simulation at the requested speed. It also initalizes a boolean that will
 * be referenced in other parts of the program to keep track of whether the simulation
 * has ended or not.
 */

static void runSimulation(Grid<int> & simulatedBoard, LifeDisplay & simulatedDisplay){
    bool hasSimulationEnded = false;
    int selectedPauseTime = askUserForPauseTime();
    if (selectedPauseTime == 0) runManualSimulation(simulatedBoard, simulatedDisplay, hasSimulationEnded);
    else runAutoSimulation(simulatedBoard, simulatedDisplay, selectedPauseTime, hasSimulationEnded);

}

/*
 * This method tells the user the directions for how to manage the progression of a manual
 * simulation, and then waits. Every time the user hits the 'enter' key, the simulation is
 * allowed to advance one step. If the user inputs 'quit' during this phase, the simulation will
 * end and exit. The program will enter the while loop without user input if the simulation has
 * met 'ending' conditions.
 */

static void runManualSimulation(Grid<int> & manualBoard, LifeDisplay & manualDisplay, bool & manualHasSimulationEnded){
    string userInput;
    cout << endl;
    cout << "You have chosen 'User-Operated' speed. In order for the simulation to progress one frame, you must press the enter bar each time. To stop the simulation, type 'quit' in all lower case letters."<< endl;
    while (!manualHasSimulationEnded) {
        userInput = getLine();
        if (userInput == "quit") break;
        advanceBoard(manualBoard, manualDisplay, manualHasSimulationEnded);
    }
}

/*
 * This method automatically runs a simulation at a previously specificed speed
 * without user input. The simulation will exit if it meets 'ending' conditions or
 * if the user clicks on the display window.
 */

static void runAutoSimulation(Grid<int> & autoBoard, LifeDisplay & autoDisplay, int pauseTime, bool & autoHasSimulationEnded){
    cout << endl;
    cout << "You have chosen an automatic simulation. To end it, click on the display where the game is playing" << endl;
    while (!autoHasSimulationEnded) {
        GMouseEvent me = getNextEvent(MOUSE_EVENT);
        if(me.getEventType() == MOUSE_CLICKED){
                return;
        } else if (me.getEventType() == NULL_EVENT){
            advanceBoard(autoBoard, autoDisplay, autoHasSimulationEnded);
            pause(pauseTime);
        }
    }
}

/*
 * This method deduces what the data for the next step of the simulation will be, and
 * updates the display with this new information.
 */

static void advanceBoard(Grid<int> & previousBoard, LifeDisplay & previousDisplay, bool & advanceHasSimulationEnded){
    createNextBoard (previousBoard, advanceHasSimulationEnded);
    fillGridAndDisplay(previousBoard, previousDisplay);
}

/*
 * This method deduces what the data for the next step of the simulation will be, and
 * updates the current Grid data structure with this new information. It also checks to see
 * if the simulation has ended yet.
 *
 * It first initalizes a new grid with the same dimmensions as the grid containing the old information.
 *
 * A 'nextHasSimulationEnded' boolean (passed down by reference, which dictates whether or not this method will be called again), is initalized to be true. If the data structure
 * is modifed in any way, this means that the simulation has not stabilized yet. In the event that the data stucture is modified, this boolean will be modified to 'false,'
 * allowing for this method to be referenced again in the near future.
 *
 * A boolean is then initalized which will be used to determine if each individual space is conducive or toxic towards the new cell's life.
 *
 * A double for loop is then used to cycle through every single cell in the Grid data structure.
 *
 * Since the age of a cell does not impact whether it survives a generation or not, each cell in the new data structure, if alive, has its
 * age added to by 1.
 *
 * It is then decided whether the cell is living in a toxic enviorment thorugh the method "howIsCellDoing" being invoked on the OLD data
 * structure. This evaluation is not done on the new grid so that deaths and births of cells in the same generation do not impact each other.
 * (Note: howIsCellDoing is the method that handles the special case of an empty cell surrounded by three neighbors.)
 *
 * If the enviorment is toxic, the cell is reinitalized in the new grid to have a value of zero. If the corresponding value in the old data structure
 * was not zero already, this means that a cell death has occured, and therefore the simulation has not ended yet. The 'hasSimulationEnded' boolean is modified to reflect this.
 *
 * Then, the cell is checked to see whether it is equal to 0 or the maximum age. If it is not equal to either of those things, that means that the simulation has not met ending conditions yet,
 * and so the 'hasSimulationEnded' boolean is modified to reflect this.
 *
 * The old board is then overwritten with the data currently in the new board, so that this new data can be both visualized to the user and passed back into this method to deduce what the next
 * step in the simulation is.
 */

static void createNextBoard(Grid<int> & priorBoard , bool & nextHasSimulationEnded) {
    Grid<int> newBoard(priorBoard.numRows(), priorBoard.numCols());
    bool isEnviormentToxic;
    nextHasSimulationEnded = true;
    for (int i = 0; i < priorBoard.numRows(); i++) {
        for (int j = 0; j < priorBoard.numCols(); j++){
            if (priorBoard[i][j] != 0) newBoard[i][j] = priorBoard[i][j] + 1;
            isEnviormentToxic = howIsCellDoing(i, j, priorBoard, newBoard, nextHasSimulationEnded);
            if (isEnviormentToxic) {
                newBoard [i][j] = 0;
                if (priorBoard[i][j] != 0) nextHasSimulationEnded = false;
            }
            if (newBoard[i][j] != 0 && newBoard [i][j] != kMaxAge) nextHasSimulationEnded = false;
        }
    }
    priorBoard = newBoard;
}

/*
 * Given a specific cell cooridate pair, this method determines how many neighbors the cell has.
 *
 * After this, it determines, from the number of neighbors, whether this grid is in the unique position to spawn a cell. If it can, the NEW Grid is modified to reflect this. Since this
 * is a modification of the data structure, the 'hasSimulationEnded' boolean is reflected to show that the simulation has not met ending conditions yet.
 *
 * Having dealt with the potential existence of this special case, the method returns a boolean reflecting whether this specific coordinate pair is adequately suited to support another generation
 * of a cell.
 */

static bool howIsCellDoing (int rows, int cols, Grid<int> priorBoard, Grid<int> & newBoard, bool & specialCaseHasSimulationEnded){
    int numNeighbors = determineNumNeighbors(rows, cols, priorBoard);
    if (numNeighbors == 3 && priorBoard[rows][cols] == 0) {
        newBoard[rows][cols] = 1;
        specialCaseHasSimulationEnded = false;
    }
    return (numNeighbors <= 1 || numNeighbors >= 4);
}

/*
 * This method determines the number of neighbors a cell has. It does so by iterating through a double for loop, which systematically checks every cell around the specific
 * coordinate pair passed into the method to see if a cell is present (a value not equal to zero.)
 *
 * A series of if statements are present to prevent the method from checking a non-existent portion of the grid, and from the method checking the specific cell-coordinate pair
 * it is passed to see if a cell is present.
 */

static int determineNumNeighbors (int rows, int cols, Grid<int> neighborBoard){
    int total = 0;
    for (int i = -1; i <= 1; i++) {
        for (int j = -1; j<= 1; j++){
            if ( (rows + i) >= 0 && (rows + i) < neighborBoard.numRows()){
                if ((cols + j) >= 0 && (cols + j) < neighborBoard.numCols()){
                    if ( (i != 0) || (j != 0) ){
                        if (neighborBoard[rows + i][cols + j] != 0){
                            total++;
                        }
                    }
                }
            }
        }
    }
    return total;
}

/*
 * This method gets a desired speed of the simulation from the user through text input from the console window.
 */
static int askUserForPauseTime() {
    string userInput;
    char firstChar;
    while (true){
        cout << endl;
        cout << "Please select a speed for the simulation. Type 's' for slow, 'm' for medium, 'f' for fast, or 'u' for user-operated. (manual) : ";
        getline(cin, userInput);
        firstChar = userInput.at(0);
        firstChar = tolower(firstChar);
        if (firstChar == 's') return 1000;
        if (firstChar == 'm') return 250;
        if (firstChar == 'f') return 100;
        if (firstChar == 'u') return 0;
        cout << "That selection is not valid, please try again." << endl;
    }
}

/*
 * After a simulation is terminated, this method determines if the user wishes to start another simulation or instead exist the program. This method
 * is obtained from the user through text input in a console window.
 */
static bool askUserToPlayAgain() {
    string userInput;
    char firstChar;
    cout << "The simulation has ended." << endl;
    while (true)    {
        cout << endl;
        cout << "Do you want to play again? Type 'y' for yes and 'n' for no: ";
        getline(cin, userInput);
        firstChar = userInput.at(0);
        firstChar = tolower(firstChar);
        if (firstChar == 'y') return true;
        if (firstChar == 'n') return false;
        cout << "That selection is not valid, please try again." << endl;
    }
}
