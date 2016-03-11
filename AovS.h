#pragma once
#include <vector>

//!Stores coordinates with vectorized variables
struct VectorizedCoordinate
{
 float_v x;
 float_v y;
};

//!Stores polarcoordinate with vectorized variables
struct VectorizedPolarCoordinate
{
 float_v radius;
 float_v phi;
};

typedef std::vector<VectorizedCoordinate, Vc::Allocator<VectorizedCoordinate>>           vectorVectorizedCoordinate;
typedef std::vector<VectorizedPolarCoordinate, Vc::Allocator<VectorizedPolarCoordinate>> vectorVectorizedPolarCoordinate;

//!Creates random numbers for AovS
void simulateInput_AovS(vectorVectorizedCoordinate &input, const size_t size)
{
 //!Creates the random numbers
 std::default_random_engine engine(time(nullptr));
 //!Adjust the random number to a range
 std::uniform_real_distribution<float> random(-1.0f, 1.0f);
 //!For iterating over the input
 vectorVectorizedCoordinate::iterator aktElement = input.begin();
 //!The end of the iteration
 vectorVectorizedCoordinate::iterator endElement = (input.begin() + size);
 size_t m;

        while(aktElement != endElement)
        {
            for(m = 0; m < float_v::size(); m++)
            {
                aktElement->x[m] = random(engine);
                aktElement->y[m] = random(engine);
            }

            aktElement++;
        }
}

//!AovS
void AovS(benchmark::State &state)
{
 //!The label for the plotter
 const std::string label(getLabelString("-AovS/", state.range_x(), 1));
 //!The size of the values to process
 const size_t inputSize = state.range_x();
 //!The size of the container
 size_t containerSize = numberOfChunks(inputSize, float_v::size());

 //!The input and output values for calculation
 vectorVectorizedCoordinate      inputValues(containerSize);
 vectorVectorizedPolarCoordinate outputValues(containerSize);
 size_t n;

        //!Creation of input values
            simulateInput_AovS(inputValues, containerSize);
            for(n = 0; n < float_v::size(); n++)
            {
                inputValues[(containerSize - 1)].x[n] = 1.0f;
                inputValues[(containerSize - 1)].y[n] = 1.0f;
            }

        //!Creation of input values completed

        while(state.KeepRunning())
        {
            //!Calculation of all the input values
            for(n = 0; n < containerSize; n++)
            {
                std::tie(outputValues[n].radius, outputValues[n].phi) = calcularePolarCoordinate(inputValues[n].x, inputValues[n].y);
            }
        }

    //!Tell the Benchmark how many values are calculated
    state.SetItemsProcessed(state.iterations() * state.range_x());
    state.SetBytesProcessed(state.items_processed() * sizeof(float));

    //!Set the label
    state.SetLabel(label);

    #ifdef USE_LOG
        std::clog << "Finnished: " << label << "\n";
    #endif
}
