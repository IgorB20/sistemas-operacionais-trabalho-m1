CXX = g++
CXXFLAGS = -std=c++17 -pthread

all: servidor cliente

servidor: servidor.cpp
	$(CXX) $(CXXFLAGS) servidor.cpp -o servidor

cliente: cliente.cpp
	$(CXX) $(CXXFLAGS) cliente.cpp -o cliente

run-servidor: servidor
	./servidor

run-cliente: cliente
	./cliente

clean:
	rm -f servidor cliente