/* 
 * This file is part of Filmulator.
 *
 * Copyright 2013 Omer Mano and Carlo Vaccari
 *
 * Filmulator is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Filmulator is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Filmulator. If not, see <http://www.gnu.org/licenses/>
 */
#include "filmSim.hpp"

bool ppmb_write_data ( ofstream &output, int xsize, int ysize,
  matrix<float> &densityr, matrix<float> &densityg, matrix<float> &densityb,
  bool sixteen_bit )

//****************************************************************************80
//
//  Purpose:
//
//    PPMB_WRITE_DATA writes the data for a binary portable pixel map file.
//
//  Licensing:
//
//    This code is distributed under the GNU LGPL license.
//
//  Modified:
//
//    11 April 2003
//
//  Author:
//
//    John Burkardt
//
//  Parameters:
//
//    Input, ofstream OUTPUT, a pointer to the file to contain the binary
//    portable pixel map data.
//
//    Input, int XSIZE, YSIZE, the number of rows and columns of data.
//
//    Input, unsigned char *R, *G, *B, the arrays of XSIZE by YSIZE
//    data values.
//
//    Output, bool PPMB_WRITE_DATA, is true if an error occurred.
//
{
    int  i;
    int  j;
    unsigned short out;
    unsigned char out2;
  
    if(sixteen_bit)
    {
      for ( j = 0; j < ysize; j++ )
      {
        for ( i = 0; i < xsize; i++ )
        {
          if(densityr(j,i) > 65535)
            out = 65535; //clip to white
          else if(densityr(j,i) < 0)
            out = 0; //clip to black
          else
          out = (unsigned short) densityr(j,i); //normal values
          out   = ((out   & 0x00ff)<<8)|((out   & 0xff00)>>8);
          output.write(reinterpret_cast<char*>(&out), sizeof(unsigned short));
          
          if(densityg(j,i) > 65535)
            out = 65535; //clip to white
          else if(densityg(j,i) < 0)
            out = 0; //clip to black
          else
            out = (unsigned short) densityg(j,i); //normal values
          out   = ((out   & 0x00ff)<<8)|((out   & 0xff00)>>8);
          output.write(reinterpret_cast<char*>(&out), sizeof(unsigned short));

          if(densityb(j,i) > 65535)
            out = 65535; //clip to white
          else if(densityb(j,i) < 0)
            out = 0; //clip to black
          else
          out = (unsigned short) densityb(j,i); //normal values
          out   = ((out   & 0x00ff)<<8)|((out   & 0xff00)>>8);
          output.write(reinterpret_cast<char*>(&out), sizeof(unsigned short));      
        }
      }
    }
    else //8 bit
    {
      for ( j = 0; j < ysize; j++ )
      {
        for ( i = 0; i < xsize; i++ )
        {
          if(densityr(j,i) > 255)
            out2 = 255; //clip to white
          else if(densityr(j,i) < 0)
            out2 = 0; //clip to black
          else
          out2 = (unsigned char) densityr(j,i); //normal values
          output.write(reinterpret_cast<char*>(&out2), sizeof(unsigned char));
          
          if(densityg(j,i) > 255)
            out2 = 255; //clip to white
          else if(densityg(j,i) < 0)
            out2 = 0; //clip to black
          else
          out2 = (unsigned char) densityg(j,i); //normal values
          output.write(reinterpret_cast<char*>(&out2), sizeof(unsigned char));

          if(densityb(j,i) > 255)
            out2 = 255; //clip to white
          else if(densityb(j,i) < 0)
            out2 = 0; //clip to black
          else
          out2 = (unsigned char) densityb(j,i); //normal values
          output.write(reinterpret_cast<char*>(&out2), sizeof(unsigned char));
        }  
      }
    }
    return false;
}
//****************************************************************************80

bool ppmb_write_header ( ofstream &output, int xsize, int ysize, int maxrgb )

//****************************************************************************80
//
//  Purpose:
//
//    PPMB_WRITE_HEADER writes the header of a binary portable pixel map file.
//
//  Licensing:
//
//    This code is distributed under the GNU LGPL license.
//
//  Modified:
//
//    11 April 2003
//
//  Author:
//
//    John Burkardt
//
//  Parameters:
//
//    Input, ofstream &OUTPUT, a pointer to the file to contain the binary
//    portable pixel map data.
//
//    Input, int XSIZE, YSIZE, the number of rows and columns of data.
//
//    Input, int MAXRGB, the maximum RGB value.
//
//    Output, bool PPMB_WRITE_HEADER, is true if an error occurred.
//
{
  output << "P6"   << " "
           << xsize  << " "
           << ysize  << " "
           << maxrgb << "\n";

  return false;
}


void imwrite(matrix<float> &densityr, matrix<float> &densityg,
    matrix<float> &densityb, string output_name, bool sixteen_bit )
{
  bool error;
  ofstream output;
  int maxrgb;
  int xsize = densityr.nc();
  int ysize = densityr.nr();
//
//  Open the output file.
//
  output.open ( output_name.c_str( ), ios::binary );

  if ( !output )
  {
    cout << "\n";
    cout << "PPMB_WRITE: Fatal error!\n";
    cout << "  Cannot open the output file " << output_name << ".\n";
    return;
  }
//
//  Compute the maximum.
//
  if ( sixteen_bit)
    maxrgb = 65535;
  else
    maxrgb = 255;
//
//  Write the header.
//
  error = ppmb_write_header ( output, xsize, ysize, maxrgb );

  if ( error )
  {
    cout << "\n";
    cout << "PPMB_WRITE: Fatal error!\n";
    cout << "  PPMB_WRITE_HEADER failed.\n";
    return;
  }
//
//  Write the data.
//
  error = ppmb_write_data ( output, xsize, ysize, densityr, densityg, densityb,
    sixteen_bit);

  if ( error )
  {
    cout << "\n";
    cout << "PPMB_WRITE: Fatal error!\n";
    cout << "  PPMB_WRITE_DATA failed.\n";
    return;
  }
//
//  Close the file.
//
  output.close ( );

  return;
}
