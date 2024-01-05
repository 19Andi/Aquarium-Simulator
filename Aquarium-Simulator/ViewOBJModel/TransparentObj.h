#pragma once
#include <glm.hpp>

class TransparentObj
{
public:
	enum class Type {
		GLASS,
		BUBBLE
	};
	enum class WindowType {
		NONE,
		SQUARE,
		RECT,
		CEILING
	};
	glm::vec3 pos;
	
	Type type;
	WindowType windowType;

	TransparentObj(glm::vec3&& pos, Type&& type, WindowType&& windowType = WindowType::NONE)
		: pos{ std::move(pos) }, type{ std::move(type) }, windowType{std::move(windowType)} {}
	TransparentObj() = default;
};

