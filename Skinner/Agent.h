/*
 * Agent class -- Contains the persistent hunger value, pain value,
 *                and time step associated with an agent's
 *                experiences in the maze, as well as methods to
 *                retrieve and update them.
 */

 #ifndef AGENT_H
 #define AGENT_H

class Agent
{
private:
  int h, p, t; // hunger value, pain value, time step

public:
  Agent(); // contructor
  ~Agent() {}; // destructor
  void timeStep(int, int); // update agent member variables
  int getH(); // get current hunger value
  int getP(); // get current pain value
  int getT(); // get current time step
};

#endif
