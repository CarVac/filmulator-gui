Halide::Func RGBtoHSV(Func in)
{
  Func out;
  Var x,y,c;
  Func minC,maxC,delta;
  Func h,s,v;
  minC(x,y) = min(in(x,y,0),min(in(x,y,1),in(x,y,2)));
  maxC(x,y) = max(in(x,y,0),max(in(x,y,1),in(x,y,2)));
  v(x,y) = maxC(x,y);
  delta(x,y) = maxC(x,y) - minC(x,y);
  s(x,y) = select(maxC(x,y) != 0, delta(x,y)/maxC(x,y), 0);//S
  h(x,y) = select(maxC(x,y) == in(0,x,y), //R is highest
               (in(1,x,y) - in(2,x,y))/delta(x,y),
               select(maxC(x,y) == in(1,x,y), //G is highest
                 2 + (in(2,x,y) - in(0,x,y))/delta(x,y),
                 4 + (in(0,x,y) - in(1,x,y))/delta(x,y)));//B is highest
  Func hNorm;
  hNorm(x,y) = select(h(x,y) < 0, 60.0f*h(x,y) + 360.0f, 60.0f*h(x,y));
  out(x,y,c) = select(c == 0, hNorm(x,y),
                      c == 1, s(x,y),
                              v(x,y));
  return out;
}

