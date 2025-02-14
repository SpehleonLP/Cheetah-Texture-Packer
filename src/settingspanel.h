#ifndef SETTINGSPANEL_H
#define SETTINGSPANEL_H
#include "widgets/qconstrainedspinbox.hpp"
#include "widgets/qclickablelabel.h"
#include "preferences.h"
#include "mainwindow.h"
#include <QObject>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QLineEdit>
#include <QPushButton>
#include <QCheckBox>
#include <QString>
#include <QHBoxLayout>
#include <vector>

class QFormLayout;
class QConstrainedSpinBox;
class MainWindow;
class PackerSettings;

/* There is a good reason that it is done this way rather than with setHidden, having
 * to do with some bugs in Qt on ubuntu...
 */

class SettingsPanel : public QObject
{
public:
	SettingsPanel(MainWindow &parent);
	virtual ~SettingsPanel();

    QString outDir() const { return t_fileDir->text(); }
    QString outFile() const { return t_fileName.text(); }
    QString imgFormat() const { return d_textureFormat.currentText(); }
//	QImage::Format pxFormat() const { return (QImage::Format)d_pixelFormat.currentData().toInt(); }

	void GetSettings(PackerSettings&);

	QRgb greenScreenColor() const
	{
		uint32_t eax = t_greenScreen.text().toUInt(0L, 16);
		return qRgb((eax >> 16)&0xFF, (eax >> 8)&0xFF, (eax)&0xFF);
	}

//nothing else cares, but it can't break anything.
	bool doesOwnObject(void * it) const;

private:
	bool crop() const { return c_crop.isChecked(); }

	typedef void (QComboBox::*QComboBoxChanged)(int);
	typedef void (QSpinBox::*QSpinBoxChanged)(int);
	typedef void (QDoubleSpinBox::*QDoubleSpinBoxChanged)(double);

	void AddItems(QComboBox * , const char * items[]);
	void SetToolTip(QComboBox *, const char * tool_tip[]);

	void removeItem(QLayoutItem *child);
	void clear();
	void populate();

	void populateData();
	void populateOutputFiles();
	void populateFormat();
	void populateScaling();
	void populatePacking();
	void populateCropping();
	void populatePadding();

	void getFolder();
	void validateFolderText();

	MainWindow & w;

	enum
	{
		e_hLines = 5,
	};

	bool m_useBasicSettings;

	QString m_directory;
	QVBoxLayout * m_wrapper;
	QWidget    * m_widget;
	QFormLayout * m_formLayout;

//wrap the box sizers in stuff so we can deal with things uniformly
	QWidget m_textureDir;
	QWidget m_maxSize;
	QWidget m_textureSizer;
	QWidget m_alignment;

	QCheckBox c_premultiplied;
	QCheckBox c_greenScreen;
	QCheckBox c_autosize;
	QCheckBox c_greenScreenAlpha;
	QCheckBox c_merge;
	QCheckBox c_multipack;
	QCheckBox c_square;
	QCheckBox c_crop;

	QClickableLabel h_data;
	QClickableLabel h_outputFiles;
	QClickableLabel h_format;
#if SCALING_ENABLED
	QClickableLabel h_scaling;
#endif
	QClickableLabel h_packing;
	QClickableLabel h_cropping;
	QClickableLabel h_padding;

	QComboBox d_algorithm;
	QComboBox d_dataFormat;
	QComboBox d_heuristics;
	QComboBox d_maxSize;
	QComboBox d_pack;
	QComboBox d_rotation;
#if SCALING_ENABLED
	QComboBox d_scaleMode;
#endif
	QComboBox d_sizeConstraints;
	QComboBox d_sort;
	QComboBox d_textureFormat;

	QSpinBox s_alphaThreshold;
	QSpinBox s_borderPadding;
	QSpinBox s_extrude;
	QSpinBox s_shapePadding;
	QSpinBox s_trimMargin;
	QSpinBox s_autosizeFillRate;
	QLineEdit t_greenScreen;
	QLineEdit t_fileName;

#if SCALING_ENABLED
	QDoubleSpinBox s_scale;
#endif

//these need to be pointers as they'll be in the box layout during our destructor.
	QToolButton * b_getDir;
	QLineEdit * t_fileDir;
    QConstrainedSpinBox * s_maxX,*s_maxY;
	QSpinBox * s_alignX,*s_alignY;

	int m_maxHeight;
	int maxLabelWidth;
	int maxFieldWidth;
};


#endif // SETTINGSPANEL_H
