#ifndef VECTOR_H
#define VECTOR_H

class Vector {
    double ∗ elem;
    //elem points to an array of sz doubles
    int sz; //size of the vector
  public:
    Vector(int s) //constructor
      : elem{new double[s]}, //acquire resources
    sz{s}
    {
      for (int i = 0; i != s; ++i) elem[i] = 0;
    }

    ~Vector() { //destructor
      delete[] elem; //release resources
    }

    double& operator[](int i) {
      return elem[i];
    }

    int size() const {
      return sz;
    }

    double& operator*(Vector v1, Vector v2) {
      Vector aux_v;
      double& aux_d;

      if (v1.size() != v2.size()) {
        Serial.println("Different lenghts!");
      }
      else {
        for (int i = 0; i != v1.size(); i++) {
          aux_d = aux_d + (v1[i] * v2[i]);
        }
      }
      
      return aux_d;
    }
};

#endif //VECTOR_H
