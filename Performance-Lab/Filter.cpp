#include "Filter.h"

Filter::Filter(int _dim)
{
  divisor = 1;
  dim = _dim;
  data = new int[dim * dim];
}


void Filter::set(int r, int c, int value)
{
  data[ r * dim + c ] = value;
}

int Filter::getDivisor()
{
  return divisor;
}

void Filter::setDivisor(int value)
{
  divisor = value;
}

int Filter::getSize()
{
  return dim;
}
