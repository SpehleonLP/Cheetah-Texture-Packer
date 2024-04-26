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

struct MapParent : public gl::iParent
{
	static Bitwise GetFlags(gl::Modifiers mod)
	{
		switch(mod)
		{
		default: return Bitwise::SET;
		case gl::And: return Bitwise::AND;
		case gl::Or: return Bitwise::OR;
		case gl::Xor: return Bitwise::XOR;
		}
	}

	static bool alt(gl::Modifiers mod)
	{
		return mod & gl::AltModifier? 0 : 1;
	}


	MapParent(MainWindow * w) : w(w) {}
	~MapParent() = default;

	MainWindow * w{};

	QScrollBar * horizontalScrollBar() override { return w->ui->horizontalScrollBar; }
	QScrollBar * verticalScrollBar() override { return w->ui->verticalScrollBar;  }
	QString      getToolTip(glm::vec2) const override { return {}; }

	float GetZoom() const override { return w->GetZoom(); }
	float SetZoom(float v) override { return w->SetZoom(v); }

	glm::vec2 GetScroll() const override { return w->GetScroll(); }
	glm::vec2 SetScroll(glm::vec2 x) override { w->SetScroll(x); return GetScroll(); }

	glm::vec2 GetScreenCenter() const override { return glm::vec2{0}; }
	glm::vec2 GetDocumentSize() const override { return w->model->GetItemSize(w->ui->treeView->currentIndex()); }
	bool NeedTrackMouse() const override { return false; }
	void UpdateStatusBarMessage(glm::vec2) override { return; }

	bool OnMouseWheel(glm::vec2 angleDelta) override
	{
		//return w->toolbox.wheelEvent(angleDelta);
		return false;
	}
	bool OnMouseMove(glm::vec2 worldPosition, gl::Modifiers mod) override
	{
		//return w->toolbox.OnMouseMove(worldPosition, GetFlags(mod));
		return false;
	}

	bool OnMouseDown(glm::vec2 worldPosition, gl::Modifiers mod) override
	{
		if(mod & gl::LeftDown)
		{
		//	return w->toolbox.OnLeftDown(worldPosition, GetFlags(mod), alt(mod));
		}

		return false;
	}

	bool OnMouseUp(glm::vec2 worldPosition, gl::Modifiers mod) override
	{
		if(mod & gl::LeftDown)
		{
		//	return w->toolbox.OnLeftUp(worldPosition, GetFlags(mod), alt(mod));
		}

		return false;
	}
	bool OnDoubleClick(glm::vec2 worldPosition, gl::Modifiers mod) override
	{
		if(mod & gl::LeftDown)
		{
		//	return w->toolbox.OnDoubleClick(worldPosition, GetFlags(mod));
		}

		return false;
	}
	std::unique_ptr<QMenu> GetContextMenu(glm::vec2) override { return nullptr; }
};


struct Matrices
{
	glm::mat4  u_projection;
	glm::mat4  u_camera;
	glm::ivec4 u_screenSize;
	glm::vec4  u_cursorColor;
	float      u_ctime;
};

GLViewWidget::GLViewWidget(QWidget * p) :
	gl::ViewWidget(p)
{
	TransparencyShader::Shader.AddRef();
	VelvetShader::Shader.AddRef();

	setFormat(QSurfaceFormat(QSurfaceFormat::DebugContext));
	current_time = std::chrono::high_resolution_clock::now();

	auto f = format();
	f.setSwapInterval(0);
	setFormat(f);

	setUpdateBehavior(QOpenGLWidget::PartialUpdate);
}

GLViewWidget::~GLViewWidget()
{
	TransparencyShader::Shader.Release(this);
	VelvetShader::Shader.Release(this);
}

void GLViewWidget::SetMainWindow(MainWindow * w)
{
	this->w = w;
	_parent.reset(new MapParent(w));
}

void GLViewWidget::set_animation(float fps)
{
	_timer.setSingleShot(false);
	_timer.setInterval(900 / fps);

	if(_refCount)
		_timerState = true;
	else
		_timer.start();
}

void GLViewWidget::need_repaint(bool set_timer)
{
	if(set_timer)
		_timer.setSingleShot(true);

	if(_refCount)
		_timerState = true;
	else if(_timer.isActive() == false)
		_timer.start();
}

void GLViewWidget::initializeGL()
{
	gl::ViewWidget::initializeGL();

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


void GLViewWidget::paintGL()
{
//    if(w->document == nullptr)		return;

  //  if(w->document->m_metaroom.m_selection.Changed())
	//    w->OnSelectionChanged();

	int width = size().width();
	int height = size().height();
	glViewport(0, 0, width, height);

	auto dimensions = w->model->GetItemSize(w->ui->treeView->currentIndex());
	auto scroll = w->GetScroll() * 2.f - 1.f;

	Matrices mat;

	mat.u_projection = glm::ortho(
		(float)-width/2,
		(float) width/2,
		(float)-height/2,
		(float) height/2,
		(float)-1,
		(float)+1);

	auto window_pos = mapFromGlobal(QCursor::pos());

	mat.u_camera = glm::mat4(1);
	mat.u_camera = glm::translate(mat.u_camera, glm::vec3(0.6f * -scroll * dimensions, 0));
	mat.u_camera = glm::scale(mat.u_camera, glm::vec3(w->GetZoom()));

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
}

void glBindUniformBlocks(QOpenGLFunctions_4_5_Core * gl, GLuint program)
{
	gl::BindUniformBlock(gl, program, "Matrices", 0);
}
