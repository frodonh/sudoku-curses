/*
 * =====================================================================================
 *
 *       Filename:  gui_curses.h
 *
 *    Description:  Definition of Ncurses user interface for Sudoku
 *
 *        Version:  1.0
 *        Created:  14/10/2014 21:51:29
 *       Revision:  none
 *       Compiler:  g++
 *
 *         Author:  François Hissel
 *   Organization:  
 *
 * =====================================================================================
 */

#ifndef  GUI_CURSES_INC
#define  GUI_CURSES_INC

/**
 * \brief Class implementing the NCurses Gui
 *
 * This class implements the NCurses Gui for Sudoku.
 */
class CursesGui {
	public:
		/**
		 * \brief Run the main game loop
		 *
		 * This function starts the game, creates a grid and waits for the user input.
		 */
		void run();

	private:
		const array<tuple<string,string,char,char>,4> menu=
			{{"&New game","Generates a new grid",'N','n'},
				{"&Clue","Displays a clue",'C','c'},
				{"&Solve","Try to solve the grid",'S','s'},
				{"&Quit","Quit the game",'Q','q'}};	//!< Items of menu
		size_t xmin;	//!< First column where the grid is displayed on screen
		size_t xmax;	//!< Number of columns of the screen
		size_t ymax;	//!< Number of lines of the screen
		size_t xspace;	//!< Number of white spaces between the border of the cell and the element at its center
		Grid solution;	//!< Solution of the grid
		size_t menu_spacing;	//!< Number of spaces between too items in the menu
		bool menu_mode;	//!< Tell if the user is in the menu
		size_t si;	//!< Selected row
		size_t sj;	//!< Selected column
		Grid solution;
		Grid maingrid;
};

#endif   /* ----- #ifndef GUI_CURSES_INC  ----- */
