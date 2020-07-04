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
#include <ctype.h>
#include <wchar.h>
#include "gui_curses.h"

using namespace std;

void CursesGui::draw_structure(const Grid &grid) {
	move(0,0);
	clrtobot();
	size_t sx;
	xspace=(grid.dim()<=3)?1:0;
	xmin=(xmax-grid.dim2()*2*(1+xspace)-1)/2;
	mvaddwstr(0,xmin,L"┏");
	sx=2*xspace+1;
	for (size_t j=0;j<grid.dim2();++j) {
		for (size_t k=0;k<sx;++k) addwstr(L"━");
		if (j==grid.dim2()-1) addwstr(L"┓"); else {
			if ((j+1) % grid.dim()==0) addwstr(L"┳"); else addwstr(L"┯");
		}
	}
	for (size_t i=0;i<grid.dim2();++i) {
		for (size_t j=1;j<=sx;++j) 
			for (size_t k=0;k<=grid.dim2();++k) {
				if (k % grid.dim()==0) mvaddwstr(i*(sx+1)+j,xmin+k*(sx+1),L"┃");
				else mvaddwstr(i*(sx+1)+j,xmin+k*(sx+1),L"│");
			}
		if (i==grid.dim2()-1) {
			mvaddwstr((i+1)*(sx+1),xmin,L"┗");
			for (size_t j=0;j<grid.dim2();++j) {
				for (size_t k=0;k<sx;++k) addwstr(L"━");
				if (j==grid.dim2()-1) addwstr(L"┛"); else {
					if ((j+1) % grid.dim()==0) addwstr(L"┻"); else addwstr(L"┷");
				}
			}
		} else {
			if ((i+1) % grid.dim()==0) {
				mvaddwstr((i+1)*(sx+1),xmin,L"┣"); 
				for (size_t j=0;j<grid.dim2();++j) {
					for (size_t k=0;k<sx;++k) addwstr(L"━");
					if (j==grid.dim2()-1) addwstr(L"┫"); else {
						if ((j+1) % grid.dim()==0) addwstr(L"╋"); else addwstr(L"┿");
					}
				}
			} else {
				mvaddwstr((i+1)*(sx+1),xmin,L"┠");
				for (size_t j=0;j<grid.dim2();++j) {
					for (size_t k=0;k<sx;++k) addwstr(L"─");
					if (j==grid.dim2()-1) addwstr(L"┨"); else {
						if ((j+1) % grid.dim()==0) addwstr(L"╂"); else addwstr(L"┼");
					}
				}
			}
		}
	}
}

void CursesGui::draw_element(const Grid &grid,size_t row,size_t column) {
	Cell *cell=grid(row,column);
	char elem;
	if (cell->value==0) elem=' ';
	else if (cell->value<=9) elem='0'+cell->value;
	else if (cell->value<=35) elem='A'-10+cell->value;
	else elem='*';
	int attrs=0;
	if (row==si && column==sj && !menu_mode) {
		if (cell->fixed) attrs=COLOR_PAIR(3); else attrs=COLOR_PAIR(1);
	} else {
		if (cell->fixed) attrs=COLOR_PAIR(2); else attrs=0;
	}
	if (attrs!=0) attron(attrs);
	for (size_t i=0;i<xspace*2+1;++i) {
		move(row*(xspace*2+2)+1+i,xmin+(column*(xspace*2+2)+1));
		for (size_t j=0;j<xspace*2+1;++j) addch(' ');
	}
	mvaddch(row*(xspace*2+2)+xspace+1,xmin+(column*(xspace*2+2))+xspace+1,elem);
	if (attrs!=0) attroff(attrs);
}

void CursesGui::display_string(string text) {
	size_t i=0;
	size_t j;
	while (i<text.length()) {
		j=i;
		while (j<text.length() && text[j]!='&') ++j;
		printw(text.substr(i,j-i).c_str());
		if (text[j]=='&') {
			if (j==text.length()-1 || text[j+1]=='&') addch(text[j]); else {
				attron(A_BOLD);
				addch(text[j+1]);
				attroff(A_BOLD);
			}
			j+=2;
		}
		i=j;
	}
}

void CursesGui::display_menu_line(int selected) {
	move(ymax-3,0);
	hline(ACS_HLINE,0xffff);
	move(ymax-2,0);
	for (size_t i=0;i<menu.size();++i) {
		if ((int)i==selected) attron(COLOR_PAIR(1));
		display_string(menu[i].name);
		if ((int)i==selected) attroff(COLOR_PAIR(1));
		for (size_t j=0;j<menu_spacing;++j) addch(' ');
	}
	if (selected>=0) {
		move(ymax-1,0);
		clrtoeol();
		printw(menu[selected].description.c_str());
	}
	refresh();
}

void CursesGui::run() {
	// Init screen
	setlocale(LC_ALL,"");
	initscr();
	raw();
	keypad(stdscr,TRUE);
	noecho();
	set_escdelay(100);
	curs_set(0);
	start_color();
	init_pair(1,COLOR_YELLOW,COLOR_BLUE);
	init_pair(2,COLOR_RED,COLOR_BLACK);
	init_pair(3,COLOR_RED,COLOR_BLUE);
	// Init some variables
	getmaxyx(stdscr,ymax,xmax);
	size_t s=0;
	for (auto entry:menu) {
		s+=entry.name.length();
		if (entry.hotkey!=0) --s;
	}
	if (s>=xmax) menu_spacing=2; else menu_spacing=(xmax-s)/(menu.size()-1);
	// Generate a first grid
	maingrid=Grid::generate(3,10,&solution);
	draw_structure(maingrid);
	si=0;sj=0;
	for (size_t i=0;i<maingrid.dim2();++i) for (size_t j=0;j<maingrid.dim2();++j) if (maingrid(i,j)->fixed) draw_element(maingrid,i,j);
	// Main loop
	int selected=-1;
	bool menu_mode=false;
	bool quit=false;
	bool ok,found;
	int ch=0;
	int chh;
	size_t min;
	size_t savi,savj;
	while (!quit) {
		display_menu_line(selected);
		// Prompt for action
		ch=getch();
		switch (ch) {
			case 'l':case KEY_RIGHT:
				if (menu_mode) {
					selected=(selected+1)%menu.size(); 
					display_menu_line(selected);
				} else {
					size_t oj=sj;
					sj=(sj+1)%maingrid.dim2();
					draw_element(maingrid,si,oj);
					draw_element(maingrid,si,sj);
				}
				break;
			case 'h':case KEY_LEFT:
				if (menu_mode) {
					selected=(selected==0)?(menu.size()-1):(selected-1);
					display_menu_line(selected);
				} else {
					size_t oj=sj;
					sj=(sj==0)?(maingrid.dim2()-1):(sj-1);
					draw_element(maingrid,si,oj);
					draw_element(maingrid,si,sj);
				}
				break;
			case 'k':case KEY_UP:
				if (!menu_mode) {
					size_t oi=si;
					si=(si==0)?(maingrid.dim2()-1):(si-1);
					draw_element(maingrid,oi,sj);
					draw_element(maingrid,si,sj);
				}
				break;
			case 'j':case KEY_DOWN:
				if (!menu_mode) {
					size_t oi=si;
					si=(si+1)%maingrid.dim2();
					draw_element(maingrid,oi,sj);
					draw_element(maingrid,si,sj);
				}
				break;
			case '\n':
				if (menu_mode) {
					if (selected>=0) ch=menu[selected].result;
				}
				break;
			case 27:
				nodelay(stdscr,true);
				chh=getch();
				if (chh==-1) {
					menu_mode=!menu_mode;
					selected=menu_mode?0:-1;
				}
				nodelay(stdscr,false);
				break;
			default:
				if (menu_mode) {
					size_t k=0;
					ok=false;
					while (!ok && k<menu.size()) {
						if (menu[k].hotkey==ch) {
							ok=true;
							ch=menu[k].result;
						}
						++k;
					}
				} else {
					ch=toupper(ch);
					if (ch==KEY_DC || (ch>='0' && ch<='9') || (maingrid.dim()==4 && ch>='A' && ch<='F')) {
						Cell *cell=maingrid(si,sj);
						if (!cell->fixed) {
							if (ch==KEY_DC) maingrid(si,sj)->value=0;
							else if (ch>='0' && ch<='9') maingrid(si,sj)->value=ch-'0';
							else maingrid(si,sj)->value=ch-'A'+10;
							draw_element(maingrid,si,sj);
						}
					}
				}
		}
		// Process menu action
		switch (ch) {
			case 'q':
				quit=true;
				break;
			case 's':
				if (solution.dim()==0) {
					solution=Grid(maingrid.dim());
					for (size_t i=0;i<maingrid.dim2();++i) for (size_t j=0;j<maingrid.dim2();++j) solution.set_value(i,j,maingrid(i,j)->value);
					found=solution.fill();
				} else found=true;
				if (found) {
					maingrid=solution;
					for (size_t i=0;i<maingrid.dim2();++i) for (size_t j=0;j<maingrid.dim2();++j) if (!maingrid(i,j)->fixed) draw_element(maingrid,i,j);
				}
				else mvprintw(ymax-1,0,"No solution found!");
				break;
			case 'c':
				if (solution.dim()==0) {
					solution=maingrid;
					solution.fill();
				}
				min=solution.dim2();
				for (size_t i=0;i<maingrid.dim2();++i) for (size_t j=0;j<maingrid.dim2();++j) if (maingrid(i,j)->npossible<min && maingrid(i,j)->npossible>0) {
					min=maingrid(i,j)->npossible;
					savi=i;savj=j;
				}
				if (min<solution.dim2()) {
					maingrid.set_value(savi,savj,solution(savi,savj)->value);
					draw_element(maingrid,savi,savj);
				}
				break;
			case 'n':
				mvprintw(ymax-1,0,"Dimension (3 or 4) ?");
				chh=0;
				while (chh!='3' && chh!='4') chh=getch();
				si=chh-'0';
				savi=22;
				while (savi<0 || savi>21) {
					mvprintw(ymax-1,0,"Difficulty (0 is harder, 20 is easier, nothing for an empty grid) ?");
					echo();
					char buf[3];
					getnstr(buf,2);
					if (buf[0]==0) savi=21;
					else {
						sscanf(buf,"%lu",&savi);
						if (savi==21) savi=22;
					}
					noecho();
				}
				if (savi==21) maingrid=Grid(si);
				else maingrid=Grid::generate(si,savi,&solution);
				draw_structure(maingrid);
				si=0;sj=0;
				for (size_t i=0;i<maingrid.dim2();++i) for (size_t j=0;j<maingrid.dim2();++j) draw_element(maingrid,i,j);
				break;
		}
	}
	// End 
	endwin();
}
