#pragma once

using Vc::float_v;

//!Creates random numbers
void simulateInput_v(float_v &inputX, float_v &inputY)
{
 //!Creates the random numbers
 std::default_random_engine engine(time(nullptr));
 //!Adjust the random number to a range
 std::uniform_real_distribution<float> random(-1.0f, 1.0f);
 size_t n;

    for(n = 0; n < float_v::size(); n++)
    {
        inputX[n] = random(engine);
        inputY[n] = random(engine);
    }
}

//!For measuring the time of the polarcoordinate calculation
void baselineCalculation(benchmark::State &state)
{
 //!The label for the plotter
 const std::string label(getLabelString("Baseline/", state.range_x(), 1));
 //!The size of the container
 size_t containerSize = numberOfChunks(state.range_x(), sizeof(float_v));

 //!Keeps the input values in a vc-vector
 float_v coordinateX;
 float_v coordinateY;

 //!Keeps the output values in a vc-vector
 float_v radius;
 float_v phi;

 size_t n;

        //!Creation of input values
            simulateInput_v(coordinateX, coordinateY);
        //!Creation of input values completed

        while(state.KeepRunning())
        {
            for(n = 0; n < containerSize; n++)
            {
                //!Prevent the optimizer from optimizing
                asm volatile("":"+x"(coordinateX), "+x"(coordinateY));
                    //!Calculates only one value
                    std::tie(radius, phi) = calcularePolarCoordinate(coordinateX, coordinateY);
                asm volatile(""::"x"(radius), "x"(phi));
            }
        }

    //!Tell the benchnmark how many values are calculated
    state.SetItemsProcessed(state.iterations() * state.range_x());
    state.SetBytesProcessed(state.items_processed() * sizeof(float));

    //!Set the label
    state.SetLabel(label);

    #ifdef USE_LOG
        std::clog << "Finnished: " << label << "\n";
    #endif
}

//!Scalar
void Scalar(benchmark::State &state)
{
 //!The label for the plotter
 const std::string label(getLabelString(".Scalar/", state.range_x(), 1));
 //!The size of the input values
 const size_t inputSize = state.range_x();

 //!The input and output values for calculation
 vectorCoordinate      inputValues(inputSize);
 vectorPolarCoordinate outputValues(inputSize);
 size_t n;

        //!Creation of input values
            simulateInput_AoS(inputValues, inputSize);
        //!Creation of input values completed

        while(state.KeepRunning())
        {
            //!Calculate alle values
            for(n = 0; n < inputSize; n ++)
            {
                //!Scalar Calculation
                std::tie(outputValues[n].radius, outputValues[n].phi) = calcularePolarCoordinate(inputValues[n].x, inputValues[n].y);
            }
        }

    //!Tell the benchmark how many values are calculates
    state.SetItemsProcessed(state.iterations() * state.range_x());
    state.SetBytesProcessed(state.items_processed() * sizeof(float));

    //!Set the label
    state.SetLabel(label);

    #ifdef USE_LOG
        std::clog << "Finnished: " << label << "\n";
    #endif
}
