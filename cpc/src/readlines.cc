#include <stdlib.h>
#include <fstream>
#include <vector>
#include <string>
#include "readlines.hh"


template<typename type>
struct string_converter
{
  static inline int convert(const std::string& s, type&)
  {
    return 0;
  }
};

template<>
struct string_converter<double>
{
  static inline int convert(const std::string& s, double& d)
  {
    d = strtof(s.c_str(), NULL);
    return 0;
  }
};

template<>
struct string_converter<unsigned int>
{
  static inline int convert(const std::string& s, unsigned int& ui)
  {
    ui = strtoul(s.c_str(), NULL, 10);
    return 0;
  }
};

template<typename vector_type>
static int readlines_and_convert
(const std::string& filename, vector_type& lines)
{
  typedef typename vector_type::value_type value_type;

  std::ifstream ifs;
  ifs.open(filename.c_str(), std::ifstream::in);
  if (ifs.is_open() == false) return -1;

  while (1)
  {
    std::string line;
    std::getline(ifs, line);
    if (line.size() == 0) break ;

    value_type tmp;
    if (string_converter<value_type>::convert(line, tmp)) return -1;
    lines.push_back(tmp);
  }

  ifs.close();

  return 0;
}


int readlines(const std::string& filename, std::vector<double>& lines)
{
  return readlines_and_convert(filename, lines);
}


int readlines(const std::string& filename, std::vector<unsigned int>& lines)
{
  return readlines_and_convert(filename, lines);
}


int readlines(const std::string& filename, std::vector<std::string>& lines)
{
  return readlines_and_convert(filename, lines);
}


#if 0 // unit

#include <iostream>
#include <stdio.h>

int main(int ac, char** av)
{
  std::vector<double> lines;
  readlines(av[1], lines);
  for (unsigned int i = 0; i < lines.size(); ++i)
    std::cout << lines[i] << std::endl;
  return 0;
}

#endif // unit
