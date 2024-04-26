#ifndef GLVIEWWIDGET_H
#define GLVIEWWIDGET_H
#include "src/enums.hpp"
#include "qt-gl/gl_viewwidget.h"
#include <glm/vec2.hpp>
#include <QTimer>
#include <chrono>
#include <map>
#include <atomic>

class MainWindow;
class QOpenGLDebugLogger;
class QOpenGLDebugMessage;
class QMouseEvent;

class GLViewWidget : public gl::ViewWidget
{
typedef QOpenGLWidget super;
//	Q_OBJECT
public:
	GLViewWidget(QWidget * p);
	virtual ~GLViewWidget();

	void AddRef() { if(++_refCount == 1) { _timerState = _timer.isActive(); _timer.stop(); }; }
	void Release() { if(--_refCount == 0) { if(_timerState) _timer.start(); } }

	void set_animation(float fps = 60.f);
	void need_repaint(bool set_timer = true);

	void SetMainWindow(MainWindow * w);

private:
	void initializeGL() Q_DECL_OVERRIDE;
	void paintGL() Q_DECL_OVERRIDE;

	std::chrono::time_point<std::chrono::high_resolution_clock> current_time;
	std::chrono::milliseconds interval;


	MainWindow * w{};
	uint32_t m_ubo{};
	bool      _timerState{};
	mutable std::atomic<int> _refCount{0};
};


#endif // GLVIEWWIDGET_H
