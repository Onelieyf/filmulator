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
#include <utility>
#include <omp.h>
#include "math.h"
#define ORDER 7 //This is the number of box blur iterations.
using namespace std;

//In real film, developer diffuses in a 2D gaussian on the film surface.
//Since 2D gaussians are separable, we perform two 1D gaussians in x and y.
//Since approximating a gaussian with a convolution is fairly computationally
//expensive, we approximate it using repeated box blurs. 

//Helper function to diffuse in x direction
void diffuse_x(matrix<float> &developer_concentration,
               int convlength,
               int convrad, int pad, int paddedwidth, int order,
               float swell_factor);

void diffuse(matrix<float> &developer_concentration, 
		     float sigma_const,
		     float pixels_per_millimeter,
             float timestep)
{
    //This is the standard deviation we want for the blur in pixels.
    float sigma = sqrt(timestep*pow(sigma_const*pixels_per_millimeter,2));
    int order = ORDER;
    
    int length = developer_concentration.nr();
    int width = developer_concentration.nc();

    //Length is the total size of the blur box determined by the desired
    //gaussian size and the number of iterations.
    int convlength = floor(sqrt(pow(sigma,2)*(12/ORDER)+1));
    if (convlength % 2 == 0)
    {
        convlength += 1;
    }

    //convrad is the radius of the convolution box. It's useful later on.
    int convrad = (convlength-1)/2;

    //We will be doing lots of averaging, but holding off on dividing by the
    //number of values averaged. The number of values averaged is always
    //<convlength>^<order>
    float swell_factor = 1.0/pow(convlength,order);

    //Here we allocate a matrix sized the same as a row, plus room for padding
    //mirrored from the internal content.
    int pad = order*convrad;
    int paddedwidth = 2*pad + width + 1;
    int paddedheight = 2*pad + length + 1;

    diffuse_x(developer_concentration,convlength,convrad,pad,paddedwidth,
              order, swell_factor);
    matrix<float> transposed_developer_concentration(width,length);
    developer_concentration.transpose_to(transposed_developer_concentration);
    diffuse_x(transposed_developer_concentration,convlength,convrad,pad,
              paddedheight, order, swell_factor);
    transposed_developer_concentration.transpose_to(developer_concentration);

    return;
}

void diffuse_x(matrix<float> &developer_concentration, int convlength,
               int convrad, int pad, int paddedwidth, int order,
               float swell_factor)
{
    int length = developer_concentration.nr();
    int width = developer_concentration.nc();

    vector<float> hpadded(paddedwidth); //stores one padded line
    vector<float> htemp(paddedwidth); // stores result of box blur

    //This is the running sum used to compute the box blur.
    float running_sum = 0;

    //These are indices for loops.
    int row = 0;
    int col = 0;
    int pass = 0;

#pragma omp parallel shared(developer_concentration,convlength,convrad,order,\
        length,width,paddedwidth,pad,swell_factor)\
    firstprivate(row,col,pass,htemp,running_sum,hpadded)
    {
#pragma omp for schedule(dynamic) nowait
        for (row = 0; row<length;row++)
        {
            //Mirror the start of padded from the row.
            for (col = 0; col < pad; col++)
            {
                hpadded[col] = developer_concentration(row,pad-col);
            }
            //Fill the center of padded with a row.
            for (col = 0; col < width; col++)
            {
                hpadded[col+pad] = developer_concentration(row,col);
            }
            //Mirror the row onto the end of padded.
            for (col = 0; col < pad + 1; col++)
            {
                hpadded[col+pad+width] = 
                    developer_concentration(row,width-2-col);
            }
            for (pass = 0; pass < order; pass++)
            {
                //Perform a box blur, but hold off on the divisions in the
                //averaging calculations
                
                //Start the running sum going at the beginning of the pad.
                running_sum = 0;
                for (col= pass*convrad; col < (pass*convrad)+convlength; col++)
                {
                    running_sum += hpadded[col];
                }

                //Start moving down the row.
                for (col = (pass+1)*convrad;
                     col < paddedwidth - 1 - (pass+1)*convrad;
                     col++)
                {
                    htemp[col] = running_sum;
                    running_sum = running_sum + hpadded[col+convrad+1];
                    running_sum = running_sum - hpadded[col-convrad];
                }
                
                //Copy what was in htemp to hpadded for the next iteration.
                //Swap just swaps pointers, so it is O(1)
                swap(htemp,hpadded);
            }
            //Now we're done with the convolution of one row, and we can copy
            //it back into developer_concentration. But we should also do the
            //divisions that we never did during our previous averaging. 
            for (col = 0; col < width; col++)
            {
                developer_concentration(row,col) =
                	hpadded[col+pad]*swell_factor;
            }
        }   
    }
}

//This uses a convolution forward and backward with a particular
// 4-length, 1-dimensional kernel to mimic a gaussian.
//In the first pass, it starts at 0, then goes out 4 standard deviations
// onto 0-clamped padding, then convolves back to the start.
//Naturally this attenuates the edges, so it does the same to all ones,
// and divides the image by that.
void diffuse_short_convolution(matrix<float> &developer_concentration,
                               const float sigma_const,
                               const float pixels_per_millimeter,
                               const float timestep)
{
    int height = developer_concentration.nr();
    int width = developer_concentration.nc();

    //Compute the standard deviation of the blur we want, in pixels.
    double sigma = sqrt(timestep*pow(sigma_const*pixels_per_millimeter,2));

    //We set the padding to be 4 standard deviations so as to catch as much as possible.
    int paddedWidth = width + 4*sigma + 3;
    int paddedHeight = height + 4*sigma + 3;

    double q;//constant for computing coefficients
    if (sigma < 2.5)
    {
        q = 3.97156 - 4.14554*sqrt(1 - 0.26891*sigma);
    }
    else
    {
        q = 0.98711*sigma - 0.96330;
    }

    double denom = 1.57825 + 2.44413*q + 1.4281*q*q + 0.422205*q*q*q;
    double coeff [4];

    coeff[1] = (2.44413*q + 2.85619*q*q + 1.26661*q*q*q)/denom;
    coeff[2] = (-1.4281*q*q - 1.26661*q*q*q)/denom;
    coeff[3] = (0.422205*q*q*q)/denom;
    coeff[0] = 1 - (coeff[1] + coeff[2] + coeff[3]);

    //We blur ones in order to cancel the edge attenuation.

    //First we do horizontally.
    vector<double> attenuationX(paddedWidth);
    //Set up the boundary
    attenuationX[0] = coeff[0]; //times 1
    attenuationX[1] = coeff[0] + coeff[1]*attenuationX[0];
    attenuationX[2] = coeff[0] + coeff[1]*attenuationX[1] + coeff[2]*attenuationX[0];
    //Go over the image width
    for (int i = 3; i < width; i++)
    {
        attenuationX[i] = coeff[0] + //times 1
                          coeff[1] * attenuationX[i-1] +
                          coeff[2] * attenuationX[i-2] +
                          coeff[3] * attenuationX[i-3];
    }
    //Fill in the padding (which is all zeros)
    for (int i = width; i < paddedWidth; i++)
    {
        //All zeros, so no coeff[0]*1 here.
        attenuationX[i] = coeff[1] * attenuationX[i-1] +
                          coeff[2] * attenuationX[i-2] +
                          coeff[3] * attenuationX[i-3];
    }
    //And go back.
    for (int i = paddedWidth - 3 - 1; i >= 0; i--)
    {
        attenuationX[i] = coeff[0] * attenuationX[i] +
                          coeff[1] * attenuationX[i+1] +
                          coeff[2] * attenuationX[i+2] +
                          coeff[3] * attenuationX[i+3];
    }

    for (int i = 0; i < width; i++)
    {
        if (attenuationX[i] <= 0)
        {
            std::cout << "gonna blow X" << std::endl;
        }
    }

    //And now vertically.
    vector<double> attenuationY(paddedHeight);
    //Set up the boundary
    attenuationY[0] = coeff[0]; //times 1
    attenuationY[1] = coeff[0] + coeff[1]*attenuationY[0];
    attenuationY[2] = coeff[0] + coeff[1]*attenuationY[1] + coeff[2]*attenuationY[0];
    //Go over the image height
    for (int i = 3; i < height; i++)
    {
        attenuationY[i] = coeff[0] + //times 1
                          coeff[1] * attenuationY[i-1] +
                          coeff[2] * attenuationY[i-2] +
                          coeff[3] * attenuationY[i-3];
    }
    //Fill in the padding (which is all zeros)
    for (int i = height; i < paddedHeight; i++)
    {
        //All zeros, so no coeff[0]*1 here.
        attenuationY[i] = coeff[1] * attenuationY[i-1] +
                          coeff[2] * attenuationY[i-2] +
                          coeff[3] * attenuationY[i-3];
    }
    //And go back.
    for (int i = paddedHeight - 3 - 1; i >= 0; i--)
    {
        attenuationY[i] = coeff[0] * attenuationY[i  ] +
                          coeff[1] * attenuationY[i+1] +
                          coeff[2] * attenuationY[i+2] +
                          coeff[3] * attenuationY[i+3];
    }

    for (int i = 0; i < height; i++)
    {
        if (attenuationY[i] <= 0)
        {
            std::cout << "gonna blow Y" << std::endl;
        }
    }


    //Set up variables for use inside loops
    int row, col, slice, offset, iter;

    //X direction blurring.
    //We slice by individual rows.
    vector<double> devel_concX(paddedWidth);
#pragma omp parallel shared(developer_concentration, height, width, coeff, attenuationX,\
    paddedWidth) firstprivate(devel_concX, row, col)
    {
#pragma omp for simd schedule(dynamic)
        for (row = 0; row < height; row++)
        {
            //Copy data into the temp.
            for (col = 0; col < width; col++)
            {
                devel_concX[col] = double(developer_concentration(row,col));
            }
            //Set up the boundary
            devel_concX[0] = coeff[0] * devel_concX[0];
            devel_concX[1] = coeff[0] * devel_concX[1] +
                             coeff[1] * devel_concX[0];
            devel_concX[2] = coeff[0] * devel_concX[2] +
                             coeff[1] * devel_concX[1] +
                             coeff[2] * devel_concX[0];
            //Iterate over the main part of the image, except for the setup
            for (col = 3; col < width; col++)
            {
                devel_concX[col] = coeff[0] * devel_concX[col  ] +
                                   coeff[1] * devel_concX[col-1] +
                                   coeff[2] * devel_concX[col-2] +
                                   coeff[3] * devel_concX[col-3];
            }
            //Iterate over the zeroed tail
            for (col = width; col < paddedWidth; col++)
            {
                devel_concX[col] = coeff[1] * devel_concX[col-1] +
                                   coeff[2] * devel_concX[col-2] +
                                   coeff[3] * devel_concX[col-3];
            }
            //And go back
            for (col = paddedWidth - 3 - 1; col >= 0; col--)
            {
                devel_concX[col] = coeff[0] * devel_concX[col  ] +
                                   coeff[1] * devel_concX[col+1] +
                                   coeff[2] * devel_concX[col+2] +
                                   coeff[3] * devel_concX[col+3];
            }
            //And undo the attenuation, copying back from the temp.
            for (col = 0; col < width; col++)
            {
                developer_concentration(row,col) = devel_concX[col]/attenuationX[col];
            }
        }
    }

    //Y direction blurring. We slice into columns a whole number of cache lines wide.
    //Each cache line is 8 doubles wide.
    matrix<double> devel_concY;
    int thickness = 8;//of the slice
    devel_concY.set_size(paddedHeight,thickness);
    int slices = ceil(width/float(thickness));
#pragma omp parallel shared(developer_concentration, height, width, coeff, attenuationY,\
    paddedHeight, slices, thickness) firstprivate(devel_concY, row, col, slice, offset, iter)
    {
#pragma omp for schedule(dynamic)
        for (slice = 0; slice < slices; slice++)
        {
            offset = slice*thickness;
            iter = thickness;//number of columns to loop through
            if (offset + thickness > width) //If it's the last row,
            {
                iter = width - offset;//Don't go beyond the bounds
            }

            //Copy data into the temp.
            if (iter < 8) //we can't SIMD this nicely
            {
                for (row = 0; row < height; row++)
                {
                    for (col = 0; col < iter; col++)
                    {
                        devel_concY(row,col) = developer_concentration(row,col+offset);
                    }
                }
            }
            else //we can simd this
            {
                for (row = 0; row < height; row++)
                {
#pragma omp simd
                    for (col = 0; col < 8; col++)
                    {
                        devel_concY(row,col) = developer_concentration(row,col+offset);
                    }
                }
            }

            //Set up the boundary.
#pragma omp simd
            for (col = 0; col < 8; col++)
            {
                devel_concY(0,col) = coeff[0] * devel_concY(0,col);
                devel_concY(1,col) = coeff[0] * devel_concY(1,col) +
                                     coeff[1] * devel_concY(0,col);
                devel_concY(2,col) = coeff[0] * devel_concY(2,col) +
                                     coeff[1] * devel_concY(1,col) +
                                     coeff[2] * devel_concY(0,col);
            }
            //Iterate over the main part of the image, except for the setup.
            for (row = 3; row < height; row++)
            {
#pragma omp simd
                for (col = 0; col < 8; col++)
                {
                    devel_concY(row,col) = coeff[0] * devel_concY(row  ,col) +
                                           coeff[1] * devel_concY(row-1,col) +
                                           coeff[2] * devel_concY(row-2,col) +
                                           coeff[3] * devel_concY(row-3,col);
                }
            }
            //Iterate over the zeroed tail
            for (row = height; row < paddedHeight; row++)
            {
#pragma omp simd
                for (col = 0; col < 8; col++)
                {
                    devel_concY(row,col) = coeff[1] * devel_concY(row-1,col) +
                                           coeff[2] * devel_concY(row-2,col) +
                                           coeff[3] * devel_concY(row-3,col);
                }
            }
            //And go back
            for (row = paddedHeight - 3 - 1; row >= 0; row--)
            {
#pragma omp simd
                for (col = 0; col < 8; col++)
                {
                    devel_concY(row,col) = coeff[0] * devel_concY(row  ,col) +
                                           coeff[1] * devel_concY(row+1,col) +
                                           coeff[2] * devel_concY(row+2,col) +
                                           coeff[3] * devel_concY(row+3,col);
                }
            }
            //And undo the attenuation, copying back from the temp.
            if (iter < 8) //we can't SIMD this nicely
            {
                for (row = 0; row < height; row++)
                {
                    for (col = 0; col < iter; col++)
                    {
                        developer_concentration(row,col+offset) = devel_concY(row,col)/attenuationY[row];
                    }
                }
            }
            else
            {
                for (row = 0; row < height; row++)
                {
#pragma omp simd
                    for (col = 0; col < 8; col++)
                    {
                        developer_concentration(row,col+offset) = devel_concY(row,col)/attenuationY[row];
                    }
                }
            }
        }
    }
}
