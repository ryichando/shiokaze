/*
**	ann2.h
**
**	This is part of Shiokaze, a research-oriented fluid solver for computer graphics.
**	Created by Ryoichi Ando <rand@nii.ac.jp> on Feb 15, 2017. 
**
**	Permission is hereby granted, free of charge, to any person obtaining a copy of
**	this software and associated documentation files (the "Software"), to deal in
**	the Software without restriction, including without limitation the rights to use,
**	copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the
**	Software, and to permit persons to whom the Software is furnished to do so,
**	subject to the following conditions:
**
**	The above copyright notice and this permission notice shall be included in all copies
**	or substantial portions of the Software.
**
**	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
**	INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
**	PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
**	HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
**	CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
**	OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
//
#include <ann/ANN.h>
#include <shiokaze/math/vec.h>
//
#ifndef SHKZ_ANN2_H
#define SHKZ_ANN2_H
//
SHKZ_BEGIN_NAMESPACE
//
class ann2 {
public:
	ann2() {
		numbers = 0;
		kdtree = nullptr;
	}
	virtual ~ann2() {
		clear();
	}
	void clear() {
		if( numbers ) {
			delete kdtree;
			delete [] dataPts[0];
			delete [] dataPts;
		}
		numbers = 0;
	}
	//
	void sort( const std::vector<vec2d> &array ) {
		clear();
		//
		numbers = (unsigned) array.size();
		if( ! numbers ) return;
		// Create data point set
		dataPts = annAllocPts(numbers,DIM);
		// Assign point information
		for( uint n=0; n<numbers; n++ ) {
			for( uint dim=0; dim<DIM; dim++) dataPts[n][dim] = array[n][dim];
		}
		// Build KD Tree
		kdtree = new ANNkd_tree(dataPts,numbers,DIM);
	}
	//
	std::vector<unsigned> get_neighbors( const vec2d &p, unsigned n ) const {
		std::vector<unsigned> res(n);
		if( ! numbers ) return std::vector<unsigned>();
		ANNpoint queryPt = annAllocPt(DIM);
		for( unsigned dim=0; dim<DIM; dim++ ) queryPt[dim] = p[dim];
		ANNidx *nnIdx = new ANNidx[n];
		ANNdist *dists = new ANNdist[n];
		kdtree->annkSearch(queryPt,n,nnIdx,dists,0.0);
		for( unsigned k=0; k<n; k++ ) {
			res[k] = nnIdx[k];
		}
		delete [] queryPt;
		delete [] nnIdx;
		delete [] dists;
		return std::move(res);
	}
	unsigned numbers;
	ANNpointArray dataPts;
	ANNkd_tree * kdtree;
};
//
SHKZ_END_NAMESPACE
//
#endif
