#ifndef VECTOR_H
#define VECTOR_H

template <class T>
class Vector {
  private:

    T* elem;

    //elem points to an array of sz Ts
    int sz; //size of the vector

  public:

    Vector(int s) //constructor
      : elem{new T[s]}, //acquire resources
    sz{s}
    {
      for (int i = 0; i != s; ++i) elem[i] = 0;
    }

    ~Vector() { //destructor
      delete[] elem; //release resources
    }

    int size() const {
      return sz;
    }

    T& operator[](int i) {
      if (i >= sz)
        Serial.println("Out of bound access!");

      return elem[i];
    }

    Vector& operator=( const Vector& a ) {
      if (this != &a) { //check self assignment
        T* p = new T[ a.sz ];
        for ( int i = 0; i != a.sz; ++i )
          p[ i ] = a.elem[ i ];
        delete[] elem; //delete old elements
        elem = p;
        sz = a.sz;
      }

      return *this;
    }

    T& operator*( const Vector& v1 ) {
      T& aux;

      if ( this.sz != v1.sz ) {
        Serial.println("Different lenghts!");
      }
      else {
        for ( int i = 0; i != v1.sz; i++ ) {
          aux = aux + (this.elem[i] * v1.elem[i]);
        }
      }

      return aux;
    }
};

#endif //VECTOR_H
