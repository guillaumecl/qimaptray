/* Based on:
   V4L2 video picture grabber
   Copyright (C) 2009 Mauro Carvalho Chehab <mchehab@infradead.org>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation version 2 of the License.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
*/
#include "webcam.h"
#include <webcam_led/webcam_led.h>

namespace qimaptray
{

webcam::webcam()
{
	controller_ = webcam_init(nullptr);
}

webcam::~webcam()
{
	if (controller_)
		webcam_free(controller_);
}

void webcam::light()
{
	if (controller_)
		webcam_light(controller_);
}

void webcam::unlight()
{
	if (controller_)
		webcam_unlight(controller_);
}

}
