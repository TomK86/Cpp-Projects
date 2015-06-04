# Skinner Project

Final coursework assignment for IN3016 - Software Agents at City University London (Spring 2015)

This project consists of two applications, 'skinner' and 'skinnerMaze', which utilize machine learning algorithms to simulate the famous operant conditioning experiments first developed by B. F. Skinner in 1938.  For more details on this project, please see the report labeled 'Thomas_Kelly_Coursework.pdf' in this repository.

To compile the two applications, download all of the files in this repository to any directory.  Navigate to the directory in your terminal and run the following command:
```
make
```
This will create two executable files, 'skinner' and 'skinnerMaze'.  To run the first application, use the following command:
```
./skinner skinner_R.csv
```
You can replace the provided R-matrix with your own .csv file (see the report for details on how to do this).  To run the second application, run any of the following commands:
```
./skinnerMaze
./skinnerMaze <max episodes>
./skinnerMaze <max episodes> <discount factor>
./skinnerMaze <max episodes> <discount factor> <learning rate>
```
If you do not provide the optional arguments, the application will use the default values for them instead.

Developed by Tom Kelly, 2015

Contact me at TomK0874@gmail.com
