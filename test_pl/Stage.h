// Represents an abstract base class for all stage objects
#ifndef _PL_STAGE_H_
#define _PL_STAGE_H_

#include <vector>
#include <tuple>

class Stage {
private:
  std::string name;

public:
  virtual bool run() = 0;
  virtual void update_data(std::vector<std::tuple<int,std::string>> data) = 0;
  virtual void print_pass_msg();
  virtual void print_fail_msg();
}

#endif
