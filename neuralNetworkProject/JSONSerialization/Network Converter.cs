﻿using MathNet.Numerics.LinearAlgebra;
using Newtonsoft.Json;

namespace Project.Network.JSONSerialization
{
    //Internal class, used to convert between neural network and JSON
    internal class IntermediateNetwork
    {
        public float[,] inputWeights;

        public float[][,] networkWeights;
        public float[][] networkBiases;

        public float[,] outputWeights;
        public float[] outputBiases;

        public int layers;
        public int nodesPerLayer;
        public int inputs;
        public int outputs;

        //Empty constructor, used by JSON library
        public IntermediateNetwork()
        {

        }

        //Creates an intermediate class from a neural network
        public IntermediateNetwork(NeuralNetwork network)
        {
            this.inputWeights = network.inputWeights.ToArray();

            this.networkWeights = new float[network.networkWeights.Length][,];
            for (int i = 0; i < network.networkWeights.Length; i++)
            {
                this.networkWeights[i] = network.networkWeights[i].ToArray();
            }

            this.networkBiases = new float[network.networkBiases.Length][];
            for (int i = 0; i < network.networkBiases.Length; i++)
            {
                this.networkBiases[i] = network.networkBiases[i].ToArray();
            }

            this.outputWeights = network.outputWeights.ToArray();
            this.outputBiases = network.outputBiases.ToArray();

            this.layers = network.layers;
            this.nodesPerLayer = network.nodesPerLayer;
            this.inputs = network.inputs;
            this.outputs = network.outputs;
        }

        //Create a neural network from data of the intermediate class
        public NeuralNetwork ToNetwork()
        {
            NeuralNetwork network = new NeuralNetwork(layers, nodesPerLayer, inputs, outputs);

            network.inputWeights = Matrix<float>.Build.DenseOfArray(this.inputWeights);

            for (int i = 0; i < this.networkWeights.Length; i++)
            {
                network.networkWeights[i] = Matrix<float>.Build.DenseOfArray(this.networkWeights[i]);
            }
            for (int i = 0; i < this.networkBiases.Length; i++)
            {
                network.networkBiases[i] = Vector<float>.Build.DenseOfArray(this.networkBiases[i]);
            }

            network.outputWeights = Matrix<float>.Build.DenseOfArray(this.outputWeights);
            network.outputBiases = Vector<float>.Build.DenseOfArray(this.outputBiases);

            return network;
        }

        //Create a JSON representation of the intermediate class
        public string ToJson()
        {
            JsonSerializerSettings settings = new JsonSerializerSettings();
            settings.Formatting = Formatting.Indented;

            return JsonConvert.SerializeObject(this, settings);
        }
    }


    static class NetworkConverter
    {
        //Creates an intermediate class from a network, then converts it to JSON
        public static string NetworkToJson(NeuralNetwork network)
        {
            IntermediateNetwork intermediate = new IntermediateNetwork(network);

            return intermediate.ToJson();
        }

        //Creates an intermediate class from a JSON input, then converts it to a network
        public static NeuralNetwork JsonToNetwork(string Json)
        {
            IntermediateNetwork intermediate = JsonConvert.DeserializeObject<IntermediateNetwork>(Json);

            return intermediate.ToNetwork();
        }
    }
}
