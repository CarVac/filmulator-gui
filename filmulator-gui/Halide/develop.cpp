Func develop(Func inputs, Expr crystalGrowthConst,
             Expr activeLayerThickness, Expr developerConsumptionConst,
             Expr silverSaltConsumptionConst, Expr timestep) {

  Expr cgc = crystalGrowthConst*timestep;
  Expr dcc = 2.0f*developerConsumptionConst / ( activeLayerThickness*3.0f);
  Expr sscc = silverSaltConsumptionConst * 2.0f;

  Func dCrystalRad;
  //                                            Silver Salt Density
  dCrystalRad(x,y,c) = inputs(x,y,DEVEL_CONC) * inputs(x,y,c+6) * cgc;

  Func dCrystalVol;
  //                                        Crystal Radius                  Active Crystals
  dCrystalVol(x,y,c) = dCrystalRad(x,y,c) * inputs(x,y,c) * inputs(x,y,c) * inputs(x,y,c+3);

  Func outputs;
  outputs(x,y,c) = select(
             c == CRYSTAL_RAD_R    , inputs(x,y,c) + dCrystalRad(x,y,0),
      select(c == CRYSTAL_RAD_G    , inputs(x,y,c) + dCrystalRad(x,y,1),
      select(c == CRYSTAL_RAD_B    , inputs(x,y,c) + dCrystalRad(x,y,2),

      select(c == DEVEL_CONC       , max(0,inputs(x,y,c) -
                                     dcc*(dCrystalVol(x,y,0) + dCrystalVol(x,y,1) + dCrystalVol(x,y,2))),

      select(c == SILVER_SALT_DEN_R, inputs(x,y,c) - sscc*dCrystalVol(x,y,0),
      select(c == SILVER_SALT_DEN_G, inputs(x,y,c) - sscc*dCrystalVol(x,y,1),
      select(c == SILVER_SALT_DEN_B, inputs(x,y,c) - sscc*dCrystalVol(x,y,2),

      //Otherwise (active crystals) output=input
                                     inputs(x,y,c))))))));

    Var x_outer, x_inner;
    outputs.split(x,x_outer,x_inner,4).reorder(x_inner,c,x_outer,y)
             .vectorize(x_inner).parallel(y);

    dCrystalVol.split(x,x_outer,x_inner,4).store_at(outputs,x_outer)
               .compute_at(outputs,x_outer).vectorize(x_inner);

    dCrystalRad.split(x,x_outer,x_inner,4).store_at(outputs,x_outer)
               .compute_at(outputs,x_outer).vectorize(x_inner);

  return outputs;
}
