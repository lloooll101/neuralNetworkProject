using Games.FlappyBird;
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
            Matchbox matchbox = new Matchbox(FlappyBird.inputs, FlappyBird.outputs, 5, limits);

            //Create the list of angles to test
            float[] angles = new float[numberOfAngles];
            for (int i = 0; i < numberOfAngles; i++)
            {
                angles[i] = ((360f / numberOfAngles) * i) * ((float)Math.PI / 180);
            }

            FlappyBird birdGame = new FlappyBird();

            long lastPrintout = 0;

            while (totalTicks < 1000000)
            {
                float score = 0;

                for (int i = 0; i < angles.Length; i++)
                {
                    birdGame.reset();

                    for (int j = 0; j < ticks; j++)
                    {
                        totalTicks++;

                        Vector<float> inputs = birdGame.getInput();

                        Vector<float> outputs = matchbox.evaluateNetwork(inputs);

                        birdGame.setAction(outputs);

                        matchbox.train(inputs, birdGame.getScoreChange());

                        if (!birdGame.tick())
                        {
                            break;
                        }
                    }

                    score += birdGame.score;
                }

                genLogs.WriteLine(totalTicks + "," + score);
                Console.WriteLine(totalTicks + "," + score);

                lastPrintout = totalTicks;
            }

            Console.WriteLine(docPath);
            Process.Start("explorer.exe", docPath);

            genLogs.Flush();
        }
    }
}