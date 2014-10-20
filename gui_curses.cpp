/*
 * =====================================================================================
 *
 *       Filename:  gui_curses.cpp
 *
 *    Description:  Implementation of Ncurses graphical user interface for Sudoku
 *
 *        Version:  1.0
 *        Created:  14/10/2014 22:08:33
 *       Revision:  none
 *       Compiler:  g++
 *
 *         Author:  François Hissel
 *   Organization:  
 *
 * =====================================================================================
 */

#include <ncurses.h>
#include <string>
#include <array>
#include <utility>
#include "gui_curses.h"

using namespace std;

/**
 * \brief Draw the structure of the grid on screen
 *
 * This function draws the full structure of the grid given as parameter on the screen.
 * \param grid Grid whose skeleton will be driven on the screen
 */
void CursesGui::draw_structure(const Grid &grid) {
	size_t sx;
	xspace=(grid.dim2()<=4)?1:0;
	xmin=xmax-grid.dim2()*2*(1+xspace)-1;
	mvaddch(0,0,ACS_ULCORNER);
	sx=2*xspace+1;
	for (size_t j=0;j<grid.dim2();++j) {
		for (size_t k=0;k<sx;++k) addch(ACS_HLINE);
		if (j==grid.dim2()-1) addch(ACS_URCORNER); else addch(ACS_TTEE);
	}
	for (size_t i=0;i<grid.dim2();++i) {
		for (size_t j=0;j<sx;++j) 
			for (size_t k=0;k<=sx;++k) mvaddch(i*sx+j,xmin+k*sx,ACS_VLINE);
		if (i==grid.dim2()-1) {
			mvaddch((i+1)*sx,xmin,ACS_LLCORNER);
			for (size_t j=0;j<grid.dim2();++j) {
				for (size_t k=0;k<sx;++k) addch(ACS_HLINE);
				if (j==grid.dim2()-1) addch(ACS_LRCORNER); else addch(ACS_BTEE);
			}
		} else {
			mvaddch((i+1)*sx,xmin,ACS_LTEE);
			for (size_t j=0;j<grid.dim2();++j) {
				for (size_t k=0;k<sx;++k) addch(ACS_HLINE);
				if (j==grid.dim2()-1) addch(ACS_RTEE); else addch(ACS_PLUS);
			}
		}
	}
}

/**
 * \brief Draw one element of the grid
 *
 * This function draws one element of the grid on the screen. If the value of the element is 0, the element already at the position is deleted.
 * \param grid Grid from which the element is taken
 * \param row Row coordinate of the element, starting with 0
 * \param column Column coordinate of the element, starting with 0
 */
void CursesGui::draw_element(const Grid &grid,size_t row,size_t column) {
	Cell *cell=grid.cell(row,column);
	char elem;
	if (cell->value()==0) elem=' ';
	else if (cell->value()<=9) elem='0'+cell->value();
	else if (cell->value()<=35) elem='A'-10+cell->value();
	else elem='*';
	if (row==si && column=sj) attron(COLOR_PAIR(1));
	for (size_t i=0;i<xspace*2+1;++i) {
		move(row*(xspace*2+2)+1+i,xmin+(column*(xpace*2+2)+1));
		for (size_t j=0;j<xspace*2+1;++j) addch(' ');
	}
	mvaddch(row*(xspace*2+2)+xspace+1,xmin+(column*(xspace*2+2))+xspace+1,elem);
	if (row==si && column=sj) attroff(COLOR_PAIR(1));
}

/**
 * \brief Display a string and highlight hotkey
 *
 * This function displays the string given as argument at the current position but highlights the character used as a hotkey. In the string, it is represented by prefixing it with '&'.
 * \param text String to display
 */
void CursesGui::display_string(string text) {
	size_t i=0;
	size_t j;
	while (i<text.length()) {
		j=i;
		while (j<text.length() && text[j]!='&') ++j;
		printw(text.substr(i,j-i).c_str());
		if (text[j]=='&' && j<text.length()-1) {
			if (text[j+1]=='&') addch(text[j]); else {
				attron(A_STANDOUT);
				addch(text[j+1]);
				attroff(A_STANDOUT);
			}
			i+=2;
		}
	}
}

/**
 * \brief Display menu line
 */
void CursesGui::display_menu_line(int selected) {
	move(ymax-3,0);
	hline(ACS_HLINE,0xffff);
	move(ymax-2,0);
	for (size_t i=0;i<menu.size();++i) {
		if (i==selected) attron(COLOR_PAIR(1));
		display_string(get<0>(menu[i]));
		if (i==selected) attroff(COLOR_PAIR(1));
		for (size_t j=0;j<menu_spacing;++j) addch(' ');
	}
	move(ymax-1,0);
	clrtoeol();
}

void CursesGui::run() {
	// Init screen
	initscr();
	raw();
	keypad(stdscr,TRUE);
	noecho();
	start_color();
	init_pair(1,COLOR_YELLOW,COLOR_BLUE);
	// Init some variables
	getmaxyx(stdscr,ymax,xmax);
	size_t s=0;
	for (auto entry:menu) {
		s+=get<0>(entry).length();
		if (get<2>(entry)!=0) --s;
	}
	if (s>=xmax) menu_spacing=2; else menu_spacing=(xmax-s)/(menu.size()-1);
	// Generate a first grid
	solution();
	maingrid=Grid::generate(3,10,&solution);
	draw_structure(maingrid);
	si=0;sj=0;
	for (size_t i=0;i<maingrid.dim2();++i) for (size_t j=0;j<maingrid.dim2();++j) if (maingrid(i,j)->fixed) draw_element(maingrid,i,j);
	// Main loop
	size_t selected=0;
	bool menu_mode=false;
	bool quit=false;
	int ch=0;
	while (!quit) {
		// Prompt for action
		ch=getch();
		switch (ch) {
			case KEY_RIGHT:
				if (menu_mode) selected=(selected+1)%menu.size(); else {
					size_t oj=sj;
					sj=(sj+1)%maingrid.dim2();
					display_element(maingrid,si,oj);
				}
				break;
			case KEY_LEFT:
				if (menu_mode) selected=(selected==0)?(menu.size()-1):(selected-1); else {
					size_t oj=sj;
					sj=(sj==0)?(maingrid.dim2()-1):(sj-1);
					display_element(maingrid,si,oj);
				}
				break;
			case KEY_UP:
				if (!menu_mode) {
					size_t oi=si;
					si=(si==0)?(maingrid.dim2()-1):(si-1);
					display_element(maingrid,oi,sj);
				}
				break;
			case KEY_DOWN:
				if (!menu_mode) {
					size_t oi=si;
					si=(si+1)%maingrid.dim2();
					display_element(maingrid,oi,sj);
				}
				break;
			case '\n':
				if (menu_mode) {
				}
				break;
			default:
				size_t k=0;
				while (!ok && k<menu.size()) {
					if (get<2>(menu[k])==ch) {
						ok=true;
						ch=get<3>(menu[k]);
					}
					++k;
				}
		}
	}
	display_menu_line(-1);
	// End 
	endwin();
}
