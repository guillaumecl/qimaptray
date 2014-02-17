#ifndef TRAY_H
#define TRAY_H

#include <QObject>
#include <QIcon>

class QSystemTrayIcon;

class tray: public QObject
{
	Q_OBJECT
public:
	tray();

public slots:
	void unread_messages(unsigned int unread, unsigned int total);

private:
	QSystemTrayIcon *icon_;
	QIcon base_icon_;

	unsigned int known_unread_;
};

#endif
