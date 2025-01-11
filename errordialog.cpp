#include "errordialog.h"
#include "ui_errordialog.h"

namespace loguru {
std::string stacktrace_as_stdstring(int skip_from, int skip);
}

ErrorDialog::ErrorDialog(QWidget *parent, QString && error) :
	QDialog(parent),
	ui(new Ui::ErrorDialog)
{
	ui->setupUi(this);

	ui->ErrorText->setText(std::move(error));
	ui->StackTrace->setText(QString::fromStdString(loguru::stacktrace_as_stdstring(3, 6)));
}

ErrorDialog::~ErrorDialog()
{
	delete ui;
}
