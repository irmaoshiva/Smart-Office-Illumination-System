#ifndef NODE_H
#define NODE_H

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "Arduino.h"


#include "vector.h"

class Node {
  private:
  
  public:

    float rho;

    Vector <int> d = Vector <int> (2);
    Vector <int> d_av = Vector <int> (2);
    Vector <float> y = Vector <float> (2);
    Vector <float> k = Vector <float> (2);

    int n;
    int m;

    int c;
    int o;
    int L;

    Node();
};

#endif //NODE_H
