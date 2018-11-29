#include "node.h"

using namespace std;

Node::Node(){
    rho = 0;

    for( int i = 0; i < 2; i++ ) {
      d[i] = 0;
      d_av[i] = 0;
      y[i] = 0;
    }
    
    k[0] = 2;
    k[1] = 1;

    n = k.getNorm();
    m = n - pow( k[1], 2 );

    c = 1;
    o = 50;
    L = 80;
}
