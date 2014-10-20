/*
 * =====================================================================================
 *
 *       Filename:  objects.h
 *
 *    Description:  Definition of the underlying objects
 *
 *        Version:  1.0
 *        Created:  24/09/2014 21:37:56
 *       Revision:  none
 *       Compiler:  g++
 *
 *         Author:  François Hissel (), francois.hissel@gmail.com
 *   Organization:  
 *
 * =====================================================================================
 */

#ifndef  OBJECTS_INC
#define  OBJECTS_INC

#include <iostream>
#include <string>
#include <functional>

typedef size_t elem_t;	//!< Basic type of elements of the grid

/**
 * \brief Generic exception raised by a Sudoku game
 *
 * This class represents a generic error raised during a game.
 */
class SudokuException:public std::exception {
	public:
		/**
		 * \brief Set of error codes
		 *
		 * The set gathers all error codes that the exception may hold.
		 */
		enum Code {
			FORMAT_ERROR	//!< Invalid data format
			} code;	//!< Code of the error
		std::string message;	//!< Message describing the error
		SudokuException(Code pcode):code(pcode) {}	//!< Standard constructor, without any error message
		SudokuException(Code pcode,std::string pmessage):code(pcode),message(pmessage) {}	//!< Standard constructor, with an error message
		~SudokuException() throw() {}	//!< Standard destructor
		const char* what() const throw();	//!< Returns the nature of the error
};

/**
 * \brief Alternative for placement of an element in a Grid
 *
 * This structure holds an alternative for the placement of a particular element (number) in a Sudoku Grid. The algorithm for the resolution of the grid cycles through all alternatives and tries to place the element until there is no alternative left.
 */
class Alternative {
	public:
		/**
		 * \brief Standard constructor for the alternative
		 *
		 * The standard constructor allocates memory for the object and initializes its members.
		 * \param ptype Type of set referred by the alternative
		 * \param pset Index of set referred by the alternative
		 * \param pvalue Value of the alternative
		 */
		Alternative(size_t ptype,size_t pset,elem_t pvalue):type(ptype),set(pset),value(pvalue) {}

		size_t type;	//!< Type of set this alternative refers to, 0 for a row, 1 for a column, 2 for an inner square
		size_t set;	//!< Index of set this alternative refers to, first is 0
		elem_t value;	//!< Value to be inserted in the grid
};

/**
 * \brief Cell of a Sudoku grid
 *
 * The class holds a unique cell of a Sudoku grid. It contains some data used by the resolution algorithm.
 */
class Cell {
	public:
		elem_t value;	//!< Value in the cell, 0 if unknown
		bool *possible;	//!< Array of possible values for the cell. Whether the value i is still possible is told by the boolean value located at index i-1 of this array
		size_t npossible;	//!< Number of possibilities still left
		bool fixed;	//!< Variable reserved for GUIs telling if the value in the cell is fixed (true) or user-chosen (false)

		friend class Grid;

		/**
		 * \brief Standard constructor
		 *
		 * The standard constructor allocates memory for the object and initializes its members.
		 * \param pvalue Value in the cell
		 * \param pnpossible Number of possible values for the element in the cell (square dimension of the grid)
		 */
		Cell(elem_t pvalue,size_t pnpossible);

		/**
		 * \brief Copy constructor
		 *
		 * The copy-constructor allocates memory for the object and creates it by deep-copying all the members of the source cell.
		 * \param source Source cell
		 */
		Cell(const Cell& source);

		/**
		 * \brief Standard destructor
		 *
		 * The standard destructor releases the memory taken by the object and all its members.
		 */
		~Cell();
	private:
		size_t _dim2;	//!< Square dimension of the underlying grid
};

/**
 * \brief Sudoku grid
 *
 * The class holds a full Sudoku grid, together will all data needed by the resolution algorithm.
 */
class Grid {
	public:
		friend class Cell;

		/**
		 * \brief Structure for coordinates representation
		 *
		 * This structure holds a coordinate, in the format (row,column).
		 */
		struct XYCoordinates {
			XYCoordinates() {}	//!< Standard constructor without initialization of members
			XYCoordinates(size_t prow,size_t pcolumn):row(prow),column(pcolumn) {}	//!< Constructor with initialization of members
			size_t row;	//!< First coordinate, row starting with 0
			size_t column;	//!< Second coordinate, column starting with 0
		};

		/**
		 * \brief Structure for coordinates representation in the Sudoku logic
		 *
		 * This structure holds a coordinate, in the format (set,index).
		 */
		struct SuCoordinates {
			SuCoordinates(size_t ptype,size_t pset,size_t pindex):type(ptype),set(pset),index(pindex) {}
			size_t type;	//!< Type of the set, 0 for row, 1 for column, 2 for inner square
			size_t set;	//!< First coordinate, index of the set (starting with 0)
			size_t index;	//!< Second coordinate, index of the element in the set (starting with 0)
		};

		/**
		 * \brief Option for the solving algorithm when there are more than one solution
		 */
		enum SolveType {
			FIND_ONE,	//!< Find first solution matching the grid and returns
			FIND_ANY,	//!< Find any solution randomly
			FIND_UNIQUE,	//!< Find any solution randomly but make sure it is unique
			FIND_ALL	//!< Find all solutions matching the grid and list them
		};

		/**
		 * \brief Warp coordinates from (row,column) to (set,index)
		 *
		 * This function calculates the coordinates in the (set,index) base.
		 * \param type Type of set expected, see also SuCoordinates::type
		 * \return Coordinates in the Sudoku base
		 */
		SuCoordinates warp(size_t type,const XYCoordinates& coords) const;

		/**
		 * \brief Warp coordinates from (set,index) to (row,column)
		 *
		 * This function calculates the coordinates in the (row,column) base.
		 * \return Coordinates in the grid base
		 */
		XYCoordinates warp(const SuCoordinates& coords) const;

		/**
		 * \brief Standard constructor
		 *
		 * The standard constructor allocates memory for the grid and initializes its members.
		 * \param pdim Dimension of the grid
		 */
		Grid(size_t pdim=0);

		/**
		 * \brief Constructor from a stream
		 *
		 * The constructor creates a new grid by reading it from an input stream. See Grid::read_from_stream for more details about the format of the input stream.
		 * \param pin Input stream
		 */
		Grid(std::istream &pin);

		/**
		 * \brief Copy constructor
		 *
		 * The copy constructor creates a new grid by deep-copying all members of the source grid.
		 * \param source Source grid
		 */
		Grid(const Grid &source);

		/**
		 * \brief Standard destructor
		 *
		 * The standard destructor releases the memory held by the object.
		 */
		~Grid();

		/**
		 * \brief Release memory used by the members of the object
		 *
		 * This method releases the memory used by the members of the object. It can be used to destroy the structure.
		 */
		void free_all();

		/**
		 * \brief Clear all values from the grid
		 *
		 * The method clears all value from the grid and updates the underlying objects. After its execution, the new grid is the same as a newly-created one.
		 */
		void clear();

		/**
		 * \brief Accessor of a cell
		 *
		 * This procedure returns the reference to a cell in the grid. The bounds are not checked for increased efficiency.
		 * \param row Index of the row (first row has index 0)
		 * \param column Index of the column (first column has index 0)
		 * \return Pointer to the cell at the given index
		 */
		Cell *operator()(size_t row,size_t column) const {return _cells[row*_dim2+column];}

		/**
		 * \brief Accessor of a cell through set number
		 *
		 * This method returns a pointer to a cell in the grid. The cell is located by its set type, its set index and its index in the set.
		 * \param ptype Type of the set, 0 for a row set, 1 for a column set, 2 for an inner square set
		 * \param pset Index of the set. Rows are numbered from 0 from top to bottom. Columns are numbered from 0 from left to right. Inner squares are numbered from 0, from left to right then top to bottom.
		 * \param pindex Index of the cell in the set. First cell has index 0. Cells are numbered the same way as sets.
		 * \return Pointer to the cell at the given coordinates
		 */
		Cell *operator()(size_t ptype,size_t pset,size_t pindex) const;

		/**
		 * \brief Copy an other grid into the current one
		 *
		 * The overloaded operator copies all fields from a source grid into the current one. The copy is in depth, which means that all the arrays are copied into the object.
		 * \param source Source grid
		 */
		void operator=(const Grid &source);

		/**
		 * \brief Read a grid from a stream
		 *
		 * The method reads a grid from a stream. Each line of the stream holds one row of the grid. Elements in columns are separated by whitespaces. The dimension of the grid is detected by the number of elements in the first line.
		 * If the object already holds a grid when the method is called, the current grid is deleted.
		 * \param in Input stream
		 */
		void read_from_stream(std::istream &in);

		/**
		 * \brief Write a grid to a stream
		 *
		 * The method writes the grid to an output stream. Each row of the grid is written on a line in the output stream. Elements in columns are separated by tabulations.
		 * \param out Output stream, default is std::cout
		 */
		void write_to_stream(std::ostream &out) const;
		void write_to_cout() const {write_to_stream(std::cout);std::cout << std::endl;}

		/**
		 * \brief Accessor to the dimension of the grid
		 *
		 * This method returns the dimension of the grid, which is the square root of the number of rows (or columns)
		 * \return Dimension of the grid
		 */
		size_t dim() const {return _dim;}

		/**
		 * \brief Accessor to the square dimension of the grid
		 *
		 * This method returns the square dimension of the grid, which is the number of rows (or columns)
		 * \return Square dimension of the grid
		 */
		size_t dim2() const {return _dim2;}

		/**
		 * \brief Set the value of a cell
		 *
		 * This function sets the value of the selected cell and updates the underlying structures used by the resolution algorithm accordingly.
		 * This function should only be used when the cell does not already have a value.
		 * \param prow Row index of the cell to be set
		 * \param pcolumn Column index of the cell to be set
		 * \param pvalue New value of the cell
		 * \param pfixed Tell if the value is fixed or chosen by the user, default is false (not fixed)
		 */
		void set_value(size_t prow,size_t pcolumn,elem_t pvalue,bool pfixed=false);

		/**
		 * \brief Construct an alternative from an index
		 *
		 * This method constructs the alternative based on its index in the _alternatives table.
		 * \param index Index of the alternative
		 * \return Corresponding alternative structure
		 */
		Alternative ind_alternative(size_t index) const {
			return Alternative(index/(_dim2*_dim2),(index%(_dim2*_dim2))/_dim2,index%_dim2+1);
		}

		/**
		 * \brief Get the level of an alternative
		 *
		 * This method gets the level of an alternative at the given position. The level is the number of choices for the placement of the value held by the alternative in the grid.
		 * \param ptype Type of set referred by the alternative
		 * \param pset Index of set referred by the alternative
		 * \param pvalue Value of the alternative
		 * \return Level of the alternative, which is the number of choices for the placement of the value in the set
		 */
		size_t get_alternative(size_t ptype,size_t pset,elem_t pvalue) const {
			return _alternatives[pvalue-1+pset*_dim2+ptype*_dim2*_dim2];
		}

		/**
		 * \brief Set the level of an alternative
		 *
		 * This method sets the level of an alternative at the given position. The level is the number of choices for the placement of the value held by the alternative in the grid.
		 * \param ptype Type of set referred by the alternative
		 * \param pset Index of set referred by the alternative
		 * \param pvalue Value of the alternative
		 * \param plevel New level of the alternative, which is the number of choices for the placement of the value in the set
		 */
		void set_alternative(size_t ptype,size_t pset,elem_t pvalue,size_t plevel) {
			_alternatives[pvalue-1+pset*_dim2+ptype*_dim2*_dim2]=plevel;
		}

		/**
		 * \brief Solve the grid
		 *
		 * This method solves the grid, that is it finds all the missing values in it. According to the value of type, it either chooses one solution or lists all solutions.
		 * For each solution found, the callback function is executed on it. The default behaviour is to print the grid on standard output.
		 * The source grid is not changed. The return value is updated to tell if a solution has been found, and how many in this case.
		 * \param type Tells if the algorithm must find any solution or all solutions, default value is FIND_ONE which means that the algorithm only tries to find one solution and reports
		 * \param callback Callback function applied on each solution grid when the type is FIND_ALL. Default function is Grid::write_to_cout which prints the grid on standard output
		 * \return Number of solutions found with this type of solving
		 */
		size_t solve(SolveType type=FIND_ONE,std::function<void(const Grid&)> callback=&Grid::write_to_cout) const;

		/**
		 * \brief Fill the grid
		 *
		 * This method is a wrapper to the Grid::solve method. It tries to fill the grid by looking at any solution and updates the grid to this solution if it is found.
		 * \return True if the grid could be filled, false otherwise
		 */
		bool fill();

		/**
		 * \brief Generate a game grid
		 *
		 * This static method creates a Sudoku grid for a game. For the highest level of difficulty, a minimal number of elements are placed so that the grid only has one solution.
		 * For lower levels of difficulty, new elements are added randomly.
		 * \param dimension Dimension of the new grid (number of cells on one row of an inner square)
		 * \param difficulty Level of difficulty, between 0 (hardest) and (Grid::_dim2-Grid::_dim)*Grid::_dim2 (easiest). The minimum number of elements provided for a generated grid is Grid::_dim2*Grid::_dim+difficulty.
		 * \param solution If the pointer is not null, it must point to an allocated Grid, and the solution of the game is stored there.
		 * \return New game grid
		 */
		static Grid generate(size_t dimension,size_t difficulty,Grid *solution=0);

	private:
		size_t _dim;	//!< Dimension of the grid (number of rows, which is the same as the number of columns)
		size_t _dim2;	//!< Square dimension of the grid, stored for quicker access
		Cell **_cells;	//!< Array of cells in the grid. Cells of the grid are numbered row by row from top to bottom, and in each row column by column from left to right. The top-left cell has index 0.
		size_t _filled;	//!< Number of values already set
		size_t *_alternatives;	//!< Array containing the levels of the alternatives (the number of choices for the placement of a value)

		static Grid _saved;	//!< Grid used to save the solution of the Grid::solve algorithm

		/**
		 * \brief Save the grid in a static variable
		 *
		 * This method is intended to be used as a callback function for the Grid::solve method. It simply saves the grid given as argument in a static variable, in order to make it accessible from other parts of the program.
		 * The method Grid::fill uses this callback function.
		 * \param source Source grid to save in the static variable
		 */
		void save() const;
};


/**
 * \brief Reads a full Sudoku grid from an input stream
 *
 * This function reads a full Sudoku grid from an input stream. The grid is constructed by the method and it is left to the programmer to release it when it is not needed any longer.
 * \param in Input stream where the grid must be read
 * \param grid Pointer to the newly-created grid
 * \return Reference to the input stream in order to allow chaining on input
 */
std::istream& operator>>(std::istream &in,Grid *grid);

/**
 * \brief Writes the grid to an output stream
 *
 * This function writes the full Sudoku grid to an output stream. 
 * \param out Output stream where the grid must be written
 * \param grid Pointer to the grid
 * \return Reference to the output stream in order to allow chaining on output
 */
std::ostream& operator<<(std::ostream &out,const Grid &grid);


#endif   /* ----- #ifndef OBJECTS_INC  ----- */
