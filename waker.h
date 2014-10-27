#ifndef WAKER_H
#define WAKER_H

#include <QObject>
#include "connection.h"

namespace qimaptray
{

class waker: public QObject
{
	Q_OBJECT
public:
	waker(connection& conn);

public slots:
	void wake();

private:
	connection& connection_;
};

}

#endif
