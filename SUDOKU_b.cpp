/**
 * This is a solution to the SUDOKU problem. It uses backtracking to solve 
 * the sudoku puzzle.
 * Author : thecodekaiser
 */

#include <iostream>
#include <sstream>
#include <cstdlib>
#include <cstdio>
#include <vector>
#include <ctime>
#include <cmath>

using namespace std;

// Defining my private MACROS
#define N 9
#define NONE 0

/**
 * Function : To find first unassigned location in the grid
 * Searches whole grid to find a location that is still unassigned
 * if it finds such a place then assigns row and col to that location
 * and returns true, else it returns false.
 */
bool FindPlace(int grid[N][N], int & row, int & col)
{
	for(row = 0; row < N; row ++)
		for(col = 0; col < N; col++)
			if(grid[row][col] == NONE)
				return true;
	return false;
}

/**
 * Function : To check if a number has been used already in the specified row
 */
bool usedInRow(int grid[N][N], int row, int num)
{
	for(int col = 0; col < N; col++)
		if(grid[row][col] == num)
			return true;
	return false;
}

/**
 * Function : To check if a number has been used already in the specified col
 */
bool usedInCol(int grid[N][N], int col, int num)
{
	for(int row = 0; row < N; row++)
		if(grid[row][col] == num)
			return true;
	return false;
}

/**
 * Function : To check if a number has been used already in the specified box
 */
bool usedInBox(int grid[N][N], int boxStartRow, int boxStartCol, int num)
{
	for(int row = 0; row < 3; row++)
		for(int col = 0; col < 3; col++)
			if(grid[row + boxStartRow][col + boxStartCol] == num)
				return true;
	return false;
}


/**
 * Function : To check if it is safe to assign a value to a given cell
 */
bool isSafeLocation(int grid[N][N], int row, int col, int num)
{
	return !usedInCol(grid, col, num) && !usedInRow(grid, row, num) && !usedInBox(grid, row - row % 3, col - col % 3, num);
}

/**
 * Function : A utility function to print grid  
 */
void printGrid(int grid[N][N])
{
    for (int row = 0; row < N; row++)
    {
       for (int col = 0; col < N; col++)
             printf("%2d", grid[row][col]);
        printf("\n");
    }
}

void printGRID(int grid[N][N])
{
    for (int row = 0; row < N; row++)
    {
    	for (int col = 0; col < N; col++)
		if(col != N-1)
        		printf("%d |", grid[row][col]);
		else
			printf("%d ", grid[row][col]);

	printf("\n");

    }
}


/**
 * Function : To initialize the SUDOKU grid and also fill in the empty cell vector
 */
void init(int grid[N][N])
{
	string line;				// To read in a line
	
	cout << "PLEASE ENTER THE SUDOKU: " << endl;

	// This will read each line from the standard input and then fill in the matrix
	for(int i = 0; i < 9; i++)
	{
		getline(cin,line);				// Reading in each line
		istringstream words(line);			// To parse that line
		int j = 0;
		char val;
		while(words >> val)
		{
			if(val >= '1' and val <= '9')
			{	
				grid[i][j++] = val - '0';
			}
			else if(val == '-')
			{
				grid[i][j++] = 0;
			}
		}
	}
	return ;
}

int fill(int grid[N][N])
{
	string line;

	int ret = 0;
	for(int i = 0; i < N; i++)
	{
		cin >> line;
		int cnt = 0;

		for(int j = 0; j < N; j++)
		{
			if(line[cnt] != '0')
				grid[i][j] = line[cnt++] - '0';
			else
			{	grid[i][j] = 0, ret++;	cnt++;	}
		}
	}
	return ret;
}

/** 
 *  Main Solver Function : Uses backtracking. Generates only one solution
 */
bool solve(int grid[N][N])
{
	int row, col;

	// If no unassigned location found then we are done
	if(!FindPlace(grid, row, col))
		return true;			// Done

	// Else first available coordinates are stored in row and col

	// consider digits 1 to 9
	for(int i = 1; i <= 9; i++)
	{
		// If we can fill this number here
		if(isSafeLocation(grid, row, col, i))
		{

			grid[row][col] = i;	// Assign i  
			if(solve(grid))		// Done.. solved
				return true;
		
			// Else undo your action
			grid[row][col] = NONE;
		}
	}

	// Nothing went good
	// Trigger backtracking 
	return false;
}

/* Driver Program to test above functions */
int main()
{
    // 0 means unassigned cells
   int t;
   cin >> t;
   while(t--)
   { 
   	int grid[N][N];

    	int unfilled = fill(grid);
	//printGRID(grid);

	if (solve(grid) == true)
		  printGRID(grid);
	
  }
    	
    return 0;
}
