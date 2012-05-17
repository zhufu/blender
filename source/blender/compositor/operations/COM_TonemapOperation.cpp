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

#include "COM_TonemapOperation.h"
#include "BLI_math.h"
#include "BLI_utildefines.h"



TonemapOperation::TonemapOperation(): NodeOperation()
{
	this->addInputSocket(COM_DT_COLOR, COM_SC_NO_RESIZE);
	this->addOutputSocket(COM_DT_COLOR);
	this->imageReader = NULL;
	this->data = NULL;
	this->cachedInstance = NULL;
	this->setComplex(true);
}
void TonemapOperation::initExecution()
{
	this->imageReader = this->getInputSocketReader(0);
	NodeOperation::initMutex();
}

void TonemapOperation::executePixel(float *color, int x, int y, MemoryBuffer *inputBuffers[], void * data)
{
	AvgLogLum * avg = (AvgLogLum*)data;

	float output[4];
	this->imageReader->read(output, x, y, inputBuffers, NULL);
	output[0] *= avg->al;
	output[1] *= avg->al;
	output[2] *= avg->al;
	float dr = output[0] + this->data->offset;
	float dg = output[1] + this->data->offset;
	float db = output[2] + this->data->offset;
	output[0] /= ((dr == 0.f) ? 1.f : dr);
	output[1] /= ((dg == 0.f) ? 1.f : dg);
	output[2] /= ((db == 0.f) ? 1.f : db);
	const float igm = avg->igm;
	if (igm != 0.f) {
		output[0] = pow((double)MAX2(output[0], 0.), (double)igm);
		output[1] = pow((double)MAX2(output[1], 0.), (double)igm);
		output[2] = pow((double)MAX2(output[2], 0.), (double)igm);
	}

	color[0] = output[0];
	color[1] = output[1];
	color[2] = output[2];
	color[3] = output[3];
}
void PhotoreceptorTonemapOperation::executePixel(float *color, int x, int y, MemoryBuffer *inputBuffers[], void * data)
{
	AvgLogLum * avg = (AvgLogLum*)data;
	NodeTonemap *ntm = this->data;

	const float f = exp((double)-this->data->f);
	const float m = (ntm->m > 0.f) ? ntm->m : (0.3f + 0.7f*pow((double)avg->auto_key, 1.4));
	const float ic = 1.f - ntm->c, ia = 1.f - ntm->a;

	float output[4];
	this->imageReader->read(output, x, y, inputBuffers, NULL);

	const float L = 0.212671f*output[0] + 0.71516f*output[1] + 0.072169f*output[2];
	float I_l = output[0] + ic*(L - output[0]);
	float I_g = avg->cav[0] + ic*(avg->lav - avg->cav[0]);
	float I_a = I_l + ia*(I_g - I_l);
	output[0] /= (output[0] + pow((double)f*I_a, (double)m));
	I_l = output[1] + ic*(L - output[1]);
	I_g = avg->cav[1] + ic*(avg->lav - avg->cav[1]);
	I_a = I_l + ia*(I_g - I_l);
	output[1] /= (output[1] + pow((double)f*I_a,(double)m));
	I_l = output[2] + ic*(L - output[2]);
	I_g = avg->cav[2] + ic*(avg->lav - avg->cav[2]);
	I_a = I_l + ia*(I_g - I_l);
	output[2] /= (output[2] + pow((double)f*I_a, (double)m));

	color[0] = output[0];
	color[1] = output[1];
	color[2] = output[2];
	color[3] = output[3];
}

void TonemapOperation::deinitExecution()
{
	this->imageReader = NULL;
	if (this->cachedInstance) {
		delete cachedInstance;
	}
	NodeOperation::deinitMutex();
}

bool TonemapOperation::determineDependingAreaOfInterest(rcti *input, ReadBufferOperation *readOperation, rcti *output)
{
	rcti imageInput;

	NodeOperation *operation = getInputOperation(0);
	imageInput.xmax = operation->getWidth();
	imageInput.xmin = 0;
	imageInput.ymax = operation->getHeight();
	imageInput.ymin = 0;
	if (operation->determineDependingAreaOfInterest(&imageInput, readOperation, output) ) {
		return true;
	}
	return false;
}

void *TonemapOperation::initializeTileData(rcti *rect, MemoryBuffer **memoryBuffers)
{
	BLI_mutex_lock(getMutex());
	if (this->cachedInstance == NULL) {
		MemoryBuffer *tile = (MemoryBuffer*)imageReader->initializeTileData(rect, memoryBuffers);
		AvgLogLum *data = new AvgLogLum();

		float * buffer = tile->getBuffer();

		float lsum = 0;
		int p = tile->getWidth() * tile->getHeight();
		float *bc = buffer;
		float avl, maxl = -1e10f, minl = 1e10f;
		const float sc = 1.f/(p);
		float Lav = 0.f;
		float cav[4] = {0.0f,0.0f,0.0f,0.0f};
		while (p--) {
			float L = 0.212671f*bc[0] + 0.71516f*bc[1] + 0.072169f*bc[2];
			Lav += L;
			cav[0] += bc[0];
			cav[1] += bc[1];
			cav[2] += bc[2];
			lsum += (float)log((double)MAX2(L, 0.0) + 1e-5);
			maxl = (L > maxl) ? L : maxl;
			minl = (L < minl) ? L : minl;
			bc+=4;
		}
		data->lav = Lav * sc;
		data->cav[0] = cav[0]*sc;
		data->cav[1] = cav[1]*sc;
		data->cav[2] = cav[2]*sc;
		maxl = log((double)maxl + 1e-5); minl = log((double)minl + 1e-5f); avl = lsum*sc;
		data->auto_key = (maxl > minl) ? ((maxl - avl) / (maxl - minl)) : 1.f;
		float al = exp((double)avl);
		data->al = (al == 0.f) ? 0.f : (this->data->key / al);
		data->igm = (this->data->gamma==0.f) ? 1 : (1.f / this->data->gamma);
		this->cachedInstance = data;
	}
	BLI_mutex_unlock(getMutex());
	return this->cachedInstance;
}

void TonemapOperation::deinitializeTileData(rcti *rect, MemoryBuffer **memoryBuffers, void *data)
{
}
