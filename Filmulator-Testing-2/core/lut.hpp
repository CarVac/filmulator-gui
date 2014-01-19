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
#define maxval 65536

class LUT
{
private:
	int table[maxval];
public:
	void fill(int (*tonecurve)(int))
	{
		for(int i = 0; i < maxval; i++)
			table[i] = (*tonecurve)(i);
	}
	
	int operator[](float index)
	{
		if (index > (maxval-1))
		    return table[maxval-1];
		if (index < 0)
		{
		    return table[0];
		}//Implicit else because everything in if must return*/
		return table[(int)index];
	}
};
