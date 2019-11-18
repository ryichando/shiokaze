/*
**	RCMatrix.cpp
**
**	This is part of Shiokaze, a research-oriented fluid solver for computer graphics.
**	Created by Ryoichi Ando <rand@nii.ac.jp> on March 30, 2018.
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
#include <shiokaze/math/RCMatrix_interface.h>
#include <shiokaze/parallel/parallel_driver.h>
#include <unordered_map>
#include "blas_wrapper.h"
//
SHKZ_USING_NAMESPACE
//
template <class N, class T> class RCMatrix_allocator : public RCMatrix_allocator_interface<N,T> {
public:
	RCMatrix_allocator( const RCMatrix_factory_interface<N,T> &factory ) : m_factory(factory) {}
	virtual RCMatrix_vector_ptr<N,T> allocate_vector( N size ) const override {
		return m_factory.allocate_vector(size);
	}
	virtual RCMatrix_ptr<N,T> allocate_matrix( N rows, N columns ) const override {
		return m_factory.allocate_matrix(rows,columns);
	}
private:
	const RCMatrix_factory_interface<N,T> &m_factory;
};
//
template <class N, class T> class RCMatrix_vector : public RCMatrix_vector_interface<N,T> {
public:
	RCMatrix_vector( N size, const parallel_driver &parallel, const RCMatrix_factory_interface<N,T> &factory ) : m_parallel(parallel), m_allocator(factory) {
		m_array.resize(size);
	}
	virtual void copy( const RCMatrix_vector_interface<N,T> *x ) override {
		const auto *v = dynamic_cast<const RCMatrix_vector<N,T> *>(x);
		if( v ) {
			m_array = v->m_array;
		} else {
			RCMatrix_vector_interface<N,T>::copy(x);
		}
	}
	virtual void resize( N size ) override {
		m_array.resize(size);
		m_array.shrink_to_fit();
	}
	virtual N size() const override {
		return m_array.size();
	}
	virtual void clear( T value ) override {
		for( auto &e : m_array ) e = value;
	}
	virtual T at( N index ) const override {
		return m_array[index];
	}
	virtual void set( N index, T value ) override {
		m_array[index] = value;
	}
	virtual void add( N index, T value ) override {
		m_array[index] += value;
	}
	virtual void subtract( N index, T value ) override {
		m_array[index] -= value;
	}
	virtual void multiply( N index, T value ) override {
		m_array[index] *= value;
	}
	virtual void divide( N index, T value ) override {
		m_array[index] /= value;
	}
	virtual void parallel_for_each( std::function<void( N row, T& value )> func) override {
		m_parallel.for_each(m_array.size(),[&]( unsigned k ) {
			func(k,m_array[k]);
		});
	}
	virtual void const_parallel_for_each( std::function<void( N row, T value )> func) const override {
		m_parallel.for_each(m_array.size(),[&]( unsigned k ) {
			func(k,m_array[k]);
		});
	}
	//
	virtual void interruptible_for_each( std::function<bool( N row, T& value )> func) override {
		for( N k=0; k<m_array.size(); ++k ) if( func(k,m_array[k]) ) break;
	}
	virtual void const_interruptible_for_each( std::function<bool( N row, T value )> func) const override {
		for( N k=0; k<m_array.size(); ++k ) if( func(k,m_array[k]) ) break;
	}
	virtual T abs_max() const override {
		return BLAS::abs_max(m_array);
	}
	virtual T dot( const RCMatrix_vector_interface<N,T> *x ) const override {
		const auto *v = dynamic_cast<const RCMatrix_vector<N,T> *>(x);
		if( v ) {
			return BLAS::dot(v->m_array,m_array);
		} else {
			T result (0.0);
			for( N k=0; k<m_array.size(); ++k ) {
				result += m_array[k] * x->at(k);
			}
			return result;
		}
	}
	virtual void add_scaled( T alpha, const RCMatrix_vector_interface<N,T> *x ) override {
		const auto *v = dynamic_cast<const RCMatrix_vector<N,T> *>(x);
		if( v ) {
			BLAS::add_scaled(alpha,v->m_array,m_array);
		} else {
			for( N k=0; k<m_array.size(); ++k ) m_array[k] += alpha * x->at(k);
		}
	}
	virtual RCMatrix_vector_ptr<N,T> allocate_vector( N size ) const override {
		return m_allocator.allocate_vector(size);
	}
	virtual RCMatrix_ptr<N,T> allocate_matrix( N rows, N columns ) const override {
		return m_allocator.allocate_matrix(rows,columns);
	}
	std::vector<T> m_array;
	const RCMatrix_allocator<N,T> m_allocator;
	const parallel_driver &m_parallel;
};
//
template <class N, class T> struct RowEntry {
	std::vector<N> index;
	std::vector<T> value;
};
//
template <class N, class T> class RCFixedMatrix : public RCFixedMatrix_interface<N,T> {
public:
	RCFixedMatrix( const std::vector<RowEntry<N,T> > &matrix, const RCMatrix_factory_interface<N,T> &factory ) : m_allocator(factory) {
		//
		m_rows = matrix.size();
		m_rowstart.resize(m_rows+1);
		m_rowstart[0] = 0;
		for(N i=0; i<m_rows; i++ ) {
			m_rowstart[i+1]=m_rowstart[i]+matrix[i].index.size();
		}
		m_value.resize(m_rowstart[m_rows]);
		m_index.resize(m_rowstart[m_rows]);
		N j (0);
		for( N i=0; i<m_rows; i++) {
			const std::vector<N> &index = matrix[i].index;
			const std::vector<T> &value = matrix[i].value;
			for( N k=0; k<index.size(); ++k ) {
				m_index[j] = index[k];
				m_value[j] = value[k];
				++ j;
			}
		}
	}
private:
	//
	virtual RCMatrix_vector_ptr<N,T> allocate_vector( N size ) const override {
		return m_allocator.allocate_vector(size);
	}
	virtual RCMatrix_ptr<N,T> allocate_matrix( N rows, N columns ) const override {
		return m_allocator.allocate_matrix(rows,columns);
	}
	//
	virtual void multiply( const RCMatrix_vector_interface<N,T> *rhs, RCMatrix_vector_interface<N,T> *result ) const override {
		//
		const auto *v = dynamic_cast<const RCMatrix_vector<N,T> *>(rhs);
		if( v ) {
			for( N i=0; i<m_rows; ++i ) {
				T value (0.0);
				for( N j=m_rowstart[i]; j<m_rowstart[i+1]; ++j ) value += v->m_array[m_index[j]] * m_value[j];
				result->set(i,value);
			}
		} else {
			for( N i=0; i<m_rows; ++i ) {
				T value (0.0);
				for( N j=m_rowstart[i]; j<m_rowstart[i+1]; ++j ) value += rhs->at(m_index[j]) * m_value[j];
				result->set(i,value);
			}
		}
	}
	//
	const RCMatrix_allocator<N,T> m_allocator;
	std::vector<N> m_rowstart;
	std::vector<N> m_index;
	std::vector<T> m_value;
	N m_rows;
};
//
template <class N, class T> class RCMatrix : public RCMatrix_interface<N,T> {
public:
	RCMatrix( const parallel_driver &parallel, const RCMatrix_factory_interface<N,T> &factory ) : m_parallel(parallel), m_allocator(factory), m_factory(factory) {}
private:
	virtual void initialize( N rows, N columns ) override {
		//
		m_matrix.resize(rows);
		m_matrix.shrink_to_fit();
		m_columns = columns;
		m_parallel.for_each(rows,[&]( size_t row ) {
			clear(row);
		});
	}
	virtual void copy(const RCMatrix_interface<N,T> *m ) override {
		//
		initialize(m->rows(),m->columns());
		const auto *mate_matrix = dynamic_cast<const RCMatrix<N,T> *>(m);
		if( mate_matrix ) {
			m_parallel.for_each(rows(),[&]( size_t row ) {
				m_matrix[row] = mate_matrix->m_matrix[row];
			});
		} else {
			m_parallel.for_each(rows(),[&]( size_t row ) {
				m->const_for_each(row,[&]( N column, T value ) {
					add_to_element(row,column,value);
				});
			});
		}
	}
	virtual void clear( N row ) override {
		//
		m_matrix[row].index.clear();
		m_matrix[row].value.clear();
		//
		m_matrix[row].index.shrink_to_fit();
		m_matrix[row].value.shrink_to_fit();
		//
	}
	virtual T get( N row, N column ) const override {
		//
		T result (0.0);
		const_interruptible_for_each(row,[&]( N _column, T value ) {
			if( column == _column ) {
				result = value;
				return true;
			}
			return false;
		});
		return result;
	}
	virtual void add_to_element( N row, N column, T increment_value ) override {
		//
		if( increment_value ) {
			assert( column < m_columns );
			std::vector<N> &index = m_matrix[row].index;
			std::vector<T> &value = m_matrix[row].value;
			for( N k=0; k<index.size(); ++k ){
				if( index[k] == column ){
					value[k] += increment_value;
					if( ! value[k] ) {
						index.erase(index.begin()+k);
						value.erase(value.begin()+k);
					}
					return;
				} else if( index[k] > column ){
					index.insert(index.begin()+k,column);
					value.insert(value.begin()+k,increment_value);
					return;
				}
			}
			index.push_back(column);
			value.push_back(increment_value);
		}
	}
	virtual void clear_element( N row, N column ) override {
		assert( column < m_columns );
		std::vector<N> &index = m_matrix[row].index;
		std::vector<T> &value = m_matrix[row].value;
		for( N k=0; k<index.size(); ++k ){
			if( index[k] == column ){
				index.erase(index.begin()+k);
				value.erase(value.begin()+k);
				return;
			}
		}
	}
	virtual void interruptible_for_each( N row, std::function<bool( N column, T& value )> func) override {
		//
		std::vector<N> &index = m_matrix[row].index;
		std::vector<T> &value = m_matrix[row].value;
		for( N k=0; k<index.size(); ) {
			bool do_break (false);
			if( func(index[k],value[k]) ) do_break = true;
			if( value[k] ) ++ k;
			else {
				index.erase(index.begin()+k);
				value.erase(value.begin()+k);
			}
			if( do_break ) break;
		}
	}
	virtual void const_interruptible_for_each( N row, std::function<bool( N column, T value )> func) const override {
		//
		const std::vector<N> &index = m_matrix[row].index;
		const std::vector<T> &value = m_matrix[row].value;
		assert(index.size() == value.size());
		for( N k=0; k<index.size(); ++k ) if( func(index[k],value[k]) ) break;
	}
	virtual N rows() const override {
		return m_matrix.size();
	}
	virtual N columns() const override {
		return m_columns;
	}
	virtual N non_zeros( N row ) const override {
		//
		const std::vector<N> &index = m_matrix[row].index;
		return index.size();
	}
	virtual void multiply(T value) override {
		//
		m_parallel.for_each(rows(),[&]( size_t row ) {
			RCMatrix_interface<N,T>::for_each(row,[&]( N column, T& _value ) {
				_value *= value;
			});
		});
	}
	virtual void multiply( const RCMatrix_vector_interface<N,T> *rhs, RCMatrix_vector_interface<N,T> *result ) const override {
		//
		result->resize(rows());
		const auto *v = dynamic_cast<const RCMatrix_vector<N,T> *>(rhs);
		if( v ) {
			m_parallel.for_each(rows(),[&]( size_t row ) {
				T sum (0.0);
				RCMatrix_interface<N,T>::const_for_each(row,[&]( N column, T value ) {
					sum += v->m_array[column] * value;
				});
				result->set(row,sum);
			});
		} else {
			m_parallel.for_each(rows(),[&]( size_t row ) {
				T sum (0.0);
				RCMatrix_interface<N,T>::const_for_each(row,[&]( N column, T value ) {
					sum += rhs->at(column) * value;
				});
				result->set(row,sum);
			});
		}
	}
	virtual void multiply( const RCMatrix_interface<N,T> *matrix, RCMatrix_interface<N,T> *result ) const override {
		//
		assert( columns() == matrix->rows());
		//
		result->initialize(rows(),matrix->columns());
		m_parallel.for_each(rows(),[&]( size_t row ) {
			RCMatrix_interface<N,T>::const_for_each(row,[&]( N A_column, T A_value ) {
				matrix->const_for_each(A_column,[&]( N B_column, T B_value ) {
					result->add_to_element(row,B_column,A_value*B_value);
				});
			});
		});
	}
	virtual void add( const RCMatrix_interface<N,T> *matrix, RCMatrix_interface<N,T> *result ) const override {
		//
		assert(matrix->rows() == rows());
		assert(matrix->columns() == columns());
		//
		result->initialize(rows(),columns());
		m_parallel.for_each(matrix->rows(),[&]( size_t row ) {
			matrix->const_for_each(row,[&]( N column, T value ) {
				result->add_to_element(row,column,value);
			});
			this->const_for_each(row,[&]( N column, T value ) {
				result->add_to_element(row,column,value);
			});
		});
	}
	virtual void transpose( RCMatrix_interface<N,T> *result ) const override {
		//
		result->initialize(columns(),rows());
		for( size_t row=0; row<rows(); ++row ) {
			RCMatrix_interface<N,T>::const_for_each(row,[&]( N column, T value ) {
				result->add_to_element(column,row,value);
			});
		}
	}
	virtual RCMatrix_vector_ptr<N,T> allocate_vector( N size ) const override {
		return m_allocator.allocate_vector(size);
	}
	virtual RCMatrix_ptr<N,T> allocate_matrix( N rows, N columns ) const override {
		return m_allocator.allocate_matrix(rows,columns);
	}
	virtual RCFixedMatrix_ptr<N,T> make_fixed() const override {
		return RCFixedMatrix_ptr<N,T>(new RCFixedMatrix<N,T>(m_matrix,m_factory));
	}
	//
	std::vector<RowEntry<N,T> > m_matrix;
	N m_columns;
	const parallel_driver &m_parallel;
	const RCMatrix_factory_interface<N,T> &m_factory;
	const RCMatrix_allocator<N,T> m_allocator;
};
//
template <class N, class T> class RCMatrix_factory : public RCMatrix_factory_interface<N,T> {
protected:
	//
	virtual RCMatrix_vector_ptr<N,T> allocate_vector( N size ) const override {
		return RCMatrix_vector_ptr<N,T>(new RCMatrix_vector<N,T>(size,m_parallel,*this));
	}
	virtual RCMatrix_ptr<N,T> allocate_matrix( N rows, N columns ) const override {
		auto result = RCMatrix_ptr<N,T>(new RCMatrix<N,T>(m_parallel,*this));
		if( rows && columns ) {
			result->initialize(rows,columns);
		}
		return result;
	}
	parallel_driver m_parallel{this};
};
//
extern "C" module * create_instance() {
	return new RCMatrix_factory<INDEX_TYPE,FLOAT_TYPE>();
}
extern "C" const char *license() {
	return "MIT";
}
//