#include "node.h"

using namespace std;

Node::Node() {
  rho = 0;

  d_best[0] = 0;
  d_best[1] = 0;
  k[0] = 0;
  k[1] = 0;
  y[0]=0;
  y[1]=0;

  n = 0;
  m = 0;

  cost_best = 0;

  c = 0;
  o = 0;
  L = 0;
  index = 0;
}                                                                                                                                                                                                                                                                                          

Node::Node(float _rho, int _c, int _o, int _L, int _index) {
  rho = _rho;

  d_best[0] = -1;
  d_best[1] = -1;
  k[0] = 2;
  k[1] = 1;
  y[0]=0;
  y[1]=0;

  n = k.quad_norm();
  m = n - pow( k[0], 2 );

  cost_best = 10000.0;

  c = _c;
  o = _o;
  L = _L;
  index = _index;
}                                                                                                                                                                                                                                                                                          

bool Node::check_feasibility( Vector <float>& _d_solut ) {

  float tol = 0.001;

  if ( _d_solut[index] < (0 - tol) )
    return false;
  if ( _d_solut[index] > (100 + tol) )
    return false;
  if ( ( _d_solut * k ) < ( L - o - tol ) )
    return false;

  return true;
}

float Node::evaluate_cost(Vector <float>& _d_solut) {

  Vector <float> aux = _d_solut - d_av;

  return ( ( c * _d_solut[index] ) + ( y * aux ) + ( ( rho / 2 ) * aux.quad_norm() ) );
}

void Node::updateBestCost( Vector <float>& _d_solut ) {

  Serial.print("check_feasibility: ");
  Serial.println(check_feasibility(_d_solut));

  if ( check_feasibility( _d_solut ) )
  {
    float cost_sol = evaluate_cost( _d_solut );
    Serial.print("cost_sol: ");
    Serial.println(cost_sol);
    if ( cost_sol < cost_best )
    {

      //tem que se fazer uma função que faça isto..
     d=_d_solut;

      //d.floatToUint(_d_solut);

      cost_best = cost_sol;
    }
  }
}

void Node::Primal_solve() {
/*
  Serial.print("d_av[0]: ");
  Serial.println(d_av[0]);
  Serial.print("d_av[1]: ");
  Serial.println(d_av[1]);

  Serial.print("y[0]: ");
  Serial.println(y[0]);
  Serial.print("y[1]: ");
  Serial.println(y[1]); */

  cost_best=10000.0;
  Vector <float> z = d_av * rho - y;

  z[index] = z[index] - c;
/*
  Serial.print("z[0]: ");
  Serial.println(z[0]);
  Serial.print("z[1]: ");
  Serial.println(z[1]);
*/
  //--------------- unconstrained minimum ---------------
  Serial.println("--------------- sol_unconstrained ---------------");

  Vector <float> d_solut = z * ( 1 / rho );

  updateBestCost(d_solut);
/*
  Serial.print("d_solut[0]: ");
  Serial.println(d_solut[0]);
  Serial.print("d_solut[1]: ");
  Serial.println(d_solut[1]); */

  //--------------- compute minimum constrained to linear boundary ---------------
  Serial.println("--------------- sol_linear_boundary ---------------");

  d_solut = ( z * ( 1 / rho ) ) - ( ( k * (1 / n) ) * ( o - L + ( ( 1 / rho ) * ( k * z ) ) ) );

  updateBestCost(d_solut);

  /* Serial.print("d_solut[0]: ");
  Serial.println(d_solut[0]);
  Serial.print("d_solut[1]: ");
  Serial.println(d_solut[1]); */

  //--------------- compute minimum constrained to 0 boundary ---------------
  Serial.println("--------------- sol_boundary_0 ---------------");

  d_solut = z * ( 1 / rho );
  d_solut[index] = 0;

  updateBestCost(d_solut);
/*
  Serial.print("d_solut[0]: ");
  Serial.println(d_solut[0]);
  Serial.print("d_solut[1]: ");
  Serial.println(d_solut[1]); */

  //--------------- compute minimum constrained to 100 boundary ---------------
  Serial.println("--------------- sol_boundary_100 ---------------");

  d_solut = z * ( 1 / rho );
  d_solut[index] = 100;

  updateBestCost(d_solut);
/*
  Serial.print("d_solut[0]: ");
  Serial.println(d_solut[0]);
  Serial.print("d_solut[1]: ");
  Serial.println(d_solut[1]); */

  //--------------- compute minimum constrained to linear and 0 boundary ---------------
  Serial.println("--------------- sol_linear_0 ---------------");

  d_solut = ( ( z * ( 1 / rho ) ) - ( k * ( ( 1 / m ) * ( o - L ) ) ) + ( k * ( ( 1 / rho ) * ( 1 / m ) * ( ( k[index] * z[index] ) - ( z * k ) ) ) ) );
  d_solut[index] = 0;

  updateBestCost(d_solut);
/*
  Serial.print("d_solut[0]: ");
  Serial.println(d_solut[0]);
  Serial.print("d_solut[1]: ");
  Serial.println(d_solut[1]); */

  //--------------- compute minimum constrained to linear and 100 boundary ---------------
  Serial.println("--------------- sol_linear_100 ---------------");

  d_solut = ( ( z * ( 1 / rho ) ) - ( ( k * (o - L + ( 100 * k[index] ) ) ) * ( 1 / m ) ) + ( k * ( ( 1 / rho ) * ( 1 / m ) * ( ( k[index] * z[index] ) - ( z * k ) ) ) ) );
  d_solut[index] = 100;

  updateBestCost(d_solut);

  /*Serial.print("d_solut[0]: ");
  Serial.println(d_solut[0]);
  Serial.print("d_solut[1]: ");
  Serial.println(d_solut[1]); */

  Serial.println("--------------- CONSENSUS SOLUTION ---------------");

  Serial.print("d[0]: ");
  Serial.println(d[0]);
  Serial.print("d[1]: ");
  Serial.println(d[1]);
  Serial.print("cost_best: ");
  Serial.println(cost_best);

  /*
    Serial.print("rho: ");
    Serial.println(rho);
    Serial.print("z[0]: ");
    Serial.println(z[0]);
    Serial.print("z[1]: ");
    Serial.println(z[1]);
    Serial.print("k[0]: ");
    Serial.println(k[0]);
    Serial.print("k[1]: ");
    Serial.println(k[1]);
    Serial.print("n: ");
    Serial.println(n);
    Serial.print("m: ");
    Serial.println(m);
    Serial.print("o: ");
    Serial.println(o);
    Serial.print("L: ");
    Serial.println(L);
  */

}
