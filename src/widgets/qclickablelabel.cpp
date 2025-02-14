#include "qclickablelabel.h"

QClickableLabel::QClickableLabel(QWidget * parent, const QString & label, bool canCheck) :
	QLabel(parent),
	m_isChecked(true),
	m_canCheck(canCheck)
{
	setFrameStyle(QFrame::Box);
	/*
	setAutoFillBackground(true);
	QPalette pal = palette();
	pal.setColor(QPalette::Window, QColor(qRgba(0x00, 0x00, 0x00, 0x80)));
	setPalette(pal);
	*/

//	setStyleSheet("QLabel { background-color : 0x000000FF; }");
	if(!m_canCheck)
	{
		setText(label);
	}
	else
	{
		m_string = label;
		update();
	}
}

QClickableLabel::~QClickableLabel() {}

void  QClickableLabel::update()
{
	setText(QString("%1 %2").arg((m_isChecked)? QChar(0x25BC) : QChar(0x25B6)).arg(m_string));
}

void QClickableLabel::mousePressEvent(QMouseEvent*)
{
	if(m_canCheck)
	{
		m_isChecked = !m_isChecked;
		update();
		emit clicked();
	}
}
