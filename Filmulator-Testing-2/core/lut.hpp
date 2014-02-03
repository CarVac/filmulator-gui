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
#define MAXVAL 65536
#include <algorithm>
#include "interface.h"

using namespace std;

class LUT
{
private:
    unsigned short table[MAXVAL];
    bool unity;
    bool linear;
    float slope;
    float y_intercept;
    float brightest;
    float darkest;
public:
    void setLinear(float slope_in, float y_intercept_in,
                   float brightest_in, float darkest_in)
    {
        linear = true;
        unity = false;
        slope = slope_in;
        y_intercept = y_intercept_in;
        brightest = brightest_in;
        darkest = darkest_in;
    }

    void setUnity()
    {
        linear = false;
        unity = true;
    }

    bool isUnity()
    {
        return unity;
    }

    void fill(Interface *interface)
	{
        linear = false;
        unity = false;

        for(int i = 0; i < MAXVAL; i++)
            table[i] = interface->lookup(i);
	}
	
    unsigned short operator[](unsigned short index)
	{
        if (unity)
            return index;
        if (linear)
            return min(max((index*slope)+y_intercept,darkest),brightest);
        return table[index];
	}
};
