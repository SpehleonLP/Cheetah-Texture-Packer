#-------------------------------------------------
#
# Project created by QtCreator 2012-04-29T22:23:49
#
#-------------------------------------------------

QT += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = cheetah-texture-packer

QT_VERSION=$$[QT_VERSION]

contains(QT_VERSION, "^5.*") {
  QT += widgets
} else {
}

TEMPLATE = app
INCLUDEPATH += src \
        ../../../Libraries/glm \
	../../../Libraries/fx-gltf/src \
	../../../Libraries/fx-gltf/test/thirdparty \
	../../../Libraries/basis_universal/

LIBS += -lopengl32 -lglu32

CONFIG += c++14

DEFINES += QT_DEPRECATED_WARNINGS GLM_EXT_INCLUDED \"_gl=gl->\"

SOURCES += src/main.cpp\
	../../../Libraries/fx-gltf/src/bufferinfo.cpp \
	../../../Libraries/fx-gltf/src/fx/gltf.cpp \
	../../../Libraries/fx-gltf/src/gltf_stl_accessor.cpp \
	src/Import/import_c16.cpp \
	src/Import/packspritesheet.cpp \
	src/Import/super_xbr.cpp \
	src/Import/upscalesprite.cpp \
	src/Shaders/colorshader.cpp \
	src/Shaders/defaulttextures.cpp \
	src/Shaders/defaultvaos.cpp \
	src/Shaders/glprogram.cpp \
	src/Shaders/gltfmetallicroughness.cpp \
	src/Shaders/spriteshaderbase.cpp \
	src/Shaders/transparencyshader.cpp \
	src/Shaders/unlitshader.cpp \
	src/Shaders/velvetshader.cpp \
	src/Sprite/animation.cpp \
	src/Sprite/document.cpp \
	src/Sprite/image.cpp \
	src/Sprite/material.cpp \
	src/Sprite/object.cpp \
	src/Sprite/spritesheet.cpp \
	src/Support/counted_string.cpp \
	src/Support/imagesupport.cpp \
	src/Support/qt_to_gl.cpp \
	src/commandlist.cpp \
	src/imagemetadata.cpp \
	src/lf_math.cpp \
        src/mainwindow.cpp \
	src/packersettings.cpp \
	src/preferences.cpp \
	src/widgets/glviewwidget.cpp \
	src/widgets/spritemodel.cpp \
        src/imagepacker.cpp \
        src/imagecrop.cpp \
        src/maxrects.cpp \
    src/support.cpp \
    src/parsearguments.cpp \
    src/widgets/qclickablelabel.cpp \
    src/settingspanel.cpp \
    src/rc_crc32.c

HEADERS  += src/mainwindow.h \
	../../../Libraries/fx-gltf/src/accessorreader.hpp \
	../../../Libraries/fx-gltf/src/accessortypeinfo.hpp \
	../../../Libraries/fx-gltf/src/bufferinfo.h \
	../../../Libraries/fx-gltf/src/componenttypeinfo.hpp \
	../../../Libraries/fx-gltf/src/fx/extensions/khr_materials.h \
	../../../Libraries/fx-gltf/src/fx/gltf.h \
	../../../Libraries/fx-gltf/src/fx/gltf_forward.hpp \
	../../../Libraries/fx-gltf/src/gltf_stl_accessor.h \
	src/Import/import_c16.h \
	src/Import/packspritesheet.h \
	src/Import/super_xbr.h \
	src/Import/upscalesprite.h \
	src/Shaders/colorshader.h \
	src/Shaders/defaulttextures.h \
	src/Shaders/defaultvaos.h \
	src/Shaders/glprogram.h \
	src/Shaders/gltfmetallicroughness.h \
	src/Shaders/spriteshaderbase.h \
	src/Shaders/transparencyshader.h \
	src/Shaders/unlitshader.h \
	src/Shaders/velvetshader.h \
	src/Sprite/animation.h \
	src/Sprite/document.h \
	src/Sprite/image.h \
	src/Sprite/material.h \
	src/Sprite/object.h \
	src/Sprite/spritesheet.h \
	src/Support/counted_ptr.hpp \
	src/Support/counted_string.h \
	src/Support/countedsizedarray.hpp \
	src/Support/glm_iostream.hpp \
	src/Support/imagesupport.h \
	src/Support/qt_to_gl.h \
	src/Support/vectoroperations.hpp \
	src/commandinterface.hpp \
	src/commandlist.h \
	src/enums.hpp \
	src/imagemetadata.h \
	src/lf_math.h \
	src/packersettings.h \
	src/widgets/glviewwidget.h \
	src/widgets/spritemodel.h \
        src/imagepacker.h \
        src/maxrects.h \
    src/support.h \
    src/parsearguments.h \
    src/widgets/qclickablelabel.h \
    src/settingspanel.h \
    src/widgets/qconstrainedspinbox.hpp \
    src/qimageptr.hpp \
    src/rc_crc32.h

FORMS    += src/mainwindow.ui

