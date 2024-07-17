# Improvements

I'm going to use this document to track where I'd like to make improvements to the runtime/processing of the measurements:

<strike>1. Leverage the Java Library to Create sample measurements</strike>
2. Try to use threading 
3. mmap the input file
4. use simd to calculate measurements   
<strike>5. fix output</strike>

## Fix Output

I just noticed while outputting the 2t.c script. The ternary expression that I'm using to toggle commas in between cities is causing there to be no gaps in between many city output values.

## Leverage Java Library to Create Sample Measurements

Im getting some wild values inside the temperature file using the python scrip that's included in the repository.

**EDIT** Looks like using the Java measurement creation script resulted in more realistic looking final measurements

## Current Performance

Tracking [here](https://docs.google.com/spreadsheets/d/1oQP-8DpzyQMM9UjE5GoIkKkJ0U-A2OhIoDmxKX1bNlU/edit?usp=sharing).