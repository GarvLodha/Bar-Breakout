CXX=g++
CXXFLAGS=-DIB_USE_STD_STRING -Wall -Wno-switch
ROOT_DIR=.
##BASE_SRC_DIR=${ROOT_DIR}/PosixSocketClient
BASE_SRC_DIR=${ROOT_DIR}
INCLUDES=-I${ROOT_DIR}/Shared/ -I${BASE_SRC_DIR}
##TARGET=PosixSocketClientTest
##TARGET=BarBreakoutbimla429Port7495ClientId0
##TARGET=BarBreakoutU551398Port7496ClientId0
##TARGET=BarBreakoutU557255Port7497ClientId0
##TARGET=BarBreakouthdpap479Port7498ClientId0
##TARGET=BarBreakoutDU109151Port7491ClientId0
TARGET=BarBreakoutedemoPort7500ClientId0


$(TARGET):
##	$(CXX) $(CXXFLAGS) $(INCLUDES) -o AliLine.o -c $(BASE_SRC_DIR)/AliLine.cxx	##appended
##	$(CXX) $(CXXFLAGS) $(INCLUDES) -o EClientSocketBase.o -c $(BASE_SRC_DIR)/EClientSocketBase.cpp
	$(CXX) $(CXXFLAGS) -o EClientSocketBase.o -c $(BASE_SRC_DIR)/EClientSocketBase.cpp
##	$(CXX) $(CXXFLAGS) $(INCLUDES) -o EPosixClientSocket.o -c $(BASE_SRC_DIR)/EPosixClientSocket.cpp
	$(CXX) $(CXXFLAGS) -o EPosixClientSocket.o -c $(BASE_SRC_DIR)/EPosixClientSocket.cpp
##	$(CXX) $(CXXFLAGS) $(INCLUDES) -o PosixTestClient.o -c PosixTestClient.cpp
##	$(CXX) $(CXXFLAGS) -o PosixTestClient.o -c PosixTestClient.cpp
	$(CXX) $(CXXFLAGS) -I /usr/local/include/ta-lib/ -o PosixTestClient.o -c PosixTestClient.cpp
##	$(CXX) $(CXXFLAGS) $(INCLUDES) -o Main.o -c Main.cpp
	$(CXX) $(CXXFLAGS) -o Main.o -c Main.cpp
##	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ EClientSocketBase.o EPosixClientSocket.o PosixTestClient.o Main.o
##	$(CXX) $(CXXFLAGS) -L/usr/local/lib/ta-lib/ -o $@ AliLine.o EClientSocketBase.o EPosixClientSocket.o PosixTestClient.o Main.o -lta_lib
	$(CXX) $(CXXFLAGS) -L/usr/local/lib/ta-lib/ -o $@  EClientSocketBase.o EPosixClientSocket.o PosixTestClient.o Main.o -lta_lib

clean:
	rm -f $(TARGET) *.o Main.o PosixTestClient.o EPosixClientSocket.o EClientSocketBase.o
