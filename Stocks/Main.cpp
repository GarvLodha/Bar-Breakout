#ifdef _WIN32
# include <Windows.h>
# define sleep( seconds) Sleep( seconds * 1000);
#else
# include <unistd.h>
#endif

#include "Contract.h"
#include "PosixTestClient.h"
#include "EClientSocketBase.h"
#include "EPosixClientSocket.h"	//appended
#include <stdio.h> //functions:printf 
#include <time.h>	//function
#include <iostream>	//appended
//#include "AliLine.h"	//appended
#include <string.h>	//appended
#include <fstream>	//appended
using namespace std;
const unsigned MAX_ATTEMPTS = 50;
//const unsigned MAX_ATTEMPTS = 5;
const unsigned SLEEP_TIME = 10;

struct param
{
	const char* fHost;
	unsigned int fPort;
	int fClientId;
	int fPortfolioSize;
	int fCapitalPerStock;
}Param;

//void ParseCommandLine(int argc, char * * argv);
void ParseCommandLine(int argc, char * * argv, char * inputFileName);
//void MakePortfolio(int portfolioSize, Contract* portfolio);
void MakePortfolio(int portfolioSize, Contract* portfolio, char * portfolioFile);
void ExitWithHelp();

int main(int argc, char** argv)
{
	char portfolioFile[1024];
//	ParseCommandLine(argc, argv);
	ParseCommandLine(argc, argv, portfolioFile);
//	const char* host = argc > 1 ? argv[1] : "";
	const char* host = Param.fHost;
	unsigned int port =Param.fPort;
	int clientId = Param.fClientId;
	unsigned attempt = 0;

	int portfolioSize = Param.fPortfolioSize;
	int capitalPerStock = Param.fCapitalPerStock;

//
	Contract portfolio[portfolioSize];
//	MakePortfolio(portfolioSize, portfolio);
	MakePortfolio(portfolioSize, portfolio, portfolioFile);

	printf( "Start of POSIX Socket Client Test %u\n", attempt);

	for(;;)
		{
		++attempt;
		printf( "Attempt %u of %u\n", attempt, MAX_ATTEMPTS);

//		PosixTestClient client(capitalPerStock, portfolioSize, portfolio);
		PosixTestClient client(capitalPerStock, clientId, portfolioSize, portfolio);
		client.connect( host, port, clientId);

		printf("Parent Process with pid %i started.", getpid());

		while(client.isConnected()) {
	
			client.processMessages();			
		}
		
		if( attempt >= MAX_ATTEMPTS) {
			break;
		}

		printf( "Sleeping %u seconds before next attempt\n", SLEEP_TIME);
		sleep( SLEEP_TIME);
		}

	printf ("End of POSIX Socket Client Test\n");
}

/*******Construct a portfolio from a given file.*******/
//void MakePortfolio(int portfolioSize, Contract* portfolio)
void MakePortfolio(int portfolioSize, Contract* portfolio, char * portfolioFile)
{
	Contract contract;
	char* stockSymbol;
	ifstream stockFile;
//	stockFile.open("Portfolio.list", ios::in);
	stockFile.open(portfolioFile, ios::in);
	if(stockFile.is_open())
	{
		int id = 0;
		while((stockFile.good()) && (id < portfolioSize))
		{		
				cout << "Reading line no." << id << endl;
				stockSymbol = new char[256];
				stockFile.getline(stockSymbol,256, '\n');
				cout << stockSymbol << endl;
				char* token;
				token =	strtok(stockSymbol,"\t");
				int i = 0;
				while(token != NULL)
				{
					switch(i)
					{
						case 0:
							contract.symbol = token;
							token = strtok(NULL, "\t");
							break;
						case 1:
							contract.secType = token;
							token = strtok(NULL, "\t");
							break;
						case 2:
							contract.conId = atoi(token);
							token = strtok(NULL, "\t");
							break;
						case 3:
							contract.exchange = token;
							token = strtok(NULL, "\t");
							break;
						case 4:
							contract.currency = token;
							token = strtok(NULL, "\t");
							break;
						case 5:
							contract.localSymbol = token;
							token = strtok(NULL, "\t");
							break;
					}
					i++;
				}
			cout << "symbol =" << contract.symbol <<endl;
			cout << "secType =" << contract.secType << endl;
			cout << "conId =" << contract.conId << endl;
			cout << "exchange =" << contract.exchange << endl;
			cout << "currency =" << contract.currency << endl;
			cout << "localSymbol =" << contract.localSymbol << endl;
			portfolio[id] = contract;
			delete[] stockSymbol;
			id++;
		}
		stockFile.close();
//disconnect();
	}

	else
	{
		cout << "Stock File does not exist." << endl;
	}
}

//void ParseCommandLine(int argc, char * * argv)
void ParseCommandLine(int argc, char * * argv, char * inputFileName)
{
	int i;
//	void (*print_func)(const char*) = NULL;	// default printing to stdout
//	bool printFunc = false;

	// default values
	Param.fHost = "";
	Param.fPort = 7496;
	Param.fClientId = 0;
//	Param.fCapitalPerStock = 100000;
	Param.fCapitalPerStock = 135000;
	Param.fPortfolioSize = 3;
	// parse options
	for(i=1;i<argc;i++)
	{
		if(argv[i][0] != '-') break;
		if(++i>=argc)
			ExitWithHelp();
		switch(argv[i-1][1])
		{
			case 'c':
				Param.fCapitalPerStock = atoi(argv[i]);
				break;
			case 'h':
				Param.fHost = argv[i];
				break;
			case 'i':
				Param.fClientId = atoi(argv[i]);
				break;
			case 'p':
				Param.fPort = atoi(argv[i]);
				cout << "Value of Param.fPort is: " << Param.fPort << endl;
				break;
			case 's':
				Param.fPortfolioSize = atoi(argv[i]);
				break;
			default:
				fprintf(stderr,"Unknown option: -%c\n", argv[i-1][1]);
				ExitWithHelp();
		}
	}

//	AliGlobalFunctions::SVMSetPrintStringFunction(printFunc);

	// determine filenames

	if(i>=argc)
		ExitWithHelp();

	strcpy(inputFileName, argv[i]);

/*	if(i<argc-1)
		strcpy(modelFileName,argv[i+1]);
	else
	{
		char *p = strrchr(argv[i],'/');
		if(p==NULL)
			p = argv[i];
		else
			++p;
		sprintf(modelFileName,"%s.model",p);
	}*/
}

void ExitWithHelp()
{
	cout << "Dude, There is something wrong." << endl;
}
/*void ParseCommandLine(int argc, char * * argv, char * inputFileName, char * modelFileName)
{
	int i;
//	void (*print_func)(const char*) = NULL;	// default printing to stdout
	bool printFunc = false;

	// default values
	param.fSVMType = AliSVMDataStructures::kCSVC;
	param.fKernelType = AliSVMDataStructures::kRBF;
	param.fDegree = 3;
	param.fGamma = 0;	// 1/num_features
	param.fCoef0 = 0;
	param.fNu = 0.5;
	param.fCacheSize = 100;
	param.fC = 1;
	param.fEPS = 1e-3;
	param.fP = 0.1;
	param.fShrinking = 1;
	param.fProbability = 0;
	param.fNrWeight = 0;
	param.fWeightLabel = NULL;
	param.fWeight = NULL;
	crossValidation = 0;

	// parse options
	for(i=1;i<argc;i++)
	{
		if(argv[i][0] != '-') break;
		if(++i>=argc)
			ExitWithHelp();
		switch(argv[i-1][1])
		{
			case 's':
				param.fSVMType = atoi(argv[i]);
				break;
			case 't':
				param.fKernelType = atoi(argv[i]);
				break;
			case 'd':
				param.fDegree = atoi(argv[i]);
				break;
			case 'g':
				param.fGamma = atof(argv[i]);
				break;
			case 'r':
				param.fCoef0 = atof(argv[i]);
				break;
			case 'n':
				param.fNu = atof(argv[i]);
				break;
			case 'm':
				param.fCacheSize = atof(argv[i]);
				break;
			case 'c':
				param.fC = atof(argv[i]);
				break;
			case 'e':
				param.fEPS = atof(argv[i]);
				break;
			case 'p':
				param.fP = atof(argv[i]);
				break;
			case 'h':
				param.fShrinking = atoi(argv[i]);
				break;
			case 'b':
				param.fProbability = atoi(argv[i]);
				break;
			case 'q':
//				print_func = &print_null;
				printFunc = true;
				i--;
				break;
			case 'v':
				crossValidation = 1;
				nrFold = atoi(argv[i]);
				if(nrFold < 2)
				{
					fprintf(stderr,"n-fold cross validation: n must >= 2\n");
					ExitWithHelp();
				}
				break;
			case 'w':
				++param.fNrWeight;
				param.fWeightLabel = (int *)realloc(param.fWeightLabel,sizeof(int)*param.fNrWeight);
				param.fWeight = (double *)realloc(param.fWeight,sizeof(double)*param.fNrWeight);
				param.fWeightLabel[param.fNrWeight-1] = atoi(&argv[i-1][2]);
				param.fWeight[param.fNrWeight-1] = atof(argv[i]);
				break;
			default:
				fprintf(stderr,"Unknown option: -%c\n", argv[i-1][1]);
				ExitWithHelp();
		}
	}

	AliGlobalFunctions::SVMSetPrintStringFunction(printFunc);

	// determine filenames

	if(i>=argc)
		ExitWithHelp();

	strcpy(inputFileName, argv[i]);

	if(i<argc-1)
		strcpy(modelFileName,argv[i+1]);
	else
	{
		char *p = strrchr(argv[i],'/');
		if(p==NULL)
			p = argv[i];
		else
			++p;
		sprintf(modelFileName,"%s.model",p);
	}
}*/


