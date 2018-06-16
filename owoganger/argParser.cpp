#include "stdafx.h"
#include "argParser.h"
#include <iostream>
#include "owoganger.h"

argParser::argParser(int argc, char **argv)
{
	this->argc = argc;
	this->argv = argv;
	parse();
}

argParser::~argParser()
{
	
}

void argParser::parse()
{
	if (argc == 1)
	{
		//std::cout << "one arg";
		printUsage();
		return;
	}
	if (std::string(argv[1]) == "--help")
	{
		//std::cout << argv[1];
		printHelp();
		return;
	}
	if (argc < 5)
	{
		printUsage();
		return;
	}
	if (argc >= 5) {
		if (std::string(argv[1]) == "--source" && std::string(argv[3]) == "--dest") {
			//fire up doppelganger
			new owoganger(std::string(argv[2]), std::string(argv[4]));
		}
	}
	else {
		printUsage();
		return;
	}
}

void argParser::printUsage() 
{
	using namespace std;
	cout << endl << endl;
	cout << "Usage: " << argv[0] << " --help" << std::endl;
	cout << "Usage: " << argv[0] << " --source %PATH --dest %PATH" << std::endl;
	//TODO Add some encoders, like XOR, b64 and etc

}

void argParser::printHelp() {
	using namespace std;
	printUsage();
	cout << "\t--source\tTo specify source path to payload" << endl;
	cout << "\t--dest\t\tTo specify destination path to file which will be transacted" << endl;

}