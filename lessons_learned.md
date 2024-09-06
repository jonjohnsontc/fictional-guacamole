# Lessons Learned trying to complete the BRC

Outside of toy examples and Leetcode, this is the first time that I'm attempting to put a project together using C. I'm compiling a list of things I've learned along the way about tackling this particular sort of problem.

I don't want to spend too many words on this post. The BRC has long finished for Java, and I don't have any insights that couldn't be gained from Danny Van Kooten's post / content. Although, maybe I can think of a few things

## Outline

- What and Why
- Observations
- Closing Thoughts

## Observations

- sscanf is slow (& unusable with mmap)
  - I should profile a mmapped iteration through a million row file with sscanf and one that uses *some other* method.
- avoid concurrency, especially if you don't need it
- the compiler is very smart
- mmapping a file makes access incredibly fast

## Post

I've spent some time over the past few months learning C. I would like to gain a better understanding of the language that is still used in plenty of commercial and open source software, along with the virtual machines that back much of the other software we use.

