#include <stdio.h>
#include "cs1300bmp.h"
#include <iostream>
#include <fstream>
#include "Filter.h"

using namespace std;

#include "rtdsc.h"

// Forward declare the functions
Filter * readFilter(string filename);
double applyFilter(Filter *filter, cs1300bmp *input,
	cs1300bmp *output, const unsigned short filt_case);

int main(int argc, char **argv) {

	if(argc < 2) {
		fprintf(stderr,
			"Usage: %s filter inputfile1 inputfile2 .... \n",
			argv[0]);
	}

	// Convert to C++ strings to simplify manipulation
	string filtername = argv[1];

	// remove any ".filter" in the filtername
	string filterOutputName = filtername;
	string::size_type loc = filterOutputName.find(".filter");
	if(loc != string::npos) {
		// Remove the ".filter" name, which should
		// occur on all the provided filters
		filterOutputName = filtername.substr(0, loc);
	}

	Filter *filter = readFilter(filtername);

	double sum = 0.0;
	int samples = 0;

	unsigned short filt_case;
	if(filterOutputName == "avg") { filt_case = 0; }
	else if(filterOutputName == "edge") { filt_case = 1; }
	else if(filterOutputName == "emboss") { filt_case = 2; }
	else if(filterOutputName == "gauss") { filt_case = 3; }
	else if(filterOutputName == "hline") { filt_case = 4; }
	else if(filterOutputName == "sharpen") { filt_case = 5; }
	else if(filterOutputName == "vline") { filt_case = 6; }
	else { filt_case = 7; } // invalid filter

	for(int inNum = 2; inNum < argc; inNum++) {
		string inputFilename = argv[inNum];
		string outputFilename = "filtered-" + filterOutputName
			+ "-" + inputFilename;
		struct cs1300bmp *input = new struct cs1300bmp;
		struct cs1300bmp *output = new struct cs1300bmp;
		int ok = cs1300bmp_readfile(
			(char *) inputFilename.c_str(), input);

		if(ok) {
			double sample = applyFilter(
				filter, input, output, filt_case);
			sum += sample;
			samples++;
			cs1300bmp_writefile(
				(char *) outputFilename.c_str(), output);
		}
	}
	fprintf(stdout, "Average cycles per sample is %f\n", sum / samples);

}

struct Filter * readFilter(string filename) {  

	ifstream input(filename.c_str());

	if(!(input.bad())) {
		int size = 0;
		input >> size;
		Filter *filter = new Filter(size);
		int div;
		input >> div;
		filter->setDivisor(div);
		for(int i = 0; i < size; ++i) {
			for(int j = 0; j < size; ++j) {
				int value;
				input >> value;
				filter->set(i, j, value);
			}
		}
		return filter;
	}
}

double applyFilter(struct Filter *filter, cs1300bmp *input,
	cs1300bmp *output, const unsigned short filt_case) {

	long long cycStart, cycStop;

	cycStart = rdtscll();

	// output image dimensions must match input image dimensions:
	output->width = input->width;
	output->height = input->height;
	
	int width = input->width; // used to iterate over columns
	int height = input->height - 1; // used to iterate over rows
	
	// color_c points to the 8192 x 8192 matrix corresponding to
	// each RGB value, starting with R (or 0) and incrementing
	unsigned char* color_c = (unsigned char*) input->color;

	switch(filt_case) { // optimize code for the current filter type
		
		case 0: { // 'avg.filter'

			// plane iterates over the RGB values in each pixel
			for(int plane = 0; plane < 3; ++plane) {
				
				// row iterates over the rows of pixels in the image
				for(int row = 1; row < height; ++row) {
					
					// col iterates over the columns of pixels in the image
					int col = 1;
					
					// row_c iterates over the rows in color_c
					unsigned char* row_c = color_c + (8192 * (row - 1));
					
					// iterate over six columns at a time
					for(; col < width - 6; col += 6) {

						// each color value is the sum of itself and
						// the values surrounding it (9 total), each
						// scaled by the corresponding filter gains:
						// ======= avg.filter =======
						//  1  1  1   <-- top row
						//  1  1  1   <-- middle row
						//  1  1  1   <-- bottom row
						
						// top row:
						int value0 = row_c[col-1] + row_c[col]
							+ row_c[col+1];
						int value1 = row_c[col] + row_c[col+1]
							+ row_c[col+2];
						int value2 = row_c[col+1] + row_c[col+2]
							+ row_c[col+3];
						int value3 = row_c[col+2] + row_c[col+3]
							+ row_c[col+4];
						int value4 = row_c[col+3] + row_c[col+4]
							+ row_c[col+5];
						int value5 = row_c[col+4] + row_c[col+5]
							+ row_c[col+6]; 

						row_c += 8192; // MAX_DIM = 8192

						// middle row:
						value0 += row_c[col-1] + row_c[col]
							+ row_c[col+1]; 
						value1 += row_c[col] + row_c[col+1]
							+ row_c[col+2];
						value2 += row_c[col+1] + row_c[col+2]
							+ row_c[col+3];
						value3 += row_c[col+2] + row_c[col+3]
							+ row_c[col+4];
						value4 += row_c[col+3] + row_c[col+4]
							+ row_c[col+5];
						value5 += row_c[col+4] + row_c[col+5]
							+ row_c[col+6];

						row_c += 8192; // MAX_DIM = 8192

						// bottom row:
						value0 += row_c[col-1] + row_c[col]
							+ row_c[col+1]; 
						value1 += row_c[col] + row_c[col+1]
							+ row_c[col+2];
						value2 += row_c[col+1] + row_c[col+2]
							+ row_c[col+3];
						value3 += row_c[col+2] + row_c[col+3]
							+ row_c[col+4];
						value4 += row_c[col+3] + row_c[col+4]
							+ row_c[col+5];
						value5 += row_c[col+4] + row_c[col+5]
							+ row_c[col+6];
							
						row_c -= 16384; // 2 * MAX_DIM = 16384

						// divide each color value by
						// avg.filter's 'div' value (9)
						value0 /= 9;
						value1 /= 9;
						value2 /= 9;
						value3 /= 9;
						value4 /= 9;
						value5 /= 9;
						
						// constrain each color value
						// to be between 0 and 255
						if(value0 > 255) { value0 = 255; }
						else if(value0 < 0) { value0 = 0; }
						if(value1 > 255) { value1 = 255; }
						else if(value1 < 0) { value1 = 0; }
						if(value2 > 255) { value2 = 255; }
						else if(value2 < 0) { value2 = 0; }
						if(value3 > 255) { value3 = 255; }
						else if(value3 < 0) { value3 = 0; }
						if(value4 > 255) { value4 = 255; }
						else if(value4 < 0) { value4 = 0; }
						if(value5 > 255) { value5 = 255; }
						else if(value5 < 0) { value5 = 0; }

						// set each color value in the output image
						output->color[plane][row][col] = value0;
						output->color[plane][row][col+1] = value1;
						output->color[plane][row][col+2] = value2;
						output->color[plane][row][col+3] = value3;
						output->color[plane][row][col+4] = value4;
						output->color[plane][row][col+5] = value5;
						
					} // end 'six-column' for loop
					
					// iterate over the last five columns, one at a time
					for(; col < width - 1; ++col) {
						
						// the color value is the sum of itself and
						// the values surrounding it (9 total), each
						// scaled by the corresponding filter gains:
						// ======= avg.filter =======
						//  1  1  1   <-- top row
						//  1  1  1   <-- middle row
						//  1  1  1   <-- bottom row
						
						// top row:
						int value = row_c[col-1] + row_c[col]
							+ row_c[col+1];
						
						row_c += 8192; // MAX_DIM = 8192
						
						// middle row:
						value += row_c[col-1] + row_c[col]
							+ row_c[col+1];
						
						row_c += 8192; // MAX_DIM = 8192
						
						// bottom row:
						value += row_c[col-1] + row_c[col]
							+ row_c[col+1];
							
						row_c -= 16384; // 2 * MAX_DIM = 16384

						// divide the color value by
						// avg.filter's 'div' value (9)
						value /= 9;
						
						// constrain the color value
						// to be between 0 and 255
						if(value > 255) { value = 255; }
						else if(value < 0) { value = 0; }
						
						// set the color value in the output image
						output->color[plane][row][col] = value;
						
					} // end 'one-column' for loop
					
				} // end 'row' for loop
				
				// color_c now points to the 8192 x 8192 matrix
				// corresponding to the next RGB value
				color_c += 67108864; // MAX_DIM * MAX_DIM = 67108864
				
			} // end 'plane' for loop
			
			break; // return from switch statement
			
		} // end case 0: 'avg.filter'
		
		case 1: { // 'edge.filter'

			// plane iterates over the RGB values in each pixel
			for(int plane = 0; plane < 3; ++plane) {
				
				// row iterates over the rows of pixels in the image
				for(int row = 1; row < height; ++row) {
					
					// col iterates over the columns of pixels in the image
					int col = 1;
					
					// row_c iterates over the rows in color_c
					unsigned char* row_c = color_c + (8192 * (row - 1));
					
					// iterate over six columns at a time
					for(; col < width - 6; col += 6) {
						
						// each color value is the sum of itself and
						// the values surrounding it (9 total), each
						// scaled by the corresponding filter gains:
						// ======= edge.filter =======
						//  1  1  1   <-- top row
						//  1 -7  1   <-- middle row
						//  1  1  1   <-- bottom row

						// top row:
						int value0 = row_c[col-1] + row_c[col]
							+ row_c[col+1];
						int value1 = row_c[col] + row_c[col+1]
							+ row_c[col+2];
						int value2 = row_c[col+1] + row_c[col+2]
							+ row_c[col+3];
						int value3 = row_c[col+2] + row_c[col+3]
							+ row_c[col+4];
						int value4 = row_c[col+3] + row_c[col+4]
							+ row_c[col+5];
						int value5 = row_c[col+4] + row_c[col+5]
							+ row_c[col+6];  

						row_c += 8192; // MAX_DIM = 8192

						// middle row:
						value0 += row_c[col-1] - (row_c[col] * 7)
							+ row_c[col+1];
						value1 += row_c[col] - (row_c[col+1] * 7)
							+ row_c[col+2];
						value2 += row_c[col+1] - (row_c[col+2] * 7)
							+ row_c[col+3];
						value3 += row_c[col+2] - (row_c[col+3] * 7)
							+ row_c[col+4];
						value4 += row_c[col+3] - (row_c[col+4] * 7)
							+ row_c[col+5];
						value5 += row_c[col+4] - (row_c[col+5] * 7)
							+ row_c[col+6];

						row_c += 8192; // MAX_DIM = 8192

						// bottom row:
						value0 += row_c[col-1] + row_c[col]
							+ row_c[col+1]; 
						value1 += row_c[col] + row_c[col+1]
							+ row_c[col+2];
						value2 += row_c[col+1] + row_c[col+2]
							+ row_c[col+3];
						value3 += row_c[col+2] + row_c[col+3]
							+ row_c[col+4];
						value4 += row_c[col+3] + row_c[col+4]
							+ row_c[col+5];
						value5 += row_c[col+4] + row_c[col+5]
							+ row_c[col+6];
							
						row_c -= 16384; // 2 * MAX_DIM = 16384
						
						// constrain each color value
						// to be between 0 and 255
						if(value0 > 255) { value0 = 255; }
						else if(value0 < 0) { value0 = 0; }
						if(value1 > 255) { value1 = 255; }
						else if(value1 < 0) { value1 = 0; }
						if(value2 > 255) { value2 = 255; }
						else if(value2 < 0) { value2 = 0; }
						if(value3 > 255) { value3 = 255; }
						else if(value3 < 0) { value3 = 0; }
						if(value4 > 255) { value4 = 255; }
						else if(value4 < 0) { value4 = 0; }
						if(value5 > 255) { value5 = 255; }
						else if(value5 < 0) { value5 = 0; }

						// set each color value in the output image
						output->color[plane][row][col] = value0;
						output->color[plane][row][col+1] = value1;
						output->color[plane][row][col+2] = value2;
						output->color[plane][row][col+3] = value3;
						output->color[plane][row][col+4] = value4;
						output->color[plane][row][col+5] = value5;
						
					} // end 'six-column' for loop
					
					// iterate over the last five columns, one at a time
					for(; col < width - 1; ++col) {
						
						// the color value is the sum of itself and
						// the values surrounding it (9 total), each
						// scaled by the corresponding filter gains:
						// ======= edge.filter =======
						//  1  1  1   <-- top row
						//  1 -7  1   <-- middle row
						//  1  1  1   <-- bottom row
						
						// top row:
						int value = row_c[col-1] + row_c[col]
							+ row_c[col+1];
						
						row_c += 8192; // MAX_DIM = 8192
						
						// middle row:
						value += row_c[col-1] - (row_c[col] * 7)
							+ row_c[col+1];
						
						row_c += 8192; // MAX_DIM = 8192
						
						// bottom row:
						value += row_c[col-1] + row_c[col]
							+ row_c[col+1];
							
						row_c -= 16384; // 2 * MAX_DIM = 16384

						// constrain the color value
						// to be between 0 and 255
						if(value > 255) { value = 255; }
						else if(value < 0) { value = 0; }
						
						// set the color value in the output image
						output->color[plane][row][col] = value;
						
					} // end 'one-column' for loop
					
				} // end 'row' for loop
				
				// color_c now points to the 8192 x 8192 matrix
				// corresponding to the next RGB value
				color_c += 67108864; // MAX_DIM * MAX_DIM = 67108864
				
			} // end 'plane' for loop
			
			break; // return from switch statement
			
		} // end case 1: 'edge.filter'
		
		case 2: { // 'emboss.filter'

			// plane iterates over the RGB values in each pixel
			for(int plane = 0; plane < 3; ++plane) {
				
				// row iterates over the rows of pixels in the image
				for(int row = 1; row < height; ++row) {
					
					// col iterates over the columns of pixels in the image
					int col = 1;
					
					// row_c iterates over the rows in color_c
					unsigned char* row_c = color_c + (8192 * (row - 1));
					
					// iterate over six columns at a time
					for(; col < width - 6; col += 6) {
						
						// each color value is the sum of itself and
						// the values surrounding it (9 total), each
						// scaled by the corresponding filter gains:
						// ======= emboss.filter =======
						//  1  1 -1   <-- top row
						//  1  1 -1   <-- middle row
						//  1 -1 -1   <-- bottom row

						// top row:
						int value0 = row_c[col-1] + row_c[col]
							- row_c[col+1];
						int value1 = row_c[col] + row_c[col+1]
							- row_c[col+2];
						int value2 = row_c[col+1] + row_c[col+2]
							- row_c[col+3];
						int value3 = row_c[col+2] + row_c[col+3]
							- row_c[col+4];
						int value4 = row_c[col+3] + row_c[col+4]
							- row_c[col+5];
						int value5 = row_c[col+4] + row_c[col+5]
							- row_c[col+6];  

						row_c += 8192; // MAX_DIM = 8192

						// middle row:
						value0 += row_c[col-1] + row_c[col]
							- row_c[col+1]; 
						value1 += row_c[col] + row_c[col+1]
							- row_c[col+2];
						value2 += row_c[col+1] + row_c[col+2]
							- row_c[col+3];
						value3 += row_c[col+2] + row_c[col+3]
							- row_c[col+4];
						value4 += row_c[col+3] + row_c[col+4]
							- row_c[col+5];
						value5 += row_c[col+4] + row_c[col+5]
							- row_c[col+6];

						row_c += 8192; // MAX_DIM = 8192

						// bottom row:
						value0 += row_c[col-1] - row_c[col]
							- row_c[col+1]; 
						value1 += row_c[col] - row_c[col+1]
							- row_c[col+2];
						value2 += row_c[col+1] - row_c[col+2]
							- row_c[col+3];
						value3 += row_c[col+2] - row_c[col+3]
							- row_c[col+4];
						value4 += row_c[col+3] - row_c[col+4]
							- row_c[col+5];
						value5 += row_c[col+4] - row_c[col+5]
							- row_c[col+6];
						
						row_c -= 16384; // 2 * MAX_DIM = 16384
						
						// constrain each color value
						// to be between 0 and 255
						if(value0 > 255) { value0 = 255; }
						else if(value0 < 0) { value0 = 0; }
						if(value1 > 255) { value1 = 255; }
						else if(value1 < 0) { value1 = 0; }
						if(value2 > 255) { value2 = 255; }
						else if(value2 < 0) { value2 = 0; }
						if(value3 > 255) { value3 = 255; }
						else if(value3 < 0) { value3 = 0; }
						if(value4 > 255) { value4 = 255; }
						else if(value4 < 0) { value4 = 0; }
						if(value5 > 255) { value5 = 255; }
						else if(value5 < 0) { value5 = 0; }

						// set each color value in the output image
						output->color[plane][row][col] = value0;
						output->color[plane][row][col+1] = value1;
						output->color[plane][row][col+2] = value2;
						output->color[plane][row][col+3] = value3;
						output->color[plane][row][col+4] = value4;
						output->color[plane][row][col+5] = value5;
						
					} // end 'six-column' for loop
					
					// iterate over the last five columns, one at a time
					for(; col < width - 1; ++col) {
						
						// the color value is the sum of itself and
						// the values surrounding it (9 total), each
						// scaled by the corresponding filter gains:
						// ======= emboss.filter =======
						//  1  1 -1   <-- top row
						//  1  1 -1   <-- middle row
						//  1 -1 -1   <-- bottom row
						
						// top row:
						int value = row_c[col-1] + row_c[col]
							- row_c[col+1];
						
						row_c += 8192; // MAX_DIM = 8192
						
						// middle row:
						value += row_c[col-1] + row_c[col]
							- row_c[col+1];
						
						row_c += 8192; // MAX_DIM = 8192
						
						// bottom row:
						value += row_c[col-1] - row_c[col]
							- row_c[col+1];
						
						row_c -= 16384; // 2 * MAX_DIM = 16384

						// constrain the color value
						// to be between 0 and 255
						if(value > 255) { value = 255; }
						else if(value < 0) { value = 0; }
						
						// set the color value in the output image
						output->color[plane][row][col] = value;
						
					} // end 'one-column' for loop
					
				} // end 'row' for loop
				
				// color_c now points to the 8192 x 8192 matrix
				// corresponding to the next RGB value
				color_c += 67108864; // MAX_DIM * MAX_DIM = 67108864
			
			} // end 'plane' for loop
			
			break; // return from switch statement
			
		} // end case 2: 'emboss.filter'
		
		case 3: { // 'gauss.filter'

			// plane iterates over the RGB values in each pixel
			for(int plane = 0; plane < 3; ++plane) {
				
				// row iterates over the rows of pixels in the image
				for(int row = 1; row < height; ++row) {
					
					// col iterates over the columns of pixels in the image
					int col = 1;
					
					// row_c iterates over the rows in color_c
					unsigned char* row_c = color_c + (8192 * (row - 1));
					
					// iterate over six columns at a time
					for(; col < width - 6; col += 6) {

						// each color value is the sum of itself and
						// the values surrounding it (9 total), each
						// scaled by the corresponding filter gains:
						// ======= gauss.filter =======
						//  0  4  0   <-- top row
						//  4  8  4   <-- middle row
						//  0  4  0   <-- bottom row
						
						// top row:
						int value0 = row_c[col] * 4;
						int value1 = row_c[col+1] * 4;
						int value2 = row_c[col+2] * 4;
						int value3 = row_c[col+3] * 4;
						int value4 = row_c[col+4] * 4;
						int value5 = row_c[col+5] * 4; 

						row_c += 8192; // MAX_DIM = 8192

						// middle row:
						value0 += (row_c[col-1] * 4) + (row_c[col] * 8)
							+ (row_c[col+1] * 4);
						value1 += (row_c[col] * 4) + (row_c[col+1] * 8)
							+ (row_c[col+2] * 4);
						value2 += (row_c[col+1] * 4) + (row_c[col+2] * 8)
							+ (row_c[col+3] * 4);
						value3 += (row_c[col+2] * 4) + (row_c[col+3] * 8)
							+ (row_c[col+4] * 4);
						value4 += (row_c[col+3] * 4) + (row_c[col+4] * 8)
							+ (row_c[col+5] * 4);
						value5 += (row_c[col+4] * 4) + (row_c[col+5] * 8)
							+ (row_c[col+6] * 4);

						row_c += 8192; // MAX_DIM = 8192

						// bottom row:
						value0 += row_c[col] * 4;
						value1 += row_c[col+1] * 4;
						value2 += row_c[col+2] * 4;
						value3 += row_c[col+3] * 4;
						value4 += row_c[col+4] * 4;
						value5 += row_c[col+5] * 4;
						
						row_c -= 16384; // 2 * MAX_DIM = 16384

						// divide each color value by
						// gauss.filter's 'div' value (24)
						value0 /= 24;
						value1 /= 24;
						value2 /= 24;
						value3 /= 24;
						value4 /= 24;
						value5 /= 24;
						
						// constrain each color value
						// to be between 0 and 255
						if(value0 > 255) { value0 = 255; }
						else if(value0 < 0) { value0 = 0; }
						if(value1 > 255) { value1 = 255; }
						else if(value1 < 0) { value1 = 0; }
						if(value2 > 255) { value2 = 255; }
						else if(value2 < 0) { value2 = 0; }
						if(value3 > 255) { value3 = 255; }
						else if(value3 < 0) { value3 = 0; }
						if(value4 > 255) { value4 = 255; }
						else if(value4 < 0) { value4 = 0; }
						if(value5 > 255) { value5 = 255; }
						else if(value5 < 0) { value5 = 0; }

						// set each color value in the output image
						output->color[plane][row][col] = value0;
						output->color[plane][row][col+1] = value1;
						output->color[plane][row][col+2] = value2;
						output->color[plane][row][col+3] = value3;
						output->color[plane][row][col+4] = value4;
						output->color[plane][row][col+5] = value5;
						
					} // end 'six-column' for loop
					
					// iterate over the last five columns, one at a time
					for(; col < width - 1; ++col) {
						
						// the color value is the sum of itself and
						// the values surrounding it (9 total), each
						// scaled by the corresponding filter gains:
						// ======= gauss.filter =======
						//  0  4  0   <-- top row
						//  4  8  4   <-- middle row
						//  0  4  0   <-- bottom row
						
						// top row:
						int value = row_c[col] * 4;
						
						row_c += 8192; // MAX_DIM = 8192
						
						// middle row:
						value += (row_c[col-1] * 4) + (row_c[col] * 8)
							+ (row_c[col+1] * 4);
						
						row_c += 8192; // MAX_DIM = 8192
						
						// bottom row:
						value += row_c[col] * 4;
						
						row_c -= 16384; // 2 * MAX_DIM = 16384

						// divide the color value by
						// gauss.filter's 'div' value (24)
						value /= 24;
						
						// constrain the color value
						// to be between 0 and 255
						if(value > 255) { value = 255; }
						else if(value < 0) { value = 0; }
						
						// set the color value in the output image
						output->color[plane][row][col] = value;
						
					} // end 'one-column' for loop
					
				} // end 'row' for loop
				
				// color_c now points to the 8192 x 8192 matrix
				// corresponding to the next RGB value
				color_c += 67108864; // MAX_DIM * MAX_DIM = 67108864
				
			} // end 'plane' for loop
			
			break; // return from switch statement
			
		} // end case 3: 'gauss.filter'
		
		case 4: { // 'hline.filter'

			// plane iterates over the RGB values in each pixel
			for(int plane = 0; plane < 3; ++plane) {
				
				// row iterates over the rows of pixels in the image
				for(int row = 1; row < height; ++row) {
					
					// col iterates over the columns of pixels in the image
					int col = 1;
					
					// row_c iterates over the rows in color_c
					unsigned char* row_c = color_c + (8192 * (row - 1));
					
					// iterate over six columns at a time
					for(; col < width - 6; col += 6) {

						// each color value is the sum of itself and
						// the values surrounding it (9 total), each
						// scaled by the corresponding filter gains:
						// ======= hline.filter =======
						// -1 -2 -1   <-- top row
						//  0  0  0   <-- middle row
						//  1  2  1   <-- bottom row
						
						// top row:
						int value0 = -(row_c[col-1]
							+ (row_c[col] * 2) + row_c[col+1]);
						int value1 = -(row_c[col]
							+ (row_c[col+1] * 2) + row_c[col+2]);
						int value2 = -(row_c[col+1]
							+ (row_c[col+2] * 2) + row_c[col+3]);
						int value3 = -(row_c[col+2]
							+ (row_c[col+3] * 2) + row_c[col+4]);
						int value4 = -(row_c[col+3]
							+ (row_c[col+4] * 2) + row_c[col+5]);
						int value5 = -(row_c[col+4]
							+ (row_c[col+5] * 2) + row_c[col+6]);  

						row_c += 16384; // 2 * MAX_DIM = 16384

						// bottom row:
						value0 += row_c[col-1] + (row_c[col] * 2)
							+ row_c[col+1];
						value1 += row_c[col] + (row_c[col+1] * 2)
							+ row_c[col+2];
						value2 += row_c[col+1] + (row_c[col+2] * 2)
							+ row_c[col+3];
						value3 += row_c[col+2] + (row_c[col+3] * 2)
							+ row_c[col+4];
						value4 += row_c[col+3] + (row_c[col+4] * 2)
							+ row_c[col+5];
						value5 += row_c[col+4] + (row_c[col+5] * 2)
							+ row_c[col+6];
						
						row_c -= 16384; // 2 * MAX_DIM = 16384
						
						// constrain each color value
						// to be between 0 and 255
						if(value0 < 0) { value0 = 0; }
						else if(value0 > 255) { value0 = 255; }
						if(value1 < 0) { value1 = 0; }
						else if(value1 > 255) { value1 = 255; }
						if(value2 < 0) { value2 = 0; }
						else if(value2 > 255) { value2 = 255; }
						if(value3 < 0) { value3 = 0; }
						else if(value3 > 255) { value3 = 255; }
						if(value4 < 0) { value4 = 0; }
						else if(value4 > 255) { value4 = 255; }
						if(value5 < 0) { value5 = 0; }
						else if(value5 > 255) { value5 = 255; }

						// set each color value in the output image
						output->color[plane][row][col] = value0;
						output->color[plane][row][col+1] = value1;
						output->color[plane][row][col+2] = value2;
						output->color[plane][row][col+3] = value3;
						output->color[plane][row][col+4] = value4;
						output->color[plane][row][col+5] = value5;
						
					} // end 'six-column' for loop
					
					// iterate over the last five columns, one at a time
					for(; col < width - 1; ++col) {
						
						// the color value is the sum of itself and
						// the values surrounding it (9 total), each
						// scaled by the corresponding filter gains:
						// ======= hline.filter =======
						// -1 -2 -1   <-- top row
						//  0  0  0   <-- middle row
						//  1  2  1   <-- bottom row
						
						// top row:
						int value = -(row_c[col-1] + (row_c[col] * 2)
							+ row_c[col+1]);
						
						row_c += 16384; // 2 * MAX_DIM = 16384
						
						// bottom row:
						value += row_c[col-1] + (row_c[col] * 2)
							+ row_c[col+1];
						
						row_c -= 16384; // 2 * MAX_DIM = 16384

						// constrain the color value
						// to be between 0 and 255
						if(value < 0) { value = 0; }
						else if(value > 255) { value = 255; }
						
						// set the color value in the output image
						output->color[plane][row][col] = value;
						
					} // end 'one-column' for loop
					
				} // end 'row' for loop
				
				// color_c now points to the 8192 x 8192 matrix
				// corresponding to the next RGB value
				color_c += 67108864; // MAX_DIM * MAX_DIM = 67108864
				
			} // end 'plane' for loop
			
			break; // return from switch statement
			
		} // end case 4: 'hline.filter'
		
		case 5: { // 'sharpen.filter'

			// plane iterates over the RGB values in each pixel
			for(int plane = 0; plane < 3; ++plane) {
				
				// row iterates over the rows of pixels in the image
				for(int row = 1; row < height; ++row) {
					
					// col iterates over the columns of pixels in the image
					int col = 1;
					
					// row_c iterates over the rows in color_c
					unsigned char* row_c = color_c + (8192 * (row - 1));
					
					// iterate over six columns at a time
					for(; col < width - 6; col += 6) {

						// each color value is the sum of itself and
						// the values surrounding it (9 total), each
						// scaled by the corresponding filter gains:
						// ======= sharpen.filter =======
						// 11 10  1   <-- top row
						// -1 -1 -1   <-- middle row
						// -1 -1 -1   <-- bottom row
						
						// top row:
						int value0 = (row_c[col-1] * 11)
							+ (row_c[col] * 10) + row_c[col+1];
						int value1 = (row_c[col] * 11)
							+ (row_c[col+1] * 10) + row_c[col+2];
						int value2 = (row_c[col+1] * 11)
							+ (row_c[col+2] * 10) + row_c[col+3];
						int value3 = (row_c[col+2] * 11)
							+ (row_c[col+3] * 10) + row_c[col+4];
						int value4 = (row_c[col+3] * 11)
							+ (row_c[col+4] * 10) + row_c[col+5];
						int value5 = (row_c[col+4] * 11)
							+ (row_c[col+5] * 10) + row_c[col+6];  

						row_c += 8192; // MAX_DIM = 8192

						// middle row:
						value0 -= row_c[col-1] + row_c[col]
							+ row_c[col+1];
						value1 -= row_c[col] + row_c[col+1]
							+ row_c[col+2];
						value2 -= row_c[col+1] + row_c[col+2]
							+ row_c[col+3];
						value3 -= row_c[col+2] + row_c[col+3]
							+ row_c[col+4];
						value4 -= row_c[col+3] + row_c[col+4]
							+ row_c[col+5];
						value5 -= row_c[col+4] + row_c[col+5]
							+ row_c[col+6];

						row_c += 8192; // MAX_DIM = 8192

						// bottom row:
						value0 -= row_c[col-1] + row_c[col]
							+ row_c[col+1];
						value1 -= row_c[col] + row_c[col+1]
							+ row_c[col+2];
						value2 -= row_c[col+1] + row_c[col+2]
							+ row_c[col+3];
						value3 -= row_c[col+2] + row_c[col+3]
							+ row_c[col+4];
						value4 -= row_c[col+3] + row_c[col+4]
							+ row_c[col+5];
						value5 -= row_c[col+4] + row_c[col+5]
							+ row_c[col+6];
						
						row_c -= 16384; // 2 * MAX_DIM = 16384

						// divide each color value by
						// sharpen.filter's 'div' value (20)
						value0 /= 20;
						value1 /= 20;
						value2 /= 20;
						value3 /= 20;
						value4 /= 20;
						value5 /= 20;
						
						// constrain each color value
						// to be between 0 and 255
						if(value0 > 255) { value0 = 255; }
						else if(value0 < 0) { value0 = 0; }
						if(value1 > 255) { value1 = 255; }
						else if(value1 < 0) { value1 = 0; }
						if(value2 > 255) { value2 = 255; }
						else if(value2 < 0) { value2 = 0; }
						if(value3 > 255) { value3 = 255; }
						else if(value3 < 0) { value3 = 0; }
						if(value4 > 255) { value4 = 255; }
						else if(value4 < 0) { value4 = 0; }
						if(value5 > 255) { value5 = 255; }
						else if(value5 < 0) { value5 = 0; }

						// set each color value in the output image
						output->color[plane][row][col] = value0;
						output->color[plane][row][col+1] = value1;
						output->color[plane][row][col+2] = value2;
						output->color[plane][row][col+3] = value3;
						output->color[plane][row][col+4] = value4;
						output->color[plane][row][col+5] = value5;
						
					} // end 'six-column' for loop
					
					// iterate over the last five columns, one at a time
					for(; col < width - 1; ++col) {
						
						// the color value is the sum of itself and
						// the values surrounding it (9 total), each
						// scaled by the corresponding filter gains:
						// ======= sharpen.filter =======
						// 11 10  1   <-- top row
						// -1 -1 -1   <-- middle row
						// -1 -1 -1   <-- bottom row
						
						// top row:
						int value = (row_c[col-1] * 11)
							+ (row_c[col] * 10) + row_c[col+1];
						
						row_c += 8192; // MAX_DIM = 8192
						
						// middle row:
						value -= row_c[col-1] + row_c[col]
							+ row_c[col+1];
						
						row_c += 8192; // MAX_DIM = 8192
						
						// bottom row:
						value -= row_c[col-1] + row_c[col]
							+ row_c[col+1];
						
						row_c -= 16384; // 2 * MAX_DIM = 16384

						// divide the color value by
						// sharpen.filter's 'div' value (20)
						value /= 20;
						
						// constrain the color value
						// to be between 0 and 255
						if(value > 255) { value = 255; }
						else if(value < 0) { value = 0; }
						
						// set the color value in the output image
						output->color[plane][row][col] = value;
						
					} // end 'one-column' for loop
					
				} // end 'row' for loop
				
				// color_c now points to the 8192 x 8192 matrix
				// corresponding to the next RGB value
				color_c += 67108864; // MAX_DIM * MAX_DIM = 67108864
				
			} // end 'plane' for loop
			
			break; // return from switch statement
			
		} // end case 5: 'sharpen.filter'
		
		case 6: { // 'vline.filter'

			// plane iterates over the RGB values in each pixel
			for(int plane = 0; plane < 3; ++plane) {
				
				// row iterates over the rows of pixels in the image
				for(int row = 1; row < height; ++row) {
					
					// col iterates over the columns of pixels in the image
					int col = 1;
					
					// row_c iterates over the rows in color_c
					unsigned char* row_c = color_c + (8192 * (row - 1));
					
					// iterate over six columns at a time
					for(; col < width - 6; col += 6) {

						// each color value is the sum of itself and
						// the values surrounding it (9 total), each
						// scaled by the corresponding filter gains:
						// ======= vline.filter =======
						// -1  0  1   <-- top row
						// -2  0  2   <-- middle row
						// -1  0  1   <-- bottom row
						
						// top row:
						int value0 = row_c[col+1] - row_c[col-1];
						int value1 = row_c[col+2] - row_c[col];
						int value2 = row_c[col+3] - row_c[col+1];
						int value3 = row_c[col+4] - row_c[col+2];
						int value4 = row_c[col+5] - row_c[col+3];
						int value5 = row_c[col+6] - row_c[col+4];

						row_c += 8192; // MAX_DIM = 8192

						// middle row:
						value0 += (row_c[col+1] - row_c[col-1]) * 2;
						value1 += (row_c[col+2] - row_c[col]) * 2;
						value2 += (row_c[col+3] - row_c[col+1]) * 2;
						value3 += (row_c[col+4] - row_c[col+2]) * 2;
						value4 += (row_c[col+5] - row_c[col+3]) * 2;
						value5 += (row_c[col+6] - row_c[col+4]) * 2;

						row_c += 8192; // MAX_DIM = 8192

						// bottom row:
						value0 += row_c[col+1] - row_c[col-1];
						value1 += row_c[col+2] - row_c[col];
						value2 += row_c[col+3] - row_c[col+1];
						value3 += row_c[col+4] - row_c[col+2];
						value4 += row_c[col+5] - row_c[col+3];
						value5 += row_c[col+6] - row_c[col+4];
						
						row_c -= 16384; // 2 * MAX_DIM = 16384
						
						// constrain each color value
						// to be between 0 and 255
						if(value0 < 0) { value0 = 0; }
						else if(value0 > 255) { value0 = 255; }
						if(value1 < 0) { value1 = 0; }
						else if(value1 > 255) { value1 = 255; }
						if(value2 < 0) { value2 = 0; }
						else if(value2 > 255) { value2 = 255; }
						if(value3 < 0) { value3 = 0; }
						else if(value3 > 255) { value3 = 255; }
						if(value4 < 0) { value4 = 0; }
						else if(value4 > 255) { value4 = 255; }
						if(value5 < 0) { value5 = 0; }
						else if(value5 > 255) { value5 = 255; }

						// set each color value in the output image
						output->color[plane][row][col] = value0;
						output->color[plane][row][col+1] = value1;
						output->color[plane][row][col+2] = value2;
						output->color[plane][row][col+3] = value3;
						output->color[plane][row][col+4] = value4;
						output->color[plane][row][col+5] = value5;
						
					} // end 'six-column' for loop
					
					// iterate over the last five columns, one at a time
					for(; col < width - 1; ++col) {
						
						// the color value is the sum of itself and
						// the values surrounding it (9 total), each
						// scaled by the corresponding filter gains:
						// ======= vline.filter =======
						// -1  0  1   <-- top row
						// -2  0  2   <-- middle row
						// -1  0  1   <-- bottom row
						
						// top row:
						int value = row_c[col+1] - row_c[col-1];
						
						row_c += 8192; // MAX_DIM = 8192
						
						// middle row:
						value += (row_c[col+1] - row_c[col-1]) * 2;
						
						row_c += 8192; // MAX_DIM = 8192
						
						// bottom row:
						value += row_c[col+1] - row_c[col-1];
						
						row_c -= 16384; // 2 * MAX_DIM = 16384

						// constrain the color value
						// to be between 0 and 255
						if(value < 0) { value = 0; }
						else if(value > 255) { value = 255; }
						
						// set the color value in the output image
						output->color[plane][row][col] = value;
						
					} // end 'one-column' for loop
					
				} // end 'row' for loop
				
				// color_c now points to the 8192 x 8192 matrix
				// corresponding to the next RGB value
				color_c += 67108864; // MAX_DIM * MAX_DIM = 67108864
			
			} // end 'plane' for loop
			
			break; // return from switch statement
			
		} // end case 6: 'vline.filter'
		
		case 7: { // invalid filter
			
			cout << "Error: Filter unrecognized or undefined!" << endl;
			
			break; // return from switch statement
			
		} // end case 7: invalid filter
		
		default: { // invalid filter
			
			cout << "Error: Filter unrecognized or undefined!" << endl;
			
			break; // return from switch statement
			
		} // end default case: invalid filter
		
	} // end switch statement
	
	cycStop = rdtscll();
	double diff = cycStop - cycStart;
	double diffPerPixel = diff / (output->width * output->height);
	fprintf(stderr,
		"Took %f cycles to process, or %f cycles per pixel\n",
		diff, diffPerPixel);
	return diffPerPixel;
}
