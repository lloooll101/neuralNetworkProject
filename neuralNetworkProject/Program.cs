using Games.Pong;
using MathNet.Numerics.LinearAlgebra;
using Project.Network;
using Project.Network.JSONSerialization;
using System.Diagnostics;
using static System.Formats.Asn1.AsnWriter;

namespace Project
{
    class Program
    {
        public static int totalTicks = 0;

        static void Main(string[] args)
        {
            Random random = new Random();

            //Settings
            int ticks = 2000;

            int numberOfAngles = 60;

            //Get the current project and append the current time
            string docPath = Environment.CurrentDirectory;
            docPath = Path.Combine(docPath, DateTime.Now.ToString("MM-dd-yy HH-mm-ss"));

            //Create the directory
            Directory.CreateDirectory(docPath);

            //Create the generation logger
            StreamWriter genLogs = new StreamWriter(Path.Combine(docPath, "genLogs.txt"));

            //Define limits
            float[][] limits =
            {
                [0, 500],
                [0, 500],
                [-5, 5],
                [-5, 5],
                [0, 500]
            };

            //Create matchbox model
            Matchbox matchbox = new Matchbox(Pong.inputs, Pong.outputs, 10, limits);

            //Create the list of angles to test
            float[] angles = new float[numberOfAngles];
            for (int i = 0; i < numberOfAngles; i++)
            {
                angles[i] = ((360f / numberOfAngles) * i) * ((float)Math.PI / 180);
            }

            Pong pongGame = new Pong(500, 500);

            int lastPrintout = 0;

            while (totalTicks < 1000000000)
            {
                if(lastPrintout + 10000000 < totalTicks)
                {
                    //Do printout
                    float score = 0;

                    for (int i = 0; i < angles.Length; i++)
                    {
                        pongGame.reset(5, angles[i]);

                        for (int j = 0; j < ticks; j++)
                        {
                            totalTicks++;

                            Vector<float> inputs = pongGame.getInput();

                            Vector<float> outputs = matchbox.evaluateNetwork(inputs);

                            pongGame.setAction(outputs);

                            matchbox.train(inputs, pongGame.getScoreChange());

                            if (!pongGame.tick())
                            {
                                break;
                            }
                        }

                        score += pongGame.score;
                    }

                    genLogs.WriteLine(totalTicks + "," + score);
                    Console.WriteLine(totalTicks + "," + score);

                    lastPrintout = totalTicks;
                }

                pongGame.reset(5, (float)(random.NextDouble() * 2 * Math.PI));

                for (int i = 0; i < ticks; i++)
                {
                    totalTicks++;

                    Vector<float> inputs = pongGame.getInput();

                    Vector<float> outputs = matchbox.evaluateNetwork(inputs);

                    pongGame.setAction(outputs);

                    matchbox.train(inputs, pongGame.getScoreChange());

                    if (!pongGame.tick())
                    {
                        break;
                    }
                }
            }

            /*
            //Run each generation
            for (int i = 0; i < generations; i++)
            {
                for (int j = 0; j < netsPerGen; j++)
                {
                    //string networkPath = Path.Combine(genPath, "network-" + j);
                    //Directory.CreateDirectory(networkPath);

                    //scoresTasks[j] = runNetwork(networks[j], angles, ticks, networkPath);
                    scoresTasks[j] = runNetwork(networks[j], angles, ticks);
                }

                for (int k = 0; k < netsPerGen; k++)
                {
                    scores[k] = scoresTasks[k].Result;

                }

                //genLogs.WriteLine("Generation: " + i + "\tMax Score: " + scores.Max());
                //Console.WriteLine("Generation: " + i + "\tMax Score: " + scores.Max());

                //CSV version
                genLogs.WriteLine(i + "," + totalTicks + "," + scores.Max());
                Console.WriteLine(i + "," + totalTicks + "," + scores.Max());

                //networks = trainer.generateNextGen(networks, scores, 0.1f, 0.25f);
                networks = trainer.generateNextGen(networks, scores, 0.1f);
            }
            */

            //Output the document path for ease of navigation
            //Also open up the output folder
            Console.WriteLine(docPath);
            Process.Start("explorer.exe", docPath);

            genLogs.Flush();
        }
    }
}