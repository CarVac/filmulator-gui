Halide::Func RGBtoHSV(Func in)
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

