#include "tray.h"

#include <QSystemTrayIcon>
#include <QPainter>

tray::tray() :
	icon_(new QSystemTrayIcon(this)),
	base_icon_(":/icons/mail"),
	known_unread_(0)
{
	icon_->setToolTip("Unread mails");
	icon_->setIcon(base_icon_);
	icon_->setVisible(true);
}

void tray::unread_messages(unsigned int unread, unsigned int /*total*/)
{
	if (unread > known_unread_)
	{
		icon_->showMessage("Mail", "Incoming mail");
	}

	if (unread > 0)
	{
		QPixmap pixmap = base_icon_.pixmap(QSize(16, 16));
		{
			QPainter painter(&pixmap);

			painter.setPen(Qt::blue);

			QFont font = painter.font();
			font.setBold(true);
			painter.setFont(font);

			painter.drawText(pixmap.rect(),
							 Qt::AlignCenter,
							 QString::number(unread));
			webcam_.light();
		}
		icon_->setIcon(QIcon(pixmap));
	}
	else
	{
		icon_->setIcon(base_icon_);
		webcam_.unlight();
	}
	known_unread_ = unread;
}
