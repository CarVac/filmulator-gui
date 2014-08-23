//Compile with:
//g++ vibranceSaturation.cpp -g -I include/ -L bin/ -lHalide `libpng-config --cflags --ldflags` -lpthread -ldl -o vibranceSaturation
//Run With:
//LD_LIBRARY_PATH=bin ./vibranceSaturation 
#include<Halide.h>
#include<stdio.h>
#include<iostream>
#include<sys/time.h>
using Halide::Image;
#include <image_io.h>
using namespace Halide;

Halide::Func hsv(Func in)
{
  Func out;
  Var x,y,c;
  Func minC,maxC,delta;
  minC(x,y) = min(in(0,x,y),min(in(1,x,y),in(2,x,y)));
  maxC(x,y) = max(in(0,x,y),max(in(1,x,y),in(2,x,y)));
  out(c,x,y) = 0.0f;
  out(2,x,y) = maxC(x,y);//V
  delta(x,y) = maxC(x,y) - minC(x,y);
  out(1,x,y) = select(maxC(x,y) != 0, delta(x,y)/maxC(x,y), 0);//S
  Func h;
  h(x,y) = select(maxC(x,y) == in(0,x,y), //R is highest
               (in(1,x,y) - in(2,x,y))/delta(x,y),
               select(maxC(x,y) == in(1,x,y), //G is highest
                 2 + (in(2,x,y) - in(0,x,y))/delta(x,y),
                 4 + (in(0,x,y) - in(1,x,y))/delta(x,y)));//B is highest
  out(0,x,y) = select(h(x,y) < 0, 60.0f*h(x,y) + 360.0f, 60.0f*h(x,y));
  return out;
}

Halide::Func rgb(Func in)
{
  Func out;
  Var x,y,c;
  Func h,s,v;
  h(x,y) = in(0,x,y);
  s(x,y) = in(1,x,y);
  v(x,y) = in(2,x,y);
  Func r,g,b;
  Func hd,i,f,p,q,t;
  hd(x,y) = h(x,y)/60;
  i(x,y) = Halide::floor(hd(x,y));
  f(x,y) = hd(x,y) - i(x,y);
  p(x,y) = v(x,y)*(1-s(x,y));
  q(x,y) = v(x,y)*(1-(s(x,y)*f(x,y)));
  t(x,y) = v(x,y)*(1-(s(x,y)*(1-f(x,y))));

  r(x,y) = select(i(x,y) == 0 || i(x,y) == 5,
      v(x,y),
      select(i(x,y) == 1,
        q(x,y),
        select(i(x,y) == 2 || i(x,y) == 3,
          p(x,y),
          select(i(x,y) == 4,
            t(x,y),
            v(x,y)))));//default
  g(x,y) = select(i(x,y) == 0,
      t(x,y),
      select(i(x,y) == 1 || i(x,y) == 2,
        v(x,y),
        select(i(x,y) == 3,
          q(x,y),
          p(x,y))));//4,5,default
  b(x,y) = select(i(x,y) == 0 || i(x,y) == 1,
      p(x,y),
      select(i(x,y) == 2,
        t(x,y),
        select(i(x,y) == 3 || i(x,y) == 4,
        v(x,y),
        q(x,y))));//5,default
  out(c,x,y) = select(s(x,y) != 0,
                select(c == 0, r(x,y), select(c == 1, g(x,y), b(x,y))),
                v(x,y));
  //out(x,y,c) = h(x,y)/360;
  return out;
}

int main(int argc, char **argv)
{
  Image<uint8_t> input = load<uint8_t>("P1070046.png");

  timeval t1, t2;
  gettimeofday(&t1, NULL);
  Var x,y,c;
  Func toFloat;
  toFloat(c,x,y) = cast<float>(input(x,y,c))/255.0;
  Func toHSV;
  toHSV = hsv(toFloat);
  Func saturated;
  saturated(c,x,y) = select(c != 1,
                            toHSV(c,x,y),
                            clamp(1*fast_pow(toHSV(c,x,y),0.5),
                                  0,1));
  Func toRGB,toInt;
  toRGB = rgb(saturated);
  toInt(x,y,c) = cast<uint8_t>(toRGB(c,x,y)*255.0);
  Var y_outer,y_inner;
  toInt.reorder(c,x,y);
  toInt.split(y,y_outer, y_inner, 256);
  toInt.parallel(y_outer);
  toHSV.compute_at(toInt,x);
  Halide::Image<uint8_t> output = toInt.realize(input.width(),input.height(),input.channels());
  gettimeofday(&t2, NULL);
  save(output,"vibSat.png");
  std::cout<<float(t2.tv_sec - t1.tv_sec) + float(t2.tv_usec - t1.tv_usec)/1000000.0f << std::endl;
  return 0;
}

