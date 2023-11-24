﻿using MathNet.Numerics;
using MathNet.Numerics.LinearAlgebra;
using System.Diagnostics;
using System.IO;
using System.Threading;

using Project.Network;

using Games.Pong;
using Trainers.RandomGeneration;

namespace Project
{
    class Program
    {
        static void Main(string[] args)
        {
            //Settings
            int generations = 50;
            int netsPerGen = 50;

            int ticks = 1000;

            int layers = 2;
            int nodesPerLayer = 3;
            int inputs = 5;
            int outputs = 3;

            int numberOfAngles = 60;

            //Get the current project and append the current time
            //Ignore the warnings, it's fine
            string docPath = Environment.CurrentDirectory;
            docPath = Path.Combine(docPath, DateTime.Now.ToString("MM-dd-yy hh-mm-ss"));

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
            RandomTrainer trainer = new RandomTrainer();

            //Create the list of angles to test
            float[] angles = new float[numberOfAngles];
            for (int i = 0; i < numberOfAngles; i++)
            {
                angles[i] = ((360f / numberOfAngles) * i) * ((float)Math.PI / 180);
            }

            //TODO: Run each generation
            for (int i = 0; i < generations; i++)
            {
                
            }

            genLogs.Flush();

            Console.WriteLine(docPath);

            //Uh what
            //Why am I allowed to put a method in a method
            
        }

        float runNetwork(NeuralNetwork network, float[] angles, int ticks)
        {
            //Keeps track of the total score of the network
            float score = 0;

            for (int i = 0; i < angles.Length; i++)
            {
                Pong pongGame = new Pong(500, 500, 1, angles[i]);

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

            return score / angles.Length;
        }
    }
}