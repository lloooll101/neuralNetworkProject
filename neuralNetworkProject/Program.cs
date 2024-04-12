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

namespace Project
{
    class Program
    {
        public static int totalTicks = 0;

        static void Main(string[] args)
        {
            //Settings
            int generations = 50;
            int netsPerGen = 50;

            int ticks = 10000;

            int layers = 2;
            int nodesPerLayer = 4;
            int inputs = 4;
            int outputs = 2;

            int numberOfConditions = 5;

            //Get the current project and append the current time
            string docPath = Environment.CurrentDirectory;
            docPath = Path.Combine(docPath, DateTime.Now.ToString("MM-dd-yy HH-mm-ss"));

            //Create the directory
            Directory.CreateDirectory(docPath);

            //Create the generation logger
            StreamWriter genLogs = new StreamWriter(Path.Combine(docPath, "genLogs.txt"));

            //Create and populate the array of networks
            NeuralNetwork[] networks = new NeuralNetwork[netsPerGen];
            for (int i = 0; i < netsPerGen; i++)
            {
                networks[i] = new NeuralNetwork(layers, nodesPerLayer, inputs, outputs);
            }

            //Create the trainer class
            MutationTrainer trainer = new MutationTrainer();

            //Create the list of conditions to test
            float[][] conditions = new float[(int)Math.Pow(numberOfConditions, 3)][];
            for (int i = 0; i < numberOfConditions; i++)
            {
               for (int j = 0; j < numberOfConditions; j++)
               {
                    for(int k = 0; k < numberOfConditions; k++)
                    {
                        float cartVelocity = (float)5 / (numberOfConditions - 1) * i - 2.5f;
                        float angle = (10 * (float)Math.PI / 180) / (numberOfConditions - 1) * j - (5 * (float)Math.PI / 180);
                        float angularVelocity = (2 * (float)Math.PI / 180) / (numberOfConditions - 1) * k - (1 * (float)Math.PI / 180);
                        float[] condition = { cartVelocity, angle, angularVelocity };
                        conditions[(numberOfConditions * numberOfConditions * i) + (numberOfConditions * j) + k] = condition;
                    }
               }
            }

            Task<float>[] scoresTasks = new Task<float>[netsPerGen];
            float[] scores = new float[netsPerGen];

            Stopwatch stopwatch = new Stopwatch();
            stopwatch.Start();

            //Run each generation
            for (int i = 0; i < generations; i++)
            {
                //string genPath = Path.Combine(docPath, "Generation-" + i);
                //Directory.CreateDirectory(genPath);

                /*//Clear the scores array*/
                for(int j = 0; j < netsPerGen; j++)
                {
                    //string networkPath = Path.Combine(genPath, "network-" + j);
                    //Directory.CreateDirectory(networkPath);

                    //scoresTasks[j] = runNetwork(networks[j], angles, ticks, networkPath);
                    scoresTasks[j] = runNetwork(networks[j], conditions, ticks);
                }

                for(int k = 0; k < netsPerGen; k++)
                {
                    scores[k] = scoresTasks[k].Result;

                }

                //genLogs.WriteLine("Generation: " + i + "\tMax Score: " + scores.Max());
                //Console.WriteLine("Generation: " + i + "\tMax Score: " + scores.Max());

                //CSV version
                genLogs.WriteLine(i + "," + totalTicks + "," + scores.Max());
                Console.WriteLine(i + "," + totalTicks + "," + scores.Max());

                networks = trainer.generateNextGen(networks, scores, 0.1f, 0.25f);
            }

            stopwatch.Stop();
            Console.WriteLine(stopwatch.ElapsedMilliseconds);

            /*
            //Convert the top network to a JSON string and output it
            string networkAsString = NetworkConverter.NetworkToJson(networks[0]);
            Console.WriteLine(networkAsString);

            //Create a new network from the imported JSON
            //This is curently used to test the import function
            NeuralNetwork importedNetwork = NetworkConverter.JsonToNetwork(networkAsString);

            //Run the imported network to compare the score
            Console.WriteLine(runNetwork(importedNetwork, angles, ticks));
            */

            //Output the document path for ease of navigation
            //Also open up the output folder
            Console.WriteLine(docPath);
            Process.Start("explorer.exe", docPath);
            
            genLogs.Flush();
        }

        //Run the network against the set of angles, returning the score
        public static async Task<float> runNetwork(NeuralNetwork network, float[][] conditions, int ticks)
        {
            return await Task.Run(() => {

                //Keeps track of the total score of the network
                float score = 0;
                CartPole cartPoleGame = new CartPole();

                for (int i = 0; i < conditions.Length; i++)
                {
                    cartPoleGame.reset(conditions[1][0], conditions[1][1], conditions[1][2]);

                    for (int j = 0; j < ticks; j++)
                    {
                        totalTicks++;

                        //Create the input vector
                        Vector<float> inputs = cartPoleGame.getInput();

                        //Evaluate the network
                        Vector<float> outputs = network.evaluateNetwork(inputs);

                        //Set the action of the network
                        cartPoleGame.setAction(outputs);

                        //Tick the game, and break if the ball falls
                        if (!cartPoleGame.tick())
                        {
                            break;
                        }
                    }

                    score += cartPoleGame.score;
                }

                return score;

            });
            
        }
    }
}