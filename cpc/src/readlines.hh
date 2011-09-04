#ifndef READLINES_HH_INCLUDED
# define READLINES_HH_INCLUDED


#include <vector>
#include <string>


int readlines(const std::string&, std::vector<double>&);
int readlines(const std::string&, std::vector<unsigned int>&);
int readlines(const std::string&, std::vector<std::string>&);


#endif // ! READLINES_HH_INCLUDED
