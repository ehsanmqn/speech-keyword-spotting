
#ifndef _OFERD_MATRIX_MATRIX_OPERATIONS_H_
#define _OFERD_MATRIX_MATRIX_OPERATIONS_H_

//*****************************************************************************
// Included Files
//*****************************************************************************
#include "infra_matrix.h"

namespace infra {
//-----------------------------------------------------------------------------
/** Performs matrix-matrix multiplication: outcome = A * B 
    @param A The left hand side matrix
    @param B The right hand side matrix
    @param outcome The matrix that is filled with the product 
*/
void prod(const infra::matrix_base& A, const infra::matrix_base& B, 
          infra::matrix_base& outcome);

//-----------------------------------------------------------------------------
/** Operator * for matrix-matrix multiplication: A * B 
    @param A The left hand side matrix
    @param B The right hand side matrix
    @return The outcome matrix
*/
infra::matrix_base operator* (const infra::matrix_base& A, 
                              const infra::matrix_base& B);

//-----------------------------------------------------------------------------
/** Performs multiplication between the transpose of the left hand matrix and
    the right hand matrix: outcome = A^T * B 
    @param A The left hand side matrix, the one that will be transposed
    @param B The right hand side matrix
    @param outcome The matrix that is filled with the product 
*/
void t_prod(const infra::matrix_base& A, const infra::matrix_base& B,
	    infra::matrix_base& outcome);

//-----------------------------------------------------------------------------
/** Performs multiplication between the left hand matrix and the transpose of
    the right hand matrix: outcome = A * B^T 
    @param A The left hand side matrix
    @param B The right hand side matrix, the one that will be transposed
    @param outcome The matrix that is filled with the product 
*/
void prod_t(const infra::matrix_base& A, const infra::matrix_base& B,
	    infra::matrix_base& outcome);

//-----------------------------------------------------------------------------
/** Performs matrix addition
    @param u A constant reference to a matrix
    @param v A constant reference to a matrix
    @param outcome A reference to where the sum matrix will be stored
*/
void sum(const infra::matrix_base& u, const infra::matrix_base& v,
	 infra::matrix_base& outcome);

//-----------------------------------------------------------------------------
/** Operator + for matrix-matrix addition: A + B 
    @param A The left hand side matrix
    @param B The right hand side matrix
    @return The outcome matrix
*/
infra::matrix_base operator+ (const infra::matrix_base& A, 
                              const infra::matrix_base& B);

//-----------------------------------------------------------------------------
/** Performs matrix subtraction
    @param u A constant reference to a matrix
    @param v A constant reference to a matrix
    @param outcome A reference to where the difference matrix will be stored
*/
void diff(const infra::matrix_base& u, const infra::matrix_base& v,
	  infra::matrix_base& outcome);

//-----------------------------------------------------------------------------
/** Operator - for matrix-matrix subtraction: A - B 
    @param A The left hand side matrix
    @param B The right hand side matrix
    @return The outcome matrix
*/
infra::matrix_base operator- (const infra::matrix_base& A, 
                              const infra::matrix_base& B);

//-----------------------------------------------------------------------------
/** Coordinate-wise multiplication
    @param u A constant reference to a matrix
    @param v A constant reference to a matrix
    @param outcome A reference to the outcome matrix
*/
void coordwise_mult(const infra::matrix_base& u, const infra::matrix_base& v,
		    infra::matrix_base& outcome);

//-----------------------------------------------------------------------------
/** Coordinate-wise division
    @param u A constant reference to a matrix
    @param v A constant reference to a matrix
    @param outcome A reference to the outcome matrix
*/
void coordwise_div(const infra::matrix_base& u, const infra::matrix_base& v,
		   infra::matrix_base& outcome);

//-----------------------------------------------------------------------------
/** Operator + for matrix-scalar coordinate-wise addition. 
    @param u A constant reference to a matrix
    @param s A scalar
    @return The outcome
*/
infra::matrix_base operator+(const infra::matrix_base& u, 
                             const double& s);

//-----------------------------------------------------------------------------
/** Operator + for scalar-matrix coordinate-wise addition. 
    @param s A scalar
    @param u A constant reference to a matrix
    @return The outcome
*/
infra::matrix_base operator+(const double& s,
                             const infra::matrix_base& u);

//-----------------------------------------------------------------------------
/** Operator - for matrix-scalar coordinate-wise subtraction;
    @param u A constant reference to a matrix
    @param s A scalar
    @return The outcome
*/
infra::matrix_base operator-(const infra::matrix_base& u, 
                             const double& s);

//-----------------------------------------------------------------------------
/** Operator - for scalar-matrix coordinate-wise subtraction;
    @param s A scalar
    @param u A constant reference to a matrix
    @return The outcome
*/
infra::matrix_base operator-(const double& s,
                             const infra::matrix_base& u);

//-----------------------------------------------------------------------------
/** Operator * for matrix-scalar coordinate-wise multiplication.
    @param u A constant reference to a matrix
    @param s A scalar
    @return The outcome
*/
infra::matrix_base operator*(const infra::matrix_base& u, 
                             const double& s);

//-----------------------------------------------------------------------------
/** Operator * for scalar-matrix coordinate-wise multiplication.
    @param s A scalar
    @param u A constant reference to a matrix
    @return The outcome
*/
infra::matrix_base operator*(const double& s,
                             const infra::matrix_base& u);

//-----------------------------------------------------------------------------
/** Operator / for matrix-scalar coordinate-wise division.
    @param u A constant reference to a matrix
    @param s A scalar
    @return The outcome
*/
infra::matrix_base operator/(const infra::matrix_base& u, 
                             const double& s);


//-----------------------------------------------------------------------------
/** Operator / for scalar-matrix coordinate-wise division.
    @param s A scalar
    @param u A constant reference to a matrix
    @return The outcome
*/
infra::matrix_base operator/(const double& s,
                             const infra::matrix_base& u);
};
#endif
//*****************************************************************************
//                                     E O F
//*****************************************************************************
