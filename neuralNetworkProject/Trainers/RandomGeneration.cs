using MathNet.Numerics.LinearAlgebra;
using MathNet.Numerics.Distributions;

using Project.Network;



namespace Trainers.RandomGeneration
{
    internal class RandomTrainer
    {
        public RandomTrainer() { }

        public NeuralNetwork[] generateNextGen(NeuralNetwork[] networks, float[] scores, float percentile)
        {
            //Sort the array of networks by their scores
            Array.Sort(scores, networks);
            Array.Reverse(scores);
            Array.Reverse(networks);

            ContinuousUniform distribution = new ContinuousUniform(-1, 1);

            for (int i = (int)Math.Floor(networks.Length * percentile) - 1; i < networks.Length; i++)
            {
                for (int j = 0; j < networks[i].inputWeights.RowCount; j++)
                {
                    for (int k = 0; j < networks[i].inputWeights.ColumnCount; k++)
                    {
                        networks[i].inputWeights[j, k] = (float)distribution.Sample();
                    }
                }

                for (int j = 0; j < networks[i].networkWeights.Length; j++)
                {
                    for (int k = 0; k < networks[i].networkWeights[j].RowCount; k++)
                    {
                        for (int l = 0; l < networks[i].networkWeights[j].ColumnCount; l++)
                        {
                            networks[i].networkWeights[j][k, l] = (float)distribution.Sample();
                        }
                    }
                }

                for (int j = 0; j < networks[i].networkBiases.Length; j++)
                {
                    for (int k = 0; k < networks[i].networkBiases[j].Count; k++)
                    {
                        networks[i].networkBiases[j][k] = (float)distribution.Sample();
                    }
                }

                for (int j = 0; j < networks[i].outputWeights.RowCount; j++)
                {
                    for (int k = 0; k < networks[i].outputWeights.ColumnCount; k++)
                    {
                        networks[i].outputWeights[j, k] = (float)distribution.Sample();
                    }
                }

                for (int j = 0; j < networks[i].outputBiases.Count; j++)
                {
                    networks[i].outputBiases[j] = (float)distribution.Sample();
                }
            }

            return networks;
        }
    }
}
