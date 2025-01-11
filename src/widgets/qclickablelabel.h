#ifndef QCLICKABLELABEL_H
#define QCLICKABLELABEL_H
#include <QLabel>
#include <QWidget>


class QClickableLabel : public QLabel
{
	Q_OBJECT

	QString				   m_string;
	bool                   m_isChecked;
	bool                   m_canCheck;

public:
	explicit QClickableLabel(QWidget * parent, const QString & label, bool canCheck);
	virtual ~QClickableLabel();

	bool isChecked() const { return m_isChecked; }

signals:
	void clicked();

protected:
    void mousePressEvent(QMouseEvent* event);
	void update();

};

#endif // QCLICKABLELABEL_H
