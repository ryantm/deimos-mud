#include <math.h>
#include <stdlib.h>

#ifndef PI
#define PI 3.141592653589793238462643
#endif


double exp(double x);
double pow(double x, double y);
int rand(void);

float np(float x, float mean, float stddev)
{
  return 1.0/(sqrt(2.0 * PI) * stddev) * exp( (-1 * pow(x - mean, 2)/(2.0 * stddev * stddev)) );
}

float normal(float mean, float stddev)
{
  float min   = mean - stddev * 7.0;
  float max   = mean + stddev * 7.0;
  float incr  = min;
  float range = (max - min)/10000.0;
  float perc  = 0;

  while (incr < max)
    {
      perc = np(incr,mean,stddev) * range;
      if ( rand() < perc)
	{
	  return incr;
	}
      incr += range;
    }
  return incr;
}


int main()
{
  int i = 0;
  for (; i< 20; i++)
    printf("%f", normal(10, 10));
  return 1;
}
