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
#include "filmsim.hpp"
#include <pwd.h>
#include <unistd.h>

int read_args(int argc, char* argv[],string &input_configuration,
               std::vector<string> &input_filename_list,
               std::vector<float> &input_exposure_compensation, int &hdr_count,
               bool &hdr, bool &tiff, bool &jpeg_in, bool &set_whitepoint,
               float &whitepoint, bool &jpeg_out, bool &tonecurve_out,
               int &highlights)
{
    int param_count = 1; //first param is program name
    bool exposure_specified = false;
    bool configuration_set = false;
    for(int i=1; i < argc; i++)
    {
        if (!strcmp(argv[i],"-c")) //If argv[i] is '-c'
        {
            i++;
            if (i < argc) //There is an argument right afterwards
            {
                input_configuration = argv[i];
                configuration_set = true;
            }
            else
            {
                cout << "Configuration file not specified.\n";
                return 1;
            }
			param_count += 2;
        }
		else if (!strcmp(argv[i],"-t"))
		{
			if(jpeg_in)
			{
				cout << "Input cannot be both tiff and jpeg.\n";
				return 1;
			}
			tiff = true;
			param_count++;
		}
		else if (!strcmp(argv[i],"-ji"))
		{
			if(tiff)
			{
				cout << "Input cannot be both tiff and jpeg.\n";
				return 1;
			}
			jpeg_in = true;
			param_count++;
		}
        else if (!strcmp(argv[i],"-h")) //HDR
        {
            hdr = true;
		    exposure_specified = true;
		    if (i+1 < argc)
            {
                hdr_count = atoi(argv[i+1]);
            }
            else
            {
                cout << "Insufficient arguments for HDR.\n";
                return 1;
            }
			if(hdr_count < 2)
			{
				cout << "HDR must use two or more images. Make sure number of images is specified after \"-h\".\n";
				return 1;
			}
            highlights = 0;
			param_count +=2;
        }
        else if(!strcmp(argv[i],"-w")) //Set a white point
        {
            set_whitepoint = true;
		    if (i+1 < argc)
            {
                char * pEnd;
                whitepoint = strtof(argv[i+1],&pEnd)/1000; //User inputs a value
			    if(pEnd == argv[i])                      //100x greater than
			    {                                        //real whitepoint
				    cout << "Invalid white point specified.\n";
				    return 1;
			    }
            }
            else
            {
                cout << "White point not set\n";
                return 1;
            }
            param_count += 2;
        }
        else if(!strcmp(argv[i],"-jo")) //JPEG output
        {
        	jpeg_out = true;
        	param_count++;
        }
        else if(!strcmp(argv[i],"-n")) //Use tonecurve
        {
        	tonecurve_out = true;
        	param_count++;
        }
        else if(!strcmp(argv[i],"-l")) //Set dcraw highlight recovery
        {
            if (i+1 < argc)
            {
                highlights = atoi(argv[i+1]);
            }
            else
            {
                cout << "Highlight recovery parameter missing" << endl;
                return 1;
            }
            if(highlights<0 || highlights>9)
            {
                cout << "Highlight recovery parameter out of range; " <<
                    "Overriding to zero" << endl;
                highlights = 0;
            }
            param_count += 2;
        }
    }
	//Check that we have the correct number of arguments
    if (hdr)
	{
		if(param_count + 2*hdr_count > argc)
		{
			cout << "Not enough images specified. For HDR, make sure each image has an exposure given.\n";
			return 1;
		}
		if(param_count + 2*hdr_count < argc)
		{
			cout << "Too many arguments.\n";
			return 1;
		}
	}
	else
	{
		if(param_count + 1 > argc)
		{
			cout << "Image not specified.\n";
			return 1;
		}
		else if(param_count + 1 == argc)
		{
			exposure_specified = false;
		}
		else if(param_count + 2 == argc)
		{
			exposure_specified = true;
		}
		else if(param_count + 2 < argc)
		{
			cout << "too many arguments.\n";
			return 1;
		}
	}
    if(configuration_set == false)
    {
        cout << "Using default configuration location" << endl;
        //Here we need to grab the user's home directory
        int myuid = getuid();
        passwd *mypasswd = getpwuid(myuid);
        input_configuration = string(mypasswd->pw_dir) +
            "/.filmulator/configuration.txt";
    }

	for(int i = param_count; i < argc; i++)
	{
        //Load the filenames onto list.
		input_filename_list.push_back(argv[i]);

		if(exposure_specified)
		{
			char * pEnd;
			float exposure_long;
			float exposure;
			i++;
			exposure_long = strtof(argv[i],&pEnd);
			//exposure_long = strtof(argv[i],&pEnd,10); didn't work
            //If strtof gives an error and outputs nothing, then pEnd
            //will equal the starting position, argv[i].
			if(pEnd == argv[i])
			{
				cout << "Invalid exposure specified.\n";
				return 1;
			}
			exposure = exposure_long;//In the future, might want to do this safely, doesn't seem too necessary now
			input_exposure_compensation.push_back(exposure);
		}
		else
			input_exposure_compensation.push_back(0);
	}
    return 0;
}
