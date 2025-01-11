#include "parsearguments.h"
#include "support.h"
#include "packersettings.h"
#include <cstring>

#define Token(tok, text) {tok, sizeof(text)-1, text}

TokenType tokens[]
{
Token(arg_outFile,           "-o"),
Token(arg_size,              "-h"),
Token(arg_size,              "-?"),
Token(arg_size,              "-s"),
Token(arg_size,              "-size"),
Token(arg_size,              "--help"),
Token(arg_square,            "--square"),
Token(arg_outFile,           "--out-file"),
Token(arg_sortOrder,         "--sort-order"),
Token(arg_borderSize,        "--border-size"),
Token(arg_extrudeSize,       "--extrude-size"),
Token(arg_disableCrop,       "--disable-crop"),
Token(arg_disableMerge,      "--disable-merge"),
Token(arg_enableRotate,      "--enable-rotate"),
Token(arg_disableBorder,     "--disable-border"),
Token(arg_cropThreshold,     "--crop-threshold"),
Token(arg_disableRecusion,   "--disable-recusion"),
Token(arg_minTextureSize,    "--min-texture-size"),
Token(arg_autosizeThreshold, "--autosize-threshold"),
{arg_badToken, 0, 0L},
};

TokId_t getToken(const char * string)
{
	unsigned int length = strlen(string);

	for(int i = 0; tokens[i].string != 0L; ++i)
	{
		if(tokens[i].length < length) continue;
		if(tokens[i].length > length) break;
		if(strcmp(tokens[i].string, string) == 0)
			return tokens[i].token;
	}

	return arg_badToken;
}

#include <QDir>

Arguments::Arguments() :
	outDir(QDir::currentPath().toStdString()),
	outFile("atlas")
{
}

Arguments::Arguments(int argc, char * argv[], PackerSettings & packer) :
	Arguments()
{
	bool merge = true;
	bool crop = true;
	int border = 1;
	int extrude = 0;
	bool rotate = false;
	bool recursion = true;
	bool square = false;
	bool autosize = false;
	int cropThreshold = 1;
	int autosizeThreshold = 80;
	int minTextureSizeX = 32;
	int minTextureSizeY = 32;
	int sortorder = 4;
	int textureWidth = 512;
	int textureHeight = 512;

	for(int i = 1; i < argc; ++i)
	{
		int token = getToken(argv[i]);

		switch(token)
		{
		case arg_help:
			printHelp();
			break;
		case arg_size:
		{
			++i;
			if(i >= argc)
			{
				printHelp("Argument needed for option -s");
			}
			if(sscanf(argv[i], "%10dx%10d", &textureWidth, &textureHeight) != 2)
			{
				if(sscanf(argv[i], "%10d", &textureWidth) != 1)
				{
					printHelp("Wrong texture size format");
				}
				else
				{
					textureHeight = textureWidth;
				}
			}
			printf("Setting texture size: %dx%d\n", textureWidth, textureHeight);
		} break;
		case arg_outFile:
		{
			++i;
			if(i >= argc)
			{
				printHelp("Argument needed for option -o");
			}
			QFileInfo info(argv[i]);
			outFile = info.baseName().toStdString();
			outDir = info.absolutePath().toStdString();
		} break;
		case arg_disableMerge:    merge = false; break;
		case arg_disableCrop:     crop  = false; break;
		case arg_disableBorder:   border = false; break;
		case arg_disableRecusion: recursion = false; break;
		case arg_cropThreshold:
		{
			++i;
			if(i >= argc)
			{
				printHelp("Argument needed for option --crop-threshold");
			}
			if((sscanf(argv[i], "%10d", &cropThreshold) != 1) ||
			(cropThreshold < 0) ||
			(cropThreshold > 255))
			{
				printHelp("Wrong crop threshold");
			}
		} break;
		case arg_borderSize:
		{
			++i;
			if(i >= argc)
			{
				printHelp("Argument needed for option --border-size");
			}
			if((sscanf(argv[i], "%10d", &border) != 1) || (border < 0))
			{
				printHelp("Wrong border size");
			}
		} break;
		case arg_extrudeSize:
		{
			++i;
			if(i >= argc)
			{
				printHelp("Argument needed for option --extrude-size");
			}
			if((sscanf(argv[i], "%10d", &extrude) != 1) || (extrude < 0))
			{
				printHelp("Wrong extrude size");
			}
		} break;
		case arg_enableRotate:       rotate = true; break;
		case arg_square:             square = true; break;
		case arg_autosizeThreshold:
		{
			autosize = true;
			++i;
			if(i >= argc)
			{
				printHelp("Argument needed for option --autosize-threshold");
			}
			if((sscanf(argv[i], "%10d", &autosizeThreshold) != 1) ||
			(autosizeThreshold < 0) ||
			(autosizeThreshold > 100))
			{
				printHelp("Wrong autosize threshold");
			}
		} break;
		case arg_minTextureSize:
		{
			++i;
			if(i >= argc)
			{
				printHelp("Argument needed for option -min-texture-size");
			}
			if(sscanf(argv[i], "%10dx%10d", &minTextureSizeX, &minTextureSizeY) != 2)
			{
				if(sscanf(argv[i], "%10d", &minTextureSizeX) != 1)
				{
					printHelp("Wrong texture size format");
				}
				else
				{
					minTextureSizeY = minTextureSizeX;
				}
			}
		} break;
		case arg_sortOrder:
		{
			++i;
			if(i >= argc)
			{
				printHelp("Argument needed for option --sort-order");
			}
			if((sscanf(argv[i], "%10d", &sortorder) != 1) ||
			(sortorder < 0) ||
			(sortorder > 4))
			{
				printHelp("Wrong sortorder must be from 0 to 4");
			}
		} break;
		default:
		/*	QFileInfo file(argv[i]);
			if(file.isFile())
			{
				packerData *data = new packerData();
				data->path = file.absoluteFilePath();
				data->file = file.fileName();
				packer.addItem(data->path, data);
			}
			else
				if(file.isDir())
				{
					RecurseDirectory(packer, file.absoluteFilePath(), file.absoluteFilePath(), recursion);
				}*/
			break;
		};
	}

	packer.rotate = (Preferences::Rotation) rotate;
	packer.sortOrder = (Preferences::Sort) sortorder;
	packer.border = border;
    packer.padding = 0;
	packer.extrude = extrude;
	packer.cropThreshold = crop ? cropThreshold : 0;
	packer.minFillRate = autosize ? autosizeThreshold : 0;
	packer.minSize = glm::u16vec2(minTextureSizeX, minTextureSizeY);
	packer.maxSize = glm::u16vec2(textureWidth, textureHeight);
	packer.merge = merge;
	packer.mergeBF = false;
	packer.square = square;
	packer.autosize = autosize;
}
