#include "settingspanel.h"
#include "mainwindow.h"
#include "support.h"
#include "ui_mainwindow.h"
#include "widgets/qconstrainedspinbox.hpp"
#include "packersettings.h"
#include <QFormLayout>
#include <QPushButton>
#include <QToolButton>

typedef void (QComboBox::*QComboBoxChanged)(int);
typedef void (QSpinBox::*QSpinBoxChanged)(int);

QStringList g_pixelFormats;

SettingsPanel::SettingsPanel(MainWindow & parent) :
	QObject((QMainWindow*)&parent),
	w(parent),
	m_useBasicSettings(false),
	h_data(0L, parent.tr("Data"), true),
	h_outputFiles(0L, parent.tr("Output Files"), true),
	h_format(0L, parent.tr("Format"), true),
#if SCALING_ENABLED
	h_scaling(0L, parent.tr("Scaling"), true),
#endif
	h_packing(0L, parent.tr("Packing"), true),
	h_cropping(0L, parent.tr("Cropping"), true),
	h_padding(0L, parent.tr("Padding"), true),
	m_maxHeight(0L),
	maxLabelWidth(0L),
	maxFieldWidth(0L)
{
	m_formLayout = 0L;
	m_widget     = new QWidget(w.ui->settingsWidget);
	m_widget->setContentsMargins(0, 0, 0, 0);

	m_wrapper = new QVBoxLayout(w.ui->settingsWidget);
	m_wrapper->setContentsMargins(0, 0, 0, 0);
	m_wrapper->addWidget(m_widget);
	m_wrapper->addSpacerItem(new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding));

	t_fileDir = new QLineEdit(&m_textureDir);
	b_getDir = new QToolButton(&m_textureDir); b_getDir->setText(tr("..."));

    s_maxX = new QConstrainedSpinBox(&m_maxSize);
    s_maxY = new QConstrainedSpinBox(&m_maxSize);
	s_alignX = new QSpinBox(&m_alignment);
	s_alignY = new QSpinBox(&m_alignment);

	c_greenScreen.setChecked(false);
	c_greenScreenAlpha.setChecked(true);
	c_merge.setChecked(true);
	c_multipack.setChecked(true);
	c_multipack.setEnabled(false);
	c_square.setChecked(true);


	s_alphaThreshold.setRange(1, 255);
	s_borderPadding.setRange(0, 8);
	s_extrude.setRange(0, 8);
	s_shapePadding.setRange(0, 8);
	s_trimMargin.setRange(0, 8);
//not set up yet.
	s_trimMargin.setEnabled(false);

	s_maxX->setRange(64, 4096);
	s_maxX->setValue(512);

//these should all match, don't repeat!
	s_maxY->setRange(s_maxX->minimum(), s_maxX->maximum());
	s_maxY->setValue(s_maxX->value());

	s_alignX->setRange(1, 8);
	s_alignY->setRange(1, 8);

	s_alignX->setToolTip(
		"I know how to implement padding OR alignment,\n"
		"but i'm pretty confused about how to do both");
	s_alignY->setToolTip(s_alignX->toolTip());
	s_alignX->setEnabled(false);
	s_alignY->setEnabled(false);


	t_greenScreen.setInputMask("HHHHHH");
	t_greenScreen.setText("000000");

	t_fileDir->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
	b_getDir->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);

	s_maxX->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
	s_maxY->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
	s_alignX->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
	s_alignY->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

	c_autosize.setChecked(true);
	s_autosizeFillRate.setValue(80);

//add options to combo boxes
	d_dataFormat.addItem(tr("UBFG .fnt"));
	d_dataFormat.setEnabled(false);

	d_textureFormat.addItems(g_extensions);
	d_textureFormat.setCurrentIndex(g_extensions.indexOf("png"));

#if SCALING_ENABLED
	d_scaleMode.addItems  (g_scaleStrings());
	d_scaleMode.setToolTip(g_scaleToolTip());
	d_scaleMode.setCurrentIndex(g_scaleDefault());

	s_scale.setRange(.125, 8);
	s_scale.setValue(1.0);
#endif

	AddItems(&d_sizeConstraints, Preferences::SizeConstraintStrings);
	SetToolTip(&d_sizeConstraints, Preferences::SizeConstraintToolTip);

	AddItems(&d_sort, Preferences::SortStrings);
	SetToolTip(&d_sort, Preferences::SortToolTip);

	d_algorithm.addItem(tr("MaxRects"));
	d_algorithm.setEnabled(false);
	d_pack.addItem(tr("Yes"));
	d_pack.setEnabled(false);

	AddItems(&d_rotation, Preferences::RotationStrings);
	SetToolTip(&d_rotation, Preferences::RotationToolTip);

	AddItems(&d_heuristics, Preferences::HeuristicStrings);
	SetToolTip(&d_heuristics, Preferences::HeuristicToolTip);


//set default values:
	d_sizeConstraints.setCurrentIndex(0);
	d_sort.setCurrentIndex(4);
	d_heuristics.setCurrentIndex(1);
	d_rotation.setCurrentIndex(1);

//populate the horizontal sizers
	QHBoxLayout * sizer;
	/*
	sizer = new QHBoxLayout(&m_textureSizer);
	sizer->setContentsMargins(0, 0, 0, 0);
	{
	QPushButton * button;
	QFont font;
	font.setPointSize(8);
	QSizePolicy sizePolicy6(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
	sizePolicy6.setHorizontalStretch(0);
	sizePolicy6.setVerticalStretch(0);

font.setPointSize(8);
#define addSizeButton(t)\
	button = new QPushButton(#t, &m_textureSizer);\
	button->setFont(font);\
	button->setMaximumSize(QSize(46, 20));\
	connect(button, &QToolButton::released, [this]()	{ s_maxX->setValue(t); s_maxY->setValue(t); });\
	sizer->addWidget(button)

	addSizeButton(256);
	addSizeButton(512);
	addSizeButton(1024);
	addSizeButton(2048);
	addSizeButton(4096);
#undef addSizeButton
	}
*/

	sizer = new QHBoxLayout(&m_textureDir);
	sizer->setContentsMargins(0, 0, 0, 0);
	sizer->addWidget(t_fileDir);
	sizer->addWidget(b_getDir);

	sizer = new QHBoxLayout(&m_maxSize);
	sizer->setContentsMargins(0, 0, 0, 0);
	sizer->addWidget(new QLabel("X"));
	sizer->addWidget(s_maxX);
	sizer->addWidget(new QLabel("Y"));
	sizer->addWidget(s_maxY);

	sizer = new QHBoxLayout(&m_alignment);
	sizer->setContentsMargins(0, 0, 0, 0);
	sizer->addWidget(new QLabel("X"));
	sizer->addWidget(s_alignX);
	sizer->addWidget(new QLabel("Y"));
	sizer->addWidget(s_alignY);

//connect stuff
	connect(&h_data, &QClickableLabel::clicked, this, &SettingsPanel::populate);
	connect(&h_outputFiles, &QClickableLabel::clicked, this, &SettingsPanel::populate);
	connect(&h_packing, &QClickableLabel::clicked, this, &SettingsPanel::populate);
	connect(&h_cropping, &QClickableLabel::clicked, this, &SettingsPanel::populate);
	connect(&h_padding, &QClickableLabel::clicked, this, &SettingsPanel::populate);
#if SCALING_ENABLED
	connect(&h_scaling, &QClickableLabel::clicked, this, &SettingsPanel::populate);
#endif
	connect(&h_format, &QClickableLabel::clicked, this, &SettingsPanel::populate);

//connect auto updates; needs to be done last so we don't try to auto update while half initialized.
	connect(&c_greenScreen,		    &QCheckBox::stateChanged, &w, &MainWindow::updateAuto);
	connect(&c_greenScreenAlpha,    &QCheckBox::stateChanged, &w, &MainWindow::updateAuto);
	connect(&c_merge,	            &QCheckBox::stateChanged, &w, &MainWindow::updateAuto);
	connect(&c_multipack,           &QCheckBox::stateChanged, &w, &MainWindow::updateAuto);
	connect(&c_autosize,            &QCheckBox::stateChanged, &w, &MainWindow::updateAuto);
    connect(&c_square,              &QCheckBox::stateChanged, [this](bool v)
    {
        int x,y;
        if(!v) return;
        x = s_maxX->value();
        y = s_maxY->value();

		bool update = false;

		if(x > y) { update = true; s_maxY->setValue(x); }
		if(y > x) { update = true; s_maxX->setValue(y); }

		if(!update) w.updateAuto();
    });
    connect(&c_crop,                &QCheckBox::stateChanged, &w, &MainWindow::updateAuto);

	QComboBoxChanged qComboBoxChanged = &QComboBox::currentIndexChanged;
	connect(&d_algorithm,           qComboBoxChanged, &w, &MainWindow::updateAuto);
	connect(&d_dataFormat,          qComboBoxChanged, &w, &MainWindow::updateAuto);
	connect(&d_heuristics,			qComboBoxChanged, &w, &MainWindow::updateAuto);
	connect(&d_maxSize,             qComboBoxChanged, &w, &MainWindow::updateAuto);
	connect(&d_pack,                qComboBoxChanged, &w, &MainWindow::updateAuto);
	connect(&d_rotation,            qComboBoxChanged, &w, &MainWindow::updateAuto);
#if SCALING_ENABLED
    connect(&d_scaleMode,           qComboBoxChanged, &w, &MainWindow::updateAuto);
#endif
	connect(&d_sort,                qComboBoxChanged, &w, &MainWindow::updateAuto);
	connect(&d_textureFormat,       qComboBoxChanged, &w, &MainWindow::updateAuto);

    connect(&d_sizeConstraints,     qComboBoxChanged, [this](int v)
    {
        s_maxX->setConstraint(v);
        s_maxY->setConstraint(v);
    });

	QSpinBoxChanged qSpinBoxChanged = &QSpinBox::valueChanged;
    connect(&s_alphaThreshold,      qSpinBoxChanged,  &w, &MainWindow::updateAplhaThreshold);
	connect(&s_borderPadding,       qSpinBoxChanged,  &w, &MainWindow::updateAuto);
	connect(&s_extrude,             qSpinBoxChanged,  &w, &MainWindow::updateAuto);
	connect(&s_shapePadding,        qSpinBoxChanged,  &w, &MainWindow::updateAuto);
	connect(&s_trimMargin,          qSpinBoxChanged,  &w, &MainWindow::updateAuto);
	connect(&s_autosizeFillRate,    qSpinBoxChanged, &w, &MainWindow::updateAuto);

    connect(s_maxX,                 qSpinBoxChanged,  [this](int v)
    {
        if(c_square.isChecked() && s_maxY->value() != v)
            s_maxY->setValue(v);
        w.updateAuto();
    });
    connect(s_maxY,                 qSpinBoxChanged,  [this](int v)
    {
        if(c_square.isChecked() && s_maxX->value() != v)
            s_maxX->setValue(v);
        w.updateAuto();
    });

	connect(s_alignX,               qSpinBoxChanged,  &w, &MainWindow::updateAuto);
	connect(s_alignY,               qSpinBoxChanged,  &w, &MainWindow::updateAuto);

	connect(&t_greenScreen, &QLineEdit::editingFinished,&w, &MainWindow::updateAuto);
	connect(b_getDir, &QToolButton::released, this, &SettingsPanel::getFolder);
	connect(t_fileDir, &QLineEdit::editingFinished, this, &SettingsPanel::validateFolderText);
//redraw
#if SCALING_ENABLED
	connect(&s_scale,       (QDoubleSpinBoxChanged)&QDoubleSpinBox::valueChanged, &w, &MainWindow::updateAuto);
#endif

	populate();
}

SettingsPanel::~SettingsPanel()
{
	clear();
}

void SettingsPanel::GetSettings(PackerSettings& packer)
{
	packer.rotate    = (Preferences::Rotation) d_heuristics.currentIndex();
	packer.heuristic = (Preferences::Heuristic) d_heuristics.currentIndex();
	packer.sortOrder = (Preferences::Sort) d_sort.currentIndex();
	packer.padding   = s_shapePadding.value();

	packer.cropThreshold =  crop()? s_alphaThreshold.value() : 0;;
	packer.extrude       = s_extrude.value();
    packer.border        = s_borderPadding.value();
	packer.minFillRate   = s_autosizeFillRate.value();

    packer.merge    =  c_merge.isChecked();
	packer.mergeBF  = false;
    packer.square   = c_square.isChecked();
    packer.autosize =  c_autosize.isChecked();

	packer.greenScreenToAlpha = c_greenScreenAlpha.isChecked();
	packer.useGreenScreen     = crop() && c_greenScreen.isChecked();

	packer.alignment = glm::u8vec2(s_alignX->value(), s_alignY->value());

	uint32_t eax = t_greenScreen.text().toUInt(0L, 16);
	packer.greenScreen = glm::u8vec4((eax >> 16)&0xFF, (eax >> 8)&0xFF, (eax)&0xFF, 0xFF);

	packer.minSize = glm::i16vec2(
				  s_maxX->value(),
				  s_maxY->value()
			);

	packer.outDir = t_fileDir->text().toStdString();
	packer.outFile =  t_fileName.text().toStdString();
	packer.imageFormat =  d_textureFormat.currentText().toStdString();
}


void SettingsPanel::AddItems(QComboBox * box, const char * items[])
{
	QStringList strings;

	for(const char ** p = items; **p != '\0'; ++p)
		strings << QApplication::translate("Preferences", *p, Q_NULLPTR);

	box->addItems(std::move(strings));
}

void SettingsPanel::SetToolTip(QComboBox * box, const char * items[])
{
	QString strings;

	strings = "<html><head/><body>";

	for(const char ** p = items; **p != '\0'; ++p)
		strings += QString("<p>%1</p>").arg(
			QApplication::translate("HelpText", *p, Q_NULLPTR));

	strings += "</body></html>";

	box->setToolTip(std::move(strings));
}

bool SettingsPanel::doesOwnObject(void * it) const
{
	return (uintptr_t)this <= (uintptr_t)it && (uintptr_t)it < (uintptr_t)(this+1);
}

void SettingsPanel::removeItem(QLayoutItem *child)
{
	if(!child) return;

	QLayout     * layout = child->layout();
	QSpacerItem * spacer = child->spacerItem();
	QWidget     * widget = child->widget();

	if(!doesOwnObject(child)) delete child;

	if(layout && !doesOwnObject(layout)) delete layout;
	if(spacer && !doesOwnObject(spacer)) delete spacer;
	if(widget)
	{
		if(doesOwnObject(widget)) widget->setParent(0L);
		else                      delete widget;
	}
}

void SettingsPanel::clear()
{
	if(!m_formLayout) return;

	for(int i = m_formLayout->rowCount()-1; i >= 0; --i)
	{
		QFormLayout::TakeRowResult row = m_formLayout->takeRow(i);
//if it spans both rows then one of these will be null
		if(row.labelItem && row.fieldItem)
		{
			maxLabelWidth = std::max(maxLabelWidth, row.labelItem->minimumSize().width());
			maxFieldWidth = std::max(maxFieldWidth, row.fieldItem->minimumSize().width());
		}

		removeItem(row.labelItem);
		removeItem(row.fieldItem);

	}

	delete m_formLayout;
	m_formLayout = 0L;
}

void SettingsPanel::populate()
{
	//add padding so the scroll doesn't mess up when we open and close groups
	if(m_formLayout && m_maxHeight < m_widget->height())
	{
		m_maxHeight = m_widget->height();
		w.ui->settingsWidget->setMinimumHeight(m_maxHeight);
	}

	clear();

	m_formLayout = new QFormLayout(m_widget);
	m_formLayout->setContentsMargins(0, 0, 0, 0);
	m_formLayout->setLabelAlignment(Qt::AlignRight);

	populateData();
	populateOutputFiles();
	populateFormat();
	populateScaling();
	populatePacking();
	populateCropping();
	populatePadding();

//pad so that the width stays consistent
	QLabel * labelPadding = new QLabel();
	QLabel * fieldPadding = new QLabel();
	labelPadding->setMinimumWidth(maxLabelWidth);
	fieldPadding->setMinimumWidth(maxFieldWidth);
	m_formLayout->addRow(labelPadding, fieldPadding);
}

static
QFrame * horizontalLine()
{
	QFrame * r = new QFrame();
	r->setFrameShape(QFrame::HLine);
	r->setFrameShadow(QFrame::Sunken);
	return r;
}

void SettingsPanel::populateData()
{
	m_formLayout->setWidget(m_formLayout->rowCount(), QFormLayout::SpanningRole, &h_data);
	if(h_data.isChecked())
	{
		m_formLayout->addRow(tr("Data Format"), &d_dataFormat);
	}
}

void SettingsPanel::populateScaling()
{
#if SCALING_ENABLED
	m_formLayout->setWidget(m_formLayout->rowCount(), QFormLayout::SpanningRole, &h_scaling);
	if(h_scaling.isChecked())
	{
		m_formLayout->addRow(tr("Scale"), &s_scale);
		m_formLayout->addRow(tr("Scale Mode"), &d_scaleMode);
	}
#endif
}

void SettingsPanel::populateFormat()
{
	m_formLayout->setWidget(m_formLayout->rowCount(), QFormLayout::SpanningRole, &h_format);

	if(h_format.isChecked())
	{
		m_formLayout->addRow(tr("Texture Size"), &m_maxSize);
	//	m_formLayout->setWidget(m_formLayout->rowCount(), QFormLayout::SpanningRole, &m_textureSizer);

		m_formLayout->addRow(tr("Size Constraints"), &d_sizeConstraints);
		m_formLayout->addRow(tr("Force Squared"), &c_square);
		m_formLayout->addRow(tr("Autosize"), &c_autosize);
		m_formLayout->addRow(tr("Fill Rate"), &s_autosizeFillRate);
	}
}


void SettingsPanel::populateOutputFiles()
{
	m_formLayout->setWidget(m_formLayout->rowCount(), QFormLayout::SpanningRole, &h_outputFiles);
	if(h_outputFiles.isChecked())
	{
		m_formLayout->addRow(tr("Texture Format"), &d_textureFormat);
	}
	m_formLayout->addRow(tr("Output Directory"), &m_textureDir);
	m_formLayout->addRow(tr("File Name"), &t_fileName);
}

void SettingsPanel::populatePacking()
{
	m_formLayout->setWidget(m_formLayout->rowCount(), QFormLayout::SpanningRole, &h_packing);

	if(h_packing.isChecked())
	{
		m_formLayout->addRow(tr("Algorithm"), &d_algorithm);
	}

	m_formLayout->addRow(tr("Sort"), &d_sort);
	m_formLayout->addRow(tr("Heuristics"), &d_heuristics);

	if(h_packing.isChecked())
	{
		m_formLayout->addRow(tr("Pack"), &d_pack);
	}

	m_formLayout->setWidget(m_formLayout->rowCount(), QFormLayout::SpanningRole, horizontalLine());

	m_formLayout->addRow(tr("Rotation"), &d_rotation);
	if(h_packing.isChecked())
	{
		m_formLayout->addRow(tr("Multipack"), &c_multipack);
	}
	m_formLayout->addRow(tr("Merge"), &c_merge);
}

void SettingsPanel::populateCropping()
{
	m_formLayout->setWidget(m_formLayout->rowCount(), QFormLayout::SpanningRole, &h_cropping);
	if(h_cropping.isChecked())
	{
		m_formLayout->addRow(tr("Crop"), &c_crop);
		m_formLayout->addRow(tr("Crop Margin"), &s_trimMargin);
		m_formLayout->addRow(tr("Alpha Threshold"), &s_alphaThreshold);

		m_formLayout->setWidget(m_formLayout->rowCount(), QFormLayout::SpanningRole, horizontalLine());

		m_formLayout->addRow(tr("Green Screen Color"), &t_greenScreen);
		m_formLayout->addRow(tr("Use Green Screen"), &c_greenScreen);
		m_formLayout->addRow(tr("Green Screen -> Alpha"), &c_greenScreenAlpha);
	}
}

void SettingsPanel::populatePadding()
{
	m_formLayout->setWidget(m_formLayout->rowCount(), QFormLayout::SpanningRole, &h_padding);
	if(h_padding.isChecked())
	{
		m_formLayout->addRow(tr("Extrude"), &s_extrude);
		m_formLayout->addRow(tr("Border Padding"), &s_borderPadding);
		m_formLayout->addRow(tr("Shape Padding"), &s_shapePadding);
		m_formLayout->addRow(tr("Alignment"), &m_alignment);
	}
}

#include <QFileDialog>
#include <QMessageBox>
#include <QDir>

void SettingsPanel::getFolder()
{
	t_fileDir->setText(QFileDialog::getExistingDirectory((QMainWindow*)&w,
                        tr("Open Directory"),
                        t_fileDir->text(),
                        QFileDialog::ShowDirsOnly));

	m_directory = t_fileDir->text();
}

void SettingsPanel::validateFolderText()
{
	if(QDir::setCurrent(t_fileDir->text()))
	{
		m_directory = t_fileDir->text();
	}
	else
	{
		QMessageBox::warning((QMainWindow*)&w, QCoreApplication::applicationName(), tr("Unable to open directory \"%1\"").arg(t_fileDir->text()));
		t_fileDir->setText(m_directory);
	}
}

