all: Main.o 
	g++ Main.o -o project4

Main.o: Main.cc
	g++ -c -std=c++11 Main.cc

clean:
	rm -rf *o  project4
