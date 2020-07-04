/*
 * =====================================================================================
 *
 *       Filename:  objects.cpp
 *
 *    Description:  Implementation of the underlying objects of a Sudoku algorithm
 *
 *        Version:  1.0
 *        Created:  25/09/2014 00:02:08
 *       Revision:  none
 *       Compiler:  g++
 *
 *         Author:  François Hissel (), francois.hissel@gmail.com
 *   Organization:  
 *
 * =====================================================================================
 */

#include <iostream>
#include <sstream>
#include <string>
#include <cmath>
#include <functional>
#include <random>
#include <list>
#include "config.h"
#include "objects.h"

using namespace std;

Grid Grid::_saved;

random_device rdevice;
std::mt19937 rgenerator(rdevice());

/**************************************************************************/
/*                           SudokuException                              */
/**************************************************************************/
const char* SudokuException::what() const throw() {
	switch (code) {
		case FORMAT_ERROR:return "Incorrect format.";
		default:return message.c_str();
	}
}

/**************************************************************************/
/*                                 Grid                                   */
/**************************************************************************/

Grid::SuCoordinates Grid::warp(size_t type,const Grid::XYCoordinates& coords) const {
	if (type==0) return Grid::SuCoordinates(0,coords.row,coords.column);
	if (type==1) return Grid::SuCoordinates(1,coords.column,coords.row);
	return Grid::SuCoordinates(2,(coords.row/_dim)*_dim+(coords.column/_dim),(coords.row%_dim)*_dim+(coords.column%_dim));
}

Grid::XYCoordinates Grid::warp(const Grid::SuCoordinates& coords) const {
	if (coords.type==0) return Grid::XYCoordinates(coords.set,coords.index);
	if (coords.type==1) return Grid::XYCoordinates(coords.index,coords.set);
	return Grid::XYCoordinates((coords.set/_dim)*_dim+coords.index/_dim,(coords.set%_dim)*_dim+coords.index%_dim);
}

Grid::Grid(size_t pdim):_dim(pdim),_filled(0) {
	if (pdim>0)	{
		pdim*=pdim;
		_dim2=pdim;
		pdim*=pdim;
		_cells=new Cell*[pdim];
		for (size_t i=0;i<_dim2;++i) for (size_t j=0;j<_dim2;++j) _cells[i*_dim2+j]=new Cell(0,_dim2);
		_alternatives=new size_t[pdim*3];
		for (size_t i=0;i<pdim*3;++i) _alternatives[i]=_dim2;
	}
}

Grid::Grid(std::istream &pin) {
	_cells=0;
	_alternatives=0;
	read_from_stream(pin);
}

Grid::Grid(const Grid &source):_dim(source._dim),_dim2(source._dim2),_filled(source._filled) {
	size_t pdim=_dim2*_dim2;
	_cells=new Cell*[pdim];
	for (size_t i=0;i<_dim2;++i) for (size_t j=0;j<_dim2;++j) _cells[i*_dim2+j]=new Cell(*(source._cells[i*_dim2+j]));
	_alternatives=new size_t[pdim*3];
	for (size_t i=0;i<pdim*3;++i) _alternatives[i]=source._alternatives[i];
}

Grid::~Grid() {
	free_all();
}

void Grid::free_all() {
	if (_cells!=0) {
		size_t pdim=_dim2*_dim2;
		for (size_t i=0;i<pdim;++i) if (_cells[i]!=0) delete _cells[i];
		delete[] _cells;
		_cells=0;
	}
	if (_alternatives!=0) {
		delete[] _alternatives;
		_alternatives=0;
	}
}

void Grid::clear() {
	free_all();
	_filled=0;
	size_t pdim=_dim2*_dim2;
	_cells=new Cell*[pdim];
	for (size_t i=0;i<_dim2;++i) for (size_t j=0;j<_dim2;++j) _cells[i*_dim2+j]=new Cell(0,_dim2);
	_alternatives=new size_t[pdim*3];
	for (size_t i=0;i<pdim*3;++i) _alternatives[i]=_dim2;
}

void Grid::read_from_stream(istream &in) {
	free_all();
	string line;
	getline(in,line);
	elem_t num;
	istringstream iss(line);
	iss.exceptions(istream::failbit);
	_dim=0;
	// Read first row and stores it in memory
	list<elem_t> row;
	while (!iss.eof()) {
		iss >> num;
		row.push_back(num);
		_dim++;
	}
	_dim2=_dim;
	_dim=(size_t)sqrt(_dim);
	if (_dim*_dim!=_dim2) throw SudokuException(SudokuException::FORMAT_ERROR,"The dimension of the grid must be a square integer.");
	// Now that the dimension is known, construct the object
	_filled=0;
	_cells=new Cell*[_dim2*_dim2];
	size_t i;
	for (i=0;i<_dim2;++i) for (size_t j=0;j<_dim2;++j) _cells[i*_dim2+j]=new Cell(0,_dim2);
	_alternatives=new size_t[_dim2*_dim2*3];
	for (i=0;i<_dim2*_dim2*3;++i) _alternatives[i]=_dim2;
	// Copy the first row back in the object
	i=0;
	for (list<elem_t>::iterator it=row.begin();it!=row.end();it++) {
		if (*it!=0) set_value(0,i,*it);
		++i;
	}
	// Read all other rows
	iss.exceptions(istream::failbit | istream::badbit);
	for (i=1;i<_dim2;++i) {
		getline(in,line);
		iss.clear();
		iss.str(line);
		for (size_t j=0;j<_dim2;++j) {
			iss >> num;
			if (num!=0) set_value(i,j,num);
		}
	}
}

void Grid::write_to_stream(ostream &out) const {
	for (size_t i=0;i<_dim2;++i) {
		out << (*this)(i,0)->value;
		for (size_t j=1;j<_dim2;++j) out << '\t' << (*this)(i,j)->value;
		out << '\n';
	}
}

Cell *Grid::operator()(size_t ptype,size_t pset,size_t pindex) const {
	Grid::XYCoordinates xy=warp(Grid::SuCoordinates(ptype,pset,pindex));
	return (*this)(xy.row,xy.column);
}

void Grid::operator=(const Grid &source) {
	_dim=source._dim;
	_dim2=source._dim2;
	_filled=source._filled;
	size_t pdim=_dim2*_dim2;
	if (source._cells!=0) {
		_cells=new Cell*[pdim];
		for (size_t i=0;i<_dim2;++i) for (size_t j=0;j<_dim2;++j) _cells[i*_dim2+j]=new Cell(*(source._cells[i*_dim2+j]));
	} else _cells=0;
	if (source._alternatives!=0) {
		_alternatives=new size_t[pdim*3];
		for (size_t i=0;i<pdim*3;++i) _alternatives[i]=source._alternatives[i];
	} else _alternatives=0;
}


void Grid::set_value(size_t prow,size_t pcolumn,elem_t pvalue,bool pfixed) {
	Cell* cell=(*this)(prow,pcolumn);
	cell->value=pvalue;
	cell->fixed=pfixed;
	_filled++;
	if (cell->possible!=0) {
		for (size_t t=0;t<3;++t) {
			Grid::SuCoordinates coords=warp(t,Grid::XYCoordinates(prow,pcolumn));
			for (size_t i=0;i<dim2();++i) if (i!=pvalue-1 && cell->possible[i]) {	// Update alternative levels for all other values for the sets containing this cell
				size_t num=get_alternative(t,coords.set,i+1);
				if (num!=0) {
					set_alternative(t,coords.set,i+1,num-1); 
#if (DEBUG_LEVEL>=3)
					cerr << "\tUpdating alternative (" << t << "," << coords.set << "," << (i+1) << "," << num-1 << ")\n";
#endif
				}
			}
		}
		delete[] cell->possible;
		cell->possible=0;
		cell->npossible=0;
	}
	// Update possible values of cells of the same row, column and inner square and alternatives levels
	for (size_t t=0;t<3;++t) {
		Grid::SuCoordinates coords=warp(t,Grid::XYCoordinates(prow,pcolumn));
		for (size_t i=0;i<dim2();++i) {
			Cell* c=(*this)(t,coords.set,i);
			if (c->possible!=0 && c->possible[pvalue-1]) {	// Update possible values
				c->npossible--;
				c->possible[pvalue-1]=false;	
				Grid::XYCoordinates xy=warp(Grid::SuCoordinates(t,coords.set,i));
#if (DEBUG_LEVEL>=3)
				cerr << "Updating cell (" << xy.row << "," << xy.column << ") because of new value " << pvalue << " in (" << prow << "," << pcolumn << ")\n";
#endif
				for (size_t s=0;s<3;++s) {	// Update alternatives levels for the value and the sets containing the cell which possible values have been updated
					Grid::SuCoordinates coor=warp(s,xy);
					size_t n=get_alternative(coor.type,coor.set,pvalue);
					if (n!=0) {
						set_alternative(coor.type,coor.set,pvalue,n-1);
#if (DEBUG_LEVEL>=3)
						cerr << "\tUpdating alternative (" << coor.type << "," << coor.set << "," << pvalue << "," << n-1 << ")\n";
#endif
					}
				}
			}
		}
		// Delete alternative for the new value in all sets containing the cell
		set_alternative(t,coords.set,pvalue,0); 
#if (DEBUG_LEVEL>=3)
		cerr << "\tUpdating alternative (" << t << "," << coords.set << "," << pvalue << "," << 0 << ")\n";
#endif
	}
}

size_t Grid::solve(SolveType type,std::function<void(const Grid&)> callback) const {
	size_t i,j;
	Grid source=*this;
#if (DEBUG_LEVEL>=1)
	cerr << "\nSolve\n" << source << "\n";
#endif
	size_t min=0;
	size_t min2=0;
	size_t ind,indi,indj;
	while ((min<=1 || min2<=1) && source._filled!=source._dim2*source._dim2) {	// Fill as much as possible by deduction
#if (DEBUG_LEVEL>=3)
		cerr << "New iteration (" << source._filled << ")\n";
#endif
		// Debug output
#if (DEBUG_LEVEL>=2)
		cerr << "Alternatives\n";
		for (i=0;i<source._dim2;++i) {
			for (size_t t=0;t<3;++t) {
				for (j=0;j<source._dim2;++j) {
					cerr << source._alternatives[t*source._dim2*source._dim2+i*source._dim2+j] << " ";
				}
				cerr << "\t";
			}
			cerr << endl;
		}
		cerr << "\nPossibles\n";
		for (i=0;i<source._dim2;++i) {
			for (j=0;j<source._dim2;++j) {
				Cell *c=source(i,j);
				if (c->possible==0) cerr << "         ";
				else for (size_t t=0;t<source._dim2;++t) {
					if (c->possible[t]) cerr << (t+1); else cerr << " ";
				}
				cerr << " | ";
			}
			cerr << endl;
		}
#endif
		// Look for the alternative with the smallest number of possibilities
		min=source._dim2+1;
		ind=0;
		for (i=0;i<source._dim2*source._dim2*3;++i) if (source._alternatives[i]>0 && source._alternatives[i]<min) {
			min=source._alternatives[i];
			ind=i;
		}
		// Look for the cell with the smallest number of possibilities
		min2=source._dim2+1;
		indi=0;indj=0;
		for (i=0;i<source._dim2;++i) for (j=0;j<source._dim2;++j) if (source(i,j)->npossible>0 && source(i,j)->npossible<min2) {
			min2=source(i,j)->npossible;
			indi=i;
			indj=j;
		}
		// Now choose the better option. If there is only one possibility, put the number.
		if (min==1) {	// Case when a new element can be found by deduction ("There must be a 4 in this row, and it can be neither here, nor here, nor here...")
			Alternative alt=source.ind_alternative(ind);
#if (DEBUG_LEVEL>=2)
			cerr << "Alternative(" << alt.type << "," << alt.set << "," << alt.value << ")" << endl;
#endif
			i=0;
			while (i<source._dim2) {
				Grid::XYCoordinates coords=source.warp(Grid::SuCoordinates(alt.type,alt.set,i));
				Cell *cell=source(coords.row,coords.column);
				if (cell->possible!=0 && cell->possible[alt.value-1]) {
					source.set_value(coords.row,coords.column,alt.value);
					i=source._dim2;
				}
				++i;
			}
		} else if (min2==1) {	// Case when a new element is found by elimination ("Here we can have neither a 1, nor a 2, nor a 4...")
			i=0;
			Cell *cell=source(indi,indj);
			while (i<source._dim2 && (cell->possible==0 || !cell->possible[i])) ++i;
#if (DEBUG_LEVEL>=2)
			cerr << "Possible(" << indi << "," << indj << "," << i << ")" << endl;
#endif
			if (i<source._dim2) source.set_value(indi,indj,i+1);
		}
	}
	// If the grid is filled, return
	if (source._filled==source._dim2*source._dim2) {
		if (callback!=0) callback(source);
		return 1;
	}
	if (min==source._dim2+1 && min2==source._dim2+1) return 0;
	// Difficult case when no value can be found either be deduction or by elimination. In this case, we just try recursively until we find a solution.
	size_t nfound=0;
	size_t maxfound;
	i=0;
	size_t num;
	size_t k;
	switch (type) {
		case FIND_ONE:
		case FIND_ANY:
			maxfound=1;
			break;
		case FIND_ALL:
			maxfound=numeric_limits<size_t>::max()-1;
			break;
		case FIND_UNIQUE:
			maxfound=2;
			break;
	}
	if (min<min2) {
		Alternative alt=source.ind_alternative(ind);
		std::uniform_int_distribution<> dis(0,source._alternatives[ind]-1);
		j=0;
		Grid::XYCoordinates coords;
		while (j<source._alternatives[ind] && nfound<maxfound) {
			if (type==FIND_ANY) {
				i=0;num=dis(rgenerator);
			} else num=0;
			k=0;
			while (i<source._dim2 && k<=num) {
				coords=source.warp(Grid::SuCoordinates(alt.type,alt.set,i));
				Cell *cell=source(coords.row,coords.column);
				if (cell->possible!=0 && cell->possible[alt.value-1]) ++k;
				++i;
			}
			--i;
			Grid *hypothesis=new Grid(source);
#if (DEBUG_LEVEL>=2)
			cerr << "Trying " << alt.value << " on cell (" << coords.row << "," << coords.column << ") based on Alternative(" << alt.type << "," << alt.set << "," << alt.value << ")\n";
#endif
			hypothesis->set_value(coords.row,coords.column,alt.value);
			size_t res=hypothesis->solve(type,callback);
			delete hypothesis;
			nfound+=res;
			++i;
			++j;
		}
	} else {
		Cell *cell=source(indi,indj);
		std::uniform_int_distribution<> dis(0,cell->npossible-1);
		j=0;
		while (j<cell->npossible && nfound<maxfound) {
			if (type==FIND_ANY) {
				i=0;num=dis(rgenerator);
			} else num=0;
			k=0;
			while (i<source._dim2 && k<=num) {
				if (cell->possible[i]) ++k;
				++i;
			}
			--i;
			Grid *hypothesis=new Grid(source);
#if (DEBUG_LEVEL>=2)
			cerr << "Trying " << (i+1) << " on cell (" << indi << "," << indj << ") based on Possible(" << indi << "," << indj << ")\n";
#endif
			hypothesis->set_value(indi,indj,i+1);
			size_t res=hypothesis->solve(type,callback);
			delete hypothesis;
			nfound+=res;
			++i;
			++j;
		}
	}
	// Execute the callback function on the final grid and return
#if (DEBUG_LEVEL>=1)
	cerr << "Return from solve with nfound=" << nfound << " and grid :\n" << source << "\n";
#endif
	return nfound;
}

bool Grid::fill() {
	size_t res=solve(FIND_ANY,&Grid::save);
	if (res==0) return false;
	free_all();
	_cells=new Cell*[_dim2*_dim2];
	for (size_t i=0;i<_dim2;++i) for (size_t j=0;j<_dim2;++j) _cells[i*_dim2+j]=new Cell(*(_saved._cells[i*_dim2+j]));
	return true;
}

void Grid::save() const {
	_saved.free_all();
	_saved._dim=_dim;
	_saved._dim2=_dim2;
	_saved._alternatives=0;
	size_t pdim=_dim2*_dim2;
	_saved._cells=new Cell*[pdim];
	for (size_t i=0;i<_dim2;++i) for (size_t j=0;j<_dim2;++j) _saved._cells[i*_dim2+j]=new Cell(*(_cells[i*_dim2+j]));
}

Grid Grid::generate(size_t dimension,size_t difficulty,Grid *solution) {
	// Generate a full valid grid
	Grid source(dimension);
	source.fill();
	if (solution!=0) *solution=source;
	std::uniform_int_distribution<> dis(0,source._dim2-1);
	// Create a grid by copying some elements from the source grid
	Grid generated(dimension);	
	size_t i=0;
	while (i<source._dim2*source._dim+difficulty) {
		size_t j=dis(rgenerator);
		size_t k=dis(rgenerator);
		if (generated(j,k)->value==0) {
			generated.set_value(j,k,source(j,k)->value,true);
			++i;
		}
	}
	// Add elements until solution is unique
	bool found=false;
	while (!found) {
		// Test new grid if the solution is unique
		Grid test(generated);
		bool done=false;
		if (test.solve(Grid::FIND_UNIQUE,0)) found=true;
		else while (!done) { // Otherwise, add one element randomly and prepare next loop
			size_t j=dis(rgenerator);
			size_t k=dis(rgenerator);
			if (generated(j,k)->value==0) {
				generated.set_value(j,k,source(j,k)->value,true);
				done=true;
			}
		}
	}
	// Return
	return generated;
}

istream& operator>>(istream &in,Grid *grid) {
	grid=new Grid(0);
	grid->read_from_stream(in);
	return in;
}

ostream& operator<<(ostream &out,const Grid &grid) {
	grid.write_to_stream(out);
	return out;
}

/**************************************************************************/
/*                                 Cell                                   */
/**************************************************************************/
Cell::Cell(elem_t pvalue,size_t pnpossible):value(pvalue),npossible(pnpossible),fixed(false),_dim2(pnpossible) {
	possible=new bool[pnpossible];
	for (size_t i=0;i<pnpossible;++i) possible[i]=true;
}

Cell::Cell(const Cell& source):value(source.value),npossible(source.npossible),fixed(source.fixed),_dim2(source._dim2) {
	if (source.possible==0) possible=0; else {
		possible=new bool[_dim2];
		for (size_t i=0;i<_dim2;++i) possible[i]=source.possible[i];
	}
}

Cell::~Cell() {
	if (possible!=0) delete[] possible;
}


