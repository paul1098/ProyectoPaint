#include "Punto.h"


Punto::Punto(){

}

Punto::Punto(int x, int y, float r, float g, float b){
    newX = x;
    newY = y;
    newR = r;
    newG = g;
    newB = b;
}

Punto::~Punto(){

}
int Punto::getX(){
    return newX;
}

int Punto::getY(){
    return newY;
}

float Punto::getR(){
    return newR;
}

float Punto::getG(){
    return newG;
}

float Punto::getB(){
    return newB;
}

void Punto::setPosition(int x, int y){
    newX = x;
    newY = y;
}
void Punto::setColour(float r, float g, float b){
    newR = r;
    newG = g;
    newB = b;
}
