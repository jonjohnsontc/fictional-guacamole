# Lessons Learned trying to complete the BRC

Outside of toy examples and Leetcode, this is the first time that I'm attempting to put a project together using C. I'm compiling a list of things I've learned along the way about tackling this particular sort of problem.

I don't want to spend too many words on this post. The BRC has long finished for Java, and I don't have any insights that couldn't be gained from Danny Van Kooten's post / content. Although, maybe I can think of a few things

## Outline

- What and Why
- Observations
  - What optimizations worked and what didn't?
    - How do I know?
- Closing Thoughts

## Observations

- sscanf is slow (& unusable with mmap)
  - I should profile a mmapped iteration through a million row file with sscanf and one that uses *some other* method.
- avoid concurrency, especially if you don't need it
- the compiler is very smart
- mmapping a file makes access incredibly fast

## Post

Title: Billion Row C

I've spent some time over the past few months learning C. Now, I know somebody doesn't just learn to program in C in just a few months, but I'm hoping it can remain something that I can come back to the well and continue to use as the situation demands it like Python, SQL, etc. For the time being, I'm taking baby steps and learning to appreciate the small surface area that the language covers.  

All that said, if I'm not trying to build some software, it's going to be hard to retain or build on any kind of knowledge about the language. Since I spent much of my time at work building and validating data pipelines, the recent billion row challenge seemed like a fun target to build towards. I set my goal for creating a submission that would allow me to explore different ways of optimizing an initial "naive" pipeline, not necessarily a submission that was competitive with other C implementations out there.

My submission can be found at <https://github.com/jonjohnsontc/fictional-guacamole>

### Observations
