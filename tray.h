#ifndef TRAY_H
#define TRAY_H

#include <QObject>
#include <QIcon>
#include "webcam.h"

class QSystemTrayIcon;
class QMenu;

namespace qimaptray
{

class tray: public QObject
{
	Q_OBJECT
public:
	tray();
	~tray();

public slots:
	void unread_messages(unsigned int unread, unsigned int total);
	void connected();
	void disconnected();
	void cannot_login();

signals:
	void reconnect();

private:
	void repaint();
	QSystemTrayIcon *icon_;
	QIcon base_icon_;

	unsigned int known_unread_;

	qimaptray::webcam webcam_;
	bool connected_;

	QMenu *context_menu_;
};

}

#endif
