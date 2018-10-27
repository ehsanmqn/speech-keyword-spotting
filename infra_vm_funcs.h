
#ifndef _OFERD_VM_FUNCS_H_
#define _OFERD_VM_FUNCS_H_

//*****************************************************************************
// Included Files
//*****************************************************************************
#include "infra_vector.h"
#include "infra_matrix.h"

namespace infra {
//-----------------------------------------------------------------------------
/** Performs matrix-vector_base multiplication: outcome = M * v 
    @param M A constant reference to a matrix
    @param v A constant reference to a vector_base
    @param outcome A reference to where the outcome will be stored
*/
void prod(const infra::matrix_base& M, const infra::vector_base& v,
	  infra::vector_base& outcome);

//-----------------------------------------------------------------------------
/** Operator * for matrix-vector multiplication: M * v 
    @param M A constant reference to a matrix
    @param v A constant reference to a vector_base
    @return The outcome vector
*/
infra::vector_base operator* (const infra::matrix_base& M, 
                              const infra::vector_base& v);

//-----------------------------------------------------------------------------
/** Performs vector_base-matrix multiplication: outcome = v * M 
    @param v A constant reference to a vector_base
    @param M A constant reference to a matrix
    @param outcome A reference to where the outcome will be stored
*/
void prod(const infra::vector_base& v, const infra::matrix_base& M,
	  infra::vector_base& outcome);

//-----------------------------------------------------------------------------
/** Operator * for vector-matrix multiplication: v * M  
    @param v A constant reference to a vector_base
    @param M A constant reference to a matrix
    @return The outcome vector
*/
infra::vector_base operator* (const infra::vector_base& v,
                              const infra::matrix_base& M);

//-----------------------------------------------------------------------------
/** Adds a matrix-vector_base multiplication to the vector_base : 
    outcome += M * v 
    @param M A constant reference to a matrix
    @param v A constant reference to a vector_base
    @param outcome A reference to where the outcome will be stored
*/
void add_prod(const infra::matrix_base& M, const infra::vector_base& v,
	      infra::vector_base& outcome);

//-----------------------------------------------------------------------------
/** Adds a vector_base-matrix multiplication to the vector_base : 
    outcome += v * M 
    @param v A constant reference to a vector_base
    @param M A constant reference to a matrix
    @param outcome A reference to where the outcome will be stored
*/
void add_prod(const infra::vector_base& v, const infra::matrix_base& M, 
	      infra::vector_base& outcome);

//-----------------------------------------------------------------------------
/** Subtracts a matrix-vector_base multiplication from the vector_base : 
    outcome -= M * v 
    @param M A constant reference to a matrix
    @param v A constant reference to a vector_base
    @param outcome A reference to where the outcome will be stored
*/
void subtract_prod(const infra::matrix_base& M, const infra::vector_base& v,
		   infra::vector_base& outcome);

//-----------------------------------------------------------------------------
/** Subtracts a vector_base-matrix multiplication from the vector_base : 
    outcome -= v * M 
    @param v A constant reference to a vector_base
    @param M A constant reference to a matrix
    @param outcome A reference to where the outcome will be stored
*/
void subtract_prod(const infra::vector_base& v, const infra::matrix_base& M, 
		   infra::vector_base& outcome);

//-----------------------------------------------------------------------------
/** Row-wise minimum of the matrix. Finds the minimal value in every
    row and writes it in the corresponding position in the outcome vector_base.
    @param M A constant matrix
    @param outcome A reference to where the outcome will be stored
*/
void row_min(const infra::matrix_base& M, infra::vector_base& outcome);

//-----------------------------------------------------------------------------
/** Row-wise maximum of the matrix. Finds the maximal value in every
    row and writes it in the corresponding position in the outcome vector_base.
    @param M A constant matrix
    @param outcome A reference to where the outcome will be stored
*/
void row_max(const infra::matrix_base& M, infra::vector_base& outcome);

//-----------------------------------------------------------------------------
/** Row-wise sum of the matrix. For every row, the sum of its entries is
    calculated and writen in the corresponding position in the outcome
    vector_base.
    @param M A constant matrix
    @param outcome A reference to where the outcome will be stored
*/
void row_sum(const infra::matrix_base& M, infra::vector_base& outcome);

//-----------------------------------------------------------------------------
/** Column-wise minimum of the matrix. Finds the minimal value in every column
    and writes it in the corresponding position in the outcome vector_base.
    @param M A constant matrix
    @param outcome A reference to where the outcome will be stored
*/
void column_min(const infra::matrix_base& M, infra::vector_base& outcome);

//-----------------------------------------------------------------------------
/** Column-wise maximum of the matrix. Finds the maximal value in every column
    and writes it in the corresponding position in the outcome vector_base.
    @param M A constant matrix
    @param outcome A reference to where the outcome will be stored
*/
void column_max(const infra::matrix_base& M, infra::vector_base& outcome);

//-----------------------------------------------------------------------------
/** Column-wise sum of the matrix. For every column, the sum of its entries is
    calculated and writen in the corresponding position in the outcome
    vector_base.
    @param M A constant matrix
    @param outcome A reference to where the outcome will be stored
*/
void column_sum(const infra::matrix_base& M, infra::vector_base& outcome);

//-----------------------------------------------------------------------------
/** Adds a constant row vector_base to all of the rows in a matrix
    @param M The matrix
    @param v The constant vector being added
*/
void add_row_vector(infra::matrix_base& M, const infra::vector_base& v);

//-----------------------------------------------------------------------------
/** Subtracts a constant row vector from all of the rows in a matrix
    @param M The matrix
    @param v The constant vector being subtracted
*/
void subtract_row_vector(infra::matrix_base& M, const infra::vector_base& v);

//-----------------------------------------------------------------------------
/** Adds a constant column vector to all of the columns in a matrix
    @param M The matrix
    @param v The constant vector being added
*/
void add_column_vector(infra::matrix_base& M, const infra::vector_base& v);

//-----------------------------------------------------------------------------
/** Subtracts a constant column vector from all of the columns in a matrix
    @param M The matrix
    @param v The constant vector being subtracted
*/
void subtract_column_vector(infra::matrix_base& M,const infra::vector_base& v);
};
#endif
//*****************************************************************************
//                                     E O F
//*****************************************************************************
