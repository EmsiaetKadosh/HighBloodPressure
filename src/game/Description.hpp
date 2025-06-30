//
// Created by EmsiaetKadosh on 25-6-29.
//

#pragma once

#include "..\utils\IText.h"

class Description {
	Vector<RenderableString> description;
	unsigned int indentLevel = 1;

public:
	Description& title(const RenderableString& title) & noexcept { return description.emplace_back(title), *this; }
	Description&& title(const RenderableString& title) && noexcept { return std::move(this->title(title)); }
	Description& title(RenderableString&& title) & noexcept { return description.emplace_back(std::move(title)), *this; }
	Description&& title(RenderableString&& title) && noexcept { return std::move(this->title(std::move(title))); }
	Description& textline(const RenderableString& text) & noexcept { return description.emplace_back(L"- "_renderable.append(text)), *this; }
	Description&& textline(const RenderableString& text) && noexcept { return std::move(this->textline(text)); }
	Description& textline(RenderableString&& text) & noexcept { return description.emplace_back(L"- "_renderable.append(std::move(text))), *this; }
	Description&& textline(RenderableString&& text) && noexcept { return std::move(this->textline(std::move(text))); }
	Description& push() & noexcept { return ++indentLevel, *this; }
	Description&& push() && noexcept { return std::move(this->push()); }
	Description& pull() & noexcept { return indentLevel ? --indentLevel : 0, *this; }
	Description&& pull() && noexcept { return std::move(this->pull()); }

	[[nodiscard]] Vector<RenderableString>::iterator begin() noexcept { return description.begin(); }
	[[nodiscard]] Vector<RenderableString>::iterator end() noexcept { return description.end(); }
	[[nodiscard]] Vector<RenderableString>::const_iterator begin() const noexcept { return description.begin(); }
	[[nodiscard]] Vector<RenderableString>::const_iterator end() const noexcept { return description.end(); }
};

struct IDescribable {
	virtual ~IDescribable() = default;
	virtual Description getDescription() const = 0;
};
