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

#include <array>
#include <tuple>
#include <string>
#include "objects.h"

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
		struct menu_item {
			std::string name;
			std::string description;
			char hotkey;
			char result;
		};
		const std::array<menu_item,4> menu={{
			{"&New game","Generates a new grid",'N','n'},
			{"&Clue","Displays a clue",'C','c'},
			{"&Solve","Try to solve the grid",'S','s'},
			{"&Quit","Quit the game",'Q','q'}
		}};	//!< Items of menu
		size_t xmin;	//!< First column where the grid is displayed on screen
		size_t xmax;	//!< Number of columns of the screen
		size_t ymax;	//!< Number of lines of the screen
		size_t xspace;	//!< Number of white spaces between the border of the cell and the element at its center
		Grid solution;	//!< Solution of the grid
		Grid maingrid;	//!< Main grid displayed on screen
		size_t menu_spacing;	//!< Number of spaces between too items in the menu
		bool menu_mode;	//!< Tell if the user is in the menu
		size_t si;	//!< Selected row
		size_t sj;	//!< Selected column

		/**
		 * \brief Draw the structure of the grid on screen
		 *
		 * This function draws the full structure of the grid given as parameter on the screen.
		 * \param grid Grid whose skeleton will be driven on the screen
		 */
		void draw_structure(const Grid &grid);

		/**
		 * \brief Draw one element of the grid
		 *
		 * This function draws one element of the grid on the screen. If the value of the element is 0, the element already at the position is deleted.
		 * \param grid Grid from which the element is taken
		 * \param row Row coordinate of the element, starting with 0
		 * \param column Column coordinate of the element, starting with 0
		 */
		void draw_element(const Grid &grid,size_t row,size_t column);

		/**
		 * \brief Display a string and highlight hotkey
		 *
		 * This function displays the string given as argument at the current position but highlights the character used as a hotkey. In the string, it is represented by prefixing it with '&'.
		 * \param text String to display
		 */
		void display_string(std::string text);

		/**
		 * \brief Display menu line
		 *
		 * This function displays the menu line at the bottom of the screen. If one of the element of the menu is selected, it is highlighted. A short description of the menu element is provided on the last line.
		 * The function uses the CursesGui::menu constant to get the items of the menu.
		 * \param selected Index of selected element in the menu
		 */
		void display_menu_line(int selected);
};

#endif   /* ----- #ifndef GUI_CURSES_INC  ----- */
