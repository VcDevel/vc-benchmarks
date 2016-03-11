#pragma once
#include <Vc/vector>
#include <vector>
#include <string>

using Vc::Common::InterleavedMemoryWrapper;

//!The type of the indices for gather and scatter
typedef float_v::IndexType IT;

//!Holds a point in 2D space
struct Coordinate
{
 float x;
 float y;
};

//!Holds a point as a polarcoordinate. Saves the results from calculation
struct PolarCoordinate
{
 float radius;
 float phi;
};

typedef std::vector<Coordinate, Vc::Allocator<Coordinate>>           vectorCoordinate;
typedef std::vector<PolarCoordinate, Vc::Allocator<PolarCoordinate>> vectorPolarCoordinate;

typedef Vc::vector<Coordinate, Vc::Allocator<Coordinate>>           VCvectorCoordinate;
typedef Vc::vector<PolarCoordinate, Vc::Allocator<PolarCoordinate>> VCvectorPolarCoordinate;

//!Creates random numbers for AoS
template <typename T>
void simulateInput_AoS(T &input, const size_t size)
{
 typedef typename T::iterator iterator;

 //!Creates the random numbers
 std::default_random_engine engine(time(nullptr));
 //!Adjust the random number to a range
 std::uniform_real_distribution<float> random(-1.0f, 1.0f);
 //!For iterating over the input
 iterator aktElement;
 //!The end of the iteration
 iterator endElement;

        aktElement = input.begin();
        endElement = (input.begin() + size);

        while(aktElement != endElement)
        {
            aktElement->x = random(engine);
            aktElement->y = random(engine);
            aktElement++;
        }
}

//!AoS with a padding
void AoS_Padding(benchmark::State &state)
{
 //!The label for the plotter
 const std::string label(getLabelString("AoS_Padding/", state.range_x(), 1));
 //!The size of the values to process
 const size_t inputSize = state.range_x();
 //!The size of the container
 const size_t containerSize = (numberOfChunks(inputSize, float_v::size()) * float_v::size());

 //!The input and output values for calculation
 vectorCoordinate      inputValues(containerSize);
 vectorPolarCoordinate outputValues(containerSize);

 //!Keeps the input values in a vc-vector
 float_v coordinateX_v;
 float_v coordinateY_v;
 //!Keeps the output values in a vc-vector
 float_v radius_v;
 float_v phi_v;

 size_t n, m;


        //!Creation of input values
            //!The values of the last container are set to 1.0f for the padding
            for(n = 1; n <= float_v::size(); n++)
            {
                inputValues[(containerSize - n)].x = 1.0f;
                inputValues[(containerSize - n)].y = 1.0f;
            }
            simulateInput_AoS(inputValues, inputSize);
        //!Creation of input values completed

        while(state.KeepRunning())
        {
            //!Calculation of all and the additional input values
            for(n = 0; n < containerSize; n += float_v::size())
            {
                //!Loads the values to vc-vector
                for(m = 0; m < float_v::size(); m++)
                {
                    coordinateX_v[m] = inputValues[(n + m)].x;
                    coordinateY_v[m] = inputValues[(n + m)].y;
                }

                    //!Calculate the polarcoordinates
                    std::tie(radius_v, phi_v) = calcularePolarCoordinate(coordinateX_v, coordinateY_v);

                //!Store the values from the vc-vector
                for(m = 0; m < float_v::size(); m++)
                {
                    outputValues[(n + m)].radius = radius_v[m];
                    outputValues[(n + m)].phi    = phi_v[m];
                }
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

//!AoS unsing interleaved memory with a padding
void AoS_Interleaved_Padding(benchmark::State &state)
{
 //!The label for the plotter
 const std::string label(getLabelString("AoS_Interleaved_Padding/", state.range_x(), 1));
 //!The size of values to process
 const size_t inputSize = state.range_x();
 //!The size of the container
 const size_t containerSize = (numberOfChunks(inputSize, float_v::size()) * float_v::size());

 //!The input and output values for calculation
 vectorCoordinate      inputValues(containerSize);
 vectorPolarCoordinate outputValues(containerSize);

 //!Using interleaved memory for loading
 InterleavedMemoryWrapper<Coordinate, float_v> wrapperInput(inputValues.data());
 //!Using interleaved memory for writing
 InterleavedMemoryWrapper<PolarCoordinate, float_v> wrapperOutput(outputValues.data());

 //!Keeps the input values in a vc-vector
 float_v coordinateX_v;
 float_v coordinateY_v;
 //!Keeps the output values in a vc-vector
 float_v radius_v;
 float_v phi_v;

 size_t n;

        //!Creation of input values
            //!The values of the last container are set to 1.0f for the padding
            for(n = 1; n <= float_v::size(); n++)
            {
                inputValues[(containerSize - n)].x = 1.0f;
                inputValues[(containerSize - n)].y = 1.0f;
            }

            simulateInput_AoS(inputValues, inputSize);
        //!Creation of input values completed

        while(state.KeepRunning())
        {
            //!Calculation of all and the additional input values
            for(n = 0; n < containerSize; n += (float_v::size()))
            {
                //!Loads the values to vc-vector
                Vc::tie(coordinateX_v, coordinateY_v) = wrapperInput[n];

                    //!Die Polarkoordinaten berechnen
                    std::tie(radius_v, phi_v) = calcularePolarCoordinate(coordinateX_v, coordinateY_v);

                //!Store the values from the vc-vector
                wrapperOutput[n] = Vc::tie(radius_v, phi_v);
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

//!Aos using Gather and Scatter as operator, with a padding
void AoS_GatherScatter_Padding(benchmark::State &state)
{
 //!The label for the plotter
 const std::string label(getLabelString("AoS_GatherScatter[]_Padding/", state.range_x(), 1));
 //!The size of the values to process
 const size_t inputSize = state.range_x();
 //!The size of the container
 const size_t containerSize = (numberOfChunks(inputSize, float_v::size()) * float_v::size());

 //!The input and output values for calculation
 VCvectorCoordinate      inputValues(containerSize);
 VCvectorPolarCoordinate outputValues(containerSize);

 //!Keeps the input values in a vc-vector
 float_v coordinateX_v;
 float_v coordinateY_v;
 //!Keeps the output values in a vc-vector
 float_v radius_v;
 float_v phi_v;

 //!The indexes for gather and scatter
 IT indexes(IT::IndexesFromZero());

 size_t n;

        //!Creation of input values
            //!The values of the last container are set to 1.0f for the padding
            for(n = 1; n <= float_v::size(); n++)
            {
                inputValues[(containerSize - n)].x = 1.0f;
                inputValues[(containerSize - n)].y = 1.0f;
            }

            simulateInput_AoS(inputValues, inputSize);
        //!Creation of input values completed

        while(state.KeepRunning())
        {
            indexes = IT::IndexesFromZero();

            //!Calculation of all and the additional input values
            for(n = 0; n < containerSize; n += float_v::size())
            {
                //!Loads the values to vc-vector
                coordinateX_v = inputValues[indexes][&Coordinate::x];
                coordinateY_v = inputValues[indexes][&Coordinate::y];

                    //!Calculate the polarcoordinates
                    std::tie(radius_v, phi_v) = calcularePolarCoordinate(coordinateX_v, coordinateY_v);

                //!Store the values from the vc-vector
                outputValues[indexes][&PolarCoordinate::radius] = radius_v;
                outputValues[indexes][&PolarCoordinate::phi]    = phi_v;

                indexes += float_v::size();
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

//!Aos using Gather and Scatter as function, with a padding
void AoS_GatherScatterFunc_Padding(benchmark::State &state)
{
 //!The label for the plotter
 const std::string label(getLabelString("AoS_GatherScatter()_Padding/", state.range_x(), 1));
 //!The size of the values to process
 const size_t inputSize = state.range_x();
 //!The size of the container
 const size_t containerSize = (numberOfChunks(inputSize, float_v::size()) * float_v::size());

 //!The input and output values for calculation
 vectorCoordinate      inputValues(containerSize);
 vectorPolarCoordinate outputValues(containerSize);

 //!Keeps the input values in a vc-vector
 float_v coordinateX_v;
 float_v coordinateY_v;
 //!Keeps the output values in a vc-vector
 float_v radius_v;
 float_v phi_v;

 //!The indexes for gather and scatter
 IT indexes;

 size_t n;

        //!Creation of input values
            //!The values of the last container are set to 1.0f for the padding
            for(n = 1; n <= float_v::size(); n++)
            {
                inputValues[(containerSize - n)].x = 1.0f;
                inputValues[(containerSize - n)].y = 1.0f;
            }

            simulateInput_AoS(inputValues, inputSize);
        //!Creation of input values completed

        while(state.KeepRunning())
        {
            //!Create the indexes
            indexes[0] = 0;
            for(n = 1; n < float_v::size(); n++)
                indexes[n] = (n << 1);

            //!Calculation of all and the additional input values
            for(n = 0; n < containerSize; n += float_v::size())
            {
                //!Loads the values to vc-vector
                coordinateX_v.gather(&inputValues[n].x, indexes);
                coordinateY_v.gather(&inputValues[n].y, indexes);

                    //!Calculate the polarcoordinates
                    std::tie(radius_v, phi_v) = calcularePolarCoordinate(coordinateX_v, coordinateY_v);

                //!Store the values from the vc-vector
                radius_v.scatter(&outputValues[n].radius, indexes);
                phi_v.scatter(&outputValues[n].phi, indexes);
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

//!AoS with rest scalar
void AoS_RestScalar(benchmark::State &state)
{
 //!The label for the plotter
 const std::string label(getLabelString("AoS_RestScalar/", state.range_x(), 1));
 //!The size of the values to process
 const size_t inputSize = state.range_x();
 //!The size of the values without a full vc-vector
 const size_t missingSize = (inputSize%float_v::size());

 //!The input and output values for calculation
 vectorCoordinate      inputValues(inputSize);
 vectorPolarCoordinate outputValues(inputSize);

 //!Using interleaved memory for loading
 InterleavedMemoryWrapper<Coordinate, float_v> wrapperInput(inputValues.data());
 //!Using interleaved memory for writing
 InterleavedMemoryWrapper<PolarCoordinate, float_v> wrapperOutput(outputValues.data());

 //!Keeps the input values in a vc-vector
 float_v coordinateX_v;
 float_v coordinateY_v;
 //!Keeps the output values in a vc-vector
 float_v radius_v;
 float_v phi_v;

 size_t n, m;

        //!Creation of input values
            simulateInput_AoS(inputValues, inputSize);
        //!Creation of input values completed

        while(state.KeepRunning())
        {
            //!Calculation of the input values without the additional ones
            for(n = 0; n < (inputSize - missingSize); n += float_v::size())
            {
                //!Loads the values to vc-vector
                for(m = 0; m < float_v::size(); m++)
                {
                    coordinateX_v[m] = inputValues[(n + m)].x;
                    coordinateY_v[m] = inputValues[(n + m)].y;
                }

                    //!Calculate the polarcoordinates
                    std::tie(radius_v, phi_v) = calcularePolarCoordinate(coordinateX_v, coordinateY_v);

                //!Store the values from the vc-vector
                for(m = 0; m < float_v::size(); m++)
                {
                    outputValues[(n + m)].radius = radius_v[m];
                    outputValues[(n + m)].phi    = phi_v[m];
                }
            }

            //!Calculate the leftover values scalar
            for(n = (inputSize - missingSize); n < inputSize; n++)
            {
                //!Scalar calculation
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

//!AoS using interleaved memory, with rest scalar
void AoS_Interleaved_RestScalar(benchmark::State &state)
{
 //!The label for the plotter
 const std::string label(getLabelString("AoS_Interleaved_RestScalar/", state.range_x(), 1));
 //!The size of the values to process
 const size_t inputSize = state.range_x();
 //!The size of the values without a full vc-vector
 const size_t missingSize = (inputSize%float_v::size());

 //!The input and output values for calculation
 vectorCoordinate      inputValues(inputSize);
 vectorPolarCoordinate outputValues(inputSize);

 //!Using interleaved memory for loading
 InterleavedMemoryWrapper<Coordinate, float_v> wrapperInput(inputValues.data());
 //!Using interleaved memory for writing
 InterleavedMemoryWrapper<PolarCoordinate, float_v> wrapperOutput(outputValues.data());

 //!Keeps the input values in a vc-vector
 float_v coordinateX_v;
 float_v coordinateY_v;
 //!Keeps the output values in a vc-vector
 float_v radius_v;
 float_v phi_v;

 size_t n;

        //!Creation of input values
            simulateInput_AoS(inputValues, inputSize);
        //!Creation of input values completed

        while(state.KeepRunning())
        {
            //!Calculation of the input values without the additional ones
            for(n = 0; n < (inputSize - missingSize); n += float_v::size())
            {
                //!Loads the values to vc-vector
                Vc::tie(coordinateX_v, coordinateY_v) = wrapperInput[n];

                    //!Calculate the polarcoordinates
                    std::tie(radius_v, phi_v) = calcularePolarCoordinate(coordinateX_v, coordinateY_v);

                //!Store the values from the vc-vector
                wrapperOutput[n] = Vc::tie(radius_v, phi_v);
            }

            //!Calculate the leftover values scalar
            for(n = (inputSize - missingSize); n < inputSize; n++)
            {
                //!Scalar calculation
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

//!AoS using gather and scatter as operator, with rest scalar
void AoS_GatherScatter_RestScalar(benchmark::State &state)
{
 //!The label for the plotter
 const std::string label(getLabelString("AoS_GatherScatter[]_RestScalar/", state.range_x(), 1));
 //!The size of the values to process
 const size_t inputSize = state.range_x();
 //!The size of the values without a full vc-vector
 const size_t missingSize = (inputSize%float_v::size());

 //!The input and output values for calculation
 VCvectorCoordinate      inputValues(inputSize);
 VCvectorPolarCoordinate outputValues(inputSize);

 //!Keeps the input values in a vc-vector
 float_v coordinateX_v;
 float_v coordinateY_v;
 //!Keeps the output in a vc-vector
 float_v radius_v;
 float_v phi_v;

 //!The indexes for gather and scatter
 IT indexes(IT::IndexesFromZero());

 size_t n;

        //!Creation of input values
            simulateInput_AoS(inputValues, inputSize);
        //!Creation of input values completed

        while(state.KeepRunning())
        {
            indexes = IT::IndexesFromZero();
            //!Calculaton of the input values without the additional ones
            for(n = 0; n < (inputSize - missingSize); n += float_v::size())
            {
                //!Loads the values to vc-vector
                coordinateX_v = inputValues[indexes][&Coordinate::x];
                coordinateY_v = inputValues[indexes][&Coordinate::y];

                    //!Calculate the polarcoordinates
                    std::tie(radius_v, phi_v) = calcularePolarCoordinate(coordinateX_v, coordinateY_v);

                //!Store the values form the vc-vector
                outputValues[indexes][&PolarCoordinate::radius] = radius_v;
                outputValues[indexes][&PolarCoordinate::phi]    = phi_v;

                indexes += float_v::size();
            }

            //!Calculate the leftover values scaler
            for(n = (inputSize - missingSize); n < inputSize; n++)
            {
                //!Scalar calculation
                std::tie(outputValues[n].radius, outputValues[n].phi) = calcularePolarCoordinate(inputValues[n].x, inputValues[n].y);
            }
        }

    //!Tell the benachmark how many values are calculated
    state.SetItemsProcessed(state.iterations() * state.range_x());
    state.SetBytesProcessed(state.items_processed() * sizeof(float));

    //!Set the label
    state.SetLabel(label);

    #ifdef USE_LOG
        std::clog << "Finnished: " << label << "\n";
    #endif
}

//!AoS using gather and scatter as function, with rest scalar
void AoS_GatherScatterFunc_RestScalar(benchmark::State &state)
{
 //!The label for the plotter
 const std::string label(getLabelString("AoS_GatherScatter()_RestScalar/", state.range_x(), 1));
 //!The size of the values to process
 const size_t inputSize = state.range_x();
 //!The size of the values without a full vc-vector
 const size_t missingSize = (inputSize%float_v::size());

 //!The input and output values for calaculation
 VCvectorCoordinate      inputValues(inputSize);
 VCvectorPolarCoordinate outputValues(inputSize);

 //!Keeps the input values in a vc-vector
 float_v coordinateX_v;
 float_v coordinateY_v;
 //!Keeps the output values in a vc-vector
 float_v radius_v;
 float_v phi_v;

 //!The indexes for gather and scatter
 IT indexes;

 size_t n;

        //!Creation of input values
            simulateInput_AoS(inputValues, inputSize);
        //!Creation of input values completed

        while(state.KeepRunning())
        {
            //!Create the indexes
            indexes[0] = 0;
            for(n = 1; n < float_v::size(); n++)
                indexes[n] = (n << 1);

            //!Calculation of the input values without the additional ones
            for(n = 0; n < (inputSize - missingSize); n += float_v::size())
            {
                //!Load the values to vc-vector
                coordinateX_v.gather(&inputValues[n].x, indexes);
                coordinateY_v.gather(&inputValues[n].y, indexes);

                    //!Calculate the polarcoordinates
                    std::tie(radius_v, phi_v) = calcularePolarCoordinate(coordinateX_v, coordinateY_v);

                //!Store the values to vc-vector
                radius_v.scatter(&outputValues[n].radius, indexes);
                phi_v.scatter(&outputValues[n].phi, indexes);
            }

            //!Calculate the leftover values scalar
            for(n = (inputSize - missingSize); n < inputSize; n++)
            {
                //!Scalar calculation
                std::tie(outputValues[n].radius, outputValues[n].phi) = calcularePolarCoordinate(inputValues[n].x, inputValues[n].y);
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
