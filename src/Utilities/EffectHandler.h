#pragma once

class EffectHandler
{
public:
	int GetActiveEffect();
	void SetActiveEffect(int activeEffect);
	int m_activeEffect = 5;
private:
};