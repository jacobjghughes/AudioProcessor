#include <iostream>
#include <cmath>
#include <vector>
#include <algorithm>
#include <thread>

#define PI 3.14159265358979323846
const int BLOCK_SIZE = 480; 
const int SAMPLE_RATE = 48000; //samples per sec (iterations per sec)
const float FREQUENCY = 1000.0; // number of cycles per sec (hertz)
const float AMPLITUDE = 1.0;
const int THREAD_COUNT = 4;

std::vector<std::vector<float>> generateSine() {
    std::vector<std::vector<float>> blocks;
    // generate a second of continuous sine wave
    // assumes block size is a factor of sample rate for consistency
    for (int i = 0; i < SAMPLE_RATE / BLOCK_SIZE; i++)
    {
        std::vector<float> newBlock;
        for (int j = 0; j < BLOCK_SIZE; j++)
        {
            // t is time elapsed (sample number / sample rate)
            float t = (i * BLOCK_SIZE + j) / static_cast<float>(SAMPLE_RATE);
            // formula for a sine wave is amplitude * sin(2pi * freq * t + phase)
            newBlock.push_back(AMPLITUDE * std::sin(2 * PI * FREQUENCY * t));
        }
        blocks.push_back(newBlock);
    }
    return blocks;
}

// thread that takes a block by reference and applies gain filter
void gainThreaded(float gain, std::vector<float> &block) { 
    for (int j = 0; j < BLOCK_SIZE; j++)
    {
        block.at(j) *= gain;
    }
    return;
}

// takes in blocks and calls the multi-threaded gain function
// returns updated blocks with gainControl filter applied
std::vector<std::vector<float>> gainControl(std::vector<std::vector<float>> &blocks, float gain = 1.0) {
    for (int i = 0; i < blocks.size(); i+= THREAD_COUNT) {

        std::vector<std::thread> threads;
        // we have to check that i+t does not exceed blocks's size 
        for (int t = 0; t < THREAD_COUNT && i + t < blocks.size(); t++){
            threads.emplace_back(gainThreaded, gain, std::ref(blocks.at(i+t)));
        }
        for (auto& thread : threads) {
            thread.join();
        }
    }
    return blocks;
}

// takes blocks and newblocks by reference to update the newblocks vector with calculated offsets
void delayThreaded(std::vector<std::vector<float>> &blocks, std::vector<std::vector<float>> &newBlocks, int innerOffset, int blockOffset, int blockIndex) {
        // with this loop logic, we don't need to check if the new position overflows into a new block
        // this cuts down on unnecessary comparisons
        for (int j = 0; j < BLOCK_SIZE - innerOffset; j++)
        {
            newBlocks.at(blockIndex + blockOffset).at(j + innerOffset) = blocks.at(blockIndex).at(j);
        }
        for (int j = BLOCK_SIZE - innerOffset; j < BLOCK_SIZE; j++)
        {
            // block index is incremented to account for overflow
            // j + innerOffset is always >= BLOCK_SIZE
            newBlocks.at(blockIndex + blockOffset + 1).at(j + innerOffset - BLOCK_SIZE) = blocks.at(blockIndex).at(j);
        }
}

// takes in blocks and a delay in ms, then calculates the offset that this delay would make
// then calls the multi-threaded delay function that updates and returns newBlocks with delay filter added
std::vector<std::vector<float>> addDelay(std::vector<std::vector<float>> &blocks, int delay = 100) {

    // total offset = Sample rate * delay convertecd to seconds
    // sampleOffset will always be an integer if Sample rate > 1000
    int sampleOffset = SAMPLE_RATE * delay / 1000;
    int blockOffset = std::floor(sampleOffset / BLOCK_SIZE);
    int innerOffset = sampleOffset % BLOCK_SIZE;
   
    // create new vector of empty blocks with enough space to move values into
    std::vector<std::vector<float>> newBlocks;
        // if innerOffset is 0 then we do not need an extra overflow block
    if (innerOffset == 0) newBlocks.resize(blocks.size() + blockOffset);
    else newBlocks.resize(blocks.size() + blockOffset + 1);

    // populate newBlocks with default values
    for (int i = 0; i < newBlocks.size(); i++) {
        newBlocks.at(i).resize(BLOCK_SIZE, 0.0);
    }

    for (int i = 0; i < blocks.size(); i += THREAD_COUNT)
    {
        std::vector<std::thread> threads;
        // we have to check that i+t does not exceed blocks's size
        for (int t = 0; t < THREAD_COUNT && i + t < blocks.size(); t++)
        {
            threads.emplace_back(delayThreaded, std::ref(blocks), std::ref(newBlocks), innerOffset, blockOffset, (i+t));
        }
        for (auto &thread : threads)
        {
            thread.join();
        }
    }
    return newBlocks;
}

void outputBlocks(std::vector<std::vector<float>> &blocks) {
    for (int i = 0; i < blocks.size(); i++) {
        for (int j = 0; j < blocks.at(i).size(); j++) {
            std::cout << blocks.at(i).at(j) << " , ";
        }
        std::cout << std::endl;
    }
    std::cout << "Number of Blocks: " << blocks.size() << std::endl;
    return;
}

int main() {
    std::vector<std::vector<float>> blocks;
    while (true)
    {
        // generate input
        blocks = generateSine();
        
        // process blocks
        blocks = gainControl(blocks, 10);
        blocks = addDelay(blocks, 100);

        // output to console 
        outputBlocks(blocks);
    } 
    return 0;
}