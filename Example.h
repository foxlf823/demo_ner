
#ifndef SRC_EXAMPLE_H_
#define SRC_EXAMPLE_H_
#include <vector>
#include "N3L.h"

using namespace std;

class Example {

public:

  vector<int> _nerLabels;

  vector<int> _words;

  int _current_idx;


public:
  Example(bool isrel)
  {

	  _current_idx = -1;

  }




};

#endif /* SRC_EXAMPLE_H_ */
