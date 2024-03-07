using MathNet.Numerics.Distributions;
using MathNet.Numerics.LinearAlgebra;

namespace Games.FlappyBird
{
    class FlappyBird
    {
        public static int inputs = 4;
        public static int outputs = 2;

        private const float gravity = 1;
        private const float flapStrength = 3;

        private const int openingHeight = 4;

        private float yPos;
        private float yVel;
        private float xDistance;
        private float openingPosition;

        public float score;
        private int action;

        private bool checkedCollision = false;
        private bool survivedCollision = true;

        Random random = new Random();

        //Constructor method
        public FlappyBird()
        {
            reset();
        }

        public void reset()
        {
            this.yPos = 50;
            this.yVel = 0;
            this.xDistance = 50;
            this.openingPosition = 50;

            this.score = 0;
            this.action = 0;
        }

        //Sets the action to be executed in the next tick
        public void setAction(Vector<float> outputs)
        {
            switch (outputs.MaximumIndex())
            {
                case 0:
                    action = 1;
                    break;
                case 1:
                    action = 0;
                    break;
                default:
                    action = 0;
                    break;
            }
        }

        public Vector<float> getInput()
        {
            float[] inputArray = [yPos, yVel, xDistance, openingPosition];
            Vector<float> inputs = Vector<float>.Build.DenseOfArray(inputArray);

            return inputs;
        }

        public float getScoreChange()
        {
            float scoreChange = 0;

            if (checkedCollision)
            {
                checkedCollision = false;
                if (survivedCollision)
                {
                    scoreChange += 10;
                }
                else
                {
                    scoreChange += -10;
                }
            }

            if (Math.Abs(openingPosition - yPos) < openingHeight)
            {
                scoreChange += 3;
            }
            else
            {
                if (openingPosition - yPos < -openingHeight && yVel < 0)
                {
                    scoreChange += 1;
                }
                else if (openingPosition - yPos > openingHeight && yVel > 0)
                {
                    scoreChange += 1;
                }
                else
                {
                    scoreChange += -3;
                }
            }


            return scoreChange;
        }

        //Ticks the game
        //Return represents if the game is still valid
        public bool tick()
        {
            yVel -= gravity;
            yVel += action * flapStrength;

            yPos += yVel;

            xDistance--;

            if (xDistance <= 0)
            {
                //TODO: Check collision
                if (Math.Abs(openingPosition - yPos) < openingHeight)
                {
                    survivedCollision = true;
                }
                else
                {
                    survivedCollision = false;
                    score += getScoreChange();
                    return false;
                }

                openingPosition = (int)Math.Round(random.NextDouble() * 80) + 10;

                xDistance = 50;

                checkedCollision = true;
            }

            score += getScoreChange();
            return true;
        }
    }
}
