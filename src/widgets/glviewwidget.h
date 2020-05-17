#ifndef GLVIEWWIDGET_H
#define GLVIEWWIDGET_H
#include "src/enums.hpp"
#include <glm/vec2.hpp>
#include <QOpenGLFunctions_3_3_Core>
#include <QOpenGLWidget>
#include <QTimer>
#include <chrono>
#include <map>

class MainWindow;

#define glAssert() displayOpenGlError(__FILE__, __FUNCTION__, __LINE__);
#define GL_ASSERT _gl glAssert();

class GLViewWidget : public QOpenGLWidget, public QOpenGLFunctions_3_3_Core
{
typedef QOpenGLWidget super;
//	Q_OBJECT
public:
	GLViewWidget(QWidget * p);
	virtual ~GLViewWidget();

	void set_animation(float fps = 60.f);
	void need_repaint(bool set_timer = true);

    MainWindow * w{};

	glm::vec2 GetWorldPosition(QMouseEvent * event);
	glm::vec2 GetScreenPosition(QMouseEvent * event);
	glm::vec2 GetWorldPosition();
	glm::vec2 GetScreenPosition();
	Bitwise   GetFlags(QMouseEvent * event);


    void  displayOpenGlError(const char * file, const char * function, int line);


private:
#if 0
	void mouseMoveEvent 		(QMouseEvent * event)	Q_DECL_OVERRIDE;
	void mousePressEvent		(QMouseEvent * event)	Q_DECL_OVERRIDE;
	void mouseReleaseEvent		(QMouseEvent * event)	Q_DECL_OVERRIDE;
	void mouseDoubleClickEvent	(QMouseEvent * event)	Q_DECL_OVERRIDE;
#endif

	void wheelEvent				(QWheelEvent * event)   Q_DECL_OVERRIDE;
	bool event					(QEvent *event)			Q_DECL_OVERRIDE;

	void initializeGL() Q_DECL_OVERRIDE;
	void paintGL() Q_DECL_OVERRIDE;
    void resizeGL(int w, int h) Q_DECL_OVERRIDE;


	QTimer timer;

	std::chrono::time_point<std::chrono::high_resolution_clock> current_time;

	uint32_t m_ubo{};
};


#endif // GLVIEWWIDGET_H
