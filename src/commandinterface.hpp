#ifndef COMMANDINTERFACE_HPP
#define COMMANDINTERFACE_HPP

class CommandInterface
{
public:
	CommandInterface() = default;
	virtual ~CommandInterface() = default;

	virtual void RollForward() = 0;
	virtual void RollBack() = 0;
};

#endif // COMMANDINTERFACE_HPP
