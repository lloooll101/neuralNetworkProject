using MathNet.Numerics.LinearAlgebra;

namespace Project.Network
{
    public class Matchbox
    {
        int inputs;
        int outputs;
        int buckets;

        float[][] limits;

        Array matchboxes;

        Vector<float> previousOutput = Vector<float>.Build.Dense(1);

        //Create random network
        public Matchbox(int inputs, int outputs, int buckets, float[][] limits)
        {
            //Store things
            this.inputs = inputs;
            this.outputs = outputs;
            this.buckets = buckets;

            this.limits = limits;

            //Create the number of dimensions of the table
            int[] dimensions = new int[inputs];
            Array.Fill(dimensions, buckets);

            matchboxes = Array.CreateInstance(typeof(float[]), dimensions);

            int[] index = new int[inputs];

            for (int i = 0; i < Math.Pow(buckets, inputs); i++)
            {
                float[] defaultOutput = new float[outputs];
                Array.Fill(defaultOutput, 3);

                matchboxes.SetValue(defaultOutput, index);

                index[inputs - 1]++;

                for (int j = inputs - 1; j > 0; j--)
                {
                    if (index[j] == buckets)
                    {
                        index[j] = 0;
                        index[j - 1]++;
                    }
                }
            }
        }

        //Evaluates the network
        public Vector<float> evaluateNetwork(Vector<float> input)
        {
            Random random = new Random();

            Vector<float> output = Vector<float>.Build.Dense(outputs);

            int[] index = getIndex(input);

            float[] matchbox = (float[])matchboxes.GetValue(index);

            float selectedOutput = (float)random.NextDouble() * matchbox.Sum();

            for (int i = 0; i < outputs; i++)
            {
                if (selectedOutput < matchbox[i])
                {
                    output[i] = 1;
                    previousOutput = output;
                    return output;
                }

                selectedOutput -= matchbox[i];
            }

            output[outputs - 1] = 1;
            previousOutput = output;
            return output;
        }

        public int[] getIndex(Vector<float> input)
        {
            int[] index = new int[inputs];

            for (int i = 0; i < inputs; i++)
            {
                index[i] = map(input[i], limits[i][0], limits[i][1], 0, buckets - 1);
            }

            return index;
        }

        public void train(Vector<float> input, float scoreChange)
        {
            int[] index = getIndex(input);
            int previousAction = previousOutput.MaximumIndex();

            float[] weights = (float[])matchboxes.GetValue(index);

            weights[previousAction] = Math.Max(weights[previousAction] + scoreChange, 1);

            matchboxes.SetValue(weights, index);
        }

        private int map(float input, float inMin, float inMax, float outMin, float outMax)
        {
            float value = (input - inMin) / (inMax - inMin) * (outMax - outMin) + outMax;
            return (int)Math.Round(clamp(value, outMin, outMax));
        }

        private float clamp(float input, float min, float max)
        {
            return Math.Max(Math.Min(input, max), min);
        }
    }
}