#include "waker.h"

using qimaptray::waker;

waker::waker(connection& conn) :
	connection_(conn)
{
}

void waker::wake()
{
	connection_.stop();
}
