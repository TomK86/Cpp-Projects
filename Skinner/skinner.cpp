#include <iostream>
#include <fstream>
#include <string>
#include <random>

using namespace std;

double Rmat[7][7][2];
double Qmat[7][7][2];
double Gamma = 0.5f;
string state[] = {"000", "001", "010", "011", "100", "101", "110", "111"};

double max(double a, double b)
{
  if (a >= b) { return a; }
  else { return b; }
}

double min(double a, double b)
{
  if (a <= b) { return a; }
  else { return b; }
}

int maxQidx(int s)
{
  int idx = 0;
  int i;

  for (i = 1; i < 7; i++) {
    if (Qmat[s][idx][0] < Qmat[s][i][0]) { idx = i; }
  }

  return idx;
}

int minQidx(int s)
{
  int idx = 0;
  int i;

  for (i = 1; i < 7; i++) {
    if (Qmat[s][idx][1] > Qmat[s][i][1]) { idx = i; }
  }
  
  return idx;
}

int getA(int s)
{
  if (s == 0) {
    int i = ((int) random()) % 3;
    if (i == 0) { return 0; }
    if (i == 1) { return 1; }
    else { return 3; }
  }
  else if (s == 1) {
    int i = ((int) random()) % 2;
    if (i == 0) { return 2; }
    else { return 4; }
  }
  else if (s == 2) {
    int i = ((int) random()) % 2;
    if (i == 0) { return 2; }
    else { return 5; }
  }
  else if (s == 3) { return 6; }
  else if (s == 4) {
    int i = ((int) random()) % 2;
    if (i == 0) { return 4; }
    else { return 5; }
  }
  else if (s == 5) { return 6; }
  else if (s == 6) { return 6; }
  else {
    cout << "Error: State index out of bounds\n";
    exit(0);
  }
}

void getQ(int s, int a)
{
  if (s == 0) { // s0 (000) -> { a0 (001), a1 (010), a3 (100) }
    if (a == 0) {
      Qmat[0][0][0] = Rmat[0][0][0] + (Gamma * max(Qmat[1][2][0], Qmat[1][4][0]));
      Qmat[0][0][1] = Rmat[0][0][1] + (Gamma * min(Qmat[1][2][1], Qmat[1][4][1]));
    }
    else if (a == 1) {
      Qmat[0][1][0] = Rmat[0][1][0] + (Gamma * max(Qmat[2][2][0], Qmat[2][5][0]));
      Qmat[0][1][1] = Rmat[0][1][1] + (Gamma * min(Qmat[2][2][1], Qmat[2][5][1]));
    }
    else if (a == 3) {
      Qmat[0][3][0] = Rmat[0][3][0] + (Gamma * max(Qmat[4][4][0], Qmat[4][5][0]));
      Qmat[0][3][1] = Rmat[0][3][1] + (Gamma * min(Qmat[4][4][1], Qmat[4][5][1]));
    }
    else {
      cout << "Error: Action index out of bounds\n";
      exit(0);
    }
  }
  else if (s == 1) { // s1 (001) -> { a2 (011), a4 (101) }
    if (a == 2) {
      Qmat[1][2][0] = Rmat[1][2][0] + (Gamma * Qmat[3][6][0]);
      Qmat[1][2][1] = Rmat[1][2][1] + (Gamma * Qmat[3][6][1]);
    }
    else if (a == 4) {
      Qmat[1][4][0] = Rmat[1][4][0] + (Gamma * Qmat[5][6][0]);
      Qmat[1][4][1] = Rmat[1][4][1] + (Gamma * Qmat[5][6][1]);
    }
    else {
      cout << "Error: Action index out of bounds\n";
      exit(0);
    }
  }
  else if (s == 2) { // s2 (010) -> { a2 (011), a5 (110) }
    if (a == 2) {
      Qmat[2][2][0] = Rmat[2][2][0] + (Gamma * Qmat[3][6][0]);
      Qmat[2][2][1] = Rmat[2][2][1] + (Gamma * Qmat[3][6][1]);
    }
    else if (a == 5) {
      Qmat[2][5][0] = Rmat[2][5][0] + (Gamma * Qmat[6][6][0]);
      Qmat[2][5][1] = Rmat[2][5][1] + (Gamma * Qmat[6][6][1]);
    }
    else {
      cout << "Error: Action index out of bounds\n";
      exit(0);
    }
  }
  else if (s == 3) { // s3 (011) -> a6 (111)
    if (a == 6) {
      Qmat[3][6][0] = Rmat[3][6][0];
      Qmat[3][6][1] = Rmat[3][6][1];
    }
    else {
      cout << "Error: Action index out of bounds\n";
      exit(0);
    }
  }
  else if (s == 4) { // s4 (100) -> { a4 (101), a5 (110) }
    if (a == 4) {
      Qmat[4][4][0] = Rmat[4][4][0] + (Gamma * Qmat[5][6][0]);
      Qmat[4][4][1] = Rmat[4][4][1] + (Gamma * Qmat[5][6][1]);
    }
    else if (a == 5) {
      Qmat[4][5][0] = Rmat[4][5][0] + (Gamma * Qmat[6][6][0]);
      Qmat[4][5][1] = Rmat[4][5][1] + (Gamma * Qmat[6][6][1]);
    }
    else {
      cout << "Error: Action index out of bounds\n";
      exit(0);
    }
  }
  else if (s == 5) { // s5 (101) -> a6 (111)
    if (a == 6) {
      Qmat[5][6][0] = Rmat[5][6][0];
      Qmat[5][6][1] = Rmat[5][6][1];
    }
    else {
      cout << "Error: Action index out of bounds\n";
      exit(0);
    }
  }
  else if (s == 6) { // s6 (110) -> a6 (111)
    if (a == 6) {
      Qmat[6][6][0] = Rmat[6][6][0];
      Qmat[6][6][1] = Rmat[6][6][1];
    }
    else {
      cout << "Error: Action index out of bounds\n";
      exit(0);
    }
  }
  else {
    cout << "Error: State index out of bounds\n";
    exit(0);
  }
}

int main(int argc, char* argv[])
{
  if (argc < 2) {
    cout << "Usage is './skinner <R matrix>'\n";
    exit(0);
  }
  else {
    int s, a, ep;

    char* infilename = argv[1];
    ifstream infile;
    infile.open(infilename);

    for (s = 0; s < 7; s++) {
      for (a = 0; a < 7; a++) {
        string Rval;
        getline(infile, Rval, ',');
        Rmat[s][a][0] = stof(Rval);
        getline(infile, Rval, ',');
        Rmat[s][a][1] = stof(Rval);
        Qmat[s][a][0] = 0.0f;
        Qmat[s][a][1] = 0.0f;
      }
    }

    infile.close();

    for (ep = 0; ep < 10; ep++) { // Limit of 10 episodes
      s = 0; // Initial state is {0, 0, 0}
      do { // Do the following while not at goal state F(5)
        a = getA(s); // Select a random action given current state
        getQ(s, a); // Update Q matrix based on selected action
        s = a + 1; // Set the selected action as the current state
      } while (s != 7); // End do-while loop when goal state is reached
    } // End for loop when 10 episodes have been completed

    // Print Qmat to outfile
    ofstream outfile;
    outfile.open("results.csv");
    for (s = 0; s < 7; s++) {
      for (a = 0; a < 7; a++) {
        outfile << Qmat[s][a][0] << "," << Qmat[s][a][1];
        if ((s != 6) || (a != 6)) { outfile << ","; }
        else { outfile << "\n"; }
      }
    }
    outfile.close();

    // Print optimal sequence to std::cout
    s = 0;
    cout << "\n Optimal Sequence\n==================\n";
    cout << state[s] << "\n";
    while (s != 7) {
      int maxi = maxQidx(s);
      int mini = minQidx(s);

      if (maxi == mini) {
        s = maxi + 1;
        cout << state[s] << "\n";
      }
      else {
        cout << "Warning: Maximum appetitive and minimum noxious sequences do not match!\n";
        cout << "Current state: " << state[s] << "\n";
        cout << "Maximum appetitive action: " << state[maxi+1] << "\n";
        cout << "Minimum noxious action: " << state[mini+1] << "\n";
        exit(0);
      }
    }
    cout << "Goal state reached!\n\n";

    return 0;
  }
}