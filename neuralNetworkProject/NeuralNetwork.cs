using MathNet.Numerics.LinearAlgebra;
using MathNet.Numerics.Distributions;

namespace Project.Network
{
    public class NeuralNetwork
    {
        public Matrix<float> inputWeights;

        public Matrix<float>[] networkWeights;
        public Vector<float>[] networkBiases;

        public Matrix<float> outputWeights;
        public Vector<float> outputBiases;

        public int layers;
        public int nodesPerLayer;
        public int inputs;
        public int outputs;

        public string ID = "";

        //Create random network
        public NeuralNetwork(int layers, int nodesPerLayer, int inputs, int outputs)
        {
            ContinuousUniform distribution = new ContinuousUniform(-1, 1);

            //Check inputs
            layers = Math.Max(1, layers);
            nodesPerLayer = Math.Max(1, nodesPerLayer);
            inputs = Math.Max(1, inputs);
            outputs = Math.Max(1, outputs);

            //Create random matrices for all the values 
            this.inputWeights = Matrix<float>.Build.Random(nodesPerLayer, inputs, distribution);

            this.networkWeights = new Matrix<float>[layers - 1];
            for (int i = 0; i < networkWeights.Length; i++)
            {
                networkWeights[i] = Matrix<float>.Build.Random(nodesPerLayer, nodesPerLayer, distribution);
            }

            this.networkBiases = new Vector<float>[layers];
            for (int i = 0; i < networkBiases.Length; i++)
            {
                networkBiases[i] = Vector<float>.Build.Random(nodesPerLayer, distribution);
            }

            this.outputWeights = Matrix<float>.Build.Random(outputs, nodesPerLayer, distribution);
            this.outputBiases = Vector<float>.Build.Random(outputs, distribution);

            //Set the identifer variables
            this.layers = layers;
            this.nodesPerLayer = nodesPerLayer;
            this.inputs = inputs;
            this.outputs = outputs;

            //Generate a random ID
            //Used to debug or something IDK
            Random random = new Random();
            for (int i = 0; i < 5; i++)
            {
                this.ID += Convert.ToChar(random.Next(65, 91));
            }

        }

        //Evaluates the network
        public Vector<float> evaluateNetwork(Vector<float> networkInput)
        {
            Vector<float> inputBuffer;
            Vector<float> outputBuffer;

            //Calcuate input layer
            outputBuffer = applyActivaction(inputWeights.Multiply(networkInput) + networkBiases[0]);
            //I really don't want to deal with shallow copies anymore
            //Which madman even made shallow copies a thing
            inputBuffer = Vector<float>.Build.DenseOfArray(outputBuffer.ToArray());

            //Calcuate hidden layers
            for (int i = 0; i < layers - 1; i++)
            {
                outputBuffer = applyActivaction(networkWeights[i].Multiply(inputBuffer) + networkBiases[i + 1]);
                inputBuffer = Vector<float>.Build.DenseOfArray(outputBuffer.ToArray());
            }

            //Calcuate output layer
            outputBuffer = applyActivaction(outputWeights.Multiply(inputBuffer) + outputBiases);

            return outputBuffer;
        }

        //Applies the activaction function to a vector
        private Vector<float> applyActivaction(Vector<float> vector)
        {
            for (int i = 0; i < vector.Count - 1; i++)
            {
                //Sigmoid
                //vector[i] = (1 / (1 + (float)Math.Pow(Math.E, vector[i])));

                //ReLU
                //vector[i] = Math.Max(0, vector[i]);

                //Tanh
                vector[i] = (float)Math.Tanh(vector[i]);

                //ArcTan * 2 / pi
                //vector[i] = (float)(Math.Atan(vector[i]) * 2 / Math.PI);
            }

            return vector;
        }

        //TODO: Add export and import
        public override string ToString()
        {
            return ID;
        }
    }
}