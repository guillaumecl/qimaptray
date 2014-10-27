#include "tray.h"

#include <QSystemTrayIcon>
#include <QPainter>
#include <QMenu>
#include <QApplication>

namespace qimaptray
{

tray::tray() :
	icon_(new QSystemTrayIcon(this)),
	base_icon_(":/icons/mail"),
	known_unread_(0),
	connected_(false)
{
	icon_->setToolTip("Unread mails");
	repaint();
	icon_->setVisible(true);

	context_menu_ = new QMenu;

	context_menu_->addAction("Quit", qApp, SLOT(quit()));
	context_menu_->addAction("Reconnect", this, SIGNAL(reconnect()));

	icon_->setContextMenu(context_menu_);
}

tray::~tray()
{
	context_menu_->deleteLater();
}


void tray::repaint()
{
	QPixmap pixmap;
	QSize size(16, 16);
	if (not connected_)
		pixmap = base_icon_.pixmap(size, QIcon::Disabled);
	else
		pixmap = base_icon_.pixmap(size);

	webcam_.set_light(connected_ and known_unread_ > 0);

	if (known_unread_ > 0)
	{
		QPainter painter(&pixmap);

		painter.setPen(Qt::blue);

		QFont font = painter.font();
		font.setBold(true);
		painter.setFont(font);

		painter.drawText(pixmap.rect(),
				 Qt::AlignCenter,
				 QString::number(known_unread_));
	}
	icon_->setIcon(QIcon(pixmap));
}

void tray::unread_messages(unsigned int unread, unsigned int /*total*/)
{
	known_unread_ = unread;
	repaint();
}


void tray::connected()
{
	connected_ = true;
	repaint();
}

void tray::disconnected()
{
	webcam_.unlight();
	connected_ = false;
	repaint();
}

void tray::cannot_login()
{
	icon_->showMessage("qimaptray", "Cannot login to the server.");
}

}
