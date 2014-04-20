#ifndef WEBCAM_H
#define WEBCAM_H

#include <cstdlib>

struct webcam;

namespace qimaptray
{

class webcam
{
public:
	webcam();
	~webcam();

	void light();
	void unlight();

private:
	::webcam *controller_;
};

}

#endif
