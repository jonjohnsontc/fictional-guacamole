# Lessons Learned trying to complete the BRC

Outside of toy examples and Leetcode, this is the first time that I'm attempting to put a project together using C. I'm compiling a list of things I've learned along the way about tackling this particular sort of problem:

OUTLINE:

- sscanf is slow (esp with mmap)
  - I should profile a mmapped iteration through a million row file with sscanf and one that uses *some other* method.
