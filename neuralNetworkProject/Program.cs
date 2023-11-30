using MathNet.Numerics;
using MathNet.Numerics.LinearAlgebra;
using System.Diagnostics;
using System.IO;
using System.Threading;

using Project.Network;

using Games.Pong;
using Project.Network.JSONSerialization;
using Trainers.RandomGeneration;
using Trainers.RandomMutation;

namespace Project
{
    class Program
    {
        static void Main(string[] args)
        {
            //Settings
            int generations = 25;
            int netsPerGen = 25;

            int ticks = 2000;

            int layers = 2;
            int nodesPerLayer = 3;
            int inputs = 5;
            int outputs = 3;

            int numberOfAngles = 60;

            //Get the current project and append the current times
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

            //Create the list of angles to test
            float[] angles = new float[numberOfAngles];
            for (int i = 0; i < numberOfAngles; i++)
            {
                angles[i] = ((360f / numberOfAngles) * i) * ((float)Math.PI / 180);
            }

            //Run each generation
            for (int i = 0; i < generations; i++)
            {
                string genPath = Path.Combine(docPath, "Generation-" + i);
                Directory.CreateDirectory(genPath);

                //Clear the scores array
                float[] scores = new float[netsPerGen];

                for(int j = 0; j < netsPerGen; j++)
                {
                    string networkPath = Path.Combine(genPath, "network-" + j);
                    Directory.CreateDirectory(networkPath);

                    //scores[j] = runNetwork(networks[j], angles, ticks, networkPath);
                    scores[j] = runNetwork(networks[j], angles, ticks);
                }

                genLogs.WriteLine("Generation: " + i + "\tMax Score: " + scores.Max());
                Console.WriteLine("Generation: " + i + "\tMax Score: " + scores.Max());

                networks = trainer.generateNextGen(networks, scores, 0.1f, 0.25f);
            }

            //Convert the top network to a JSON string and output it
            string networkAsString = NetworkConverter.NetworkToJson(networks[0]);
            Console.WriteLine(networkAsString);

            //Create a new network from the imported JSON
            //This is curently used to test the import function
            NeuralNetwork importedNetwork = NetworkConverter.JsonToNetwork(networkAsString);

            //Run the imported network to compare the score
            Console.WriteLine(runNetwork(importedNetwork, angles, ticks));

            //Output the document path for ease of navigation
            //Also open up the output folder
            Console.WriteLine(docPath);
            Process.Start("explorer.exe", docPath);

            genLogs.Flush();
        }

        //Run the network against the set of angles, returning the score
        public static float runNetwork(NeuralNetwork network, float[] angles, int ticks)
        {
            //Keeps track of the total score of the network
            float score = 0;

            for (int i = 0; i < angles.Length; i++)
            {
                Pong pongGame = new Pong(500, 500, 5, angles[i]);

                for (int j = 0; j < ticks; j++)
                {
                    //Create the input vector
                    float[] inputArray = [pongGame.ball.X, pongGame.ball.Y, pongGame.ball.Xvel, pongGame.ball.Yvel, pongGame.paddle.X];
                    Vector<float> inputs = Vector<float>.Build.DenseOfArray(inputArray);

                    //Evaluate the network
                    Vector<float> outputs = network.evaluateNetwork(inputs);

                    //Set the action of the network
                    switch (outputs.MaximumIndex())
                    {
                        case 0:
                            pongGame.setAction("left");
                            break;
                        case 1:
                            pongGame.setAction("right");
                            break;
                        case 2:
                            pongGame.setAction("");
                            break;
                        default:
                            pongGame.setAction("");
                            break;
                    }

                    //Tick the game, and break if the ball falls
                    if (!pongGame.tick())
                    {
                        break;
                    }
                }

                score += pongGame.score;
            }

            return score;
        }

        //Run the network, logging every run
        public static float runNetwork(NeuralNetwork network, float[] angles, int ticks, string path)
        {
            StreamWriter networkLog = new StreamWriter(Path.Combine(path, "networkLog.txt"));
            string gamesPath = Path.Combine(path, "Games");
            Directory.CreateDirectory(gamesPath);

            //Keeps track of the total score of the network
            float score = 0;

            for (int i = 0; i < angles.Length; i++)
            {
                StreamWriter gameLog = new StreamWriter(Path.Combine(gamesPath, "games-" + i + ".txt"));

                Pong pongGame = new Pong(500, 500, 5, angles[i]);

                for (int j = 0;; j++)
                {
                    //Create the input vector
                    float[] inputArray = [pongGame.ball.X, pongGame.ball.Y, pongGame.ball.Xvel, pongGame.ball.Yvel, pongGame.paddle.X];
                    Vector<float> inputs = Vector<float>.Build.DenseOfArray(inputArray);

                    //Evaluate the network
                    Vector<float> outputs = network.evaluateNetwork(inputs);

                    //Set the action of the network
                    switch (outputs.MaximumIndex())
                    {
                        case 0:
                            pongGame.setAction("left");
                            break;
                        case 1:
                            pongGame.setAction("right");
                            break;
                        case 2:
                            pongGame.setAction("");
                            break;
                        default:
                            pongGame.setAction("");
                            break;
                    }

                    gameLog.WriteLine(pongGame.ball.X + "," + pongGame.ball.Y + "," + pongGame.paddle.X);

                    //Tick the game, and break if the ball falls
                    if (!pongGame.tick() || j >= ticks)
                    {
                        gameLog.Flush();
                        networkLog.WriteLine("Score: " + Math.Round(pongGame.score, 4) + "\tTicks: " + j);
                        break;
                    }
                }
                
                score += pongGame.score;
            }

            networkLog.Flush();
            return score;
        }
    }
}