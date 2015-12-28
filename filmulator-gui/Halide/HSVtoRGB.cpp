Halide::Func HSVtoRGB(Func in)
{
  Func out;
  Var x,y,c;
  Func h,s,v;
  h(x,y) = in(x,y,0);
  s(x,y) = in(x,y,1);
  v(x,y) = in(x,y,2);
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
  out(x,y,c) = select(s(x,y) == 0,
                      v(x,y),
                      select(c == 0, r(x,y),
                             c == 1, g(x,y),
                                     b(x,y)));
  return out;
}

