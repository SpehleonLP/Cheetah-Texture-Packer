#ifndef APPLICATION_H
#define APPLICATION_H

// Minimal stand-in for the engine's application.h. The shared Spehleon
// errordialog.cpp only needs Application::Title() to supply a default
// window title when the caller passes none. Lives in the Cheetah tree so
// it never shadows the engine's own application.h.

#include <string>
#include <QCoreApplication>

namespace Application {

// Returns a stable reference (not a temporary) so callers can safely hold
// the result of .c_str(); initialized once from the name main() sets via
// QCoreApplication::setApplicationName().
inline const std::string & Title()
{
	static const std::string title = QCoreApplication::applicationName().toStdString();
	return title;
}

}

#endif // APPLICATION_H
