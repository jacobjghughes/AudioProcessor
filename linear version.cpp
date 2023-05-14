#include <iostream>
#include <cmath>
#include <vector>
#include <algorithm>

/*
A single channel digital audio signal is represented as a continuous stream of signed floating point samples, at a rate of 48000 samples per second. The samples are stored and manipulated in fixed size data blocks of 480 samples each. The data is initialised with a 1kHz sine wave, and then goes through these subsequent processing stages:

•	a static gain control (i.e. scaling of the audio samples by a constant value)
•	a delay of 100 milliseconds.

The processed data is then output as simple numbers in text form to the output console.

Implement this system in C++ as a program that runs indefinitely, continually generating, processing, and outputting data in real-time. Write the code so that the data generation and processing stages can be easily swapped for different ones. Maximise the use of a multi-core CPU by running the different processing stages in parallel, even if this is at the expense of increased input-to-output delay.

Make assumptions where you are unclear about the requirements - the emphasis should be on well structured, robust, and maintainable code. Any C++ compiler can be used together with appropriate external libraries for e.g. threading/signalling/timing (such as the C++ standard template library).
*/

#define PI 3.14159265358979323846
const int BLOCK_SIZE = 480; 
const int SAMPLE_RATE = 48000; //samples per sec (iterations per sec)
const float FREQUENCY = 1000.0; // number of cycles per sec (hertz)
const float AMPLITUDE = 1.0;

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

// takes in a block and returns updated block with gainControl filter applied
std::vector<std::vector<float>> gainControl(std::vector<std::vector<float>> &blocks, float gain = 1.0) {
    for (int i = 0; i < blocks.size(); i++) {
        for (int j = 0; j < BLOCK_SIZE; j++)
        {
            blocks.at(i).at(j) *= gain;
        }
    }
    return blocks;
}

// adds delay in ms and returns a new vector with adjusted size
std::vector<std::vector<float>> addDelay(std::vector<std::vector<float>> &blocks, int delay = 100) {
    // total offset = Sample rate * delay converted to seconds
    // sampleOffset will always be an integer if Sample rate > 1000
    int sampleOffset = SAMPLE_RATE * delay / 1000;
    int blockOffset = std::floor(sampleOffset / BLOCK_SIZE);
    int innerOffset = sampleOffset % BLOCK_SIZE;
   
    // create new vector of empty populated blocks with enough space to move values into
    std::vector<std::vector<float>> newBlocks;
    // if innerOffset is 0 then we do not need an extra overflow block
    if (innerOffset == 0) newBlocks.resize(blocks.size() + blockOffset);
    else newBlocks.resize(blocks.size() + blockOffset + 1);

    for (int i = 0; i < newBlocks.size(); i++) {
        newBlocks.at(i).resize(BLOCK_SIZE, 0.0);
    }

    for (int i = 0; i < blocks.size(); i++) {
        // with this loop logic, we don't need to check if the new position overflows into a new block
        // this cuts down on unnecessary comparisons
        for (int j = 0; j < BLOCK_SIZE - innerOffset; j++){
            newBlocks.at(i + blockOffset).at(j + innerOffset) = blocks.at(i).at(j);
        }
        for (int j = BLOCK_SIZE - innerOffset; j < BLOCK_SIZE; j++){
            // block index is incremented to account for overflow
            // j + innerOffset is always >= BLOCK_SIZE
            newBlocks.at(i + blockOffset + 1).at(j + innerOffset - BLOCK_SIZE) = blocks.at(i).at(j);
        }
    }
    return newBlocks;
}

void outputBlocks(std::vector<std::vector<float>> &blocks) {
    for (int i = 0; i < blocks.size(); i++) {
        for (int j = 0; j < blocks.at(i).size(); j++) {
            std::cout << blocks.at(i).at(j) << ", ";
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