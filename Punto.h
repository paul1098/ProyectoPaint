
#ifndef Punto_H
#define Punto_H

#include <iostream>
#include <string>
using namespace std;

class Punto
{
public:
  Punto();
  Punto(int, int, float, float, float);
  ~Punto();

  int getX();
  int getY();
  float getR();
  float getG();
  float getB();

  void setPosition(int, int);
  void setColour(float, float, float);

private:
  int newX;
  int newY;
  float newR;
  float newG;
  float newB;
};

#endif
