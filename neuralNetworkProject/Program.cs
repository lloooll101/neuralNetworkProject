using MathNet.Numerics;
using MathNet.Numerics.LinearAlgebra;
using System.Diagnostics;
using System.IO;
using System.Threading;

using Project.Network;

using Games.CartPole;
using Project.Network.JSONSerialization;
using Trainers.RandomGeneration;
using Trainers.RandomMutation;
using Games.Pong;

namespace Project
{
    class Program
    {
        public static int totalTicks = 0;

        static void Main(string[] args)
        {
            Random random = new Random();

            //Settings
            int ticks = 10000;

            int numberOfConditions = 5;

            //Get the current project and append the current time
            string docPath = Environment.CurrentDirectory;
            docPath = Path.Combine(docPath, DateTime.Now.ToString("MM-dd-yy HH-mm-ss"));

            //Create the directory
            Directory.CreateDirectory(docPath);

            //Create the generation logger
            StreamWriter genLogs = new StreamWriter(Path.Combine(docPath, "genLogs.txt"));

            //Create the list of conditions to test
            float[][] conditions = new float[(int)Math.Pow(numberOfConditions, 3)][];
            for (int i = 0; i < numberOfConditions; i++)
            {
                for (int j = 0; j < numberOfConditions; j++)
                {
                    for (int k = 0; k < numberOfConditions; k++)
                    {
                        float cartVelocity = (float)5 / (numberOfConditions - 1) * i - 2.5f;
                        float angle = (10 * (float)Math.PI / 180) / (numberOfConditions - 1) * j - (5 * (float)Math.PI / 180);
                        float angularVelocity = (2 * (float)Math.PI / 180) / (numberOfConditions - 1) * k - (1 * (float)Math.PI / 180);
                        float[] condition = { cartVelocity, angle, angularVelocity };
                        conditions[(numberOfConditions * numberOfConditions * i) + (numberOfConditions * j) + k] = condition;
                    }
                }
            }

            //Define limits
            float[][] limits =
            {
                [-100, 100],
                [-10, 10],
                [-10 * (float)Math.PI / 180, 10 * (float)Math.PI / 180],
                [-5 * (float)Math.PI / 180, 5 * (float)Math.PI / 180]
            };

            //Create matchbox model
            Matchbox matchbox = new Matchbox(CartPole.inputs, CartPole.outputs, 5, limits);

            CartPole cartPoleGame = new CartPole();

            while (totalTicks < 250000000)
            {
                //Do printout
                float score = 0;

                for (int i = 0; i < conditions.Length; i++)
                {
                    cartPoleGame.reset(conditions[1][0], conditions[1][1], conditions[1][2]);

                    for (int j = 0; j < ticks; j++)
                    {
                        totalTicks++;

                        Vector<float> inputs = cartPoleGame.getInput();

                        Vector<float> outputs = matchbox.evaluateNetwork(inputs);

                        cartPoleGame.setAction(outputs);

                        matchbox.train(inputs, cartPoleGame.getScoreChange());

                        if (!cartPoleGame.tick())
                        {
                            break;
                        }
                    }

                    score += cartPoleGame.score;
                }

                genLogs.WriteLine(totalTicks + "," + score);
                Console.WriteLine(totalTicks + "," + score);
            }

            //Output the document path for ease of navigation
            //Also open up the output folder
            Console.WriteLine(docPath);
            Process.Start("explorer.exe", docPath);

            genLogs.Flush();
        }
    }
}