I completed this task in roughly 8 hours. It took me a bit longer than I would have liked, but I learnt some new techniques along the way, which contributed to the time taken.
I've never done multi-threading in CPP before, so I first designed a linear version of the program, which I then refactored to utilise multi-threading. I used online resources and ChatGPT to help learn and debug errors I encountered when implementing multi-threading.
I have attached both the linear version and the threaded version in case this is of interest, but the threaded version should be considered the final solution.

## Error checking
I think the most important error checking feature that should be implemented is checking that each block in the `blocks` vector is the same size as the `BLOCK_SIZE` constant. I think if something were to make this not the case, then there would be many problems with my processing algorithms.

## Code improvement 
I have tried to comment the code wherever the function is not explicit, and included any assumptions I have made about the domain of values.
My code should be simple to expand on, if new processing, input, or output functions were to be added. `int main()` is deliberately kept simple and clear, to help achieve this.