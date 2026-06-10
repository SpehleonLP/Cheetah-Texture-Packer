// Cheetah-only stub for WindowSystem::Thread::Push.
//
// The engine uses WindowSystem::Thread to marshal work from arbitrary threads
// onto the thread that owns the GUI. In a Qt application the thread invoking
// the error dialog is always the GUI thread, so there is nothing to marshal --
// just run the task synchronously on the calling thread (which is exactly what
// the real implementation does when no window thread has been created).
//
// Providing this here lets us link the shared errordialog.cpp without dragging
// in the whole threading subsystem (lf_thread, LockFreeQueue, ThreadManager).

#include "Windows/windowsystemthread.h"
#include "Windows/windowsystemtask.h"

void WindowSystem::Thread::Push(Task task)
{
	task->Process();
}
