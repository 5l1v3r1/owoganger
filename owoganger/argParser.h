#pragma once

class argParser // we dont need any extra libraries, i'd better shitcode
{
public:
	argParser(int, char**);
	~argParser();

private:
	void parse();
	void printUsage();
	void printHelp();
	void printAscii();
	int argc;
	char **argv;
};



