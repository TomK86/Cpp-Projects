/*
 * Agent class implementation -- see header file 'Agent.h' for more info
 */

#include "Agent.h"

// Constructor
Agent::Agent()
{
  h = 0;
  p = 0;
  t = 0;
}

// Increment time step and update hunger/pain values
void Agent::timeStep(int hunger, int pain)
{
  if (h + hunger < 0) { h = 0; } // hunger value cannot be negative
  else { h += hunger; }

  if (p + pain < 0) { p = 0; } // pain value cannot be negative
  else { p += pain; }

  t++; // increment current time step by 1
}

// Get current hunger value
int Agent::getH()
{
  return h;
}

// Get current pain value
int Agent::getP()
{
  return p;
}

// Get current time step
int Agent::getT()
{
  return t;
}