using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Text.Json;

namespace Project.Network.JSONSerialization
{
    internal class NeuralNetworkJSONType
    {
        public struct Metadata
        {
            public int layers { get; set; }
            public int nodesPerLayer { get; set; }
            public int inputs { get; set; }
            public int outputs { get; set; }

            public string ID { get; set; }
        }
        public Metadata metadata { get; set; }

        public struct Input
        {
            public float[] inputWeights { get; set; }
        }
        public Input inputWeights { get; set; }

        public struct NetworkLayers
        {
            public float[] networkWeights { get; set; }
            public float[] networkBiases { get; set; }
        }
        public NetworkLayers networkLayers { get; set; }

        public struct Output
        {
            public float[] outputWeights { get; set; }
            public float[] outputBiases { get; set; }
        }
        public Output outputWeights { get; set; }
    }

    public class NetworkSerializer
    {

        public static string JSONSerializeNeuralNetwork(NeuralNetwork network)
        {
            //fill json type
            NeuralNetworkJSONType jsonType = new NeuralNetworkJSONType();

            //metadata
            NeuralNetworkJSONType.Metadata m = new NeuralNetworkJSONType.Metadata();
            m.layers = network.layers;
            m.nodesPerLayer = network.nodesPerLayer;
            m.inputs = network.inputs;
            m.outputs = network.outputs;
            m.ID = network.ID;
            jsonType.metadata = m;
            //input weights
            /*
             * Even if the data is an array of floats, the matrix should
             * be able to be reconstructed using the additional data in 
             * the metadata.
            */
            NeuralNetworkJSONType.Input inp = new NeuralNetworkJSONType.Input();
            int inputWeightsCount = network.inputWeights.Enumerate().Count();
            inp.inputWeights = new float[inputWeightsCount];
            int idx = 0;
            foreach (float value in network.inputWeights.Enumerate())
            {
                inp.inputWeights[idx] = value;
                idx++;
            }
            jsonType.inputWeights = inp;
            //network layers values
            NeuralNetworkJSONType.NetworkLayers nl = new NeuralNetworkJSONType.NetworkLayers();
            int weightLayers = network.networkWeights.Length;
            int weightsPerLayer = network.networkWeights.Length > 0 ? network.networkWeights[0].Enumerate().Count() : 0;
            int biasesLayers = network.networkBiases.Length;
            int biasesPerLayer = network.networkBiases.Length > 0 ? network.networkBiases[0].Enumerate().Count() : 0 ;
            nl.networkWeights = new float[weightLayers * weightsPerLayer];
            nl.networkBiases = new float[biasesLayers * biasesPerLayer];
            //weights first
            for (int i = 0; i < weightLayers; i++)
            {
                idx = 0;
                foreach (float value in network.networkWeights[i].Enumerate())
                {
                    nl.networkWeights[i*weightsPerLayer + idx] = value;
                    idx++;
                }
            }
            //biases
            for(int i = 0; i < biasesLayers; i++)
            {
                idx = 0;
                foreach (float value in network.networkBiases[i].Enumerate())
                {
                    nl.networkBiases[i*biasesPerLayer + idx] = value; 
                    idx++;
                }
            }
            jsonType.networkLayers = nl;

            //output values
            NeuralNetworkJSONType.Output outp = new NeuralNetworkJSONType.Output();
            int outputWeightsCount = network.outputWeights.Enumerate().Count();
            int outputBiasesCount = network.outputBiases.Enumerate().Count();

            outp.outputWeights = new float[outputWeightsCount];
            outp.outputBiases = new float[outputBiasesCount];

            //weights
            idx = 0;
            foreach (float value in network.outputWeights.Enumerate())
            {
                outp.outputWeights[idx] = value;
                idx++;
            }
            //biases
            idx = 0;
            foreach (float value in network.outputBiases.Enumerate())
            {
                outp.outputBiases[idx] = value;
                idx++;
            }
            jsonType.outputWeights = outp;

            var options = new JsonSerializerOptions { WriteIndented = true };
            return JsonSerializer.Serialize(jsonType, options);
        }
    }


}
