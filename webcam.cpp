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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <linux/videodev2.h>
#include "libv4l2.h"

#define CLEAR(x) memset(&(x), 0, sizeof(x))

static void xioctl(int fh, int request, void *arg)
{
	int r;

	do {
		r = v4l2_ioctl(fh, request, arg);
	} while (r == -1 && ((errno == EINTR) || (errno == EAGAIN)));

	if (r == -1) {
		fprintf(stderr, "error %d, %s\n", errno, strerror(errno));
		exit(EXIT_FAILURE);
	}
}

webcam::webcam()
{
	struct v4l2_buffer              buf;
	struct v4l2_requestbuffers      req;
	const char                      *dev_name = "/dev/video0";

	fd = v4l2_open(dev_name, O_RDWR | O_NONBLOCK, 0);
	if (fd < 0) {
		perror("Cannot open device");
		return;
	}

	CLEAR(req);
	req.count = 1;
	req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	req.memory = V4L2_MEMORY_MMAP;
	xioctl(fd, VIDIOC_REQBUFS, &req);

	CLEAR(buf);

	buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	buf.memory      = V4L2_MEMORY_MMAP;
	buf.index       = 0;

	xioctl(fd, VIDIOC_QUERYBUF, &buf);

	map_length = buf.length;
	map_start = v4l2_mmap(NULL, buf.length,
						  PROT_READ | PROT_WRITE, MAP_SHARED,
						  fd, buf.m.offset);

	if (MAP_FAILED == map_start)
	{
		perror("mmap");
		close(fd);
		fd = -1;
		return;
	}

	CLEAR(buf);
	buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	buf.memory = V4L2_MEMORY_MMAP;
	buf.index = 0;
	xioctl(fd, VIDIOC_QBUF, &buf);
}

webcam::~webcam()
{
	if (fd < 0)
		return;
	unlight();
	v4l2_munmap(map_start, map_length);
	v4l2_close(fd);
}

void webcam::light()
{
	if (lighted)
		return;

	enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	xioctl(fd, VIDIOC_STREAMON, &type);
	lighted = true;
}

void webcam::unlight()
{
	if (not lighted)
		return;

	enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	xioctl(fd, VIDIOC_STREAMOFF, &type);
	lighted = false;
}
