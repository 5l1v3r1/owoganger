#include "stdafx.h"
#include "argParser.h"
#include <iostream>
#include "owoganger.h"

argParser::argParser(int argc, char **argv)
{
	this->argc = argc;
	this->argv = argv;
	printAscii();
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
	else if (std::string(argv[1]) == "--help")
	{
		//std::cout << argv[1];
		printHelp();
		return;
	}
	else if (argc < 5)
	{
		printUsage();
		return;
	}
	else if (argc == 5) {
		if (std::string(argv[1]) == "--source" && std::string(argv[3]) == "--dest") {
			//fire up doppelganger
			new owoganger(std::string(argv[2]), std::string(argv[4]), SOURCE_RAW);

		}
	}
	else if (argc > 5) { //we have options then
		if (std::string(argv[5]) == "--xor") {
			//check if we have key supplied
			if (argc >= 7) {
				std::string xor_key = std::string(argv[6]);
				new owoganger(std::string(argv[2]), std::string(argv[4]), SOURCE_XOR,std::string(argv[7]));
			}
			else {
				std::cout << "No XOR key supplied" << std::endl;
			}
		}
		else if (std::string(argv[5]) == "--base64") {
			new owoganger(std::string(argv[2]), std::string(argv[4]), SOURCE_B64);
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
	cout << "Usage: " << argv[0] << " --source %PATH --dest %PATH [OPTIONS]" << std::endl;
	//TODO Add some encoders, like XOR, b64 and etc

}

void argParser::printHelp() {
	using namespace std;
	printUsage();
	cout << "\t--source\tTo specify source path to payload" << endl;
	cout << "\t--dest\t\tTo specify destination path to file which will be transacted" << endl;
	cout << "\t--xor KEY\tsource file will be read and decrypted with supplied key" << endl;
	cout << "\t--b64\t\tsource file will be read and decoded from base64" << endl;
}

void argParser::printAscii() {
	using namespace std;
	cout << endl;
	cout << R"(  _____      _____   __ _  __ _ _ __   __ _  ___ _ __ )" << endl;
	cout << R"( / _ \ \ /\ / / _ \ / _` |/ _` | '_ \ / _` |/ _ \ '__|)" << endl;
	cout << R"(| (_) \ V  V / (_) | (_| | (_| | | | | (_| |  __/ |   )" << endl;
	cout << R"( \___/ \_/\_/ \___/ \__, |\__,_|_| |_|\__, |\___|_|   )" << endl;
	cout << R"(                     __/ |             __/ |          )" << endl;
	cout << R"(                    |___/             |___/           )" << endl;
	cout << endl << "[!] Currently works only under 64bit with 64bit injectable binary" << endl;
}