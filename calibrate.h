/*
 * calibrate.h
 *
 *  Created on: Oct 23, 2013
 *      Author: mark
 */

#ifndef CALIBRATE_H_
#define CALIBRATE_H_

#define PSI2BAR 0.068947573
#define PSI2KPA   6.8947573
#define FTLB2NM 1.35581795

struct point
	{
	int count;
	float pressure;
	float torque;
	};

#define NUM_CAL_POINTS 5
struct point pointlist[NUM_CAL_POINTS];
struct point calRawPointList[NUM_CAL_POINTS];
struct point calPointList[NUM_CAL_POINTS];


int loadPoints(void);
float ConvertFromCounts(long counts, int type, int limitZero);
void ClearCalibration(void);
int calibrationSanityIndexCheck(unsigned int sensorIndex);
int calibrationSanityCheck(void);

#endif /* CALIBRATE_H_ */
