/**
 * File: maze-generator.cpp
 * ------------------------
 * Presents an adaptation of Kruskal's algorithm to generate mazes.
 */

#include <iostream>
using namespace std;

#include "console.h"
#include "random.h"
#include "simpio.h"
#include "maze-graphics.h"
#include "maze-types.h"
#include "set.h"
#include "vector.h"

//Prototypes
static int getMazeDimension(string prompt);
static Vector<wall> initializeWallsAndChambers(int initialDimensions, Set<cell> & initialCellList, MazeGeneratorView & mazeWindow);
static void initializeCellWalls(int cellX, int cellY, Vector<wall> & originalWallVector, int originalDimensions, MazeGeneratorView & originalWindow);
static Vector<wall> shuffleWalls(Vector<wall> unshuffledWallVector);
static void removeSeparatingWalls(Vector<wall> wallOrder, MazeGeneratorView & finalWindow, Set<cell> & uneliminatedCells);
static void createNewMergedCell(Vector<Set <cell> > & origMergedCells, Set<cell> & origUnmergedCells, cell unmergedOne, cell unmergedTwo);
static void addToMergedCell(Vector<Set <cell> > & origMergedCells, Set<cell> & origUnmergedCells, cell mergedCell, cell unmergedCell);
static void combineTwoMergedCells(Vector<Set <cell> > & origMergedCells, cell mergedOne, cell mergedTwo);
static const int minDimension = 7;
static const int maxDimension = 50;

/*
 * This main method organizes the data necessary to animate the maze generation.
 * (Graphics details are covered in maze-graphics.cpp)
 *
 * This method prompts the user for the dimmensions of a maze, creates a new display
 * to show that maze, stores a representation of all current cells in a set, and stores
 * all the walls (in random order) in a vector.
 *
 * These two data structures are then fed into a third method which removes every wall
 * in the vector that separates two chambers (leaving walls that separate the "same" chamber.)
 *
 * The user then hits enter to either generate another maze or to terminate the program, depending
 * on their choice.
 */
int main() {
    while(true){
        int dimension = getMazeDimension("What should the dimension of your maze be [0 to exit]? ");
        if (dimension == 0) break;
        MazeGeneratorView mazeWindow;
        Set<cell> cellList;
        mazeWindow.setDimension(dimension);
        Vector<wall> shuffledWallVector = initializeWallsAndChambers(dimension, cellList, mazeWindow);
        removeSeparatingWalls(shuffledWallVector, mazeWindow, cellList);
        getLine("Press enter to play again.");
        cout << endl;
	}

	return 0;
}

/*
 * Prompts a number from the user which is returned to the main method. This number will determine the height & width (in cells) of
 * the maze.
 */
static int getMazeDimension(string prompt) {
    while (true) {
        int response = getInteger(prompt);
        if (response == 0) return response;
        if (response >= minDimension && response <= maxDimension) return response;
        cout << "Please enter a number between "
             << minDimension << " and "
             << maxDimension << ", inclusive." << endl;
    }
}

/*
 * This method both returns a shuffled vectors of all walls in the current maze to the main method, as well as modifying a set of cells
 * to reflect the total number of cells currently present in the maze. In addition, this method is also responsible for drawing the intitial
 * grid (without any walls removed) on the MazeGeneratorView.
 *
 * Speficially, this method creates an initial wall vector to start. A cell is generated for every possible coordinate pair given the dimensions of the
 * maze, and for each cell, the walls that bind that cell are added to the initial wall vector. All cells are also added to a Set, which will later be
 * used to keep track of which cells have been 'merged' and which cells have been 'unmerged.'
 *
 * Once this is all done, the initial wall vector (which now contains every wall in the maze), is shuffled. This shuffled vector is what is returned to the main
 * method.
 */
static Vector<wall> initializeWallsAndChambers(int initialDimensions, Set<cell> & initialCellList, MazeGeneratorView & mazeWindow){
    Vector <wall> initialWallVector;
    mazeWindow.drawBorder();
    for (int i = 0; i < initialDimensions; i++){
        for (int j = 0; j < initialDimensions; j++){
            cell newCell;
            newCell.row = i;
            newCell.col = j;
            initialCellList.add(newCell);
            initializeCellWalls(i, j, initialWallVector, initialDimensions, mazeWindow);
        }
    }
    Vector<wall> newlyShuffledWallVector = shuffleWalls(initialWallVector);
    return newlyShuffledWallVector;
}

/*
 * This method checks to see if a cell has space directly south and directly east of it for a new wall. If there is space for it, a wall is drawn on
 * the MazeGeneratorView and a wall is added to a vector.
 */
static void initializeCellWalls(int cellX, int cellY, Vector <wall> & originalWallVector, int originalDimensions, MazeGeneratorView & originalWindow){
    wall newWall;
    cell cellOne;
    cellOne.row = cellX;
    cellOne.col = cellY;
    cell cellTwo;
    for (int i = 0; i <= 1; i++){
        for (int j = 0; j <= 1; j++){
            if ( i != j ) {
                if ((cellX + i < originalDimensions) && (cellY + j < originalDimensions)) {
                    cellTwo.row = cellX + i;
                    cellTwo.col = cellY + j;
                    newWall.one = cellOne;
                    newWall.two = cellTwo;
                    originalWallVector.add(newWall);
                    originalWindow.drawWall(newWall);
                }
             }
           }
    }
}

/*
 * This method creates a new vector, called "shuffled walls." The method then removes a wall from a random index in
 * a vector passed in from a parameter, and adds it to the end of the new vector. This process continues until the vector
 * passed in from the parameter is empty. The "shuffled" vector is then returned to the calling method.
 */

static Vector<wall> shuffleWalls(Vector<wall> unshuffledWallVector){
    Vector<wall> shuffledWalls;
    wall randomWall;
    int randomOrdering;
    while ( !unshuffledWallVector.isEmpty() ) {
        randomOrdering = randomInteger(0, unshuffledWallVector.size() - 1);
        randomWall = unshuffledWallVector.get(randomOrdering);
        shuffledWalls.add(randomWall);
        unshuffledWallVector.remove(randomOrdering);
    }
    return shuffledWalls;
}

/*
 * This method cycles through a list of walls, and removes the wall from the display if and only if the wall sepearates two
 * distinct cells.
 *
 * This is done by keeping track of all 'mergedCells' through a vector of sets of cells. Before every single wall is removed,
 * a check is performed to make sure that the two cells the wall separates aren't included together in any one of the sets included
 * in the 'mergedCells' vector.
 *
 * If a wall is removed, this means that either a new cell was formed, a previously unmerged cell joined a merged cell, or that two
 * merged cells have joined. The rest of the method checks each one of these individual cases, and modifies the 'mergedCell' vector appropriately.
 */
static void removeSeparatingWalls(Vector<wall> wallOrder, MazeGeneratorView & finalWindow, Set<cell> & unmergedCells) {
    Vector<Set <cell> > mergedCells;
    bool cellsAreSame;
    for(int i = wallOrder.size() - 1; i >= 0 ; i--){
        cellsAreSame = false;
        cell testCellOne = wallOrder[i].one;
        cell testCellTwo = wallOrder[i].two;
        for (int j = 0; j < mergedCells.size(); j++){
            if (mergedCells[j].contains(testCellOne) && mergedCells[j].contains(testCellTwo)){
                cellsAreSame = true;
            }
        }
        if (!cellsAreSame) {
            finalWindow.removeWall(wallOrder[i]);
            if (unmergedCells.contains(testCellOne) && unmergedCells.contains(testCellTwo)) {
                createNewMergedCell (mergedCells, unmergedCells, testCellOne, testCellTwo);
            } else if (!unmergedCells.contains(testCellOne) && unmergedCells.contains(testCellTwo)) {
                addToMergedCell (mergedCells, unmergedCells, testCellOne, testCellTwo);
            } else if (unmergedCells.contains(testCellOne) && !unmergedCells.contains(testCellTwo)){
                addToMergedCell (mergedCells, unmergedCells, testCellTwo, testCellOne);
            } else {
                combineTwoMergedCells (mergedCells, testCellOne, testCellTwo);
            }
        }
    }
}

/*
 * This method takes two cells known to be previously unmerged, creates a new 'mergedcell' set from the two of them, and adds it to the master 'merged' cell
 * vector. As both of these cells are now merged, they are removed from the unmerged cell set.
 */
static void createNewMergedCell(Vector<Set <cell> > & origMergedCells, Set<cell> & origUnmergedCells, cell unmergedOne, cell unmergedTwo){
    Set<cell> newMergedCell;
    origUnmergedCells.remove(unmergedOne);
    origUnmergedCells.remove(unmergedTwo);
    newMergedCell.add(unmergedOne);
    newMergedCell.add(unmergedTwo);
    origMergedCells.add(newMergedCell);
}

/*
 * This method takes one cell that is known to be part of a merged cell and one cell that is known to be previously unmerged, and adds the formerly unmerged cell
 * to the appropriate merged cell set in the master merged cell vector. The formerly unmerged cell is removed from the unmerged cell set.
 */
static void addToMergedCell(Vector<Set <cell> > & origMergedCells, Set<cell> & origUnmergedCells, cell mergedCell, cell unmergedCell){
    for (int i = 0; i < origMergedCells.size(); i++){
        if (origMergedCells[i].contains(mergedCell)){
            origMergedCells[i].add(unmergedCell);
            break;
        }
    }
    origUnmergedCells.remove(unmergedCell);
}
/*
 * This method takes in two cells that are known to be part of a merged cell (but not the same merged cell), and combines the two sets that each individual
 * cell was a part of. Any duplication (multiple instances of the same cell in the master merged cell vector) that results from this is deleted at the end
 * of the method.
 */
static void combineTwoMergedCells(Vector<Set <cell> > & origMergedCells, cell mergedOne, cell mergedTwo){
    int addressToSend;
    int addressToEvict;
    for (int i = 0; i < origMergedCells.size(); i++){
        if (origMergedCells[i].contains(mergedOne)){
            addressToSend = i;
        }
        if (origMergedCells[i].contains(mergedTwo)){
            addressToEvict = i;
        }
    }
    origMergedCells[addressToSend] += origMergedCells[addressToEvict];
    origMergedCells.remove(addressToEvict);
}

