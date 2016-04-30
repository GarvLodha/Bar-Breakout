/*******On Wed, 8 June, 2011: Modified Function 'processMessages' to request details of all Orders executed on a day at the end of the trading session.*******/
/*******On Thu, 9 June, 2011: Modified Function 'reqExecutions' to request details of all Orders placed after '09:05:00' on a trading day.*******/
# include <iostream>
# include <fstream>	//object:'stockFile'
#include <stdio.h>
#include <string.h>	//appended
#include "ta_libc.h"	//appended
#include <stdlib.h>	//function: 'exit()'

using namespace std; 
#include "PosixTestClient.h"

#include "EPosixClientSocket.h"
#include "EPosixClientSocketPlatform.h"

#include "Contract.h"
#include "Order.h"
#include "Execution.h"
//#include "AliLine.h"

const int PING_DEADLINE = 2; // seconds
//const int SLEEP_BETWEEN_PINGS = 180; // seconds
//const int SLEEP_BETWEEN_PINGS = 170; // seconds
//const int SLEEP_BETWEEN_PINGS = 60; // seconds
const int SLEEP_BETWEEN_PINGS = 120; // seconds
//static int j = 0;
///////////////////////////////////////////////////////////
// member funcs
PosixTestClient::PosixTestClient()
	: m_pClient(new EPosixClientSocket(this))
	, m_state(ST_CONNECT)
	, m_sleepDeadline(0)
	, m_orderId(0)
{
}

//PosixTestClient::PosixTestClient(int capitalPerStock, int portfolioSize, Contract* portfolio)
PosixTestClient::PosixTestClient(int capitalPerStock, int clientId, int portfolioSize, Contract* portfolio)
	: m_pClient(new EPosixClientSocket(this))
	, m_state(ST_CONNECT)
	, m_sleepDeadline(0)
	, m_orderId(0)
	, m_portfolioSize(portfolioSize)
	, m_portfolio(portfolio)
	, m_capitalPerStock(capitalPerStock)
	, m_totalQuantity(new long[portfolioSize])
	, m_resetBuffers(1)
	, m_closeAllShortPositions(0)
	, m_closeAllLongPositions(0)
	, m_clientId(clientId)
{
/*******Assign memory.*******/
	m_signalBuffer = new char *[m_portfolioSize];
	
	for(int i = 0; i < m_portfolioSize ; i++)
	{
		m_signalBuffer[i] = new char[2];
	}

/*******Initialize the array.*******/
/*	for(int i = 0; i < m_portfolioSize; i++)
	{
//		for(int j = 0; j < 2; j++)
//		{
			m_signalBuffer[i][0] = 'N';
			m_signalBuffer[i][1] = 'N';
//		}
	}*/
}

PosixTestClient::~PosixTestClient()
{
}

bool PosixTestClient::connect(const char *host, unsigned int port, int clientId)
{
	// trying to connect
	printf( "Connecting to %s:%d clientId:%d\n", !( host && *host) ? "127.0.0.1" : host, port, clientId);

	bool bRes = m_pClient->eConnect( host, port, clientId);

	if (bRes) {
		printf("In connect(). Value of m_state is: %i.\n", m_state);
		printf( "Connected to %s:%d clientId:%d\n", !( host && *host) ? "127.0.0.1" : host, port, clientId);
	}
	else
		printf( "Cannot connect to %s:%d clientId:%d\n", !( host && *host) ? "127.0.0.1" : host, port, clientId);

	return bRes;
}

void PosixTestClient::disconnect() const
{
	m_pClient->eDisconnect();

	printf ( "Disconnected\n");
}

bool PosixTestClient::isConnected() const
{
//	printf("In isConnected(). Value of m_state is: %i.\n", m_state);
	return m_pClient->isConnected();
}

void PosixTestClient::processMessages()
{
	fd_set readSet, writeSet, errorSet;
	struct timeval tval;
	tval.tv_usec = 0;
	tval.tv_sec = 0;
	time_t now = time(NULL);
//	printf("In processMessages(). Value of m_state is: %i.\n", m_state);
	switch (m_state) {
		case ST_REQEXECUTIONS:
			cout << "Previous Execution Details are being requested." << endl;
			reqExecutions();
			break;
		case ST_REQHISTORICALDATA:
			cout << "Historical Data request is being placed." << endl;
			reqHistoricalData();
			break;
		case ST_REQHISTORICALDATA_ACK:
			break;
		case ST_PLACEORDER:		
			placeOrder();
		case ST_PLACEORDER_ACK:
			break;
		case ST_CANCELORDER:
			cancelOrder();
			break;
		case ST_CANCELORDER_ACK:
			break;
		case ST_PING:
//			reqCurrentTime();
			printf("Ping.Historical Data request is being placed.\n");
			reqHistoricalData();
			break;
		case ST_PING_ACK:
			printf("Checking Condition.\n");
			if( m_sleepDeadline < now) {
			printf("Condition Checked.\n");
				disconnect();
				return;
			}
//			printf("m_sleepDeadline > now.\n");
			break;
		case ST_IDLE:

			{
				time_t now = ::time(NULL);
				tm * nowPtr = localtime(&now);

				if((nowPtr->tm_hour == 15) && (nowPtr->tm_min == 11))
//				if(m_closeAllShortPositions == 0)
				{
					cout << "Checking condition to exit short positions." << endl;
//					if((nowPtr->tm_hour == 15) && (nowPtr->tm_min == 15))
//					if((nowPtr->tm_hour == 15) && (nowPtr->tm_min == 0))
//					if((nowPtr->tm_hour == 15) && (nowPtr->tm_min == 11))
					if(m_closeAllShortPositions == 0)
					{//close all short positions.
						cout << "Condition to exit short postions is checked." << endl;
//						m_state = ST_CLOSEOPENPOSITIONS;
//						m_state = ST_REQEXECUTIONS;
						reqExecutions();	//Call 'functions' rather than changing state variable('m_state') to prevent socket timeout resulting into API disconnections.
					}
				}

				else
				{ 
//					if(m_closeAllLongPositions == 0)
					if((nowPtr->tm_hour == 15) && (nowPtr->tm_min == 14))
					{					
						cout << "Checking condition to exit long positions." << endl;
//						if((nowPtr->tm_hour == 15) && (nowPtr->tm_min == 28))
//						if((nowPtr->tm_hour == 15) && (nowPtr->tm_min == 10))
//						if((nowPtr->tm_hour == 15) && (nowPtr->tm_min == 14))
						if(m_closeAllLongPositions == 0)
						{//close all long positions.
							cout << "Condition to exit long postions is checked." << endl;
//							m_state = ST_CLOSEOPENPOSITIONS;
//							m_state = ST_REQEXECUTIONS;
							reqExecutions();
						}
					}
				
				/*	else
					{
						m_sleepDeadline = now + SLEEP_BETWEEN_PINGS;
						m_state = ST_IDLE;
					}*/
					else
					{
//						if((nowPtr->tm_hour == 15) && (nowPtr->tm_min == 47))
						if((nowPtr->tm_hour == 15) && (nowPtr->tm_min == 35))
						{//exit the strategy after requesting all executions for the Day.					
							cout << "Request for all Orders executed on today is being placed." << endl;
							reqExecutions();
//							cout << "Thank You for the day. Hope to see you tomorrow again." << endl << "Good Bye.";
//							exit(0);
						}

						else
						{	
							cout << "Value of now is: " << now << endl;
							if( m_sleepDeadline < now) 
							{
								cout << "m_sleepDeadline < now." << endl;
								m_state = ST_PING;
								return;
							}
						}
					}
				}
			}

/*			cout << "Value of now is: " << now << endl;
			if( m_sleepDeadline < now) 
			{
				cout << "m_sleepDeadline < now." << endl;
				m_state = ST_PING;
				return;
			}*/
			break;
		case ST_CLOSEOPENPOSITIONS:
			CloseOpenPositions();
			break;
		}

	
	if( m_sleepDeadline > 0) {
		// initialize timeout with m_sleepDeadline - now
		tval.tv_sec = m_sleepDeadline - now;
//		printf("Hey, Now I am here. Value of m_sleepDeadline is: %i\n", m_sleepDeadline);
//		printf("Hey, Now I am here. Value of now is: %i\n", now);
		cout << "Value of tval.tv_sec is: " <<  tval.tv_sec << endl;
	}

	if( m_pClient->fd() >= 0 ) {
//		printf("Hey again, Now I am here. Condition: m_pClient->fd() >= 0 is evaluated as TRUE.\n");

		
		FD_ZERO( &readSet);
		errorSet = writeSet = readSet;

		FD_SET( m_pClient->fd(), &readSet);

		if( !m_pClient->isOutBufferEmpty())
			FD_SET( m_pClient->fd(), &writeSet);

		FD_CLR( m_pClient->fd(), &errorSet);

		int ret = select( m_pClient->fd() + 1, &readSet, &writeSet, &errorSet, &tval);

		if( ret == 0) { // timeout
			return;
		}

		if( ret < 0) {	// error
			printf("There is an error.\n");
			disconnect();
			return;
		}

		if( m_pClient->fd() < 0)
			return;

		if( FD_ISSET( m_pClient->fd(), &errorSet)) {
			// error on socket
			m_pClient->onError();
		}

		if( m_pClient->fd() < 0)
			return;

		if( FD_ISSET( m_pClient->fd(), &writeSet)) {
			// socket is ready for writing
			m_pClient->onSend();
		}

		if( m_pClient->fd() < 0)
			return;

		if( FD_ISSET( m_pClient->fd(), &readSet)) {
			// socket is ready for reading
			m_pClient->onReceive();
		}
	}
}

//////////////////////////////////////////////////////////////////
// methods
void PosixTestClient::reqCurrentTime()
{
	printf("Requesting Current Time\n");

	// set ping deadline to "now + n seconds"
	m_sleepDeadline = time( NULL) + PING_DEADLINE;

	m_state = ST_PING_ACK;

	m_pClient->reqCurrentTime();
}

//void PosixTestClient::getHistoricalData()
void PosixTestClient::getHistoricalData(char* contractParameter)
{
	
	Contract contract;
//	getContractDetails();
//	contract.symbol = "NIFTY50";
//	contract.symbol = "BOCI";
//	contract.secType = "OPT";
	contract.secType = "STK";
//	contract.conId = 73842992;
//	contract.expiry = "201103";
	contract.expiry = "";
//	contract.right = "C";
	contract.right = "";
//	contract.multiplier = "1";
	contract.multiplier = "";
//	contract.exchange = "NSE";
	contract.exchange = "NSE";
	contract.primaryExchange = "";
	contract.currency = "INR";
//	contract.includeExpired = 0;
//	contract.strike = 5500;
//	contract.localSymbol = "NIFTY11MAR5500CE";
//	contract.localSymbol = "RELCAPITAL";
//	contract.localSymbol =  contractParameter;
	contract.localSymbol =  contractParameter;
	contract.secIdType = "";
	contract.secId = "";

	int durationDays = 7;	//'endDataTime' variable in reqHistoricalData increments by 'durationDays'
	time_t pTime;	//local time
	char str[80];
	struct tm *pPtr;	//pointer to past time

	pTime = time(NULL);	//return system time
	pPtr = localtime(&pTime);	//return time in the form of tm structure
	if(pTime != (time_t)-1)
      	{
/*******Set the beginning date and time*******/
//		pPtr -> tm_year = 111;	//years since 1900
		pPtr -> tm_mon = 2;	//months since January(0-11)
		pPtr -> tm_mday = 13;	//day of the month(1-31)
		pTime = mktime(pPtr);
		while(pTime <= time(NULL))
		{//request historical data in blocks of 5 days
      			strftime(str,80,"%Y%m%d",pPtr);	//format time to string
			string endDateTime;	//endDateTime in IB  format
/*******Concatenate the characters in 'str' array.*******/
			for(int i = 0; i <=7 ; i++)
			{
				endDateTime += str[i];
			}	
			
			endDateTime = endDateTime + " " + "03:30:00";
			cout << "endDateTime is: " << endDateTime << endl; 
			cout << pPtr->tm_mday << "day of the month: " << pPtr->tm_mon << endl;
//			m_pClient -> reqHistoricalData(200, contract, endDateTime, "5 D", "1 min", "Trades", 1, 1);
			m_pClient -> reqHistoricalData(200, contract, endDateTime, "1260 S", "1 min", "Trades", 1, 1);
			pPtr->tm_mday += durationDays;
			pTime = mktime(pPtr);
			sleep(15);	//sleep for 15 secs between each consecutive requests	
		}
	}
	else
	puts("Time is not available.");
}


void PosixTestClient::placeOrder()
{
	Contract contract;
	Order order;

	contract.symbol = "RCAPT";
	contract.secType = "STK";
	contract.conId = 56984773;	//appended
	contract.expiry = "";	//appended
	contract.right = "";	//appended
	contract.multiplier = "";	//appended
//	contract.exchange = "SMART";
	contract.exchange = "NSE";
	contract.primaryExchange = "";	//appended
	contract.currency = "INR";
//	contract.includeExpired = 0;	//appended
//	contract.strike = 5500;	//appended
	contract.localSymbol = "RELCAPITAL";	//appended
	contract.secIdType = "";	//appended
	contract.secId = "";	//appended*/

/*	if(m_signalBuffer[reqId][1] == 66)
	{
		order.action = "BUY";
	}
	
	else
	{
		if(m_signalBuffer[reqId][1] == 83)
		{
			order.action = "SELL";
		}
	}*/

	order.totalQuantity = 1000;
	order.orderType = "LMT";
	order.lmtPrice = 106.10;
//	order.trailStopPrice = UNSET_DOUBLE;
	printf( "Placing Order %ld: %s %ld %s at %f\n", m_orderId, order.action.c_str(), order.totalQuantity, contract.symbol.c_str(), order.lmtPrice);

	m_state = ST_PLACEORDER_ACK;
	m_pClient->placeOrder(m_orderId, contract, order);
//	m_pClient->placeOrder(m_orderId, m_portfolio[reqId], order);
}

void PosixTestClient::placeOrder(int reqId, char* currentSignal, double closePrice)
{	
	cout<<"Hey Garv. This is your order."<<endl;
	printf("Current signal is: %s.\n\n", currentSignal);
	Contract contract;
	contract = m_portfolio[reqId];
	Order order;
	switch(*currentSignal)
	{
		case 66:
			order.action = "BUY";
			break;
		case 83:
			order.action = "SELL";
			break;
	}
//	order.totalQuantity = 1000;
	order.orderType = "MKT";
//	long lotSize = 1000;
//	long lotSize = (int)(m_capitalPerStock/closePrice);	//For Stocks.
//	long lotSize = 1;	//For Stocks.
	cout << "Checking the condition." << endl;
	if((m_signalBuffer[reqId][1] == 66) | (m_signalBuffer[reqId][1] == 83))
	{		
		cout << "Condition checked." << endl;
		if(m_signalBuffer[reqId][1] != *currentSignal) 
		{//Square-Off open positions.
//			order.totalQuantity = m_totalQuantity[reqId] + lotSize; 
//			order.totalQuantity = abs(m_totalQuantity[reqId]) + lotSize; //For Stocks.
//			order.totalQuantity = 2*abs(m_totalQuantity[reqId]) + 1;	//For Futures. 
			order.totalQuantity = 2*abs(m_totalQuantity[reqId]);	//For Futures. 
//			printf("Since previous signal was: %i, your open positions of total quantity:%li will be squared-off and new position of %li will be craeted.\n", m_signalBuffer[reqId][1], order.totalQuantity, lotSize);  
//			cout << "Since previous signal was: " << m_signalBuffer[reqId][1] << "your open positions of total quantity: " << m_totalQuantity[reqId] << "will be squared-off and new position of: " << lotSize << "will be created." << endl;	//For Stocks.
			cout << "Since previous signal was: " << m_signalBuffer[reqId][1] << "your open positions of total quantity: " << m_totalQuantity[reqId] << "will be squared-off and new position of: " << m_totalQuantity[reqId] << "will be created." << endl;	//For Futures.
//			m_totalQuantity[reqId] = lotSize;	//For Stocks.
		}
	
		else
		{
			return;	//Do nothing.
/*******Code to add on long/short positions.*******/
/*			order.totalQuantity = lotSize;
			m_totalQuantity[reqId] += order.totalQuantity;*/
		}
		m_signalBuffer[reqId][1] = *currentSignal;
	}
	
	else
	{//1st order of the strategy.
		m_signalBuffer[reqId][0] = *currentSignal;	//'m_signalBuffer[reqId][0]' stores the value of the first signal generated for a stock.
		m_signalBuffer[reqId][1] = *currentSignal;
		long lotSize = 1;	//For Futures.
		order.totalQuantity = lotSize;	//initial order quantity;
/////		m_totalQuantity[reqId] = order.totalQuantity; //For 'Stocks'.
	}
	
	delete currentSignal;
	time_t ctime = time(NULL);
	printf( "Placing Order %ld: %s %ld %s at %f at time: %s\n", m_orderId, order.action.c_str(), order.totalQuantity, contract.symbol.c_str(), order.lmtPrice, asctime(localtime(&ctime)));
	m_pClient->placeOrder(m_orderId, contract, order);
	m_orderId++;
	cout << "Value of next valid OrderId is: " << m_orderId++ << endl;
	cout << "Exiting Place Order()." << endl;
}

void PosixTestClient::cancelOrder()
{
	printf( "Cancelling Order %ld\n", m_orderId);

	m_state = ST_CANCELORDER_ACK;

	m_pClient->cancelOrder(m_orderId);
}

///////////////////////////////////////////////////////////////////
// events
void PosixTestClient::orderStatus(OrderId orderId, const IBString &status, int filled,
	   int remaining, double avgFillPrice, int permId, int parentId,
	   double lastFillPrice, int clientId, const IBString& whyHeld)

{
//	m_orderId++;	//appended
	if( orderId == m_orderId) {

		if( m_state == ST_PLACEORDER_ACK && (status == "PreSubmitted" || status == "Submitted"))
			m_state = ST_CANCELORDER;

		if( m_state == ST_CANCELORDER_ACK && status == "Cancelled")
			m_state = ST_PING;

		printf( "Order: id=%ld, status=%s\n", orderId, status.c_str());
	}
	printf( "Order: id=%ld, status=%s\n", orderId, status.c_str());	//appended
}

void PosixTestClient::nextValidId( OrderId orderId)
{
	printf("In nextValidId(). Value of m_state is:%i.\n", m_state);
	m_orderId = orderId;
	cout << "Next valid Order Id is: " << orderId << endl;
//	m_state = ST_PLACEORDER;
//	m_state = ST_REQHISTORICALDATA;
	m_state = ST_REQEXECUTIONS;
	printf("In nextValidId(). Value of m_state is:%i.\n", m_state);
}

void PosixTestClient::currentTime( long time)
{
	if ( m_state == ST_PING_ACK) {
		time_t t = ( time_t)time;
		struct tm * timeinfo = localtime (&t);
		printf( "The current date/time is: %s", asctime( timeinfo));

		time_t now = ::time(NULL);
		m_sleepDeadline = now + SLEEP_BETWEEN_PINGS;

		m_state = ST_IDLE;
	}
}

void PosixTestClient::error(const int id, const int errorCode, const IBString errorString)
{
	printf( "Error id=%d, errorCode=%d, msg=%s\n", id, errorCode, errorString.c_str());

	if( id == -1 && errorCode == 1100) // if "Connectivity between IB and TWS has been lost"
		disconnect();
}

void PosixTestClient::tickPrice( TickerId tickerId, TickType field, double price, int canAutoExecute) {}
void PosixTestClient::tickSize( TickerId tickerId, TickType field, int size) {}
void PosixTestClient::tickOptionComputation( TickerId tickerId, TickType tickType, double impliedVol, double delta,
											 double optPrice, double pvDividend,
											 double gamma, double vega, double theta, double undPrice) {}
void PosixTestClient::tickGeneric(TickerId tickerId, TickType tickType, double value) {}
void PosixTestClient::tickString(TickerId tickerId, TickType tickType, const IBString& value) {}
void PosixTestClient::tickEFP(TickerId tickerId, TickType tickType, double basisPoints, const IBString& formattedBasisPoints,
							   double totalDividends, int holdDays, const IBString& futureExpiry, double dividendImpact, double dividendsToExpiry) {}
void PosixTestClient::openOrder( OrderId orderId, const Contract&, const Order&, const OrderState& ostate) {}
void PosixTestClient::openOrderEnd() {}
void PosixTestClient::winError( const IBString &str, int lastError) {}
void PosixTestClient::connectionClosed() {}
void PosixTestClient::updateAccountValue(const IBString& key, const IBString& val,
										  const IBString& currency, const IBString& accountName) {}
void PosixTestClient::updatePortfolio(const Contract& contract, int position,
		double marketPrice, double marketValue, double averageCost,
		double unrealizedPNL, double realizedPNL, const IBString& accountName){}
void PosixTestClient::updateAccountTime(const IBString& timeStamp) {}
void PosixTestClient::accountDownloadEnd(const IBString& accountName) {}
void PosixTestClient::contractDetails( int reqId, const ContractDetails& contractDetails) {}
void PosixTestClient::bondContractDetails( int reqId, const ContractDetails& contractDetails) {}
void PosixTestClient::contractDetailsEnd( int reqId) {}
void PosixTestClient::execDetails( int reqId, const Contract& contract, const Execution& execution) 
{
//	cout << "reqId: " << reqId << "contract.symbol: " << contract.symbol << " execution.execId: " << execution.execId << " execution.exchange: " << execution.exchange << " execution.side: " << execution.side << " execution.shares: " << execution.shares << " execution.price: " << execution.price << " execution.permId: " << execution.permId << " execution.clientId: " << execution.clientId << " execution.orderId: " << execution.orderId <<" execution.cumQty: " << execution.cumQty << endl;
	cout << "reqId: " << reqId << " contract.symbol: " << contract.symbol << " execution.execId: " << execution.execId << " execution.time: " << execution.time << " execution.acctNumber: " << execution.acctNumber << " execution.exchange: " << execution.exchange << " execution.side: " << execution.side << " execution.shares: " << execution.shares << " execution.price: " << execution.price << " execution.permId: " << execution.permId << " execution.clientId: " << execution.clientId << " execution.orderId: " << execution.orderId << " execution.liquidation: " << execution.liquidation << " execution.cumQty: " << execution.cumQty << " execution.avgPrice: " << execution.avgPrice << endl;
//	m_totalQuantity[reqId] = execution.cumQty;
//	if(reqId > 0)
	if(reqId > -1)
	{
		if(execution.side == "BOT")
		{
			m_totalQuantity[reqId] += execution.shares;
			m_signalBuffer[reqId][1] = 66;
			cout << "m_totalQuantity for a stock: " << contract.symbol << " is: " << m_totalQuantity[reqId] << endl; 
		}
		else
		{
			if(execution.side == "SLD")
			{
				m_totalQuantity[reqId] -= execution.shares;
				m_signalBuffer[reqId][1] = 83;
				cout << "m_totalQuantity for a stock: " << contract.symbol << " is: " << m_totalQuantity[reqId] << endl; 
			}

			else
			{
				m_totalQuantity[reqId] = 0;
				m_signalBuffer[reqId][1] = 0;
				cout << "m_totalQuantity for a stock: " << contract.symbol << " is: " << m_totalQuantity[reqId] << endl; 
			}
		}

		if(m_closeAllLongPositions==1)
		{
			ofstream executionReport;
			executionReport.open("ClientExecutions.report", ios::app);	
			executionReport << "reqId: " << reqId << " contract.symbol: " << contract.symbol << " execution.execId: " << execution.execId << " execution.time: " << execution.time << " execution.acctNumber: " << execution.acctNumber << " execution.exchange: " << execution.exchange << " execution.side: " << execution.side << " execution.shares: " << execution.shares << " execution.price: " << execution.price << " execution.permId: " << execution.permId << " execution.clientId: " << execution.clientId << " execution.orderId: " << execution.orderId << " execution.liquidation: " << execution.liquidation << " execution.cumQty: " << execution.cumQty << " execution.avgPrice: " << execution.avgPrice << endl;
		}
	}
	
	else
	{//For Futures.
		if(contract.symbol=="HOE")
		{
			m_totalQuantity[0] = execution.shares - m_totalQuantity[0];
			cout << "m_totalQuantity for a stock: " << contract.symbol << " is: " << m_totalQuantity[0] << endl; 
		}

//		if(contract.symbol=="JPA")
		if(contract.symbol=="SBIN")
		{
			m_totalQuantity[1] = execution.shares - m_totalQuantity[1];
			cout << "m_totalQuantity for a stock: " << contract.symbol << " is: " << m_totalQuantity[1] << endl; 
		}

	/*	if(contract.symbol=="SBIN")
		{
			m_totalQuantity[2] = execution.shares - m_totalQuantity[2];
			cout << "m_totalQuantity for a stock: " << contract.symbol << " is: " << m_totalQuantity[2] << endl;
		}

		if(contract.symbol=="SUEL")
		{
			m_totalQuantity[3] = execution.shares - m_totalQuantity[3];
			cout << "m_totalQuantity for a stock: " << contract.symbol << " is: " << m_totalQuantity[3] << endl;
		}*/
	}
		
//	m_signalBuffer[reqId][0] = execution.side;
/*	if(m_resetBuffers == true)
	{
		if(execution.side == "BOT")
		{
			m_signalBuffer[reqId][1] = 66;
		}
		else
		{
			if(execution.side == "SLD")
			{
				m_signalBuffer[reqId][1] = 83;
			}
		
			else
			{
				m_signalBuffer[reqId][1] = 0;
			}
		}
	
		cout << "m_signalBuffer for a stock: " << contract.symbol << " is: " << m_signalBuffer[reqId][1] << endl; 
	}*/

/*	if(m_closeAllLongPositions==1)
	{
		ofstream executionReport;
		executionReport.open("ClientExecutions.report", ios::app);	
		executionReport << "reqId: " << reqId << " contract.symbol: " << contract.symbol << " execution.execId: " << execution.execId << " execution.time: " << execution.time << " execution.acctNumber: " << execution.acctNumber << " execution.exchange: " << execution.exchange << " execution.side: " << execution.side << " execution.shares: " << execution.shares << " execution.price: " << execution.price << " execution.permId: " << execution.permId << " execution.clientId: " << execution.clientId << " execution.orderId: " << execution.orderId << " execution.liquidation: " << execution.liquidation << " execution.cumQty: " << execution.cumQty << " execution.avgPrice: " << execution.avgPrice << endl;
	}*/
}

void PosixTestClient::execDetailsEnd(int reqId) 
{
	cout << "Net Position(m_totalQuantity) for a stock: " << m_portfolio[reqId].symbol << " is: " << m_totalQuantity[reqId] << endl; 
//	m_totalQuantity[reqId] = abs(m_totalQuantity[reqId]);
//	cout << "Value of 'm_totalQuantity' for a stock: " << m_portfolio[reqId].symbol << " is: " << m_totalQuantity[reqId] << endl; 


	if(reqId == m_portfolioSize - 1)
	{
		if(m_resetBuffers==true)
		{//It is executed when program starts.
			m_state = ST_REQHISTORICALDATA;
			m_resetBuffers=false;
			cout << "Value of 'm_resetBuffers' is set to: " << m_resetBuffers << endl;
			cout << "Value of 'm_state' is set to: " << m_state << endl;
		}
		else
		{
			if(!(m_closeAllLongPositions == 1))
			{		
/*				if(m_state == ST_PING_ACK) 
				{
					time_t now = ::time(NULL);
//					m_sleepDeadline = now + SLEEP_BETWEEN_PINGS;
					m_sleepDeadline = now + PING_DEADLINE;
//					m_state = ST_IDLE;
					m_state = ST_CLOSEOPENPOSITIONS;
					cout << "Value of 'm_state' is set to: " << m_state << endl;
				}*/

				m_state = ST_CLOSEOPENPOSITIONS;
				cout << "Value of 'm_state' is set to: " << m_state << endl;

			}
			
			else
			{ 
				cout << "Thank You for the day. Hope to see you tomorrow again." << endl << "Good Bye.";
				exit(0);
			}
			
		}
//		m_resetBuffers = false;
	}

	if(m_state == ST_PING_ACK) 
	{//after each request is received increment the values of 'm_sleepDeadline' to prevent disconnections.
		time_t now = ::time(NULL);
//		m_sleepDeadline = now + SLEEP_BETWEEN_PINGS;
		m_sleepDeadline = now + PING_DEADLINE;
		cout << "After each request is completely responded, value of 'm_sleepDeadline' is incremented to prevent disconnection." << endl;
		cout << "Value of 'm_state' is: " << m_state << endl;
	}
}


void PosixTestClient::updateMktDepth(TickerId id, int position, int operation, int side,
									  double price, int size) {}
void PosixTestClient::updateMktDepthL2(TickerId id, int position, IBString marketMaker, int operation,
										int side, double price, int size) {}
void PosixTestClient::updateNewsBulletin(int msgId, int msgType, const IBString& newsMessage, const IBString& originExch) {}
void PosixTestClient::managedAccounts( const IBString& accountsList) {}
void PosixTestClient::receiveFA(faDataType pFaDataType, const IBString& cxml) {}

void PosixTestClient::historicalData(TickerId reqId, const IBString& date, double open, double high,
									  double low, double close, int volume, int barCount, double WAP, int hasGaps) 
{
//	printf("Hey, Here is the requested data:\n");
	cout << "Hey, Here is the requested data:" << endl;
	cout << reqId << "	" << date << "	" << open << "	" << high <<	"	" << low << "	" << close << "	" << volume << "	" << barCount << "	" << WAP << "	" << hasGaps << endl;
	
	const int dataPoints = 41;
	static int dataElement = 0;
	static double highPrice[dataPoints];
	static double lowPrice[dataPoints];
	static double closePrice[dataPoints];
	if(high!=-1)
	{
		highPrice[dataElement] = high;
		lowPrice[dataElement] = low;
		closePrice[dataElement] = close;
		cout << "Value of dataElement is:" << dataElement << endl;
		dataElement++;
	}
	
	else
	{
		cout << "All above data was retreived for " << m_portfolio[reqId].localSymbol << " by process " << getpid() << endl;
		TechnicalAnalysisMax(reqId, dataElement, highPrice, lowPrice, closePrice);
		if(m_state == ST_PING_ACK) 
		{//Block will be executed only for the first request.
			time_t now = ::time(NULL);
//			tm * nowPtr = localtime(&now);
//			char endTime[80];
//			strftime(endTime, 80, "%H:%M", nowPtr); 
/*			if((nowPtr->tm_hour == 15) && (nowPtr->tm_min == 15))
			{//close all short positions.
				m_state = ST_CLOSEOPENPOSITIONS;
			}

			else
			{ 
				if((nowPtr->tm_hour == 15) && (nowPtr->tm_min == 28))
//				if((nowPtr->tm_hour == 16) && (nowPtr->tm_min == 0))
				{//close all long positions.
					m_state = ST_CLOSEOPENPOSITIONS;
				}
				
				else
				{*/
					m_sleepDeadline = now + SLEEP_BETWEEN_PINGS;
					m_state = ST_IDLE;
					cout << "Value of 'm_state' is set to: " << m_state << endl;
/*				}
			}*/
			
		}
		dataElement = 0;
	}
	
}


void PosixTestClient::scannerParameters(const IBString &xml) {}
void PosixTestClient::scannerData(int reqId, int rank, const ContractDetails &contractDetails,
	   const IBString &distance, const IBString &benchmark, const IBString &projection,
	   const IBString &legsStr) {}
void PosixTestClient::scannerDataEnd(int reqId) {}
void PosixTestClient::realtimeBar(TickerId reqId, long time, double open, double high, double low, double close,
								   long volume, double wap, int count) {}
void PosixTestClient::fundamentalData(TickerId reqId, const IBString& data) {}
void PosixTestClient::deltaNeutralValidation(int reqId, const UnderComp& underComp) {}
void PosixTestClient::tickSnapshotEnd(int reqId) {}

bool PosixTestClient::TechnicalAnalysisMax(int reqId, int dataElement, double* histHighPrice, double* histLowPrice, double* histClosePrice)
{

//	int dataPoints = 42;	//no. of data points in a single historical data request for a 3 mins bar over last 2 hours 3 mins.
	int dataPoints = dataElement;	//no. of data points in a single historical data request for a 3 mins bar over last 2 hours 3 mins.
	cout << "No. of Data Points are:" << dataPoints << endl;
//	int dataPoints = 7;	//no. of data points in a single historical data request for a 1 min bar over last 5 mins and 1 min.
/*	TA_RetCode retCode;
	retCode = TA_Initialize( );

   if( retCode != TA_SUCCESS )
      printf( "Cannot initialize TA-Lib (%d)!\n", retCode );
   else
   {
      printf( "TA-Lib correctly initialized.\n" );*/

      /* ... other TA-Lib functions can be used here. */
//	TA_Real highPrice[42];
	TA_Real highPrice[dataPoints];
//	TA_Real lowPrice[42];
	TA_Real lowPrice[dataPoints];
//	TA_Real closePrice[42];
	TA_Real closePrice[dataPoints];
//	double highPrice[40];

/*******Initialize your high price, low price, close price here.*******/
//	for(int j = 0; j < 42; j++)
	for(int j = 0; j < dataPoints; j++)
	{
//		highPrice[j] = high;
		highPrice[j] = histHighPrice[j];
		lowPrice[j] = histLowPrice[j];
		closePrice[j] = histClosePrice[j];
//		highPrice[j] = j;
	}


/*******Calculates last 30 units Moving Average of Close Price*******/
/*	TA_RetCode retCode;
	retCode = TA_MA(0, 399, &closePrice[0], 30, TA_MAType_SMA, &outBeg, &outNbElement, &out[0]);*/

/*******Buy Signal.*******/	
	TA_Real tamaxOut[1];
	TA_Integer tamaxOutBeg;
	TA_Integer tamaxOutNbElement;
/*******Calculates Maximum 'High Price' of 3 mins Bars over the last 2 Hours.*******/
	TA_RetCode ta_max;
//	ta_max = TA_MAX(0, 40, &highPrice[0], 41, &tamaxOutBeg, &tamaxOutNbElement, &tamaxOut[0]);
//	ta_max = TA_MAX(0, dataPoints - 2, &highPrice[0], dataPoints -1, &tamaxOutBeg, &tamaxOutNbElement, &tamaxOut[0]);

/*******Calculates the Maximum 'Close Price' of 3 mins Bars over the last 2 Hours.*******/
	ta_max = TA_MAX(0, dataPoints - 2, &closePrice[0], dataPoints -1, &tamaxOutBeg, &tamaxOutNbElement, &tamaxOut[0]);

/*******The output is displayed here.*******/
//	printf("tamaxOutNbElement is %d.\n", tamaxOutNbElement);	
	for(int i = 0; i < tamaxOutNbElement; i++)
	{
//		printf("Day %d = %f\n", outBeg + i, out[i]);
//		printf("Highest high over last 2 hours as calculated on %dth period = %f\n", tamaxOutBeg + i, tamaxOut[i]);
//		printf("Highest high over last 30 mins as calculated on %dth period = %f\n", tamaxOutBeg + i, tamaxOut[i]);
//		printf("Highest high over last 5 mins as calculated on %dth period = %f\n", tamaxOutBeg + i, tamaxOut[i]);
		printf("Highest 'Close Price' over last 2 hours as calculated on %dth period = %f\n", tamaxOutBeg + i, tamaxOut[i]);
	}

/*******Sell Signal.*******/	
	TA_Real taminOut[1];
	TA_Integer taminOutBeg;
	TA_Integer taminOutNbElement;
/*******Calculates Minimum Low Price of 3 mins Bars over the last 2 Hours.*******/
	TA_RetCode ta_min;
//	ta_min = TA_MIN(0, 40, &lowPrice[0], 41, &taminOutBeg, &taminOutNbElement, &taminOut[0]);
//	ta_min = TA_MIN(0, dataPoints - 2, &lowPrice[0], dataPoints - 1, &taminOutBeg, &taminOutNbElement, &taminOut[0]);

/*******Calculates Minimum 'Close Price' of 3 mins Bars over the last 2 Hours.*******/
	ta_min = TA_MIN(0, dataPoints - 2, &closePrice[0], dataPoints - 1, &taminOutBeg, &taminOutNbElement, &taminOut[0]);

/*******The output is displayed here.*******/
	
	for(int i = 0; i < taminOutNbElement; i++)
	{
//		printf("Day %d = %f\n", outBeg + i, out[i]);
//		printf("Lowest low over last 2 hours as calculated on %dth period = %f\n", taminOutBeg + i, taminOut[i]);
//		printf("Lowest low over last 30 mins as calculated on %dth period = %f\n", taminOutBeg + i, taminOut[i]);
//		printf("Lowest low over last 5 mins as calculated on %dth period = %f\n", taminOutBeg + i, taminOut[i]);
		printf("Lowest 'Close Price' over last 2 hours as calculated on %dth period = %f\n", taminOutBeg + i, taminOut[i]);
	}



/*******Generate Signals.*******/
	char *  signal = new char;
	int factor = 3;
//	if(tamaxOut[0] < closePrice[41])
	if(tamaxOut[0] < closePrice[dataPoints - 1])
	{//Test for Buy Signal.
		cout << "tamaxOut is:" << tamaxOut[0] << endl;
//		cout << closePrice[41] << endl;
		cout << closePrice[dataPoints - 1] << endl;
		cout << "Hey, Your condition is satisfied for Stock: " << m_portfolio[reqId].localSymbol << ".\n" << endl; 
		strncpy(signal,"BUY",3);
		time_t ptime = time(NULL);	//localtime
		tm * pPtr;
		pPtr = localtime(&ptime);
		while(((pPtr->tm_min) % factor) != 0)
		{
			ptime = time(NULL);
			pPtr = localtime(&ptime);
//			cout << "Current time is:" << asctime(pPtr) << endl;
		}
		cout << "Current time is:" << asctime(pPtr) << endl;
//		cout<<"Your BUY order for Stock: " << m_portfolio[reqId].localSymbol << " will be placed now.\n" << endl;
//		placeOrder(reqId, signal);
//		if(!((pPtr->tm_hour==15) && (pPtr->tm_min>=28)))
//		if(!((pPtr->tm_hour==15) && (pPtr->tm_min>=10)))
//		if(!((pPtr->tm_hour==15) && (pPtr->tm_min>=12)))
//		if((!((pPtr->tm_hour>=15) && (pPtr->tm_min>=12))) && (!((pPtr->tm_hour<=9) && (pPtr->tm_min<=15))))
//		if(!((!((pPtr->tm_hour==9) && (pPtr->tm_min<=15))) ^ (!((pPtr->tm_hour==15) && (pPtr->tm_min>=12)))))
		if(!((!((pPtr->tm_hour==9) && (pPtr->tm_min<=15))) ^ (!((pPtr->tm_hour==15) && (pPtr->tm_min>=0)))))
//		if(!((pPtr->tm_hour==20) && (pPtr->tm_min>=12)))
		{//execute the 'BUY' order only if time is between 0915 and 1512. 
			cout<<"Your BUY order for Stock: " << m_portfolio[reqId].localSymbol << " will be placed now.\n" << endl;
			placeOrder(reqId, signal, closePrice[dataPoints - 1]);
		}
		return true;
	}

	else
	{
//		if(taminOut[0] > closePrice[41])
		if(taminOut[0] > closePrice[dataPoints - 1])
		{//Test for Sell Signal.
			cout << "taminOut is:" << taminOut[0] << endl;
//			cout << closePrice[41] << endl;
			cout << closePrice[dataPoints - 1] << endl;
			cout << "Hey, Your condition is satisfied for Stock: " << m_portfolio[reqId].localSymbol << ".\n" << endl; 
			strncpy(signal, "SELL", 4);
			time_t ptime = time(NULL);	//localtime
			tm * pPtr;
			pPtr = localtime(&ptime);
			while(((pPtr->tm_min) % factor) != 0)
			{
				ptime = time(NULL);
				pPtr = localtime(&ptime);
//				cout << "Current time is:" << asctime(pPtr) << endl;
			}
			cout << "Current time is:" << asctime(pPtr) << endl;
//			cout<<"Your SELL order for Stock: " << m_portfolio[reqId].localSymbol << " will be placed now.\n" << endl;
//			placeOrder(reqId, signal);
//			if(!((pPtr->tm_hour==15) && (pPtr->tm_min>=15))) 
//			if(!((pPtr->tm_hour==15) && (pPtr->tm_min>=0))) 
//			if(!((pPtr->tm_hour==15) && (pPtr->tm_min>=9))) 
//			if((!((pPtr->tm_hour==15) && (pPtr->tm_min>=9))) && (!((pPtr->tm_hour<=9) && (pPtr->tm_min<=15))))
//			if(!((!((pPtr->tm_hour==9) && (pPtr->tm_min<=15))) ^ (!((pPtr->tm_hour==15) && (pPtr->tm_min>=9)))))
			if(!((!((pPtr->tm_hour==9) && (pPtr->tm_min<=15))) ^ (!((pPtr->tm_hour==15) && (pPtr->tm_min>=0)))))
//			if(!((pPtr->tm_hour==20) && (pPtr->tm_min>=9))) 
			{//execute the 'SELL' order only if time is between 0915 and 1509.
				cout<<"Your SELL order for Stock: " << m_portfolio[reqId].localSymbol << " will be placed now.\n" << endl;
				placeOrder(reqId, signal, closePrice[dataPoints - 1]);
			}
			return true;
		}
	}			

	return false;
	
/*      TA_Shutdown();
   }*/
}

void PosixTestClient::reqExecutions()
{
	ExecutionFilter filter;
//	filter.m_clientId = 0;
	filter.m_clientId = m_clientId;
//	filter.m_symbol = "SBIN";
//	filter.m_secType = "STK";
	filter.m_secType = "FUT";
	filter.m_exchange = "NSE";
//	filter.m_side = "BUY";
	
	
/*******Request Execution Details for Orders placed after '09:05:05'.*******/ 
/*	time_t pTime = time(NULL);
	struct tm *pPtr = localtime(&pTime); 
	char * str = new char[80];
	strftime(str, 9, "%Y%m%d", pPtr);
	string timeFilter;
	for(int i = 0; i < 8; i++)
	{
		timeFilter = timeFilter + str[i];
	}
	
	timeFilter = timeFilter + "-" + "09:05:00";
	filter.m_time = timeFilter;*/

	m_sleepDeadline = time( NULL) + PING_DEADLINE;
	m_state = ST_PING_ACK;
	cout << "Value of m_state is: " << m_state << endl;
	for(int reqId = 0; reqId < m_portfolioSize; reqId++)
	{
		m_totalQuantity[reqId]=0;
		filter.m_symbol = m_portfolio[reqId].symbol; 
		cout << "Execution Details with filters: " << endl;
		cout << "filter.m_clientId = " << m_clientId << endl;
//		cout << "filter.m_secType = " << "STK" << endl;
		cout << "filter.m_secType = " << "FUT" << endl;
		cout << "filter.m_exchange = " << "NSE" << endl;
		cout << "filter.m_time = " << filter.m_time << endl;
		cout << "filter.m_symbol = " << filter.m_symbol << endl;
		cout << "will be requested now." << endl;
		m_pClient -> reqExecutions(reqId, filter);
	}
/*	if(m_resetBuffers==true)
	{
		m_state = ST_REQHISTORICALDATA;
		m_resetBuffers=false;
	}
	else
	{
		m_state = ST_CLOSEOPENPOSITIONS;
	}*/
}

void PosixTestClient::reqHistoricalData()
{
	char buffer[80];
//	time_t now = time(NULL);
/*******Code appended to make requests at tm_min = integral multiple of 3 and tm_sec = 0.*******/
	int factor = 3;
	time_t ptime = time(NULL);	//localtime
	tm * pPtr;
	pPtr = localtime(&ptime);
	while(((pPtr->tm_sec !=0)))
	{//loose seconds first
		ptime = time(NULL);
		pPtr = localtime(&ptime);
//		cout << "Current time is:" << asctime(pPtr) << endl;
	}

	while(((pPtr->tm_min) % factor) != 0)
	{//loose minutes then.
		ptime = time(NULL);
		pPtr = localtime(&ptime);
//		cout << "Current time is:" << asctime(pPtr) << endl;
	}	
	
//	strftime(buffer, 80, "%Y%m%d %H:%M:%S", localtime(&now));
	strftime(buffer, 80, "%Y%m%d %H:%M:%S", pPtr);
	m_sleepDeadline = time( NULL) + PING_DEADLINE;
	m_state = ST_PING_ACK;
	for(int reqId = 0; reqId < m_portfolioSize; reqId++)
	{
//		cout << "Requesting 3 min bars over last 2 hours and 3 mins at time:" << buffer << " for Contract: " << m_portfolio[reqId].localSymbol << "at time:" << buffer << endl;
		cout << "Requesting 3 min bars over last 2 hours and 3 mins for Contract: " << m_portfolio[reqId].localSymbol << " at time: " << buffer << endl;
//		m_pClient -> reqHistoricalData(reqId, m_portfolio[reqId], buffer, "360 S", "1 min", "Trades", 1, 1);	//request data over last 5 mins and 1 min
		m_pClient -> reqHistoricalData(reqId, m_portfolio[reqId], buffer, "7380 S", "3 mins", "Trades", 1, 1);	//request data over last 2 hours and 3 mins
	}
		
	cout << "Hey, I am out of the loop." << endl;
}

void PosixTestClient::CloseOpenPositions()
{	
	Order order;
	order.orderType = "MKT";

	time_t now = ::time(NULL);
	tm * nowPtr = localtime(&now);

//	reqExecutions();
//	if(m_closeAllShortPositions == 0)
//	if((m_closeAllShortPositions == 0) && (nowPtr->tm_min==0))
	if((m_closeAllShortPositions == 0) && (nowPtr->tm_min==11))
//	if((m_closeAllShortPositions == 0) && (nowPtr->tm_min==41))
	{//close all short positions.
		for(int reqId = 0; reqId < m_portfolioSize; reqId++)
		{//close all short positions.
//			if(nowPtr->tm_min == 15 && m_signalBuffer[reqId][1] == 83)
//			if(nowPtr->tm_min == 0 && m_signalBuffer[reqId][1] == 83)
//			if(m_signalBuffer[reqId][1] == 83)
//			if((m_signalBuffer[reqId][1] == 83) && (m_totalQuantity[reqId] != 0))
			if(m_totalQuantity[reqId] < 0)
			{//close all short positions.
				cout << "Net Position(m_totalQuantity) for a stock: " << m_portfolio[reqId].symbol << " is: " << m_totalQuantity[reqId] << endl; 
//				m_totalQuantity[reqId] = abs(m_totalQuantity[reqId]);
				cout << "Value of 'm_totalQuantity' for a stock: " << m_portfolio[reqId].symbol << " is: " << m_totalQuantity[reqId] << endl;
				order.action = "BUY";
				order.totalQuantity = abs(m_totalQuantity[reqId]);
				printf("Placing Order %ld: %s %ld %s at %f at time: %s\n", m_orderId, order.action.c_str(), order.totalQuantity, m_portfolio[reqId].symbol.c_str(), order.lmtPrice, asctime(localtime(&now)));
				m_pClient->placeOrder(m_orderId, m_portfolio[reqId], order);
				m_orderId++;
				cout << "Value of next valid OrderId is: " << m_orderId++ << endl;
			}
		}
		m_closeAllShortPositions = 1;
		cout << "Value of m_closeAllShortPositions is: " << m_closeAllShortPositions << endl;
		cout << "All short positions are closed." << endl;

	}
		
//	else
//	if(m_closeAllLongPositions == 0)
//	if((m_closeAllLongPositions == 0) && (nowPtr->tm_min==10))
	if((m_closeAllLongPositions == 0) && (nowPtr->tm_min==14))
//	if((m_closeAllLongPositions == 0) && (nowPtr->tm_min==44))
	{//close all long positions.
		for(int reqId = 0; reqId < m_portfolioSize; reqId++)
		{
//			if(nowPtr->tm_min == 28 && m_signalBuffer[reqId][1] == 66)
//			if(nowPtr->tm_min == 10 && m_signalBuffer[reqId][1] == 66)
//			if(m_signalBuffer[reqId][1] == 66)
//			if((m_signalBuffer[reqId][1] == 66) && (m_totalQuantity[reqId] != 0))
			if(m_totalQuantity[reqId] > 0)
			{//close all long positions.
				cout << "Net Position(m_totalQuantity) for a stock: " << m_portfolio[reqId].symbol << " is: " << m_totalQuantity[reqId] << endl; 
//				m_totalQuantity[reqId] = abs(m_totalQuantity[reqId]);
				cout << "Value of 'm_totalQuantity' for a stock: " << m_portfolio[reqId].symbol << " is: " << m_totalQuantity[reqId] << endl;
				order.action = "SELL";
//				order.totalQuantity = m_totalQuantity[reqId];
				order.totalQuantity = abs(m_totalQuantity[reqId]);
				printf( "Placing Order %ld: %s %ld %s at %f at time: %s\n", m_orderId, order.action.c_str(), order.totalQuantity, m_portfolio[reqId].symbol.c_str(), order.lmtPrice, asctime(localtime(&now)));
				m_pClient->placeOrder(m_orderId, m_portfolio[reqId], order);
				m_orderId++;
				cout << "Value of next valid OrderId is: " << m_orderId++ << endl;
			}
		}	
		m_closeAllLongPositions = 1;
		cout << "Value of m_closeAllLongPositions is: " << m_closeAllShortPositions << endl;
		cout << "All Long positions are closed." << endl;
	}

//	}
/*
	int factor = 2;
//	time_t ptime = time(NULL);	//localtime
	now = time(NULL);	//localtime
//	tm * pPtr;
//	pPtr = localtime(&ptime);
	nowPtr = localtime(&now);
//	while(((pPtr->tm_sec !=0)))
	while(((nowPtr->tm_sec !=0)))
	{//loose seconds first
//		ptime = time(NULL);
		now = time(NULL);
//		pPtr = localtime(&ptime);
		nowPtr = localtime(&now);
//		cout << "Current time is:" << asctime(pPtr) << endl;
	}


//	while(((pPtr->tm_min) % factor) != 0)
	while(((nowPtr->tm_min) % factor) != 0)
	{//loose minutes then.
//		ptime = time(NULL);
		now = time(NULL);
//		pPtr = localtime(&ptime);
		nowPtr = localtime(&now);
//		cout << "Current time is:" << asctime(pPtr) << endl;
	}	*/
	m_state = ST_PING;
	cout << "Value of 'm_state' is set to: " << m_state << endl;
}

/*void PosixTestClient::CloseOpenPositions()
{
	for(int reqId = 0; reqId < m_portfolioSize; reqId++)
	{
		if(m_signalBuffer[reqId][1] == m_signalBuffer[reqId][0])
		{//check for open position.
			char* orderAction;
			orderAction = new char;
			switch(m_signalBuffer[reqId][1])
				{
					case 66:
					{//close open long position.
//						strncpy(orderAction, "SELL", 4);
						orderAction[0] = 'S';
						placeOrder(reqId, orderAction, 0);
					}
					case 83:
					{//close open short position.
//						strncpy(orderAction, "BUY", 3);
						orderAction[0] = 'B';
						placeOrder(reqId, orderAction, 0);
					}
				}
			delete orderAction;
		}
	}
}*/
			
