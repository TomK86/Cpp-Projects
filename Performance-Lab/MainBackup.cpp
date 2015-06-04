#include <stdio.h>
#include "cs1300bmp.h"
#include <iostream>
#include <fstream>
#include "Filter.h"

using namespace std;

#include "rtdsc.h"

//
// Forward declare the functions
//
Filter * readFilter(string filename);
double applyFilter(Filter *filter, cs1300bmp *input, cs1300bmp *output);

int
main(int argc, char **argv)
{

  if ( argc < 2) {
    fprintf(stderr,"Usage: %s filter inputfile1 inputfile2 .... \n", argv[0]);
  }

  //
  // Convert to C++ strings to simplify manipulation
  //
  string filtername = argv[1];

  //
  // remove any ".filter" in the filtername
  //
  string filterOutputName = filtername;
  string::size_type loc = filterOutputName.find(".filter");
  if (loc != string::npos) {
    //
    // Remove the ".filter" name, which should occur on all the provided filters
    //
    filterOutputName = filtername.substr(0, loc);
  }

  Filter *filter = readFilter(filtername);

  double sum = 0.0;
  int samples = 0;

  for (int inNum = 2; inNum < argc; inNum++) {
    string inputFilename = argv[inNum];
    string outputFilename = "filtered-" + filterOutputName + "-" + inputFilename;
    struct cs1300bmp *input = new struct cs1300bmp;
    struct cs1300bmp *output = new struct cs1300bmp;
    int ok = cs1300bmp_readfile( (char *) inputFilename.c_str(), input);

    if ( ok ) {
      double sample = applyFilter(filter, input, output);
      sum += sample;
      samples++;
      cs1300bmp_writefile((char *) outputFilename.c_str(), output);
    }
  }
  fprintf(stdout, "Average cycles per sample is %f\n", sum / samples);

}

struct Filter *
readFilter(string filename)
{
  ifstream input(filename.c_str());

  if ( ! input.bad() ) {
    int size = 0;
    input >> size;
    Filter *filter = new Filter(size);
    int div;
    input >> div;
    filter -> setDivisor(div);
    for (int i=0; i < size; i++) {
      for (int j=0; j < size; j++) {
	int value;
	input >> value;
	filter -> set(i,j,value);
      }
    }
    return filter;
  }
}


double
applyFilter(struct Filter *filter, cs1300bmp *input, cs1300bmp *output)
{

  long long cycStart, cycStop;

  cycStart = rdtscll();

  output -> width = input -> width;
  output -> height = input -> height;
  //int size = filter -> getSize(); 
  int Divisor = filter -> divisor;
  int width = input->width;
  int height = input->height;
  //int block = 5;
 
  int filter_00 = filter->data[0];
  int filter_01 = filter->data[1];
  int filter_02 = filter->data[2];
  int filter_10 = filter->data[3]; 
  int filter_11 = filter->data[4]; 
  int filter_12 = filter->data[5]; 
  int filter_20 = filter->data[6];
  int filter_21 = filter->data[7];
  int filter_22 = filter->data[8];
  
//Land of fencepost errors. 
	
for(int row = 1; row <  height - 1; row++) {
	for(int col = 1; col < width - 2; col += 2) {	
	
	int value00 = 0;
	int value01 = 0; 
	int value02 = 0;
	int value10 = 0;
	int value11 = 0;
	int value12 = 0;
	
	value00 +=  input -> color[0][row - 1][col - 1] * filter_00;
	value01 +=  input -> color[1][row - 1][col - 1] * filter_00;
    value02 +=  input -> color[2][row - 1][col - 1] * filter_00;
    value10 +=  input -> color[0][row - 1][col] * filter_00; 
    value11 +=  input -> color[1][row - 1][col] * filter_00;
    value12 +=  input -> color[2][row - 1][col] * filter_00;

    value00 +=  input -> color[0][row - 1][col] * filter_01;
	value01 +=  input -> color[1][row - 1][col] * filter_01;
    value02 +=  input -> color[2][row - 1][col] * filter_01;
    value10 +=  input -> color[0][row - 1][col+1] * filter_01; 
    value11 +=  input -> color[1][row - 1][col+1] * filter_01;
    value12 +=  input -> color[2][row - 1][col+1] * filter_01;
   
    value00 +=  input -> color[0][row - 1][col + 1] * filter_02;
	value01 +=  input -> color[1][row - 1][col + 1] * filter_02;
    value02 +=  input -> color[2][row - 1][col + 1] * filter_02;
    value10 +=  input -> color[0][row - 1][col+2] * filter_02; 
    value11 +=  input -> color[1][row - 1][col+2] * filter_02;
    value12 +=  input -> color[2][row - 1][col+2] * filter_02;
  
    value00 +=  input -> color[0][row][col - 1] * filter_10;
	value01 +=  input -> color[1][row][col - 1] * filter_10;
    value02 +=  input -> color[2][row][col - 1] * filter_10;
    value10 +=  input -> color[0][row][col] * filter_10; 
    value11 +=  input -> color[1][row][col] * filter_10;
    value12 +=  input -> color[2][row][col] * filter_10;

    value00 +=  input -> color[0][row][col] * filter_11;
	value01 +=  input -> color[1][row][col] * filter_11;
    value02 +=  input -> color[2][row][col] * filter_11;
    value10 +=  input -> color[0][row][col + 1] * filter_11; 
    value11 +=  input -> color[1][row][col + 1] * filter_11;
    value12 +=  input -> color[2][row][col + 1] * filter_11;

    value00 +=  input -> color[0][row][col + 1] * filter_12;
	value01 +=  input -> color[1][row][col + 1] * filter_12;
    value02 +=  input -> color[2][row][col + 1] * filter_12;
    value10 +=  input -> color[0][row][col+2] * filter_12; 
    value11 +=  input -> color[1][row][col+2] * filter_12;
    value12 +=  input -> color[2][row][col+2] * filter_12;
  
    value00 +=  input -> color[0][row + 1][col - 1] * filter_20;
	value01 +=  input -> color[1][row + 1][col - 1] * filter_20;
    value02 +=  input -> color[2][row + 1][col - 1] * filter_20;
    value10 +=  input -> color[0][row + 1][col] * filter_20; 
    value11 +=  input -> color[1][row + 1][col] * filter_20;
    value12 +=  input -> color[2][row + 1][col] * filter_20;
    
    value00 +=  input -> color[0][row + 1][col] * filter_21;
	value01 +=  input -> color[1][row + 1][col] * filter_21;
    value02 +=  input -> color[2][row + 1][col] * filter_21;
    value10 +=  input -> color[0][row + 1][col + 1] * filter_21; 
    value11 +=  input -> color[1][row + 1][col + 1] * filter_21;
    value12 +=  input -> color[2][row + 1][col + 1] * filter_21;
    
    value00 +=  input -> color[0][row + 1][col + 1] * filter_22;
	value01 +=  input -> color[1][row + 1][col + 1] * filter_22;
    value02 +=  input -> color[2][row + 1][col + 1] * filter_22;
    value10 +=  input -> color[0][row + 1][col + 2] * filter_22; 
    value11 +=  input -> color[1][row + 1][col + 2] * filter_22;
    value12 +=  input -> color[1][row + 1][col + 2] * filter_22; 
   
  
    //READABILITY
	
	value00 = value00 / Divisor;
	if ( value00  < 0 ) { value00 = 0; }
	if ( value00  > 255 ) { value00 = 255; }
	value01 = value01 / Divisor;
	if ( value01  < 0 ) { value01 = 0; }
	if ( value01  > 255 ) { value01 = 255; }
	value02 = value02 / Divisor;
	if ( value02  < 0 ) { value02 = 0; }
	if ( value02  > 255 ) { value02 = 255; }
	
	value10 = value10 / Divisor;
	if ( value10  < 0 ) { value10 = 0; }
	if ( value10  > 255 ) { value10 = 255; }
	value11 = value11 / Divisor;
	if ( value11  < 0 ) { value11 = 0; }
	if ( value11  > 255 ) { value11 = 255; }
	value12 = value12 / Divisor;
	if ( value12  < 0 ) { value12 = 0; }
	if ( value12  > 255 ) { value12 = 255; }
	
	
	output -> color[0][row][col] = value00;
	output -> color[0][row][col+1] = value10;
	
	output -> color[1][row][col] = value01;
	output -> color[1][row][col+1] = value11;
	
	output -> color[2][row][col] = value02;
	output -> color[2][row][col+1] = value12;
	
	
      }
      
    int col = width - 2;  
    int value00 = 0;
	int value01 = 0; 
	int value02 = 0;
	
	value00 +=  input -> color[0][row - 1][col - 1] * filter_00;
	value01 +=  input -> color[1][row - 1][col - 1] * filter_00;
    value02 +=  input -> color[2][row - 1][col - 1] * filter_00;
    
    value00 +=  input -> color[0][row - 1][col] * filter_01;
	value01 +=  input -> color[1][row - 1][col] * filter_01;
    value02 +=  input -> color[2][row - 1][col] * filter_01;
    
    value00 +=  input -> color[0][row - 1][col + 1] * filter_02;
	value01 +=  input -> color[1][row - 1][col + 1] * filter_02;
    value02 +=  input -> color[2][row - 1][col + 1] * filter_02;
    
    value00 +=  input -> color[0][row][col - 1] * filter_10;
	value01 +=  input -> color[1][row][col - 1] * filter_10;
    value02 +=  input -> color[2][row][col - 1] * filter_10;
    
    value00 +=  input -> color[0][row][col] * filter_11;
	value01 +=  input -> color[1][row][col] * filter_11;
    value02 +=  input -> color[2][row][col] * filter_11;
    
    value00 +=  input -> color[0][row][col + 1] * filter_12;
	value01 +=  input -> color[1][row][col + 1] * filter_12;
    value02 +=  input -> color[2][row][col + 1] * filter_12;
    
    value00 +=  input -> color[0][row + 1][col - 1] * filter_20;
	value01 +=  input -> color[1][row + 1][col - 1] * filter_20;
    value02 +=  input -> color[2][row + 1][col - 1] * filter_20;
    
    value00 +=  input -> color[0][row + 1][col] * filter_21;
	value01 +=  input -> color[1][row + 1][col] * filter_21;
    value02 +=  input -> color[2][row + 1][col] * filter_21;
    
    value00 +=  input -> color[0][row + 1][col + 1] * filter_22;
	value01 +=  input -> color[1][row + 1][col + 1] * filter_22;
    value02 +=  input -> color[2][row + 1][col + 1] * filter_22;
    
    value00 = value00 / Divisor;
	if ( value00  < 0 ) { value00 = 0; }
	if ( value00  > 255 ) { value00 = 255; }
	value01 = value01 / Divisor;
	if ( value01  < 0 ) { value01 = 0; }
	if ( value01  > 255 ) { value01 = 255; }
	value02 = value02 / Divisor;
	if ( value02  < 0 ) { value02 = 0; }
	if ( value02  > 255 ) { value02 = 255; }
	
	output -> color[0][row][col] = value00;
	output -> color[1][row][col] = value01;
	output -> color[2][row][col] = value02;
	
    }

/*Clean Up Loops
for (int row = height-((height-2)%block + 1); row < height - 1; row++){
for (int col = 1; col < width - 1; col++){
	int value0 = 0;
	int value1 = 0; 
	int value2 = 0; 
	int filter_t = filter_00;
	value0 +=  input -> color[0][row - 1][col - 1] * filter_t;
	value1 +=  input -> color[1][row - 1][col - 1] * filter_t;
    value2 +=  input -> color[2][row - 1][col - 1] * filter_t;
    filter_t = filter_01;
    value0 +=  input -> color[0][row - 1][col] * filter_t;
	value1 +=  input -> color[1][row - 1][col] * filter_t;
    value2 +=  input -> color[2][row - 1][col] * filter_t;
    filter_t = filter_02;
    value0 +=  input -> color[0][row - 1][col + 1] * filter_t;
	value1 +=  input -> color[1][row - 1][col + 1] * filter_t;
    value2 +=  input -> color[2][row - 1][col + 1] * filter_t;
    filter_t = filter_10;
    value0 +=  input -> color[0][row][col - 1] * filter_t;
	value1 +=  input -> color[1][row][col - 1] * filter_t;
    value2 +=  input -> color[2][row][col - 1] * filter_t;
    filter_t = filter_11;
    value0 +=  input -> color[0][row][col] * filter_t;
	value1 +=  input -> color[1][row][col] * filter_t;
    value2 +=  input -> color[2][row][col] * filter_t;
    filter_t = filter_12;
    value0 +=  input -> color[0][row][col + 1] * filter_t;
	value1 +=  input -> color[1][row][col + 1] * filter_t;
    value2 +=  input -> color[2][row][col + 1] * filter_t;
    filter_t = filter_20;
    value0 +=  input -> color[0][row + 1][col - 1] * filter_t;
	value1 +=  input -> color[1][row + 1][col - 1] * filter_t;
    value2 +=  input -> color[2][row + 1][col - 1] * filter_t;
    filter_t = filter_21;
    value0 +=  input -> color[0][row + 1][col] * filter_t;
	value1 +=  input -> color[1][row + 1][col] * filter_t;
    value2 +=  input -> color[2][row + 1][col] * filter_t;
    filter_t = filter_22;
    value0 +=  input -> color[0][row + 1][col + 1] * filter_t;
	value1 +=  input -> color[1][row + 1][col + 1] * filter_t;
    value2 +=  input -> color[2][row + 1][col + 1] * filter_t;
    //MAINTAINABILITY
	
	value0 = value0 / Divisor;
	if ( value0  < 0 ) { value0 = 0; }
	if ( value0  > 255 ) { value0 = 255; }
	value1 = value1 / Divisor;
	if ( value1  < 0 ) { value1 = 0; }
	if ( value1  > 255 ) { value1 = 255; }
	value2 = value2 / Divisor;
	if ( value2  < 0 ) { value2 = 0; }
	if ( value2  > 255 ) { value2 = 255; }
	output -> color[0][row][col] = value0;
	output -> color[1][row][col] = value1;
	output -> color[2][row][col] = value2;
    }
  }
    
for (int row = 1 ; row < height - 1; row++){
for (int col = width-((width-2)%block + 1); col < width - 1; col++){
	int value0 = 0;
	int value1 = 0; 
	int value2 = 0; 
	int filter_t = filter_00;
	value0 +=  input -> color[0][row - 1][col - 1] * filter_t;
	value1 +=  input -> color[1][row - 1][col - 1] * filter_t;
    value2 +=  input -> color[2][row - 1][col - 1] * filter_t;
    filter_t = filter_01;
    value0 +=  input -> color[0][row - 1][col] * filter_t;
	value1 +=  input -> color[1][row - 1][col] * filter_t;
    value2 +=  input -> color[2][row - 1][col] * filter_t;
    filter_t = filter_02;
    value0 +=  input -> color[0][row - 1][col + 1] * filter_t;
	value1 +=  input -> color[1][row - 1][col + 1] * filter_t;
    value2 +=  input -> color[2][row - 1][col + 1] * filter_t;
    filter_t = filter_10;
    value0 +=  input -> color[0][row][col - 1] * filter_t;
	value1 +=  input -> color[1][row][col - 1] * filter_t;
    value2 +=  input -> color[2][row][col - 1] * filter_t;
    filter_t = filter_11;
    value0 +=  input -> color[0][row][col] * filter_t;
	value1 +=  input -> color[1][row][col] * filter_t;
    value2 +=  input -> color[2][row][col] * filter_t;
    filter_t = filter_12;
    value0 +=  input -> color[0][row][col + 1] * filter_t;
	value1 +=  input -> color[1][row][col + 1] * filter_t;
    value2 +=  input -> color[2][row][col + 1] * filter_t;
    filter_t = filter_20;
    value0 +=  input -> color[0][row + 1][col - 1] * filter_t;
	value1 +=  input -> color[1][row + 1][col - 1] * filter_t;
    value2 +=  input -> color[2][row + 1][col - 1] * filter_t;
    filter_t = filter_21;
    value0 +=  input -> color[0][row + 1][col] * filter_t;
	value1 +=  input -> color[1][row + 1][col] * filter_t;
    value2 +=  input -> color[2][row + 1][col] * filter_t;
    filter_t = filter_22;
    value0 +=  input -> color[0][row + 1][col + 1] * filter_t;
	value1 +=  input -> color[1][row + 1][col + 1] * filter_t;
    value2 +=  input -> color[2][row + 1][col + 1] * filter_t;
    
	
	value0 = value0 / Divisor;
	if ( value0  < 0 ) { value0 = 0; }
	if ( value0  > 255 ) { value0 = 255; }
	value1 = value1 / Divisor;
	if ( value1  < 0 ) { value1 = 0; }
	if ( value1  > 255 ) { value1 = 255; }
	value2 = value2 / Divisor;
	if ( value2  < 0 ) { value2 = 0; }
	if ( value2  > 255 ) { value2 = 255; }
	output -> color[0][row][col] = value0;
	output -> color[1][row][col] = value1;
	output -> color[2][row][col] = value2;
   }
}		
*/		

  cycStop = rdtscll();
  double diff = cycStop - cycStart;
  double diffPerPixel = diff / (output -> width * output -> height);
  fprintf(stderr, "Took %f cycles to process, or %f cycles per pixel\n",
	  diff, diff / (output -> width * output -> height));
  return diffPerPixel;
}


	/*value0 +=  input -> color[0][row - 1][col - 1] * filter_00;
	value0 +=  input -> color[0][row - 1][col] * filter_01;
	value0 +=  input -> color[0][row - 1][col + 1] * filter_02;
	value0 +=  input -> color[0][row][col - 1] * filter_10;
	value0 +=  input -> color[0][row][col] * filter_11;
	value0 +=  input -> color[0][row][col + 1] * filter_12;
	value0 +=  input -> color[0][row + 1][col - 1] * filter_20;
	value0 +=  input -> color[0][row + 1][col] * filter_21;
	value0 +=  input -> color[0][row + 1][col + 1] * filter_22;
	
	value1 +=  input -> color[1][row - 1][col - 1] * filter_00;
	value1 +=  input -> color[1][row - 1][col] * filter_01;
	value1 +=  input -> color[1][row - 1][col + 1] * filter_02;
	value1 +=  input -> color[1][row][col - 1] * filter_10;
	value1 +=  input -> color[1][row][col] * filter_11;
	value1 +=  input -> color[1][row][col + 1] * filter_12;
	value1 +=  input -> color[1][row + 1][col - 1] * filter_20;
	value1 +=  input -> color[1][row + 1][col] * filter_21;
	value1 +=  input -> color[1][row + 1][col + 1] * filter_22;
	
	value2 +=  input -> color[2][row - 1][col - 1] * filter_00;
	value2 +=  input -> color[2][row - 1][col] * filter_01;
	value2 +=  input -> color[2][row - 1][col + 1] * filter_02;
	value2 +=  input -> color[2][row][col - 1] * filter_10;
	value2 +=  input -> color[2][row][col] * filter_11;
	value2 +=  input -> color[2][row][col + 1] * filter_12;
	value2 +=  input -> color[2][row + 1][col - 1] * filter_20;
	value2 +=  input -> color[2][row + 1][col] * filter_21;
	value2 +=  input -> color[2][row + 1][col + 1] * filter_22;
    */

