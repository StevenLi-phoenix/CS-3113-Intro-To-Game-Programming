#ifndef PID_CONTROLLER_H
#define PID_CONTROLLER_H

class PIDController
{
public:
    PIDController(float kp = 0.0f, float ki = 0.0f, float kd = 0.0f);

    void setGains(float kp, float ki, float kd);
    void setIntegralLimits(float minimum, float maximum);
    void setOutputLimits(float minimum, float maximum);
    void reset(float integral = 0.0f, float previousError = 0.0f);

    float compute(float setpoint, float measurement, float deltaTime);

    float getProportionalTerm() const { return mProportionalTerm; }
    float getIntegralTerm()     const { return mIntegralTerm;     }
    float getDerivativeTerm()   const { return mDerivativeTerm;   }

private:
    float clamp(float value, float minimum, float maximum) const;

    float mKp;
    float mKi;
    float mKd;

    float mIntegral;
    float mIntegralMin;
    float mIntegralMax;
    bool  mHasIntegralLimits;

    float mOutputMin;
    float mOutputMax;
    bool  mHasOutputLimits;

    float mPreviousError;
    bool  mHasPreviousError;

    float mProportionalTerm;
    float mIntegralTerm;
    float mDerivativeTerm;
};

#endif // PID_CONTROLLER_H