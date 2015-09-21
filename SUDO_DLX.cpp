/**
 * Author: thecodekaiser
 * Note  : The following code is an attempt to write the famous Algorithm X (that solves)
 * 	   exact-cover problem using the Dancing-Links Algorithm.I studied it from various
 * 	   sources and but I would like to credit: Andy Giese, whose blog helped me in the implementation
 *         This is the link: https://gieseanw.wordpress.com/2011/06/16/solving-sudoku-revisited/
 * 
 * 	   Following algo can be implemented for all sizes, you just need to change the constants defined
 * 	   in the beginning. It uses the S-heuristic in selecting a constraint (selects that column that has
 *	   the min no of 1s in it) to reduce the branching factor.
 */
#include <bits/stdc++.h>
using namespace std;

// Defining the dimensions of the matrix
#define MATRIX_SIZE 9		// No of digits we are using currently
#define MAX_COLS    324		// Actually Columns represent constraints.. No of constraints = 4 * (MATRIX_SIZE)^2
#define MAX_ROWS    729	 	// Actually Rows    represent possibility.. No of possibility = (MATRIX_SIZE) ^ 3

#define ROW_OFFSET   0		// This is is how I am going to divide the matrix.. 
#define COL_OFFSET   81		// Row Constraints -> Col Constraints -> Cell Constraints -> Box Constraints
#define CELL_OFFSET  162
#define BOX_OFFSET   243

#define COL_BOX_DIVISOR 3	// No of columns in a box
#define ROW_BOX_DIVISOR 3	// No of rows in a box


// Now a word about the node of the linked list..it is a quadruply linked circular list node 
// that has connections in all four directions.
struct Node
{
	Node * left, * right, * up, * down;	// Usual connections
	Node * colHeader;			// To identify the column
	
	int row, col, value; 		// , size..will use size may be later	
	bool header;				// To identify if it is a column header node

	// Constructor
	Node()	{ left = right = up = down = colHeader = NULL; row = col = value = -1; header = false; }
	
	Node(int r, int c, int v)
	{ left = right = up = down = colHeader = NULL; row = r, col = c, value = v;  header = false; }
};

// Class : Sudoku Matrix
class SudokuMatrix
{
private:
	Node * root;			// Points to the first node in the list of the column headers
	stack<Node> * workingSolution;	// The partial or full solution to current puzzle
	bool SOLVED;			// To check if a working solution has been found

	// Function: Helps in adding a new column node to the list of already created column nodes
	bool AddColumnHelp(Node * newNode, Node * r)
	{
		// Don't allow duplicate nodes..
		if	(r -> right == root and r != newNode)				// Initially we call this function as this: AddColumnHelp(newNode, root)
		{
			// r will be the last node in the list currently
			r -> right -> left = newNode;		// r -> right is currently root
			newNode -> right   = r -> right;	
			r -> right	   = newNode;		// Now point root back to newNode
			newNode -> left    = r;

			return true;
		}
		else if	(newNode == r)
			return false;
		else
			return AddColumnHelp(newNode, r -> left);	// Now when we call again on r->left then we reach the end of the column list and hence we have O(1) insertion
									
	}

	// Function: Helper function in the destructor
	void deleteMatrix()
	{

		Node * deleteNextCol = root -> right; 	// First column to be deleted
		Node * deleteNextRow, * temp;

		// While you don't reach the root again
		while(deleteNextCol != root)
		{
			deleteNextRow = deleteNextCol -> down;
			// While it's not a special column header node
			while(! deleteNextRow -> header)	
			{
				temp = deleteNextRow -> down;
				delete deleteNextRow;
				deleteNextRow = temp;		// Move down one step
			}

			temp = deleteNextCol -> right;
			delete deleteNextCol;
			deleteNextCol = temp;
		}	
	}

	// Function: To insert a new column in the list of already present column headers
	bool AddColumn(Node * newNode)
	{
		if(! newNode -> header)
			return false;	// If it is not a column header node then return false
		else
			return AddColumnHelp(newNode, root);
	}

	// Function : To cover some column	*Very Important*
	void cover(Node * r)
	{
		Node * rowNode, * rightNode, * colNode = r -> colHeader;
	
		colNode -> left -> right = colNode -> right;
		colNode -> right -> left = colNode -> left;
	
		for(rowNode = colNode -> down; rowNode != colNode; rowNode = rowNode -> down)
		{
			for(rightNode = rowNode -> right; rightNode != rowNode; rightNode = rightNode -> right)
			{
				rightNode -> up -> down = rightNode -> down;
				rightNode -> down -> up = rightNode -> up;
			}
		}
	}

	// Function : To uncover a column.Now we go in the opposite direction as we went in the cover function
	void uncover(Node * r)
	{
		Node * rowNode, * leftNode, * colNode = r -> colHeader;
		
		for(rowNode = colNode -> up; rowNode != colNode; rowNode = rowNode -> up)
		{
			for(leftNode = rowNode -> left; leftNode != rowNode; leftNode = leftNode -> left)
			{
				leftNode -> up -> down = leftNode;	
				leftNode -> down -> up = leftNode;
			}
		}

		colNode -> right -> left = colNode;		// Redirected the links back to colNode
		colNode -> left -> right = colNode;
	}

	// Function : To choose the best column from the remaining matrix. We want to choose the column with max no of nodes in it
	Node * chooseNextColumn(int & count)
	{
		Node * currBest = root -> right;
		int tempCnt, bestCnt = -1;

		// Do simple iteration among all columns
		Node * next    = currBest -> down;
		Node * nextCol = currBest;

		while(nextCol != root)
		{
			next = nextCol -> down;
			tempCnt = 0;

			while(next != nextCol)
			{
				if(next == next -> down)
				{
					cout << "Error: Can't choose next Column\n";
				}
				tempCnt++;
				next = next -> down;
			}

			if(tempCnt < bestCnt or bestCnt == -1)
			{
				bestCnt = tempCnt;
				currBest = nextCol;
			}

			nextCol = nextCol -> right;
		}

		if(currBest == root)		// Then it's a problem
		{
			cout << "Error: Couldn't choose the right Column\n";
			system("exit");
		}

		count = bestCnt;
		return currBest;
	}

	/* Function: To solve the problem using exact cover. Algo: 

	   If (the matrix is empty) then
	   		the problem is solved
	   Else
	   		1. Choose a column (deterministically)
	   		2. Choose a row (un-deterministically)
	   		3. Cover the column and every row in other columns where there is a node in this column
	   		4. If solved then output so
	   		5. Else uncover previously covered columns and rows and output no solution
	*/
	int solve(int cnt)
	{
		if(cnt > 4)
			return cnt;

		if(isEmpty())
			return cnt + 1;		// The matrix is empty..the solution has been found
	
		int numCol;
		Node * nextCol = NULL;
		nextCol = chooseNextColumn(numCol);	

		if(numCol < 1)
			return cnt;	// Couldn't find the node

		Node * nextRowInCol = nextCol -> down;
		Node * rowNode;

		cover(nextCol);

		while(nextRowInCol != nextCol)
		{
			workingSolution -> push(*nextRowInCol);


			rowNode = nextRowInCol -> right;

			while(rowNode != nextRowInCol)
			{
				cover(rowNode -> colHeader);
				rowNode = rowNode -> right;
			}

			cnt = solve(cnt);

			if(!cnt)
			{
				workingSolution -> pop();
			}

			rowNode =  nextRowInCol -> right;

			while(rowNode != nextRowInCol)
			{
				uncover(rowNode -> colHeader);
				rowNode = rowNode -> right;
			}

			nextRowInCol = nextRowInCol -> down;
		}

		uncover(nextCol);
		return cnt;			// Couldn't solve this problem
	}

	// Function: To check if the matrix is empty
	bool isEmpty()
	{
		return (root -> right == root);
	}

	// Function : To find a node in the matrix for a given (i, j, val) tuple
	Node * find(Node * gvn)
	{
		Node * rightNode = root -> right, * downNode;

		while(rightNode != root)
		{
			downNode = rightNode -> down;

			while(downNode != rightNode)
			{
				if(downNode -> row == gvn -> row and downNode -> col == gvn -> col and downNode -> value == gvn -> value)
				{
					return downNode;
				}
				downNode = downNode -> down;
			}

			rightNode = rightNode -> right;
		}

		// Couldn't find
		return NULL;
	}

public: 
	// Constructor
	SudokuMatrix()
	{
		root = new Node();
		root -> header = true;		// Yes, it's a column header

		// Root points to itself in all directions
		root -> left = root -> right = root -> up = root -> down = root; 
		workingSolution = new stack<Node>();
		SOLVED = false;
	}

	// Destructor
	~SudokuMatrix()
	{
		deleteMatrix();
		delete root;
		delete workingSolution;
	}

	
	/* Function : To initialize all things
	* 	      It first creates the exact cover matrix and then reads in the initial input
	* 	      From the initial input it creates the Dancing Links structure based on the initial input
	* STEP 1. Construct the matrix A (729 x 324)
	* STEP 2. Use A to build the dancing links structure
	* STEP 3. Read in the initial sudoku grid of 1s and 0s
	* STEP 4. Parse the sudoku grid for (r,c,b,v) and add those the partical solution S (remove from A) (r -> Row, c -> Column, b -> Box, v -> Value) 
	* STEP 5. Apply Algorithm X on the remaining structure until the final solution is found (S * is the final solution)
	* STEP 6. Output the final solution
	*/
	bool initialize()
	{
		Node * matrix[MAX_ROWS][MAX_COLS];

		// Initialize the matrix
		for(int i = 0; i < MAX_ROWS; i++)
		{	for(int j = 0; j < MAX_COLS; j++)	matrix[i][j] = NULL;	}

		// Set appropriate non-null values in the matrix
		int row = 0;
		Node * rowNode, * colNode, * boxNode, * cellNode;

		for(int i = 0; i < MATRIX_SIZE; i++)		// Rows
		{
			for(int j = 0; j < MATRIX_SIZE; j++)	// Columns
			{	
				for(int k = 0; k < MATRIX_SIZE; k++)	// Values
				{
					// Each one of the 729 combinations leads to 4 constraints being satisfied 
					// i.e 4 NULL values will be converted to non-null values 
					// Since this will finally be a very sparse matrix we will convert it to DLX structure later
					row = (i * COL_OFFSET) + (j * MATRIX_SIZE) + k;			// This is the row where we are doing the change

					rowNode  = matrix[row][ROW_OFFSET + (i * MATRIX_SIZE) + k]  = new Node(i, j, k);
					colNode  = matrix[row][COL_OFFSET + (j * MATRIX_SIZE) + k]  = new Node(i, j, k);
					cellNode = matrix[row][CELL_OFFSET + (i * MATRIX_SIZE + j)] = new Node(i, j, k);

					boxNode  = matrix[row][BOX_OFFSET + ((i / ROW_BOX_DIVISOR + j / COL_BOX_DIVISOR * COL_BOX_DIVISOR) * MATRIX_SIZE) + k] = new Node(i, j, k);

					// Now link the above nodes	in this order :   <- rowNode -> colNode -> cellNode -> boxNode -> (Circular)
					rowNode  -> right = colNode;
					rowNode  -> left  = boxNode;
					colNode  -> right = cellNode;
					colNode  -> left  = rowNode;
					cellNode -> right = boxNode;
					cellNode -> left  = colNode;
					boxNode  -> right = rowNode;
					boxNode  -> left  = cellNode;

				//	cout << "i: " << i << " j: " << j << " value: " << k + 1 << endl;
				}
			}
		}

		//cout << "Converting to DLX\n";
		// Now let's convert them to DLX structure
		Node * nextColHeader;
		Node * nextColRow;
		for(int j = 0; j < MAX_COLS; j++)
		{
			nextColHeader = new Node();
			nextColHeader -> header = true;
			nextColHeader -> left = nextColHeader -> right = nextColHeader -> up = nextColHeader -> down = nextColHeader -> colHeader = nextColHeader; 	// Point header to itself

			nextColRow = nextColHeader;
			//int CNT = 0;

			for(int i = 0; i < MAX_ROWS; i++)
			{
				if(matrix[i][j] != NULL)		// Search down rows to add to this column
				{
					matrix[i][j] -> up        = nextColRow;
					nextColRow -> down        = matrix[i][j];
					matrix[i][j] -> down      = nextColHeader;
					nextColHeader -> up       = matrix[i][j];
					matrix[i][j] -> colHeader = nextColHeader;
					nextColRow                = matrix[i][j];
				//	cout << "i: " << i << " j: " << j << " value: " << matrix[i][j] -> value + 1 << endl;
				}
			}
			

			if(nextColHeader -> up == nextColHeader)
			{
				cout << "Error : There are no rows to add to this column: " << (j+1) << endl;
			}
			if(!AddColumn(nextColHeader))
			{
				cout << "Error in adding column to the matrix" << endl;

				// Clean - Up
				for(int i = 0; i < MAX_ROWS; i++)
					for(int j = 0; j < MAX_COLS; j++)
						if(matrix[i][j] != NULL)
							delete matrix[i][j];

				return false;
			}			
		}

		return true;		// Successful intialisation of the DLX structure
	}


	// Function: To actually solve the SUDOKU puzzle
	stack<Node> * SOLVE(int & cnt)
	{
		// Doing initialisation work 
		SOLVED = false;			
		while(!workingSolution -> empty())
			workingSolution -> pop();

		Node * toFind = NULL;
		Node * insertNext = NULL;
		stack<Node*> puzzleNodes;		// List of nodes that were covered as part of puzzle initially
		Node * colNode, * nextRowInCol, * rowNode;

		cout << "Enter the sudoku matrix: \n";
		for(int i = 0; i < MATRIX_SIZE; i++)
		{
			for(int j = 0; j < MATRIX_SIZE; j++)
			{
				int val;
				cin >> val;

				if(val < 0 or val > MATRIX_SIZE)
				{
					cout << "Wrong Sudoku input\n";
					return NULL;					// Wrong input and hence no output
				}

				if(val != 0)
				{
					toFind = new Node(i, j, val-1);
				//	cout << "VAL: " << val << " I: " << i << " J: " << j << endl;
					insertNext = find(toFind);

					if(insertNext == NULL)
					{
						cout << "Error in Sudoku puzzle\n";
						return NULL;
					}


				//	cout << insertNext -> row << "," << insertNext -> col << "," << insertNext -> value << endl;

					colNode = insertNext -> colHeader;
					nextRowInCol = insertNext;
					cover(colNode);

				//	cout << "HERE" << endl;

					rowNode = nextRowInCol -> right;
					while(rowNode != nextRowInCol)
					{
						cover(rowNode -> colHeader);
						rowNode = rowNode -> right;
					}

					puzzleNodes.push(insertNext);
					workingSolution -> push(*insertNext);
					delete toFind;
				}
			}
		}

		cout << "Solving ...\n";

		cnt = solve(cnt);

		if(cnt)
			cout << "Puzzle solved successfully\n";
		else
			cout << "Puzzle not solveable\n";

		// Uncover the nodes that we covered earlier so tha we can solve other puzzles also
		while(!puzzleNodes.empty())
		{
			colNode = (puzzleNodes.top()) -> colHeader;
			nextRowInCol = puzzleNodes.top();

			rowNode = nextRowInCol -> right;

			while(rowNode != nextRowInCol)
			{
				uncover(rowNode -> colHeader);
				rowNode = rowNode -> right;
			}

			uncover(colNode);
			puzzleNodes.pop();
		}

		stack <Node> temp, * ret;
		while(!workingSolution -> empty())
		{
			temp.push(workingSolution->top());
			workingSolution -> pop();
		}
		
		//cout << "I am here\n";
		ret = new stack<Node>();

		while(!temp.empty())
		{
			ret -> push(temp.top());
			temp.pop();
		}

		return ret;

	}

};
	

SudokuMatrix sd;

int solve()
{
	stack<Node> * solution;
	Node next;

	int puzzle[MATRIX_SIZE][MATRIX_SIZE];
	int val;
	cout << "Want to solve SUDOKU puzzle? Press 1 for yes and 0 for no: ";
	cin >> val;

	if(val < 0 or val > 1)
	{
		cout << "You entered wrong input. Going to shut off this now\n";
		return 0;
	}
	else if(val == 0)
	{
		return 0;
	}
	else if(val == 1)
	{
		int cnt = 0;

		solution = sd.SOLVE(cnt);
		if(solution != NULL)
		{
			if(cnt < 5)
				cout << "No of solutions: " << cnt << endl;
			else
				cout << "No of solutions: " << cnt << " or more.\n";

			while(!solution -> empty())
			{
				next = solution -> top();
				puzzle[next.row][next.col] = next.value + 1;
				solution -> pop();
			}

			cout << "SOLUTION: \n";
			for(int i = 0; i < MATRIX_SIZE; i++)
			{
				for(int j = 0; j < MATRIX_SIZE; j++)
					cout << puzzle[i][j] << " ";
				cout << endl;
			}
			cout << endl;
		}
		else
			cout << "SOLUTION Couldn't be found\n";

		delete solution;
		solution = NULL;

		return 1;
	}

	if(solution != NULL)
	{
		while(!solution -> empty())
			solution -> pop();

		delete solution;
	}
}

int main()
{

	if(!sd.initialize())
	{
		cout << "Couldn't initialize\n";
		return 1;
	}


	//cout << "MATRIX HAS BEEN INTIALIZED\n";
	int val;

	while(1)
	{
		val = solve();
		if(!val) break;
	}
	
	return 0;

}
