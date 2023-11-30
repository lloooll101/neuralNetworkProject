using MathNet.Numerics.LinearAlgebra;
using MathNet.Numerics.Distributions;

using Project.Network;

namespace Trainers.RandomMutation
{
    internal class MutationTrainer
    {
        public MutationTrainer() { }
        
        public NeuralNetwork[] generateNextGen(NeuralNetwork[] networks, float[] scores, float percentile, float mutationRate)
        {
            Random random = new Random();

            //Sort the array of networks by their scores
            Array.Sort(scores, networks);
            Array.Reverse(scores);
            Array.Reverse(networks);

            ContinuousUniform distribution = new ContinuousUniform(-mutationRate, mutationRate);

            //Calcuate how many networks should survive, and store those networks
            int survivingNum = Math.Max((int)Math.Floor(networks.Length * percentile), 1);

            NeuralNetwork[] survivingNetworks = new NeuralNetwork[survivingNum];
            float[] survivingScores = new float[survivingNum];

            for (int i = 0; i < survivingNum; i++)
            {
                survivingNetworks[i] = networks[i];
                survivingScores[i] = scores[i];
            }

            //For every network that needs to be replaced, create a new network based on a parent
            for (int i = survivingNum - 1; i < networks.Length; i++)
            {
                //Get a parent, weighted by the scores of the parent
                NeuralNetwork parent = ObjectWeightedRandom(survivingNetworks, survivingScores, random);

                for (int j = 0; j < networks[i].inputWeights.RowCount; j++)
                {
                    for (int k = 0; k < networks[i].inputWeights.ColumnCount; k++)
                    {
                        networks[i].inputWeights[j, k] = parent.inputWeights[j, k] + (float)distribution.Sample();
                    }
                }

                for (int j = 0; j < networks[i].networkWeights.Length; j++)
                {
                    for (int k = 0; k < networks[i].networkWeights[j].RowCount; k++)
                    {
                        for (int l = 0; l < networks[i].networkWeights[j].ColumnCount; l++)
                        {
                            networks[i].networkWeights[j][k, l] = parent.networkWeights[j][k, l] + (float)distribution.Sample();
                        }
                    }
                }

                for (int j = 0; j < networks[i].networkBiases.Length; j++)
                {
                    for (int k = 0; k < networks[i].networkBiases[j].Count; k++)
                    {
                        networks[i].networkBiases[j][k] = parent.networkBiases[j][k] + (float)distribution.Sample();
                    }
                }

                for (int j = 0; j < networks[i].outputWeights.RowCount; j++)
                {
                    for (int k = 0; k < networks[i].outputWeights.ColumnCount; k++)
                    {
                        networks[i].outputWeights[j, k] = parent.outputWeights[j, k] + (float)distribution.Sample();
                    }
                }

                for (int j = 0; j < networks[i].outputBiases.Count; j++)
                {
                    networks[i].outputBiases[j] = parent.outputBiases[j] + (float)distribution.Sample();
                }
            }

            return networks;
        }

        public T ObjectWeightedRandom<T>(T[] objects, float[] weights, Random random)
        {
            //Ensure that all the weights are greater than 0
            for (int i = 0; i < weights.Length; i++)
            {
                weights[i] = Math.Max(0.01f, weights[i]);
            }

            float totalWeight = weights.Sum();

            float targetWeight = (float)random.NextDouble() * totalWeight;

            for (int i = 0; i < objects.Length; i++)
            {
                if (targetWeight < weights[i])
                {
                    return objects[i];
                }

                targetWeight -= weights[i];
            }

            throw new Exception("The weighted random function broke again, somehow\nHonestly, if this ever triggers, I give up");
        }
    }
}
