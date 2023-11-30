namespace Games.Pong
{
    using gameObjects;
    class Pong
    {
        //Declare variables for the pong game
        public int Xsize;
        public int Ysize;

        public Ball ball;
        public Paddle paddle;

        public float score;
        public string action;

        //Constructor method
        public Pong(int Xsize, int Ysize, float ballSpeed, float direction)
        {
            this.Xsize = Xsize;
            this.Ysize = Ysize;

            //Calcuate the components of the ball speed based on the direction and the total speed
            float Xvel = (float)Math.Cos(direction) * ballSpeed;
            float Yvel = (float)Math.Sin(direction) * ballSpeed;

            //Place the ball in the middle of the screen
            this.ball = new Ball(Xsize / 2, Ysize / 2, Xvel, Yvel);

            //Place the paddle on the screen, and set its size
            this.paddle = new Paddle(Xsize / 2, Ysize / 10, Xsize / 10);

            this.score = 0;
            this.action = "";
        }

        //Sets the action to be executed in the next tick
        public void setAction(string action)
        {
            this.action = action;
        }

        //Ticks the game
        //Return represents if the game is still valid
        public bool tick()
        {
            //Moves the paddle based on the current setAction
            if (action == "left")
            {
                paddle.X -= 3;
            }
            else if (action == "right")
            {
                paddle.X += 3;
            }

            //Check to see if the ball needs to be checked for collision
            if (ball.Y + ball.Yvel < paddle.Y)
            {
                //Calcuate the subtick and the x position of the potential collision
                float collideSubtick = (paddle.Y - ball.Y) / ball.Yvel;
                float collideX = ball.X + ball.Xvel * collideSubtick;

                //Check if the paddle was in the path of the ball
                if ((collideX < paddle.X + paddle.width) && (collideX > paddle.X - paddle.width))
                {
                    //Increment score
                    score += 1;

                    //Update the ball position and velocity
                    ball.X += ball.Xvel;
                    ball.Y += ball.Yvel;

                    ball.Y = -(ball.Y - paddle.Y) + paddle.Y;

                    ball.Yvel *= -1;
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

            return true;
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

            public Ball(float X, float Y, float Xvel, float Yvel)
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
            public readonly float Y;
            public int width;

            public Paddle(float X, float Y, int width)
            {
                this.X = X;
                this.Y = Y;
                this.width = width;
            }
        }
    }
}
