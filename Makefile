Boids: main.o
	g++ main.o -o Boids -lsfml-graphics -lsfml-window -lsfml-system

main.o: main.cpp
	g++ -c main.cpp