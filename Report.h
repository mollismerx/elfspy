#ifndef ELFSPY_REPORT_H
#define ELFSPY_REPORT_H

#include <sstream>

namespace spy
{

class Report : public std::ostringstream
{
public:
  Report();
  ~Report();

protected:
  void show();

private:
  int error_;
};

} // namespace spy

#endif
