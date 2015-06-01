# delaunay
C program that constructs the Delaunay triangulation and Voronoi diagram of a set of points. Built as a final project in the NYUAD Data Structures class in spring 2014.

## Instructions
### Running the program
The program can be run through terminal:
`./run <num <delaunay voronoi>>`
where the `num` argument determines the number of points added to the input file; the default value is `20`.
The `delaunay` argument determines wheteher the delaunay triangulation should be shown; the default is `true`.
The `voronoi` argument determines whether the voronoi diagram should be shown; the default is `true`.

This produces an image file named `output.png` with the resulting diagram.

### Updating source code
If changes were made to the source files, the program has to be rebuilt first:
`./build`

### Building individual parts of the program
To produce a specific number of random points in the accepted format:
`g++ randinput.cpp -o randinput`
`./randinput <num>`
where the `num` argument determines the number of points produced; the default value is `20`.

To produce the delaunay triangulation and/or voronoi diagram based on a list of points:
`g++ delaunay.cpp -o delaunay`
`./delaunay <input output <delaunay voronoi>>`
where the `input` argument determines the address of the input file; the default is `input.txt`.
The `output` argument determines the address of the output file; the default is `output.txt`.
The `delaunay` argument determines wheteher the delaunay triangulation should be shown; the default is `true`.
The `voronoi` argument determines whether the voronoi diagram should be shown; the default is `true`.

This produces a `output.txt` file with instructions for `gnuplot` program to draw the diagram.

### Input data formatting
If one does not wish to use the provided `randinput` executable to produce input values, these can be input manually. The formatting is as follows:
`x	y`
The `x` and `y` values are separated by `\t` (tab).


### Output data formatting:
The output file contains information for the `gnuplot` program (`/usr/local/bin/gnuplot`) to draw the diagram. It can be edited; however, next run of the `delaunay` executable will overwrite any changes. For the changes to be permanent, they have to be made directly in the source code of `delaunay.cpp`. The first line of the output file has to be:
`#!/usr/local/bin/gnuplot`
to ensure that the diagram will be drawn through gnuplot.

Additionally, for the file to be drawn, it must be made executable. If the `gnuplot` program does not open, use:
`chmod +x output.txt`
to ensure correct permissions.