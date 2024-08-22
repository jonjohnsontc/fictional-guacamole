# Improvements

I'm going to use this document to track where I'd like to make improvements to the runtime/processing of the measurements:

<strike>1. Leverage the Java Library to Create sample measurements</strike>
2. Try to use threading
<strike>3. mmap the input file</strike>
4. use simd to calculate measurements
<strike>5. fix output</strike>
<strike>6. No allocations</strike>
7. Sorting the output inclusive of special characters

## Try to use threading

### After a test attempt counting city frequency with threads

I have implemented a basic threaded script that walks through the input file in a reader thread, and then uses threading to count each row. It's really slow, and I think it's because of a few reasons:

- threads seem to spend most of their time waiting
  - the threads all wait on the buffer while their reading in their input
- the threaded process is extremely trivial
  - it's a simple add, there's no reason that operation would be given to a thread in a real-world scenario
  - the overhead associated with the threadpool probably takes longer than the time the threads processing function does any work related to the problem

### A second approach with mmap

I'm thinking that leveraging mmap as well as threads would allow me to better shape the problem for threads.

I think to parallelize the problem better, I'd have to read the file in parallel and operate using `nthreads` number of streams to the file. If I mmap the input file, I can just copy `nthreads` number of pointers and have them operate on contiguous regions of it.

- statically allocate space for structure which holds data. Made up of:
  - array for data
  - hash-array for hashmap for O(1) access to data
- Initialize N number of threads
- mmap input file
- divide memory region into 10 contiguous regions, each delimited/ended with a newline character
- start each thread with function that operates over its memory region using a shared hashmap with mutex locked keys
- compute and print final result in main thread

#### Looking at an initial result

So, I took the code from 2th, and exchanged the components that read in the input file with ones that dealt with a super large character string after memory mapping it from a file.

Wow, does it run slow...

Operating against the 1m row sample file, it came with a `real` time of 50.984s. I have it running against the billion row file, as I figured memory mapping a smaller file might come with some overhead that is more manageable with larger files, but performance is so bad I'm considering just stopping it.

I did stop it at 28m15s without any result

**EDIT** After doing some google searching, it turns out that sscanf will scan the entire string before it adds the values to the variables it's been given, which explains why it had been running so slow.

### Initial Thoughts

After learning of the pretty poor support for C11 threads cross platform, I think I'm just going to use posix threads for the attempt. Eventually, I may add support for Windows threads, but there's not a particularly big pull to do that yet.

Right now, I'm reading through Programming with POSIX Threads until I get a good enough understanding to work my own example. I'm digging into condition variables specifically, and wondering if I'll need to use them at all.

**Do I need condition variables?**

My understanding is that each thread in the script is going to be performing two different operations.

- Reading a line from the input file
- Performing a cumulative calculation on the data structure holding the temperatures and cities (on one given city)

If I know the amount of threads available to me *n*, I could read each *% n* line and just require some sort of coordination on the temperature structure.

I may need one, if a thread trys to unlock a thread, but can't...I would think the thread should just wait until the thread signals to the next waiting thread that it's finished (maybe it can be in some kind of queue)?

I want to avoid broadcasting and potentially causing some pileups, although that could probably be pretty easy to implement and validate through some basic testing

**Are there alternatives to condition variables?**

I could just check again in *x* ticks, or something similar. Given that it's a super small set of operations, I can't imagine it would take much time at all, or that I would need to recheck more than once or twice.

## No allocations

Learning about how one can store trees in an array has me thinking that it's possible to store some sort of self balancing tree that would be easier to iterate through. In addition, I could avoid any malloc implementations by allocating the entire thing statically at the beginning.

Super ignorantly, I think I could store some entries in a binary tree (and I did!)

I ended up storing the entries in an array with some functions to make it act like a BTree. Space for the entire struct was statically allocated.

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
