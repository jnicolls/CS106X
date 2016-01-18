/**
 * File: boggle.cpp
 * ----------------
 * Implements the game of Boggle.
 */
 
#include <cctype>
#include <iostream>
using namespace std;

#include "simpio.h"
#include "gwindow.h"
#include "gboggle.h"
#include "random.h"
#include "grid.h"
#include "set.h"
#include "vector.h"
#include "lexicon.h"
#include "coord.h" // Copied/Imported from Dominosa assignment

const string kEnglishLexiconFilename = "EnglishWords.dat";
const int kBoggleWindowWidth = 650;
const int kBoggleWindowHeight = 350;
const int kNormalBoggleDim = 4;
const int kBigBoggleDim = 5;
const int kMinGuessLength = 4;
const int highlightPause = 100;

const string kStandardCubes[16] = {
   "AAEEGN", "ABBJOO", "ACHOPS", "AFFKPS",
   "AOOTTW", "CIMOTU", "DEILRX", "DELRVY",
   "DISTTY", "EEGHNW", "EEINSU", "EHRTVW",
   "EIOSST", "ELRTTY", "HIMNQU", "HLNNRZ"
};

const string kBigBoggleCubes[25] = {
   "AAAFRS", "AAEEEE", "AAFIRS", "ADENNN", "AEEEEM",
   "AEEGMU", "AEGMNN", "AFIRSY", "BJKQXZ", "CCNSTW",
   "CEIILT", "CEILPT", "CEIPST", "DDLNOR", "DDHNOT",
   "DHHLOR", "DHLNOR", "EIIITT", "EMOTTT", "ENSSSU",
   "FIPRSY", "GORRVW", "HIPRRY", "NOOTUW", "OOOTTU"
};


//Prototypes

static void welcome();
static void giveInstructions();
static void displayManualInitializationTextPrompt();
static void displayProperDimensionsTextPrompt();
static void instructUserHowToInput(int dim);
static Grid<char> makeBoggleBoard();
static Grid<char> getProperDimmensions();
static Vector<char> generateShuffledCubes(int boggleDimensions);
static Vector<string> getCubeSet(int boggleDimensions);
static void fillBoard(Grid<char> & emptyBoggleBoard, Vector<char> charsToFill);
static Vector<char> getValidUserInput (int dim);
static Set<string> playerTurn(Grid<char> & boggleBoard, Lexicon & english);
static void computerTurn(const Grid<char> & boggleBoard, Set<string> & playerAnswers,
                         const Lexicon & english);
static void tryPlayerGuess(const Grid<char> & boggleBoard, Set<string> & playerAnswers,
                           string playerGuess, const Lexicon & english);
static void clearBoard(Set<coord> & wordPath);
static void generateAllPossibleWords(const Grid<char> & boggleBoard, Set<string> & wordsSpottedSoFar,
                                     const Lexicon & english, int rowIndex, int colIndex, string buildingWord,
                                     Set<coord> wordPath);
static bool isShiftValid(const Grid<char> & boggleBoard, int initRow, int initCol, int deltRow, int deltCol);


// Welcomes the user to the game.

static void welcome() {
    cout << "Welcome!  You're about to play an intense game ";
    cout << "of mind-numbing Boggle.  The good news is that ";
    cout << "you might improve your vocabulary a bit.  The ";
    cout << "bad news is that you're probably going to lose ";
    cout << "miserably to this little dictionary-toting hunk ";
    cout << "of silicon.  If only YOU had a gig of RAM..." << endl << endl;
}

// Gives instructions on how to play Boggle.

static void giveInstructions() {
    cout << endl;
    cout << "The boggle board is a grid onto which I ";
    cout << "I will randomly distribute cubes. These ";
    cout << "6-sided cubes have letters rather than ";
    cout << "numbers on the faces, creating a grid of ";
    cout << "letters on which you try to form words. ";
    cout << "You go first, entering all the words you can ";
    cout << "find that are formed by tracing adjoining ";
    cout << "letters. Two letters adjoin if they are next ";
    cout << "to each other horizontally, vertically, or ";
    cout << "diagonally. A letter can only be used once ";
    cout << "in each word. Words must be at least four ";
    cout << "letters long and can be counted only once. ";
    cout << "You score points based on word length: a ";
    cout << "4-letter word is worth 1 point, 5-letters ";
    cout << "earn 2 points, and so on. After your puny ";
    cout << "brain is exhausted, I, the supercomputer, ";
    cout << "will find all the remaining words and double ";
    cout << "or triple your paltry score." << endl << endl;
    cout << "Hit return when you're ready...";
    getLine();
}


/*
 * The method makeBoggleBoard() creates both the grid that keeps track of the configuration
 * of letters in the Boggle board and initializes the display to reflect
 * the state of the board.
 *
 * First, the grid is initialized to a specific set of dimmensions (Either 4*4
 * or 5*5).
 *
 * Then, either a randomized or user-inputed vector of chars is generated, where
 * the "0th" index refers to the character in the top left corner, the "1st" index
 * refers to the character directly to its right, and so and so forth until the row
 * is filled, wherein the second row begins filling, starting from the left.
 *
 * It is assumed that the vectors will be initalzied so that it has exactly enough
 * elements to fill the entire board, no more or less.
 *
 * This initalized grid is then returned to the main method, where it will be referred
 * to when both the human and the computer take their turns.
 */

static Grid<char> makeBoggleBoard(){
   Grid<char> boggleBoard = getProperDimmensions();
   Vector<char> rolledBoggleCubes;
   displayManualInitializationTextPrompt();
   if(!getYesOrNo("Do you want to force the board configuration?")){
       rolledBoggleCubes = generateShuffledCubes(boggleBoard.numRows());
   } else rolledBoggleCubes = getValidUserInput(boggleBoard.numRows());
   fillBoard(boggleBoard, rolledBoggleCubes);
   return boggleBoard;
}

/*
 * In getProperDimmensions(),
 * the grid is initalized to either 4*4 or 5*5, depending on the
 * perference of the user.
 */

static Grid<char> getProperDimmensions() {
   int dimensions;
   displayProperDimensionsTextPrompt();
   if (getYesOrNo("Would you like Big Boggle?")){
        dimensions = kBigBoggleDim;
   } else dimensions = kNormalBoggleDim;
   drawBoard(dimensions, dimensions);
   Grid<char> newBoggleBoard (dimensions, dimensions);
   return newBoggleBoard;
}

//displayProperDimensionsTextPromt() tells the user about their choice of game size.

static void displayProperDimensionsTextPrompt() {
    cout << endl << "You can choose standard Boggle (4x4 grid)" << endl;
    cout << "or Big Boggle (5x5)." << endl;
}

/*
 * displayManualInitializationTextPrompt() tells the user about the option to
 * manually create the game board.
 */

static void displayManualInitializationTextPrompt() {
    cout << endl << "I'll give you a chance to set up the board ";
    cout << "to your specification, which makes it easier ";
    cout <<  "for you to stand a chance against the computer. (Cheater.) " << endl << endl;
}

/*
 * In generateShuffledCubes, a vector of chars, representing shuffled and rolled
 * letter dice, is returned to be properly placed on the boggle grid.
 *
 * A vector of strings is initalized to be the set of dice appropriate for the size of the
 * grid being filled.
 *
 * Then, starting at index 0, each cube is switched with a random position between it's
 * current position and the last position in the vector. (This represents the random
 * arrangment of dice on the game board.
 *
 * Then, a random character from each die string is selected to be the char for that square (representing
 * the randomness of rolling the dice.) This selected char is added to a vector.
 *
 * Once all dice have had one character randomly selected from them, the char vector is returned.
 */

static Vector<char> generateShuffledCubes(int boggleDimensions){
    Vector <string> cubeVector = getCubeSet(boggleDimensions);
    Vector <char> boggleChars;
    int randPos;
    string temp;
    for(int i = 0; i < cubeVector.size(); i++){
        randPos = randomInteger(i, cubeVector.size() - 1);
        temp = cubeVector[randPos];
        cubeVector[randPos] = cubeVector[i];
        cubeVector[i] = temp;
    }
    for (int i = 0; i < cubeVector.size(); i++){
        randPos = randomInteger(0, cubeVector[i].size() - 1);
        boggleChars.add(cubeVector[i][randPos]);
    }
    return boggleChars;
}

/*
 * getCubeSet initalizes a vector to either be a represenation of the array
 * representing the die set for either normal boggle or big boggle, depending
 * on the size of the grid to be filled.
 */

static Vector<string> getCubeSet(int dim){
    Vector<string> finalizedCubeSet;
    if (dim == kNormalBoggleDim){
        for (int i = 0; i < (dim*dim) ; i++) {
            finalizedCubeSet.add(kStandardCubes[i]);
        }
    } else {
        for (int i = 0; i < (dim*dim); i++){
            finalizedCubeSet.add(kBigBoggleCubes[i]);
        }
    }
    return finalizedCubeSet;
}

/*
 * fillBoard() initializes a grid to represent an ordered chars to fill so that
 * it has the proper Boggle oriention. This oriention is reflected on a display
 * for the user.
 */

static void fillBoard(Grid<char> & emptyBoggleBoard, Vector<char> charsToFill){
    int pos = 0;
    for (int i = 0; i < emptyBoggleBoard.numRows(); i++){
        for (int j = 0; j < emptyBoggleBoard.numCols(); j++){
            emptyBoggleBoard[i][j] = charsToFill[pos];
            labelCube(i,j,charsToFill[pos]);
            pos++;
        }
    }
}

/*
 * getValidUserInput retrieves a valid input from the user
 * to manually initalize the boggle grid. It makes sure that
 * the user has inputed a string that is both of proper length
 * and that is entirely composed of alphabetic characters.
 *
 * If not already done, the method converts the valid string
 * into all uppercase letters, to maintain consistency.
 *
 * Once this is done, the string retrieved from the user is
 * changed into a vector of chars in order to maintain consistency
 * within the program, (to later fill the Boggle grid), and is returned.
 */

static Vector<char> getValidUserInput (int dim) {
    instructUserHowToInput(dim);
    Vector <char> manualDice;
    string userInput = "";
    bool isStringValid = false;
    while (!isStringValid){
        cout << endl;
        userInput = getLine("Enter the string: ");
        if (!(userInput.size() == dim * dim) ) {
            cout << endl << "String must include ";
            cout << integerToString(dim*dim);
            cout << " characters." << endl;
            continue;
        }
        isStringValid = true;
        for (int i = 0; i < userInput.size(); i++){
            if(!isalpha(userInput.at(i))){
                cout << endl << "Not a valid string. ";
                cout << "All characters in string ";
                cout << "must be alphabetic." << endl;
                isStringValid = false;
                break;
            }
        }
    }
    userInput = toUpperCase(userInput);
    for(int i = 0; i < userInput.size(); i++){
        manualDice.add(userInput.at(i));
    }
    return manualDice;
}

/*
 * instructUserHowToInput tells the user the requirements
 * of the string they input in order to properly initialize
 * the boggle grid.
 */

static void instructUserHowToInput(int dim) {
    int dimSqu = (dim * dim);
    string stringDim = integerToString(dim);
    string stringDimSqu = integerToString(dimSqu);
    cout << endl << "Enter a " + stringDimSqu + "-character string to identify ";
    cout << "which letters you want on the cubes. ";
    cout << "The first " + stringDim + " letters are the cubes on the ";
    cout << "top row from left to right, the next " + stringDim;
    cout << " letters are the second row, and so on." << endl;
}

/*
 * playerTurn() allows the user to guess as many words that they can that can legally
 * be found according to the rules of Boggle, and shows them on the score. (Showing
 * a graphical representation of the arrangment of cubes briefly every time a new word
 * is found). When they have run out of words to guess, and reflect this through the
 * input of a blank string, the method returns a set of all the answers that the player
 * has guessed.
 */

static Set<string> playerTurn(Grid<char> & boggleBoard, Lexicon & english){
    Set<string> playerAnswers;
    string playerGuess;
    cout << "Enter words you see on the board." << endl << endl;
    while (true) {
        playerGuess = getLine("Enter word: ");
        if (playerGuess == "") break;
        tryPlayerGuess(boggleBoard, playerAnswers, playerGuess, english);
    }
    return playerAnswers;
}

/*
 * This is the helper method that implments the recursive call necessary to
 * discover whether the word that the user inputed is legal or not.
 *
 * The method starts by adding the char that is currently indexed (through rowIndex
 * and colIndex) to the word that has accumulated thus far. If it is, the word the
 * player has selected is recorded, the cube is highlighted, the cube is noted, and
 * the method returns "true," indicating that the method was sucessful in finding
 * an orientation that properly spells the user's word.
 *
 * Then, a check is done to see if the word could possibly be spelled out using the
 * characters that have accumulated thus far. If not, the method returns false, indicating
 * that this path is a dead end.
 *
 * Else, every character surrouding the character, that has not already been used in
 * the path that led to this cube, is checked using this recursive call. If one of these
 * recursive calls is sucessful, the cube that led to it is highlighted and noted, and the method
 * returns true, doing a similar thing for potential previous recursive calls.
 *
 * If every recursive call ends in failure, then this road is a dead end, and is indicated as such
 * through a return of "fasle."
 */

static bool tryPlayerGuess(const Grid<char> & boggleBoard, Set<string> & playerAnswers,
                           string playerGuess, const Lexicon & english, string buildingWord,
                           int rowIndex, int colIndex, Set<coord> & wordPath) {
    buildingWord += boggleBoard[rowIndex][colIndex];
    if (playerGuess.find(buildingWord) != 0) return false;
    if (playerGuess == buildingWord){
        playerAnswers.add(playerGuess);
        highlightCube(rowIndex, colIndex, true);
        coord pos;
        pos.row = rowIndex;
        pos.col = colIndex;
        wordPath.add(pos);
        return true;
    }
    for(int i = -1; i <= 1; i++){
        for(int j = -1; j <= 1; j++) {
            if(isShiftValid(boggleBoard, rowIndex, colIndex, i, j)){
                coord nextPos;
                nextPos.row = rowIndex + i;
                nextPos.col = colIndex + j;
                if (!wordPath.contains(nextPos)) {
                    wordPath.add(nextPos);
                    if (tryPlayerGuess(boggleBoard, playerAnswers, playerGuess,
                                       english, buildingWord, rowIndex + i,
                                       colIndex + j, wordPath)){
                        highlightCube(rowIndex, colIndex, true);
                        return true;
                    } else {
                        wordPath.remove(nextPos);
                    }
                }
            }
        }
    }
    return false;
}

/*
 * This is a helper function that initalizes the conditions necessary to sucessfully
 * call the recursive search that determines whether the user's word can be legally
 * found on the grid.
 *
 * First, the guess is checked to see if it is long enough, in the english language,
 * and not already guessed.
 *
 * Then, each spot on the board is checked to see if the word could be built from that
 * spot using a recursive search.
 *
 * If it is, then all the words that make up the word are briefly highlighted, the word
 * is added to the scoreborad, and the word is also added to an internal list of words
 * the player has found so far.
 *
 * If not, the user is told that the word cannot be found on the board.
 */

static void tryPlayerGuess(const Grid<char> & boggleBoard, Set<string> & playerAnswers,
                           string playerGuess, const Lexicon & english) {
    if (playerGuess.size() < kMinGuessLength){
        cout << endl << "Words need to be at least " +
             integerToString(kMinGuessLength) + " characters long" << endl;
        return;
    }
    if (!english.contains(playerGuess)){
        cout << endl << "That word is not in the english language" << endl;
        return;
    }
    playerGuess = toUpperCase(playerGuess);
    if (playerAnswers.contains(playerGuess)){
        cout << endl << "You have already guessed that word" << endl;
        return;
    }
    for(int i = 0; i < boggleBoard.numRows(); i++){
        for(int j = 0; j < boggleBoard.numCols(); j++){
            Set <coord> wordPath;
            coord pos;
            pos.row = i;
            pos.col = j;
            wordPath.add(pos);
            if(tryPlayerGuess(boggleBoard, playerAnswers,
                           playerGuess, english, "", i, j, wordPath)){
                pause(highlightPause);
                clearBoard(wordPath);
                recordWordForPlayer(playerGuess, HUMAN);
                return;
            }
        }
    }
    cout << endl << "That word is not on the board." << endl;
}

//clearBoard() ensures that all characters that were highlighted become unhighlighted

static void clearBoard(Set<coord> & wordPath){
    for(coord pos: wordPath){
        highlightCube(pos.row, pos.col, false);
    }
}

/*
 * computerTurn() conducts an exhaustive recursive search on every cell in the grid to
 * discover every word that can be legally obtained from the boggleGrid that has not
 * already been discoverd by the user.
 */

static void computerTurn(const Grid<char> & boggleBoard, Set<string> & wordsAlreadySpotted,
                         const Lexicon & english){
    Set<coord> wordPath;
    for (int i = 0; i < boggleBoard.numRows(); i++) {
        for (int j = 0; j < boggleBoard.numCols(); j++) {
            coord pos;
            pos.row = i;
            pos.col = j;
            wordPath.add(pos);
            generateAllPossibleWords(boggleBoard, wordsAlreadySpotted, english, i, j, "", wordPath);
            wordPath.remove(pos);
        }
    }
}

/*
 * generateAllPossibleWords functions to exhaustively find every word that can be obtained, give a
 * pervious prefix passed in, and a current location on the board.
 *
 * First, the char at the location (rowIndex, colIndex) is added, and it is checked to see if it an
 * undiscovered english word of legal size. If it is, it is added to the score board and list
 * of words discovered so far.
 *
 * Then, the word is checked to see if it is a prefix of any english word. If it is not, this branch
 * is seen as a dead end, and the method terminates.
 *
 * Else, every cell surrounding the indexed cell is checked to see if they can be used to potentially
 * create new words.
 *
 * Once all of these exhaustive searchs are made, the method terminates.
 */

static void generateAllPossibleWords(const Grid<char> & boggleBoard, Set<string> & wordsSpottedSoFar,
                                     const Lexicon & english, int rowIndex, int colIndex, string buildingWord,
                                     Set<coord> wordPath){
    buildingWord += boggleBoard[rowIndex][colIndex];
    if (buildingWord.size() >= kMinGuessLength && english.contains(buildingWord)
           && !wordsSpottedSoFar.contains(buildingWord)) {
         wordsSpottedSoFar.add(buildingWord);
         recordWordForPlayer(buildingWord, COMPUTER);
    }
    if (!english.containsPrefix(buildingWord)) return;
    for (int i = -1; i <= 1; i++) {
        for (int j = -1; j <=1; j++){
            if(isShiftValid(boggleBoard, rowIndex, colIndex, i, j)){
                coord nextPos;
                nextPos.row = rowIndex + i;
                nextPos.col = colIndex + j;
                if (!wordPath.contains(nextPos)){
                    wordPath.add(nextPos);
                    generateAllPossibleWords(boggleBoard, wordsSpottedSoFar, english,
                                             rowIndex + i, colIndex + j, buildingWord, wordPath);
                    wordPath.remove(nextPos);
                }
            }
        }
    }
}

//isShiftValid checks to make sure that the cell to shift focus to is both in bounds and not the current cell.

static bool isShiftValid(const Grid<char> & boggleBoard, int initRow, int initCol, int deltRow, int deltCol){
    return (( (deltRow != 0) || (deltCol != 0) ) && boggleBoard.inBounds(initRow + deltRow, initCol + deltCol));
}

/*
 * This is the main method for the Boggle Program.
 *
 * The user is welcomed, and asked if they need instructions. They are
 * provided if the user wants them.
 *
 * A boggle board is then initalized. It can either be randomly generated
 * from a specific set of dice, or manually inputed by the user. It can
 * have dimensions of either 4*4 or 5*5.
 *
 * The player is then allowed to find as many words as they can. These words
 * are graphically displayed, and a score is put aside them.
 *
 * The computer finds all remaining words. These wrods are graphically displayed,
 * and a score is put aside them.
 *
 * The user is then asked to play again. If they say yes, another board is made,
 * and the computer and user have another turn. if not, the program ends.
 */

int main() {
   GWindow gw(kBoggleWindowWidth, kBoggleWindowHeight);
   Grid<char> boggleBoard;
   Set<string> playerAnswers;
   Lexicon english (kEnglishLexiconFilename);
   initGBoggle(gw);
   welcome();
   if (getYesOrNo("Do you need instructions?")) {
      giveInstructions();
   }
   while(true){
      boggleBoard = makeBoggleBoard();
      playerAnswers = playerTurn(boggleBoard, english);
      computerTurn(boggleBoard, playerAnswers, english);
      if (!getYesOrNo("Do you want to play again?")) break;
   }
   return 0;
}
