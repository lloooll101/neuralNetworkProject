using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Text.Json;
using MathNet.Numerics.LinearAlgebra;
using MathNet.Numerics.LinearAlgebra.Single;

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
        private static float[] FlattenMatrixRowWise(Matrix<float> matrix)
        {
            int rows = matrix.RowCount;
            int cols = matrix.ColumnCount;
            float[] arr = new float[rows * cols];

            for(int i = 0; i < rows; i++) {
                for(int j = 0; j < cols; j++)
                {
                    arr[i * cols + j] = matrix.At(i, j);
                }
            }
            return arr;
        }
        private static float[] FlattenListRowWise(List<float[]> matrix)
        {
            int rows = matrix.Count;
            int cols = rows > 0 ? matrix.ElementAt(0).Length : 0;
            float[] arr = new float[rows * cols];

            for (int i = 0; i < rows; i++)
            {
                for (int j = 0; j < cols; j++)
                {
                    arr[i * cols + j] = matrix[i][j];
                }
            }
            return arr;
        }

        private static Matrix<float> ReinflateMatrix(float[] floats, int rows, int columns) {
           return Matrix.Build.Dense(columns, rows, floats).Transpose();
        }
        private static List<float[]> ReinflateList(float[] floats, int entries, int floatsPerEntry)
        {
            List<float[]> reinflated = new List<float[]>();
            for(int i = 0; i < entries; i++)
            {
                float[] f = new float[floatsPerEntry];
                for(int j = 0; j < floatsPerEntry; j++)
                {
                    f[j] = floats[i*floatsPerEntry + j];
                }
                reinflated.Add(f);
            }
            return reinflated;
        }

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
            inp.inputWeights = FlattenMatrixRowWise(network.inputWeights);
            jsonType.inputWeights = inp;

            //network layers values
            NeuralNetworkJSONType.NetworkLayers nl = new NeuralNetworkJSONType.NetworkLayers();
            int weightLayers = network.networkWeights.Length;
            int biasesLayers = network.networkBiases.Length;
            //weights first
            List<float[]> networkWeightsList = new List<float[]>();
            for (int i = 0; i < weightLayers; i++)
            {
                networkWeightsList.Add(FlattenMatrixRowWise(network.networkWeights[i]));
            }
            nl.networkWeights = FlattenListRowWise(networkWeightsList);
            //biases
            List<float[]> networkBiasesList = new List<float[]>();
            for (int i = 0; i < biasesLayers; i++)
            {
                networkBiasesList.Add(network.networkBiases[i].ToArray());
            }
            nl.networkBiases = FlattenListRowWise(networkBiasesList);
            jsonType.networkLayers = nl;

            //output values
            NeuralNetworkJSONType.Output outp = new NeuralNetworkJSONType.Output();

            //weights
            outp.outputWeights = FlattenMatrixRowWise(network.outputWeights);
            //biases
            outp.outputBiases = network.outputBiases.ToArray();

            jsonType.outputWeights = outp;

            var options = new JsonSerializerOptions { WriteIndented = true };
            return JsonSerializer.Serialize(jsonType, options);
        }
        public static NeuralNetwork JSONDeserializeNeuralNetwork(string json)
        {
            NeuralNetworkJSONType? deserialized = JsonSerializer.Deserialize<NeuralNetworkJSONType>(json);
            if (deserialized == null)
            {
                throw new Exception("Could not serialize JSON into a Neural network");
            }

            NeuralNetwork network = new NeuralNetwork(
                deserialized.metadata.layers,
                deserialized.metadata.nodesPerLayer,
                deserialized.metadata.inputs,
                deserialized.metadata.outputs
            );
            //convert input weights
            network.inputWeights = ReinflateMatrix(
                deserialized.inputWeights.inputWeights,
                deserialized.metadata.nodesPerLayer,
                deserialized.metadata.inputs
            );

            //convert network values
            List<float[]> networkWeightsList = ReinflateList(
                deserialized.networkLayers.networkWeights,
                deserialized.metadata.layers - 1,
                deserialized.metadata.nodesPerLayer * deserialized.metadata.nodesPerLayer
            );
            List<float[]> networkBiasesList = ReinflateList(
                deserialized.networkLayers.networkBiases,
                deserialized.metadata.layers,
                deserialized.metadata.nodesPerLayer
            );

            for ( int i = 0; i < networkWeightsList.Count; i++)
            {
                network.networkWeights[i] = ReinflateMatrix(
                    networkWeightsList[i],
                    deserialized.metadata.nodesPerLayer,
                    deserialized.metadata.nodesPerLayer
                );
            }
            for (int i = 0; i < networkBiasesList.Count; i++)
            {
                network.networkBiases[i] = Vector<float>.Build.Dense(networkBiasesList[i]);
            }

            //convert output
            network.outputWeights = ReinflateMatrix(
                deserialized.outputWeights.outputWeights,
                deserialized.metadata.outputs,
                deserialized.metadata.nodesPerLayer
            );
            network.outputBiases = Vector<float>.Build.Dense(deserialized.outputWeights.outputBiases);

            return network;
        }
    }


}
