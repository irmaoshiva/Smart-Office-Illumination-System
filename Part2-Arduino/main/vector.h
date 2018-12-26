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

    T& operator[](byte i) {
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

    //produto de um vetor a uma constante (sem mudar o this)
    Vector operator*( double a ) {
      Vector <T> aux( this->sz );

      for ( int i = 0; i != sz; ++i )
        aux[i] = elem[i] * a;

      return aux;
    }


    //produto interno de vetores
    T operator*( const Vector& v1 ) {
      T aux = 0;

      if ( this->sz != v1.sz ) {
        Serial.println("Different lenghts!");
      }
      else {
        for ( int i = 0; i != v1.sz; i++ ) {
          aux = aux + (this->elem[i] * v1.elem[i]);
        }
      }

      return aux;
    }

    Vector operator-( const Vector& a ) {

      Vector <T> aux(this->sz);

      for ( int i = 0; i < this-> sz; i++ )
        aux[i] = elem[i] - a.elem[i];

      return aux;
    }

    Vector operator+( const Vector& a ) {

      Vector aux(this->sz);

      for ( int i = 0; i < this-> sz; i++ )
        aux[i] = elem[i] + a.elem[i];

      return aux;
    }


    float quad_norm( )
    {
      int i;
      float sum = 0;

      for (i = 0; i < sz; i++)
        sum = sum + pow(elem[i], 2);

      return sum;
    }

    float norm( )
    {
      int i;
      float sum = 0;

      for (i = 0; i < sz; i++)
        sum = sum + pow(elem[i], 2);

      return pow(sum, 0.5);
    }
};

#endif //VECTOR_H
