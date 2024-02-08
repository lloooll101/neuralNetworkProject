using MathNet.Numerics.Distributions;
using MathNet.Numerics.LinearAlgebra;

namespace Games.CartPole
{
    class CartPole
    {
        public static int inputs = 4;
        public static int outputs = 2;

        //Declare variables for the cartpole simulation
        private const float g = 10;
        private const float length = 200;
        private const float massRatio = 1;
        private const float strength = 15;
        private const float friction = 0.15f;
        private const float dt = 0.001f;

        //Game end states
        private int XLimit = 100;
        private float angleLimit = 10 * (float)Math.PI / 180;

        //How far to the left or right the cart can move
        private int XSize;

        private float cartX;
        private float cartVelocity;
        private float angle;
        private float angularVelocity;

        public float score;
        private int action;

        //Constructor method
        public CartPole(int XSize)
        {
            this.XSize = XSize;
            reset(0, 0, 0);
        }

        public void reset(float cartVelocity, float angle, float angularVelocity)
        {
            this.cartX = 0;
            this.cartVelocity = cartVelocity;
            this.angle = angle;
            this.angularVelocity = angularVelocity;

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
                default:
                    action = 0;
                    break;
            }
        }

        public Vector<float> getInput()
        {
            float[] inputArray = [cartX, cartVelocity, angle, angularVelocity];
            Vector<float> inputs = Vector<float>.Build.DenseOfArray(inputArray);

            return inputs;
        }

        public float getScoreChange()
        {
            //Score based on time
            //return 1;

            //Score based on stability
            return (1 - Math.Abs(angle) * 0.05f - Math.Abs(cartX) * 0.01f - Math.Abs(angularVelocity) * 0.10f - Math.Abs(cartVelocity) * 0.10f);
        }

        //Ticks the game
        //Return represents if the game is still valid
        public bool tick()
        {
            //Math or something IDK
            //Stolen from "https://jeffjar.me/cartpole.js"
            float cos = (float)Math.Cos(angle);
            float sin = (float)Math.Sin(angle);

            float force = action * strength;
            float det = (length * cos * cos - length * (1 + massRatio));

            float cartAcceleration = (-length * force + length * length * sin * (angularVelocity * angularVelocity) - g * length * sin * cos) / det;
            float angularAcceleration = (-cos * force + length * cos * sin * (angularVelocity * angularVelocity) - g * sin * (1 + massRatio)) / det;

            cartAcceleration -= friction * cartVelocity;

            cartX += cartVelocity * dt;
            angle += angularVelocity * dt;

            cartVelocity = cartAcceleration * dt;
            angularVelocity = angularAcceleration * dt;

            //Check if the pole is out of bounds
            if(Math.Abs(cartX) > XLimit || Math.Abs(angle) > angleLimit)
            {
                return false;
            }

            score += getScoreChange();

            return true;
        }
    }
}
