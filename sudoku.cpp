/*
 * =====================================================================================
 *
 *       Filename:  sudoku.cpp
 *
 *    Description:  Ncurses Sudoku game
 *
 *        Version:  1.0
 *        Created:  24/09/2014 21:32:54
 *       Revision:  none
 *       Compiler:  g++
 *
 *         Author:  François Hissel (), francois.hissel@gmail.com
 *   Organization:  
 *
 * =====================================================================================
 */

#include <iostream>
#include <fstream>
#include "objects.h"
#include "gui_curses.h"

using namespace std;

/**
 * \brief Main program
 *
 * The main program starts the game and the main execution loop.
 * \param argc Number of arguments in command line, including the name of the program
 * \param argv Array of arguments in command line, the first one being the name of the program
 */
int main(int argc,char **argv) {
/* 	Grid *grid;
 * 	if (argc>1) {
 * 		ifstream ifs(argv[1]);
 * 		grid=new Grid(ifs);
 * 	} else grid=new Grid(cin);
 * 	grid->solve(Grid::FIND_ALL);
 */


/* 	Grid *grid=new Grid(3);
 * 	grid->solve(Grid::FIND_ANY);
 * 	delete grid;
 */


/* 	Grid grid=Grid::generate(3,0);
 * 	cout << "Grille :\n" << grid;
 * 	cout << "\nSolution :\n";
 * 	grid.solve(Grid::FIND_ONE);
 */

	CursesGui gui;
	gui.run();
	return 0;
}
