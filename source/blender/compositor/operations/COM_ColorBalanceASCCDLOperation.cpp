/*
 * Copyright 2011, Blender Foundation.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * Contributor: 
 *		Jeroen Bakker 
 *		Monique Dewanchand
 */

#include "COM_ColorBalanceASCCDLOperation.h"
#include "BLI_math.h"

inline float colorbalance_cdl(float in, float offset, float power, float slope)
{
	float x = in * slope + offset;

	/* prevent NaN */
	CLAMP(x, 0.0f, 1.0f);

	return powf(x, power);
}

ColorBalanceASCCDLOperation::ColorBalanceASCCDLOperation() : NodeOperation()
{
	this->addInputSocket(COM_DT_VALUE);
	this->addInputSocket(COM_DT_COLOR);
	this->addOutputSocket(COM_DT_COLOR);
	this->m_inputValueOperation = NULL;
	this->m_inputColorOperation = NULL;
	this->setResolutionInputSocketIndex(1);
}

void ColorBalanceASCCDLOperation::initExecution()
{
	this->m_inputValueOperation = this->getInputSocketReader(0);
	this->m_inputColorOperation = this->getInputSocketReader(1);
}

void ColorBalanceASCCDLOperation::executePixel(float output[4], float x, float y, PixelSampler sampler)
{
	float inputColor[4];
	float value[4];
	
	this->m_inputValueOperation->read(value, x, y, sampler);
	this->m_inputColorOperation->read(inputColor, x, y, sampler);
	
	float fac = value[0];
	fac = min(1.0f, fac);
	const float mfac = 1.0f - fac;
	
	output[0] = mfac * inputColor[0] + fac * colorbalance_cdl(inputColor[0], this->m_lift[0], this->m_gamma[0], this->m_gain[0]);
	output[1] = mfac * inputColor[1] + fac * colorbalance_cdl(inputColor[1], this->m_lift[1], this->m_gamma[1], this->m_gain[1]);
	output[2] = mfac * inputColor[2] + fac * colorbalance_cdl(inputColor[2], this->m_lift[2], this->m_gamma[2], this->m_gain[2]);
	output[3] = inputColor[3];

}

void ColorBalanceASCCDLOperation::deinitExecution()
{
	this->m_inputValueOperation = NULL;
	this->m_inputColorOperation = NULL;
}
