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

    double rho;

    Vector <float> d = Vector <float> (2);
    Vector <float> d_av = Vector <float> (2);
    Vector <float> y = Vector <float> (2);
    Vector <float> aux_soma = Vector <float>(2);

    float n;
    int m;
    float cost_best;
    int c;
    int o;
    int L;
    int index;

    Node();
    Node( float _rho, int _c, int _o, int _L, int _index );
    void Primal_solve( Vector <float>& _k );
    bool check_feasibility( Vector <float>& _d_solut, Vector <float>& _k );
    float evaluate_cost( Vector <float>& _d_solut );
    void updateBestCost( Vector <float>& _d_solut, Vector <float>& _k );

};

#endif //NODE_H
