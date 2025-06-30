//
// Created by EmsiaetKadosh on 25-5-11.
//

#include "..\interact\InteractManager.h"
#include "TextEditor.h"


void TextEditor::focus(const bool value) {
	interactManager.setInputMethodEditor(focusing = value);
	Logger.of(L"ime:", value).trace();
}
void TextEditor::insert(String&& str, TextEditorCursor at) {}
void TextEditor::insert(wchar ch, TextEditorCursor at) {}
void TextEditor::removeOne(TextEditorCursor at) {}
void TextEditor::deleteOne(TextEditorCursor at) {}
void TextEditor::remove(TextEditorCursor to, TextEditorCursor from) {}
void TextEditor::render(const double tickDelta, const QWORD tickRendering) const noexcept {}
void TextEditor::tick() {}
TextEditorLine& TextEditor::getCurrentLine() noexcept { return cursor.line < lines.size() ? lines[cursor.line] : lines.back(); }
const TextEditorLine& TextEditor::getCurrentLine() const noexcept { return cursor.line < lines.size() ? lines[cursor.line] : lines.back(); }
TextEditorLine& TextEditor::getLine(const unsigned int line) noexcept { return line == static_cast<unsigned int>(-1) ? getCurrentLine() : line < lines.size() ? lines[line] : lines.back(); }
const TextEditorLine& TextEditor::getLine(const unsigned int line) const noexcept { return line == static_cast<unsigned int>(-1) ? getCurrentLine() : line < lines.size() ? lines[line] : lines.back(); }
