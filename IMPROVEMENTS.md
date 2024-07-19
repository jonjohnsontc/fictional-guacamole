# Improvements

I'm going to use this document to track where I'd like to make improvements to the runtime/processing of the measurements:

<strike>1. Leverage the Java Library to Create sample measurements</strike>
2. Try to use threading
3. mmap the input file
4. use simd to calculate measurements
<strike>5. fix output</strike>
6. No allocations
7. Sorting the output inclusive of special characters

## No allocations

Learning about how one can store trees in an array has me thinking that it's possible to store some sort of self balancing tree that would be easier to iterate through. In addition, I could avoid any malloc implementations by allocating the entire thing statically at the beginning.

Super ignorantly, I think I could store some entries in a binary tree. 

### No Allocations, storing tree in array

Right now, I'm very quickly running out of indices to use, so I don't think a binary tree will work in this context. I want to dig back into Algorithmic Thinking to see where the author discussed storing 4 times the amount of space for the number of potential nodes.

It looks like this was back in chapter 8 when they introduced segment trees. A segment tree is a full binary tree where each node is associated with a segment of an array. You can then answer questions about the array leveraging a few queries of the segments.

In order to store enough space for an odd-numbered sized array, it would be safe to allocate 4n the space, where n is the size of the array.

With regards to a binary tree that doesn't even try and balance itself, I imagine it would take some time. I know of red-black and B-trees. B-trees seem more approachable, and they're part of many db architectures, so that could be useful to try and leverage.

Although, come to think of it, I could use a treap to store the entries. Insertion could be O(n) for some entities, but that will quickly change to O(1) once everyone's been read in once.

B-trees seem to require multiple structures. Each node is going to have Ki keys, and each internal (non-leaf) version will need i+1 pointers to children and i pointers to the actual data being captured (although, if the key is the same as the data, I imagine this isn't an issue).

Because B trees have an order of some number, and there's a number of properties that each node must have, I'm thinking it would be possible to capture in an array.

## Fix Output

I just noticed while outputting the 2t.c script. The ternary expression that I'm using to toggle commas in between cities is causing there to be no gaps in between many city output values.

I ended up changing things, so that I always print a comma in-between values -- so it's not formatted properly still. Something to fix in final output

## Leverage Java Library to Create Sample Measurements

Im getting some wild values inside the temperature file using the python scrip that's included in the repository.

**EDIT** Looks like using the Java measurement creation script resulted in more realistic looking final measurements

## Current Performance

Tracking [here](https://docs.google.com/spreadsheets/d/1oQP-8DpzyQMM9UjE5GoIkKkJ0U-A2OhIoDmxKX1bNlU/edit?usp=sharing).
