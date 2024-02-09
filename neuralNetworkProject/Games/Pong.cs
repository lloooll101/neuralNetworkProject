using MathNet.Numerics.LinearAlgebra;

namespace Games.Pong
{
    using gameObjects;
    class Pong
    {
        public static int inputs = 5;
        public static int outputs = 3;

        //Declare variables for the pong game
        private int Xsize;
        private int Ysize;

        public Ball ball;
        public Paddle paddle;

        public float score;
        private int action;

        private bool hasBounced = false;

        //Constructor method
        public Pong(int Xsize, int Ysize)
        {
            this.Xsize = Xsize;
            this.Ysize = Ysize;

            this.ball = new Ball();
            this.paddle = new Paddle();

            reset(0, 0);
        }

        //Resets the game
        public void reset(float ballSpeed, float direction)
        {
            //Calcuate the components of the ball speed based on the direction and the total speed
            float Xvel = (float)Math.Cos(direction) * ballSpeed;
            float Yvel = (float)Math.Sin(direction) * ballSpeed;

            //Place the ball in the middle of the screen
            this.ball.reset(Xsize / 2, Ysize / 2, Xvel, Yvel);

            //Place the paddle on the screen, and set its size
            this.paddle.reset(Xsize / 2, Xsize / 10);

            this.score = 0;
            this.action = 0;
        }

        //Sets the action to be executed in the next tick
        public void setAction(Vector<float> outputs)
        {
            switch (outputs.MaximumIndex())
            {
                case 0:
                    action = -1;
                    break;
                case 1:
                    action = 1;
                    break;
                case 2:
                    action = 0;
                    break;
                default:
                    action = 0;
                    break;
            }
        }

        public Vector<float> getInput()
        {
            float[] inputArray = [ball.X, ball.Y, ball.Xvel, ball.Yvel, paddle.X];
            Vector<float> inputs = Vector<float>.Build.DenseOfArray(inputArray);

            return inputs;
        }

        public float getScoreChange()
        {
            //Score based on number of bounces
            /*
            if (hasBounced)
            {
                hasBounced = false;
                return 1;
            }
            */

            //Score based on survival time
            //return 1;

            //Score based on action
            float collideX = getCollisionPoint();

            //If collision is to the left
            if (collideX < paddle.X - paddle.width)
            {
                if (action == -1)
                {
                    return 1;
                }
                return -1;
            }
            //If collision is to the right
            else if (collideX > paddle.X + paddle.width)
            {
                if (action == 1)
                {
                    return 1;
                }
                return -1;
            }
            //If collision is at the paddle
            else
            {
                if (action == 0)
                {
                    return 2;
                }
                return -2;
            }
        }

        public float getCollisionPoint()
        {
            float collisionY = 0;
            if (ball.Yvel > 0)
            {
                collisionY = Ysize * 2;
            }

            float ticks = (collisionY - ball.Y) / ball.Yvel;
            float collisionX = ticks * ball.Xvel + ball.X;

            if (mod((int)Math.Floor(collisionX / Xsize), Xsize) == 0)
            {
                return mod(collisionX, Xsize);
            }

            return Xsize - mod(collisionX, Xsize);
        }

        //Ticks the game
        //Return represents if the game is still valid
        public bool tick()
        {
            //Moves the paddle based on the current setAction
            paddle.X += action * 3;

            //Check to see if the ball needs to be checked for collision
            if (ball.Y + ball.Yvel < 0)
            {
                float collideX = getCollisionPoint();

                //Check if the paddle was in the path of the ball
                if ((collideX < paddle.X + paddle.width) && (collideX > paddle.X - paddle.width))
                {
                    //Update the ball position and velocity
                    ball.X += ball.Xvel;
                    ball.Y += ball.Yvel;

                    ball.Y = -ball.Y;

                    ball.Yvel = -ball.Yvel;

                    //Set the hasBounced tag
                    hasBounced = true;
                }
                //If the paddle was not in the path, update the score based on the distance and return false
                else
                {
                    score += (Xsize - Math.Abs(collideX - paddle.X)) / (Xsize * 2);
                    return false;
                }

            }
            else
            {
                //Update the ball position
                ball.X += ball.Xvel;
                ball.Y += ball.Yvel;
            }

            //Bounces the ball if it is beyond the borders
            if (ball.X < 0)
            {
                ball.X *= -1;
                ball.Xvel *= -1;
            }

            if (ball.X > Xsize)
            {
                ball.X = Xsize - (ball.X - Xsize);
                ball.Xvel *= -1;
            }

            if (ball.Y > Ysize)
            {
                ball.Y = Ysize - (ball.Y - Ysize);
                ball.Yvel *= -1;
            }

            score += getScoreChange();

            return true;
        }

        public float mod(float number, float modulus)
        {
            return ((number % modulus) + modulus) % modulus;
        }
    }

    namespace gameObjects
    {
        public class Ball
        {
            public float X;
            public float Y;
            public float Xvel;
            public float Yvel;

            public Ball()
            {
                reset(0, 0, 0, 0);
            }
            public void reset(float X, float Y, float Xvel, float Yvel)
            {
                this.X = X;
                this.Y = Y;
                this.Xvel = Xvel;
                this.Yvel = Yvel;
            }
        }

        public class Paddle
        {
            public float X;
            public int width;

            public Paddle()
            {
                reset(0, 0);
            }

            public void reset(float X, int width)
            {
                this.X = X;
                this.width = width;
            }
        }
    }
}
