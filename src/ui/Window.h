//
// Created by EmsiaetKadosh on 25-1-14.
//

#pragma once

#include "..\def.h"
#include "..\utils\gc.h"
#include "..\render\Renderer.h"
#include "..\game\Animation.h"
#include "..\utils\IText.h"

class WindowManager;

enum class MouseActionCode : unsigned int {
	MAC_MOVE = 0,
	MAC_HOVER = 1,
	MAC_DOWN = 2,
	MAC_UP = 3,
	MAC_DOUBLE = 4,
	MAC_LEAVE = 5
};

enum class MouseButtonCodeEnum : unsigned int {
	MBC_L_DOWN = 0x1,
	MBC_R_DOWN = 0x2,
	MBC_M_DOWN = 0x4,
	MBC_L_CHANGE = 0x10,
	MBC_R_CHANGE = 0x20,
	MBC_M_CHANGE = 0x40,
};

/**
 * @see MouseButtonCodeEnum
 */
using MouseButtonCode = unsigned int;

class Widget : public IRenderable, public ITickable {
protected:
	int left = 0, top = 0, width = 0, height = 0;

public:
	using Action = Function<void(Widget&, MouseButtonCode)>;
	double x, y, w, h;
	Action mouseHover; // 传入int为：0，鼠标移动；其余，鼠标在其上的长时间悬浮
	Action mouseDown; // 传入int表示变更按键。0左, 1中, 2右
	Action mouseUp; // 传入int表示变更按键。0左, 1中, 2右
	Action mouseLeave; // 传入int忽略
	Action mouseClick; // 传入int表示变更按键。0x0左, 0x1中, 0x2右；0x8表示是否双击
	Action onTick; // 传入int忽略
	Color backgroundColor;
	Color foregroundColor{ TextColor };

protected:
	mutable bool hasMouse = false;
	mutable bool isActive = true;
	mutable bool hasMouseTrigger = false;
	bool isAbsoluteLocation = false;

public:
	UILocation location;
	UILocation textLocation = UILocation::CENTER; // 多余字节预声明备用
	char unused[2]{}; // [0]: [1]: maxRestore_flag_IsZoomed

	Widget(const double x, const double y, const double w, const double h, const UILocation location) : x(x), y(y), w(w), h(h), location(location) {}

	unsigned int colorSelector(const Color& clr) const;
	void render(double tickDelta, QWORD tickRendering) const noexcept override;

	Widget& alignLocation(const UILocation loc) noexcept {
		location = loc;
		return *this;
	}

	Widget& alignTextLocation(const UILocation loc) noexcept {
		textLocation = loc;
		return *this;
	}

	Widget& absolute(const bool value = true) {
		if (value == isAbsoluteLocation) return *this;
		isAbsoluteLocation = value;
		if (renderer.getWidth() != 0 && renderer.getHeight() != 0) onResize();
		return *this;
	}

	bool containsMouse() const noexcept { return hasMouse; }

	virtual bool isMouseIn(int x, int y) noexcept {
		x -= left;
		y -= top;
		return 0 <= x and x <= width and 0 <= y and y <= height;
	}

	virtual void onResize();
	virtual void onHover(const int value) noexcept { if (mouseHover) mouseHover(*this, value); }

	virtual void onMouseDown(const MouseButtonCode code) noexcept {
		hasMouseTrigger = true;
		if (mouseDown) mouseDown(*this, code);
	}

	virtual void onMouseUp(const MouseButtonCode code) noexcept {
		if (mouseUp) mouseUp(*this, code);
		if (hasMouseTrigger) onMouseClick(code);
		hasMouseTrigger = false;
	}

	virtual void onMouseLeave(const MouseButtonCode value) noexcept {
		hasMouseTrigger = false;
		if (mouseLeave) mouseLeave(*this, value);
	}

	virtual void onMouseClick(const MouseButtonCode value) noexcept { if (mouseClick) mouseClick(*this, value); }
	void tick() noexcept override { if (onTick) onTick(*this, 0); }

	virtual int passEvent(const MouseActionCode action, const MouseButtonCode value, const int x, const int y) noexcept {
		if (action == MouseActionCode::MAC_LEAVE || !isMouseIn(x, y)) {
			if (hasMouse) onMouseLeave(0);
			hasMouse = false;
			return 0;
		}
		hasMouse = true;
		switch (action) {
			case MouseActionCode::MAC_HOVER:
				onHover(1);
				break;
			case MouseActionCode::MAC_MOVE:
				onHover(0);
				break;
			case MouseActionCode::MAC_DOWN:
				onMouseDown(value);
				break;
			case MouseActionCode::MAC_UP:
				onMouseUp(value);
				break;
			case MouseActionCode::MAC_DOUBLE:
				onMouseClick(0x80);
				break;
			default:
				break;
		}
		return 1;
	}
};

class Window : public AnywhereEditable<Window, WindowManager>, public IRenderable, public ITickable {
protected:
	friend class Garbage<Window>;
	friend class AnywhereEditableList<Window, WindowManager>;
	List<ObjectHolder<Widget>> widgets;

	Window() = default;

	~Window() override = default;

public:
	int pop() noexcept override;
	void render(double tickDelta, QWORD tickRendering) const noexcept override;
	void tick() noexcept override;
	/**
	 * 在Game.setWindow()时，本窗口开启时调用。
	 * 不应当外部调用。
	 * 如果返回false，则拒绝设置窗口。
	 * @return 是否允许将显示窗口设为自身
	 */
	virtual bool onOpen() { return true; }
	/**
	 * 由于键盘响应是非同步的，为了防止一些线程间的冲突，此方法由MainThread调用，在GameThread实施。
	 */
	virtual void syncClose(const bool value = true) { reserved[0] = value; }
	/**
	 * 在Game.setWindow()时，本窗口关闭时调用。
	 * 不应当外部调用。
	 * 注意，关闭未必就是删除。
	 */
	virtual void onClose() = 0;
	virtual void onResize();
	virtual int passEvent(MouseActionCode action, MouseButtonCode value, int x, int y) noexcept;
};

class WindowManager final : public AnywhereEditableList<Window, WindowManager>, public IRenderable, public ITickable {
public:
	void render(const double tickDelta, QWORD tickRendering) const noexcept override { for (const Window& i : *this) i.render(tickDelta, tickRendering); }
	void tick() noexcept override { for (Window& i : *this) i.tick(); }
	int pop(Window* value) noexcept override;
	void clear() noexcept;
	void onResize() noexcept { for (Window& i : *this) i.onResize(); }
};

class CaptionWindow final : public Window {
public:
	CaptionWindow();
	bool onOpen() override;
	void onClose() override;
	void render(double tickDelta, QWORD tickRendering) const noexcept override;
	void onResize() override;
};

class FloatWindow final : public Window {
	mutable int x = 0; // 标记渲染起始点
	mutable int y = 0; // 标记渲染起始点
	typedef List<ObjectHolder<RenderableString>> lt;
	SynchronizedHolder<lt> strings;

public:
	FloatWindow() : Window() {
		strings.setNew<lt>(std::move(lt()));
		strings.ok();
		strings.async();
	}

	void clear() { strings.setNew<lt>(std::move(lt())); }

	void push(const ObjectHolder<RenderableString>& string) const { if (strings.ptrNew()) strings.getNew().push_back(string); }
	void push(ObjectHolder<RenderableString>&& string) const { if (strings.ptrNew()) strings.getNew().push_back(std::move(string)); }
	void render(double tickDelta, QWORD tickRendering) const noexcept override;
	void tick() noexcept override {}
	bool onOpen() override { return true; }
	void onClose() override {}
	void update() const noexcept { strings.ok(); }
};

class Button : public Widget {
public:
	ObjectHolder<IText> name;
	Animation animation = Animation().features(Animation::AS_CUBIC).setDuration(20);
	Button(const double x, const double y, const double w, const double h, const UILocation location, const ObjectHolder<IText>& text) : Widget(x, y, w, h, location), name(text) {}
	Button(const double x, const double y, const double w, const double h, const UILocation location, ObjectHolder<IText>&& text) : Widget(x, y, w, h, location), name(std::move(text)) {}
	void render(double tickDelta, QWORD tickRendering) const noexcept override;
};


class ConfirmWindow : public Window {
public:
	ObjectHolder<IText> text;
	Button
		*confirm = nullptr,
		*cancel = nullptr;

private:
	ConfirmWindow(const ObjectHolder<IText>& text) : Window(), text(text) {}
	ConfirmWindow(ObjectHolder<IText>&& text) : Window(), text(std::move(text)) {}

public:
	~ConfirmWindow() override = default; // 不需要delete，析构时Window会自动delete

	void render(const double tickDelta, QWORD tickRendering) const noexcept override {
		int w, h;
		w = renderer.getWidth(), h = renderer.getHeight();
		w >>= 2, h >>= 2;
		renderer.fill(w, h, w + w, h + h, 0xcc222222);
		fontManager.getDefault().drawCenter(text->getRenderableString(), w, h, w + w, h + (h >> 1), 0xffeeeeee);
		for (const ObjectHolder<Widget>& widget : widgets) widget->render(tickDelta, tickRendering);
	}

	ConfirmWindow& requireConfirm(const Function<void(Button&)>& func = {});

	ConfirmWindow& requireCancel(const Function<void(Button&)>& func = {});

	ConfirmWindow&& move() noexcept { return std::move(*this); }

	void onClose() override;

	static ConfirmWindow* of(const ObjectHolder<IText>& text) { return allocatedFor(new ConfirmWindow(text)); }
	static ConfirmWindow* of(ObjectHolder<IText>&& text) { return allocatedFor(new ConfirmWindow(std::move(text))); }
};
