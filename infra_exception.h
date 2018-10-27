
#ifndef _JKESHET_EXCEPTION_H_
#define _JKESHET_EXCEPTION_H_

//*****************************************************************************
// Included Files
//*****************************************************************************
#include <string>
#include <sstream>

namespace infra {

//-----------------------------------------------------------------------------
/** A callback function which is called whenever an infra_exception is
    not caught. 
*/
void terminate();

//-----------------------------------------------------------------------------
/** Implements an exception which indicates that the infra library was somehow
    misused.
*/
class exception  
{
 public:
//-----------------------------------------------------------------------------
/** Default constructor 
*/
  exception( std::string expression, std::string message, 
	     std::string file, int line);

//-----------------------------------------------------------------------------
/** Resets the exception parameters. This function should never be called
    explicitly, except using the infra_verify() macro
*/
  void set_external (std::string expression, std::string file, int line);

//-----------------------------------------------------------------------------
/** Prints the error message to cerr. Should not be called explicitly, except
    using the infra_assert() macro.
*/
  void print();
  
//=============================================================================
// Data Members
//=============================================================================
  protected:
  std::string _expression;
  std::string _message;
  std::string _file;
  int _line;
  bool _internal;
};
};

//-----------------------------------------------------------------------------
#ifdef NDEBUG
#define infra_verify( statement ) statement;

#else
#define infra_verify( statement )                                     \
try { statement; }                                                    \
catch (infra::exception& exc) {                                       \
exc.set_external(#statement, __FILE__, __LINE__);                     \
throw exc; } 
#endif // NDEBUG


//-----------------------------------------------------------------------------
#ifdef NDEBUG
#define infra_assert( expression, message )                           

#else
#define infra_assert( expression, message )                           \
if ( !( expression ) ) {                                              \
  std::ostringstream __infra_str_;                                    \
  __infra_str_ << message;                                            \
  throw infra::exception(#expression, __infra_str_.str(),             \
                               __FILE__, __LINE__);                   \
}                                                                     
#endif // NDEBUG
#endif // _JKESHET_EXCEPTION_H_

//*****************************************************************************
//                                     E O F
//*****************************************************************************
