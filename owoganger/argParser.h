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
	int argc;
	char **argv;
};



