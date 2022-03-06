# POKEY-Frequencies-Calculator
A simple tool for generating the best possible approximation of what could be musically useful for the Atari POKEY soundchip
Most of this code was originally written from scratch for Raster Music Tracker 1.31+, but this is a revamped version of the tuning code for a different purposed.
This will eventually be backported into RMT for the bugfixes and improvements that were done in this little side project.

How to use:
- run the executable
- input the necessary parameters such as A-4 tuning and machine region
- wait for the program to do its thing
- get all the necessary data from the 'output.txt' file generated a few seconds later

How to build:
- Linux: run **cmake** to generate the necessary files, then run **make** to build it.
- Windows: it should be fine with Visual Studio 2019 (which was originally used to make it), open project directory, then build solution.
- macOS: no idea, but I imagine **cmake** would also work?
