#-------------------------------------------------
#
# Project created by QtCreator 2012-04-29T22:23:49
#
#-------------------------------------------------

QT += core gui
CONFIG += c++2a

TARGET = cheetah-texture-packer
TEMPLATE = app

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
lessThan(QT_MAJOR_VERSION, 5):	  QT += opengl
greaterThan(QT_MAJOR_VERSION, 5): QT += opengl openglwidgets

INCLUDEPATH += src \
	../../../Libraries/Spehleon/lib/ \
	../../../Libraries/fx-gltf/src \
	../../../Libraries/fx-gltf/test/thirdparty \
	../../../Libraries/basis_universal/ \
	../../../Libraries/loguru \
	../../../Libraries \
	/usr/include/gtk-3.0 \
	/usr/include/at-spi2-atk/2.0 \
	/usr/include/at-spi-2.0 \
	/usr/include/dbus-1.0 \
	/usr/lib/x86_64-linux-gnu/dbus-1.0/include \
	/usr/include/gtk-3.0 \
	/usr/include/gio-unix-2.0 \
	/usr/include/cairo \
	/usr/include/pango-1.0 \
	/usr/include/harfbuzz \
	/usr/include/pango-1.0 \
	/usr/include/fribidi \
	/usr/include/harfbuzz \
	/usr/include/atk-1.0 \
	/usr/include/cairo \
	/usr/include/pixman-1 \
	/usr/include/uuid \
	/usr/include/freetype2 \
	/usr/include/gdk-pixbuf-2.0 \
	/usr/include/libpng16 \
	/usr/include/x86_64-linux-gnu \
	/usr/include/libmount \
	/usr/include/blkid \
	/usr/include/glib-2.0 \
	/usr/lib/x86_64-linux-gnu/glib-2.0/include

LIBS += -lGLEW -lGL -lGLU -ldrm -lz \
    -L\"/mnt/Passport/Libraries/lz4/build/cmake\" -llz4

DEFINES += CHEETAH=1 QT_DEPRECATED_WARNINGS \"_gl=gl->\"
DEFINES += GLM_EXT_INCLUDED GLM_FORCE_INLINE GLM_ENABLE_EXPERIMENTAL

SOURCES += src/main.cpp\
	../../../Libraries/Spehleon/lib/ErrorDialogs/errordialog.cpp \
	../../../Libraries/Spehleon/lib/ErrorDialogs/errordialog_gtk.cpp \
	../../../Libraries/Spehleon/lib/ErrorDialogs/errordialog_macos.cpp \
	../../../Libraries/Spehleon/lib/ErrorDialogs/errordialog_qt.cpp \
	../../../Libraries/Spehleon/lib/ErrorDialogs/errordialog_win32.cpp \
	../../../Libraries/Spehleon/lib/Support/counted_string.cpp \
	../../../Libraries/Spehleon/lib/qt-gl/initialize_gl.cpp \
	../../../Libraries/Spehleon/lib/qt-gl/simpleshaderbase.cpp \
	../../../Libraries/Spehleon/lib/qt-gl/gl_viewwidget.cpp \
	../../../Libraries/Spehleon/lib/qt-gl/viewparentinterface.cpp \
	../../../Libraries/loguru/loguru.cpp \
	../../../Libraries/Spehleon/lib/gl/compressedshadersource.cpp \
	../../../Libraries/Spehleon/lib/gl/renderdoc.cpp \
	../../../Libraries/fx-gltf/src/bufferinfo.cpp \
	../../../Libraries/fx-gltf/src/fx/extensions/khr_materials.cpp \
	../../../Libraries/fx-gltf/src/fx/extensions/msft_texture_dds.cpp \
	../../../Libraries/fx-gltf/src/fx/gltf.cpp \
	../../../Libraries/fx-gltf/src/gltf_stl_accessor.cpp \
	errordialog.cpp \
	src/Import/import_c16.cpp \
	src/Import/linearizesprite.cpp \
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
	src/Sprite/countedgltfimage.cpp \
	src/Sprite/document.cpp \
	src/Sprite/image.cpp \
	src/Sprite/imagekey.cpp \
	src/Sprite/imagemanager.cpp \
	src/Sprite/imagetexturecoordinates.cpp \
	src/Sprite/material.cpp \
	src/Sprite/object.cpp \
	src/Sprite/spritecutter.cpp \
	src/Sprite/spritejson.cpp \
	src/Sprite/spritesheet.cpp \
	src/Support/getuniquecountedarray.cpp \
	src/Support/imagesupport.cpp \
	src/Support/packaccessor.cpp \
	src/Support/qt_to_gl.cpp \
	src/Support/unpackmemo.cpp \
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
	../../../Libraries/Spehleon/lib/ErrorDialogs/errordialog.h \
	../../../Libraries/Spehleon/lib/Support/counted_string.h \
	../../../Libraries/Spehleon/lib/Support/counted_ptr.hpp \
	../../../Libraries/Spehleon/lib/Support/shared_array.hpp \
	../../../Libraries/Spehleon/lib/Support/lockfreequeue.hpp \
	../../../Libraries/Spehleon/lib/Support/numeric_range.hpp \
	../../../Libraries/Spehleon/lib/Support/singleton_base.hpp \
	../../../Libraries/Spehleon/lib/Support/unsafe_view.hpp \
	../../../Libraries/Spehleon/lib/gl/compressedshadersource.h \
	../../../Libraries/Spehleon/lib/gl/renderdoc.h \
	../../../Libraries/Spehleon/lib/qt-gl/gl_viewwidget.h \
	../../../Libraries/Spehleon/lib/qt-gl/initialize_gl.h \
	../../../Libraries/Spehleon/lib/qt-gl/simpleshaderbase.h \
	../../../Libraries/Spehleon/lib/qt-gl/viewparentinterface.h \
	../../../Libraries/Spehleon/lib/universal_include.h \
	../../../Libraries/fx-gltf/src/accessorreader.hpp \
	../../../Libraries/fx-gltf/src/accessortypeinfo.hpp \
	../../../Libraries/fx-gltf/src/bufferinfo.h \
	../../../Libraries/fx-gltf/src/componenttypeinfo.hpp \
	../../../Libraries/fx-gltf/src/fx/extensions/khr_materials.h \
	../../../Libraries/fx-gltf/src/fx/extensions/msft_texture_dds.h \
	../../../Libraries/fx-gltf/src/fx/gltf.h \
	../../../Libraries/fx-gltf/src/fx/gltf_forward.hpp \
	../../../Libraries/fx-gltf/src/gltf_stl_accessor.h \
	errordialog.h \
	src/Import/import_c16.h \
	src/Import/linearizesprite.h \
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
	src/Sprite/countedgltfimage.h \
	src/Sprite/document.h \
	src/Sprite/image.h \
	src/Sprite/imagekey.h \
	src/Sprite/imagemanager.h \
	src/Sprite/imagetexturecoordinates.h \
	src/Sprite/material.h \
	src/Sprite/object.h \
	src/Sprite/spritecutter.h \
	src/Sprite/spritejson.h \
	src/Sprite/spritesheet.h \
	src/Support/getuniquecountedarray.h \
	src/Support/glm_iostream.hpp \
	src/Support/imagesupport.h \
	src/Support/packaccessor.h \
	src/Support/qt_to_gl.h \
	src/Support/unpackmemo.h \
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

FORMS    += src/mainwindow.ui \
	errordialog.ui

