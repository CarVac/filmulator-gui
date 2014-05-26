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

void rotate_image(matrix<unsigned short> &input, matrix<unsigned short> &output,
                  int rotation)
{
    int nrows, ncols;
    nrows = input.nr();
    ncols = input.nc();

    switch(rotation)
    {
        case 2://upside down
            output.set_size(nrows,ncols);
            for(int i = 0; i < nrows; i++)
            {
                // out   in
                /* 12345 00000
                 * 00000 00000
                 * 00000 54321
                 */
                //Reversing the row index
                int r = nrows-1-i;
                for(int j = 0; j < ncols; j = j + 3)
                {
                    //Reversing the column index
                    int c = ncols - 3 - j;
                    output(i,j  ) = input(r,c  );
                    output(i,j+1) = input(r,c+1);
                    output(i,j+2) = input(r,c+2);
                }
            }
            break;
        case 3://right side down
            output.set_size(ncols/3,nrows*3);
            for(int j = 0; j < ncols/3; j++)
                {
                    //index of an output row as a column on the input matrix
                    // out   in
                    /* 123   30000
                     * 000   20000
                     * 000   10000
                     * 000
                     * 000
                     */
                    //Remember that the columns are interlaced.
                    int c = j*3;
                    for(int i = 0; i < nrows; i++)
                    {
                        //Also, in this case, the order is reversed for the rows of the input.
                        int r = nrows-1-i;
                        output(j,i*3  ) = input(r,c  );
                        output(j,i*3+1) = input(r,c+1);
                        output(j,i*3+2) = input(r,c+2);
                    }
                }
            break;
        case 1://left side down
            output.set_size(ncols/3,nrows*3);
            for(int j = 0; j < ncols/3; j++)
            {
                //index of an output row as a column on the input matrix
                // out   in
                /* 123   00001
                 * 000   00002
                 * 000   00003
                 * 000
                 * 000
                 */
                //Remember that the columns are interlaced, and scanned in reverse.
                int c = ncols - 3 - j*3;
                for(int i = 0; i < nrows; i++)
                {
                    //Here the order is not reversed for the rows of the input.
                    int r = i;
                    output(j,i*3  ) = input(r,c  );
                    output(j,i*3+1) = input(r,c+1);
                    output(j,i*3+2) = input(r,c+2);
                }
            }
            break;
        default:
            output = input;
    }

    return;
}
