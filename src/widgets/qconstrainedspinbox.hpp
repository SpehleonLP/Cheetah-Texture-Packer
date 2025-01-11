#ifndef QCONSTRAINEDSPINBOX_H
#define QCONSTRAINEDSPINBOX_H
#include "src/preferences.h"
#include <QSpinBox>

class QConstrainedSpinBox : public QSpinBox
{
    Q_OBJECT

    int m_constraint;

public:
    QConstrainedSpinBox(QWidget *parent) :
        QSpinBox(parent), m_constraint(0) {}
    virtual ~QConstrainedSpinBox() {}

    void setConstraint(int c)
    {
        m_constraint = c;
        setValue(Preferences::RoundConstrainedValue(c, value()));
    }

    void stepBy(int step) override
    {
        QSpinBox::stepBy(Preferences::StepConstrainedValue(m_constraint, value(), step));
    }

    int valueFromText(const QString &text) const override
    {
        return Preferences::RoundConstrainedValue(m_constraint, text.toUInt());
    }

};

#endif // QCONSTRAINEDSPINBOX_H
