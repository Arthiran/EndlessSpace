#pragma once


#include "Graphics/Post/PostEffect.h"

class PixelatedEffect : public PostEffect
{
public:
	//Initializes framebuffer
	//Overrides post effect Init
	void Init(unsigned width, unsigned height) override;

	//Applies the effect to this buffer
	//passes the previous framebuffer with the texture to apply as parameter
	void ApplyEffect(PostEffect* buffer) override;

	//Getters
	float GetPixels() const;

	//Setters
	void SetPixels(float pixels);
private:
	float _pixels = 1024.0f;
	float _decrement = 10.0f;
};