#include "pid_controller.h"

PIDController::PIDController(float kp, float ki, float kd)
    : mKp(kp),
      mKi(ki),
      mKd(kd),
      mIntegral(0.0f),
      mIntegralMin(0.0f),
      mIntegralMax(0.0f),
      mHasIntegralLimits(false),
      mOutputMin(0.0f),
      mOutputMax(0.0f),
      mHasOutputLimits(false),
      mPreviousError(0.0f),
      mHasPreviousError(false),
      mProportionalTerm(0.0f),
      mIntegralTerm(0.0f),
      mDerivativeTerm(0.0f)
{
}

void PIDController::setGains(float kp, float ki, float kd)
{
    mKp = kp;
    mKi = ki;
    mKd = kd;
}

void PIDController::setIntegralLimits(float minimum, float maximum)
{
    if (minimum > maximum)
    {
        float temp = minimum;
        minimum = maximum;
        maximum = temp;
    }

    mIntegralMin = minimum;
    mIntegralMax = maximum;
    mHasIntegralLimits = true;
}

void PIDController::setOutputLimits(float minimum, float maximum)
{
    if (minimum > maximum)
    {
        float temp = minimum;
        minimum = maximum;
        maximum = temp;
    }

    mOutputMin = minimum;
    mOutputMax = maximum;
    mHasOutputLimits = true;
}

void PIDController::reset(float integral, float previousError)
{
    mIntegral = integral;
    if (mHasIntegralLimits)
    {
        mIntegral = clamp(mIntegral, mIntegralMin, mIntegralMax);
    }

    mPreviousError = previousError;
    mHasPreviousError = true;

    mProportionalTerm = 0.0f;
    mIntegralTerm     = 0.0f;
    mDerivativeTerm   = 0.0f;
}

float PIDController::compute(float setpoint, float measurement, float deltaTime)
{
    if (deltaTime <= 0.0f)
    {
        return 0.0f;
    }

    float error = setpoint - measurement;

    mProportionalTerm = mKp * error;

    mIntegral += error * deltaTime;
    if (mHasIntegralLimits)
    {
        mIntegral = clamp(mIntegral, mIntegralMin, mIntegralMax);
    }
    mIntegralTerm = mKi * mIntegral;

    float derivative = 0.0f;
    if (mHasPreviousError)
    {
        derivative = (error - mPreviousError) / deltaTime;
    }
    else
    {
        mHasPreviousError = true;
    }
    mDerivativeTerm = mKd * derivative;

    mPreviousError = error;

    float output = mProportionalTerm + mIntegralTerm + mDerivativeTerm;
    if (mHasOutputLimits)
    {
        output = clamp(output, mOutputMin, mOutputMax);
    }

    return output;
}

float PIDController::clamp(float value, float minimum, float maximum) const
{
    if (value < minimum) return minimum;
    if (value > maximum) return maximum;
    return value;
}
