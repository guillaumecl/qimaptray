#ifndef WEBCAM_H
#define WEBCAM_H

#include <cstdlib>

class webcam
{
public:
	webcam();
	~webcam();

	void light();
	void unlight();

private:
	int fd;
	bool lighted;

	void   *map_start;
	size_t map_length;
};

#endif
