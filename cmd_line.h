
#ifndef _CMD_LINE_H_
#define _CMD_LINE_H_

//*****************************************************************************
// Included Files
//*****************************************************************************
#include <string>
#include <vector>


namespace learning {

class cmd_option;

//*****************************************************************************
/** Implements a class that parse the command line.
  
*/
class cmd_line
{
 public:
//-----------------------------------------------------------------------------
/** Adds general information that will be printed when -h is called
 */
  void info(const std::string &info) { info_ = info; }
//-----------------------------------------------------------------------------
/** Adds command line option of int type
 */
  void add(const std::string &tag, const std::string &helper, int *value,
	   const int default_value);
//-----------------------------------------------------------------------------
/** Adds command line option of unsigned int type
 */
  void add(const std::string &tag, const std::string &helper, 
	   unsigned int *value, const int default_value);
//-----------------------------------------------------------------------------
/** Adds command line option of double type
 */
  void add(const std::string &tag, const std::string &helper, double *value,
	   const double default_value);
//-----------------------------------------------------------------------------
/** Adds command line option of bool type
 */
  void add(const std::string &tag, const std::string &helper, bool *value,
	   const bool default_value);
//-----------------------------------------------------------------------------
/** Adds command line option of std::string type
 */
  void add(const std::string &tag, const std::string &helper, 
	   std::string *value, const std::string &default_value);
//-----------------------------------------------------------------------------
/** Adds command line option
 */
  void add(cmd_option *cmd_option);
//-----------------------------------------------------------------------------
/** Adds command line option
 */
  void add_master_option(const std::string &helper, 
			 std::string *master_option);
//-----------------------------------------------------------------------------
/** Parses command line. Returns the number of arguments proceeded.
 */  
  int parse(int argc, char **argv) ;
//-----------------------------------------------------------------------------
/** Prints help information for program and usage of tags
 */  
  void print_help();
  //-----------------------------------------------------------------------------
  /** Destructor
    */  
  ~cmd_line();
 private:
  std::string info_;
  std::vector<cmd_option*> options_vector;
  std::vector<cmd_option*> master_options_vector;
  std::string program_name;
};

}; // namespace learning

#endif // _CMD_LINE_H_
