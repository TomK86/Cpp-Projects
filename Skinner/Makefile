SOURCE=skinnerMaze.cpp Agent.cpp Room.cpp
CC=g++ -Wall -Wextra

all: skinner skinnerMaze

skinner: skinner.cpp
	$(CC) $^ -o skinner

skinnerMaze: $(SOURCE)
	$(CC) $(SOURCE) -o skinnerMaze

clean:
	rm -f skinner skinnerMaze results.csv