#include "glviewwidget.h"
#include "mainwindow.h"
#include "Sprite/document.h"
#include "widgets/spritemodel.h"
#include "ui_mainwindow.h"
#include "Shaders/defaultvaos.h"
#include "Shaders/transparencyshader.h"
#include "Shaders/velvetshader.h"
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <QPainter>
#include <QCursor>
#include <QHelpEvent>
#include <QToolTip>
#include <QKeyEvent>
#include <QWheelEvent>
#include <QScrollBar>
#include <QEvent>
#include <cmath>
#include <chrono>
#include <iostream>
#include <cassert>
#include <QOpenGLDebugLogger>
#include <QSurfaceFormat>

#include <GL/glu.h>

struct Matrices
{
	glm::mat4  u_projection;
	glm::mat4  u_camera;
	glm::ivec4 u_screenSize;
	glm::vec4  u_cursorColor;
	float      u_ctime;
};

GLViewWidget::GLViewWidget(QWidget * p) :
	QOpenGLWidget(p),
	timer(this)
{
	TransparencyShader::Shader.AddRef();
	VelvetShader::Shader.AddRef();

    setFormat(QSurfaceFormat(QSurfaceFormat::DebugContext));

	timer.setSingleShot(false);
	timer.setInterval(10);
//	timer.setTimerType(Qt::PreciseTimer);
	connect(&timer, &QTimer::timeout, this, [this]() { repaint(); } );

	current_time = std::chrono::high_resolution_clock::now();

	auto f = format();
	f.setSwapInterval(0);
	setFormat(f);

	setUpdateBehavior(QOpenGLWidget::PartialUpdate);
}

GLViewWidget::~GLViewWidget()
{
	glAssert();
    TransparencyShader::Shader.Release(this);
	VelvetShader::Shader.Release(this);
}

bool GLViewWidget::PauseTimer()
{
	bool active = timer.isActive();
	timer.stop();
	return active;
}

void GLViewWidget::ResumeTimer(bool active)
{
	if(active) timer.start();
}

glm::vec2 GLViewWidget::GetScreenPosition(QMouseEvent * event)
{
	return glm::vec2(event->pos().x() - width()*.5f,
			   -1 * (event->pos().y() - height()*.5f));
}

glm::vec2 GLViewWidget::GetScreenPosition()
{
	if(!QWidget::underMouse())
		return glm::vec2(0, 0);

	QPoint pos = mapFromGlobal(QCursor::pos());

	return glm::vec2(pos.x() - width()*.5f,
			   -1 * (pos.y() - height()*.5f));
}

Bitwise   GLViewWidget::GetFlags(QMouseEvent * event)
{
	auto modifier = event->modifiers();

	if(modifier == Qt::ControlModifier)
		return Bitwise::XOR;
	else if(modifier == Qt::ShiftModifier)
		return Bitwise::OR;
	else if(modifier == (Qt::ShiftModifier|Qt::ControlModifier))
		return Bitwise::AND;

	return Bitwise::SET;
}

void GLViewWidget::set_animation(float fps)
{
	timer.setSingleShot(false);
	timer.setInterval(900 / fps);
	timer.start();
}

void GLViewWidget::need_repaint(bool set_timer)
{
	if(set_timer) timer.setSingleShot(true);
	timer.start();
}

void GLViewWidget::initializeGL()
{
	static bool initialized = false;

	if(!initialized)
	{
		initialized = true;
		QOpenGLFunctions_3_3_Core::initializeOpenGLFunctions();
	}

    glClearColor(0, 0, 0, 1);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glGenBuffers(1, &m_ubo);
	glBindBuffer(GL_UNIFORM_BUFFER, m_ubo);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(Matrices), nullptr, GL_DYNAMIC_DRAW);

	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAX_LEVEL, 0);

	logger = new QOpenGLDebugLogger(this);
	logger->initialize();

	connect(logger, &QOpenGLDebugLogger::messageLogged, this, &GLViewWidget::handleLoggedMessage);

    glAssert();
    logger->startLogging(); glAssert();
}

typedef std::pair<int, const char*> Flag;

#define FlagDef(x) { QOpenGLDebugMessage:: x, #x }
const static Flag g_Source[] =
{
FlagDef(InvalidSource),
FlagDef(APISource),
FlagDef(WindowSystemSource),
FlagDef(ShaderCompilerSource),
FlagDef(ThirdPartySource),
FlagDef(ApplicationSource),
FlagDef(OtherSource),
{0, nullptr}
};

const static Flag g_Type[] =
{
FlagDef(InvalidType),
FlagDef(ErrorType),
FlagDef(DeprecatedBehaviorType),
FlagDef(UndefinedBehaviorType),
FlagDef(PortabilityType),
FlagDef(PerformanceType),
FlagDef(OtherType),
FlagDef(MarkerType),
FlagDef(GroupPushType),
FlagDef(GroupPopType),
{0, nullptr}
};

const static Flag g_Severity[] =
{
FlagDef(InvalidSeverity),
FlagDef(HighSeverity),
FlagDef(MediumSeverity),
FlagDef(LowSeverity),
FlagDef(NotificationSeverity),
{0, nullptr}
};

std::string OutputFlags(uint32_t flag, Flag const* txt)
{
	std::string outs;

	for(auto itr = txt; itr->second != nullptr; ++itr)
	{
		if(itr->first & flag)
		{
			if(outs.size())
				outs += "|";

			outs += itr->second;
		}
	}

	if(outs.empty())
	{
		outs = txt->second;
	}

	return outs;
}

void GLViewWidget::handleLoggedMessage(QOpenGLDebugMessage const& debugMessage)
{
	if(debugMessage.severity() == QOpenGLDebugMessage::NotificationSeverity)
		return;

	std::cerr << "Source  : " << OutputFlags(debugMessage.source(),   g_Source)   << std::endl;
	std::cerr << "Type    : " << OutputFlags(debugMessage.type(),     g_Type)     << std::endl;
	std::cerr << "Severity: " << OutputFlags(debugMessage.severity(), g_Severity) << std::endl;
	std::cerr << "Id      : " << debugMessage.id() << std::endl;
	std::cerr << "Message : " << debugMessage.message().toStdString() << std::endl;
	std::cerr << std::endl;
}

#if 0

void GLViewWidget::mouseMoveEvent(QMouseEvent * event)
{
	super::mouseMoveEvent(event);

    if(w->toolbox.OnMouseMove(GetWorldPosition(event), GetFlags(event)))
		need_repaint();
}


void GLViewWidget::mousePressEvent(QMouseEvent * event)
{
	if((event->button() & Qt::LeftButton) == false)
		super::mousePressEvent(event);
	else
	{
        if(w->toolbox.OnLeftDown(GetWorldPosition(event), GetFlags(event)))
			need_repaint();

		if(w->toolbox.HaveTool() == false)
		{
			setMouseTracking(false);
			w->SetStatusBarMessage();
		}
	}
}

void GLViewWidget::mouseReleaseEvent(QMouseEvent * event)
{
	if((event->button() & Qt::LeftButton) == false)
		super::mouseReleaseEvent(event);
	else
	{
        if(w->toolbox.OnLeftUp(GetWorldPosition(event), GetFlags(event)))
			need_repaint();

        if(w->toolbox.HaveTool() == false)
		{
			setMouseTracking(false);
			w->SetStatusBarMessage();
		}
	}
}

void GLViewWidget::mouseDoubleClickEvent(QMouseEvent * event)
{
	if((event->button() & Qt::LeftButton) == false)
		super::mouseDoubleClickEvent(event);
	else
	{
        if(w->toolbox.OnDoubleClick(GetWorldPosition(event), GetFlags(event)))
			need_repaint();

		if(w->toolbox.HaveTool() == false)
		{
			setMouseTracking(false);
			w->SetStatusBarMessage();
		}
	}
}
#endif

template<typename T>
inline int get_sign(T it)
{
	return it < (T)0? -1 : 1;
}

void GLViewWidget::wheelEvent(QWheelEvent * wheel)
{
    if(!w)
	{
		super::wheelEvent(wheel);
		return;
	}

	if(wheel->modifiers() & Qt::ControlModifier)
	{
		if(std::fabs(wheel->angleDelta().y()) > std::fabs(wheel->angleDelta().x()))
		{
      //      if(w->document == nullptr) return;
#if 0
			auto pos = wheel->posF();
			float angle = wheel->angleDelta().y();
			float factor = std::pow(1.0015, angle);

            factor = w->SetZoom(w->GetZoom() * factor);

			glm::vec2 scroll = glm::mix(scroll_destination, scroll_start, glm::vec2(factor));

			if(factor != 1)
			{
                glm::vec2 scroll_start       = w->GetScroll();
                glm::vec2 dimensions         = w->document->GetDimensions();
				glm::vec2 mouse_position     = glm::vec2(pos.x() - width()*.5f, -1 * (pos.y() - height()*.5f));
				glm::vec2 world_position     = scroll_start * dimensions + mouse_position * (1 - factor);
				glm::vec2 scroll_destination = world_position / dimensions;

                w->SetScroll(scroll_destination);
			}
#endif

			return;
		}
	}
	else if(wheel->buttons() != Qt::MiddleButton)
	{
		if(std::fabs(wheel->angleDelta().y()) < std::fabs(wheel->angleDelta().x()))
		{
            w->ui->horizontalScrollBar->event(wheel);
		}
		else
		{
            w->ui->verticalScrollBar->event(wheel);
		}

		return;
	}

	super::wheelEvent(wheel);
}


bool GLViewWidget::event(QEvent *event)
{
	if(event->type() != QEvent::ToolTip)
		return super::event(event);

   QHelpEvent *helpEvent = static_cast<QHelpEvent *>(event);

   QString string; // = window->getToolTip(helpEvent->pos());

   if(!string.isEmpty())
   {
	   QToolTip::showText(helpEvent->globalPos(), string);
   }
   else
   {
	   QToolTip::hideText();
	   event->ignore();
   }

   return true;
}

void GLViewWidget::paintGL()
{
//    if(w->document == nullptr)		return;

  //  if(w->document->m_metaroom.m_selection.Changed())
    //    w->OnSelectionChanged();

    glAssert();
	int width = size().width();
	int height = size().height();

	Matrices mat;

	mat.u_projection = glm::ortho(
		(float)-width/2,
		(float) width/2,
		(float)-height/2,
		(float) height/2,
		(float)-1,
		(float)+1);

	auto window_pos = mapFromGlobal(QCursor::pos());

    mat.u_camera = glm::scale(glm::mat4(1), glm::vec3(w->GetZoom()));
//    mat.u_camera = glm::translate(mat.u_camera, glm::vec3(-w->document->GetScreenCenter(), 0));
	mat.u_screenSize     = glm::ivec4(width, height, window_pos.x(), window_pos.y());
	mat.u_cursorColor   = glm::vec4(window_pos.x(), size().height() - window_pos.y(), 0, 1);

	long long time =
		std::chrono::duration_cast<std::chrono::milliseconds>(
			std::chrono::high_resolution_clock::now() - current_time
				).count();

	mat.u_ctime = time / 1000;

	glBindBuffer(GL_UNIFORM_BUFFER, m_ubo);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(Matrices), &mat);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, m_ubo);

	glViewport(0, 0, width, height);
	glClear(GL_DEPTH_BUFFER_BIT);

	VelvetShader::Shader.bind(this, glm::vec4(.3, .5, .5, .975));
	glDefaultVAOs::BindVAO(this);
	glDefaultVAOs::RenderSquare(this);

	w->model->Render(this, w->ui->treeView->currentIndex());

    glAssert();
}

void 	GLViewWidget::resizeGL(int w, int h)
{
    QOpenGLWidget::resizeGL(w, h);
    glViewport(0, 0, w, h);
}

#include <GL/glu.h>
#include <QMessageBox>

void GLViewWidget::displayOpenGlError(const char * file, const char * function, int line)
{
    GLenum error = glGetError();

    if(error == GL_NO_ERROR) return;

    do
    {
#ifdef NDEBUG
        QMessageBox::information(this, QGuiApplication::applicationDisplayName(),
                                 tr("FILE: %1\nFUNC: %2\nLINE: %3\nERROR: %4")
                                 .arg(file).arg(function).arg(line).arg((const char *) gluErrorString(error)));
#else
		fprintf(stderr, "\nFILE: %s\nFUNC: %s\nLINE: %i\nERROR: %s\n", file, function, line, (const char *) gluErrorString(error));
#endif
    } while((error = glGetError()) != GL_NO_ERROR);

    w->close();
}
