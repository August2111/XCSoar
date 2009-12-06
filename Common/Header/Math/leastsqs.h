/**
 * \file leastsqs.hxx
 * Implements a simple linear least squares best fit routine.
 */

// Written by Curtis Olson, started September 1997.
//
// Copyright (C) 1997  Curtis L. Olson  - http://www.flightgear.org/~curt
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Library General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Library General Public License for more details.
//
// You should have received a copy of the GNU Library General Public
// License along with this library; if not, write to the
// Free Software Foundation, Inc., 59 Temple Place - Suite 330,
// Boston, MA  02111-1307, USA.
//
// $Id$


#ifndef _LEASTSQS_H
#define _LEASTSQS_H

#ifndef __cplusplus
# error This library requires C++
#endif

#define MAX_STATISTICS 1000

/**
 * A solver for least squares problems
 *
 * Classical least squares fit:
 *
 * \f[
 *     y = b_0 + b_1 * x
 * \f]
 *
 * \f[
 *     b_1 = \frac{n * \sum_0^{i-1} (x_i*y_i) - \sum_0^{i-1} x_i* \sum_0^{i-1} y_i}
 *           {n*\sum_0^{i-1} x_i^2 - (\sum_0^{i-1} x_i)^2}
 * \f]
 *
 * \f[
 *     b_0 = \frac{\sum_0^{i-1} y_i}{n} - b_1 * \frac{\sum_0^{i-1} x_i}{n}
 * \f]
 *
 * the least squares error:
 *
 * \f[
 *     \frac{(y_i - \hat{y}_i)^2}{n}
 * \f]
 *
 * the maximum least squares error:
 *
 * \f[
 *     (y_i - \hat{y}_i)^2
 * \f]
 */
class LeastSquares
{
public:
  double sum_xi, sum_yi, sum_xi_2, sum_xi_yi;
  int sum_n;
  double m;
  double b;
  double sum_error;

  double rms_error;
  double max_error;
  double sum_weights;
  double y_max;
  double y_min;
  double x_min;
  double x_max;

  double y_ave;

  float xstore[MAX_STATISTICS];
  float ystore[MAX_STATISTICS];
  float weightstore[MAX_STATISTICS];

  LeastSquares()
  {
    Reset();
  }

  void Reset();
/**
 * Incrementally update existing values with a new data point.
 */
  void LeastSquaresUpdate(double x, double y, double weight=1.0);

  // assume x is the sample number
  void LeastSquaresUpdate(double y);

  void LeastSquaresErrorUpdate();

  // add one point, for single update later
  void LeastSquaresAdd(double x, double y, double weight=1.0);
  // global update
  void LeastSquaresUpdate();
};

#endif // _LEASTSQS_H
