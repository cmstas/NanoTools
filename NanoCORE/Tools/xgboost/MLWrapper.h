#ifndef MLWRAPPER_H
#define MLWRAPPER_H

#include <unordered_map>
#include <vector>

class MLWrapper{
public:
  
  MLWrapper(){};
  virtual ~MLWrapper(){};

  virtual bool build(std::string fname, std::vector<std::string> const& varnames, float missing_entry_val) = 0;

};

#endif
