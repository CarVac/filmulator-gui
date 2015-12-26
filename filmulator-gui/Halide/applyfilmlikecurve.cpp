#include <cfloat>
Func applyFilmlikeCurve(Func input, Func LUT){

    Var x,y,c,o;
    Func order;
    Expr rVal = input(x,y,0), gVal = input(x,y,1), bVal = input(x,y,1);
    Expr r = 0, g = 1, b = 2;
    order(x,y,o) =  select(rVal >= gVal,
                           select(gVal >= bVal,
                                         select(o == 0, b, o == 1, g, r),    //Order is BGR
                                  select(rVal >= bVal,
                                         select(o == 0, g, o == 1, b, r),    //GBR
                                         select(o == 0, g, o == 1, r, b))),  //GRB
                           select(rVal >= bVal,
                                         select(o == 0, b, o == 1, r, g),    //BRG
                                  select(gVal >= bVal,
                                         select(o == 0, r, o == 1, b, g),    //RBG
                                         select(o == 0, r, o == 1, g, b)))); //RGB
    Func curved;
    Expr maxVal = 2^16;
    curved(x,y,o) = LUT(cast(UInt(16),maxVal*input(x,y,order(x,y,o))));
    
    Expr lowOld = input(x,y,order(x,y,0));
    Expr midOld = input(x,y,order(x,y,1));
    Expr hiOld  = input(x,y,order(x,y,2));

    Expr lowNew = curved(x,y,0);
    Expr hiNew  = curved(x,y,2);

    Expr epsilon = FLT_MIN;
    Expr multFactor = (hiNew - lowNew + epsilon)/(hiOld - lowOld + epsilon);
    Expr correction = (lowNew - multFactor*lowOld);
    curved(x,y,1) = multFactor*midOld + correction;

    Func output;
    output(x,y,c) = undef<float>();
    output(x,y,order(x,y,0)) = curved(x,y,0);
    output(x,y,order(x,y,1)) = curved(x,y,1);
    output(x,y,order(x,y,2)) = curved(x,y,2);

    return output;
}
                
