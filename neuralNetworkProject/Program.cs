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
        public static long totalTicks = 0;

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
            Matchbox matchbox = new Matchbox(Pong.inputs, Pong.outputs, 5, limits);

            //Create the list of angles to test
            float[] angles = new float[numberOfAngles];
            for (int i = 0; i < numberOfAngles; i++)
            {
                angles[i] = ((360f / numberOfAngles) * i) * ((float)Math.PI / 180);
            }

            Pong pongGame = new Pong(500, 500);

            long lastPrintout = 0;

            while (totalTicks < 10000000000)
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

            Console.WriteLine(docPath);
            Process.Start("explorer.exe", docPath);

            genLogs.Flush();
        }
    }
}