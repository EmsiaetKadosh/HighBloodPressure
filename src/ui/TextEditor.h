//
// Created by EmsiaetKadosh on 25-5-11.
//

#pragma once

#include "..\utils\utils.h"
#include "..\utils\IText.h"

/**
 * 该结构中的x和y可以用于指代一个字符。被指代的字符就是其指代插入点紧邻的下一个字符。
 * 其中，指定-1的指代的是编辑器目前的光标位置。
 */
struct TextEditorCursor {
	unsigned int line = static_cast<unsigned int>(-1), column = static_cast<unsigned int>(-1);
};

class TextEditorLine final {
	String content;
	RenderableString render = L"";
	bool shouldRerender = false; // 指定String是否已经更改，是否需要重新构建RenderableString

public:
	TextEditorLine() = default;
	~TextEditorLine() = default;
};

class TextEditor {
	Vector<TextEditorLine> lines = {TextEditorLine()}; // 保证至少有一行
	TextEditorCursor cursor = {0, 0};
	AtomicStorage atomic;
	bool focusing = false;
	bool autoLineBreak = false;

public:
	void focus(bool value);
	void insert(String&& str, TextEditorCursor at = {});
	void insert(wchar ch, TextEditorCursor at = {});
	void removeOne(TextEditorCursor at = {});
	void deleteOne(TextEditorCursor at = {});
	void remove(TextEditorCursor to, TextEditorCursor from = {});
	void render(double, QWORD) const noexcept;
	void tick();
	[[nodiscard]] TextEditorLine& getCurrentLine() noexcept;
	[[nodiscard]] const TextEditorLine& getCurrentLine() const noexcept;
	[[nodiscard]] TextEditorLine& getLine(unsigned int line) noexcept;
	[[nodiscard]] const TextEditorLine& getLine(unsigned int line) const noexcept;
};
