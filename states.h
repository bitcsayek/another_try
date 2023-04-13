/*
 * states.h
 *
 *  Created on: Oct 8, 2013
 *      Author: mark
 */

#ifndef STATES_H_
#define STATES_H_

enum display_type { D_PRESSURE, D_TORQUE } display;
enum pressure_units_type { D_PSI, D_BAR, D_KPA } pressure_units;
enum torque_units_type { D_NM, D_FTLBS } torque_units;


// all power_types greater that P_ON will use a fast display refresh rate...
enum power_type { P_OFF, P_VERY_LOW_BATTERY, P_ON, P_FACTORY_CALIBRATE, P_FIELD_CALIBRATE,P_FIELD_TORQUE_INCREMENT,P_FIELD_DISPLAY_REFRESH, P_MAX_SETUP } power_state;

enum maxAdjustStateType
{
	S_adjustOff,
	S_adjust,
	S_confirm,
	S_saveMax,
	S_confirmingMax,
	S_errorMax
} maxAdjustState;

enum displayRateStateType
{
	S_Rate_adjustOff,
	S_Rate_adjust,
	S_Rate_confirm,
	S_Rate_saveMax,
	S_Rate_confirmingMax,
	S_Rate_errorMax
} rateAdjustState;

enum incrementAdjustStateType
{
	S_Increment_adjustOff,
	S_Increment_adjust,
	S_Increment_confirm,
	S_Increment_saveMax,
	S_Increment_confirmingMax,
	S_Increment_errorMax
} incrementAdjustState;

enum fieldCalibrationStateType
{
	S_calOff,
	S_set20,
	S_confirm20,
	S_torque20,
	S_confirmTorque20,

	S_set40,
	S_confirm40,
	S_torque40,
	S_confirmTorque40,

	S_set60,
	S_confirm60,
	S_torque60,
	S_confirmTorque60,

	S_set80,
	S_confirm80,
	S_torque80,
	S_confirmTorque80,

	S_set90,
	S_confirm90,
	S_torque90,
	S_confirmTorque90,

	S_saveQuery,
	S_confirming,

	S_error,

} fieldCalibrationState;

#endif /* STATES_H_ */
