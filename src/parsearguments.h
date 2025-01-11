#ifndef PARSEARGUMENTS_H
#define PARSEARGUMENTS_H
#include <string>

class MainWindow;
class PackerSettings;

enum TokId_t : char
{
	arg_help,
	arg_size,
	arg_outFile,
	arg_disableMerge,
	arg_disableCrop,
	arg_disableBorder,
	arg_disableRecusion,
	arg_cropThreshold,
	arg_borderSize,
	arg_extrudeSize,
	arg_enableRotate,
	arg_square,
	arg_autosizeThreshold,
	arg_minTextureSize,
	arg_sortOrder,
	arg_badToken = -1
};

struct TokenType
{
	TokId_t token;
	unsigned int length;
	const char * string;
};

struct Arguments
{
	Arguments();
	Arguments(int argc, char * argv[], PackerSettings & packer);

	std::string outDir;
	std::string outFile;
};

TokId_t getToken(const char * string);

#endif // PARSEARGUMENTS_H
