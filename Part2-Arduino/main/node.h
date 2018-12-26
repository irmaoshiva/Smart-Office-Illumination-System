#ifndef NODE_H
#define NODE_H

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "Arduino.h"
#include "vector.h"
#define N 2

class Node {
  private:

  public:

    double rho;

    Vector <float> d = Vector <float> (N);
    Vector <float> d_av = Vector <float> (N);
    Vector <float> y = Vector <float> (N);
    Vector <float> aux_soma = Vector <float>(N);
    Vector <float> z  = Vector <float>(N);
     Vector <float> d_solut = Vector <float>(N);

    float n;
    float m;
    float cost_best;
    int c;
    float o;
    float L;
    int index;

    Node();
    Node( float _rho, int _c );
    void consensus_init( int _nr_nos );
    uint8_t check_feasibility( Vector <float>& _d_solut, Vector <float>& _k );
    float evaluate_cost( Vector <float>& _d_solut );
    void updateBestCost( Vector <float>& _d_solut, Vector <float>& _k );
    void Primal_solve( Vector <float>& _k );
};

#endif //NODE_H
