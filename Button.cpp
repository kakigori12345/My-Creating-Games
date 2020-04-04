#include "Button.h"
#include "GameLib/GameLib.h"
#include "GameLib/Input/Manager.h"
#include "GameLib/Input/Keyboard.h"
#include "SoundGenerator.h"
using namespace GameLib::Input;

Button* Button::mInstance = 0;

Button::Button():demoPlay(false) {}

Button::~Button() {}

void Button::create() {
	ASSERT(!mInstance);
	mInstance = new Button();
}

void Button::destroy() {
	ASSERT(mInstance);
	SAFE_DELETE(mInstance);
}

Button* Button::instance() {
	return mInstance;
}

bool Button::isOn(Button::Key key) const {
	bool r = false;
	Keyboard k = Manager::instance().keyboard();
	char c = 0;
	Keyboard::Key in;
	bool flag = false;//“ü—Í2Ží—Þ‚Ì”»•Ê
	switch (key) {
	case KEY_W: c = 'w'; break;
	case KEY_A: c = 'a'; break;
	case KEY_S: c = 's'; break;
	case KEY_D: c = 'd'; break;
	case KEY_O: c = 'o'; break;
	case KEY_P: c = 'p'; break;
	case KEY_R: c = 'r'; break;
	case KEY_SPACE: c = ' '; break;
	case KEY_LEFT: in = Keyboard::Key::KEY_LEFT; flag = true; break;
	case KEY_RIGHT: in = Keyboard::Key::KEY_RIGHT; flag = true; break;
	case KEY_SHIFT: in = Keyboard::Key::KEY_SHIFT; flag = true; break;
	default: ASSERT(false); break;
	}
	if (flag)
		r = k.isOn(in);
	else
		r = k.isOn(c);

	return r;
}

bool Button::isTriggered(Button::Key key) const {
	//Se‚ðŠÇ—‚·‚é‚½‚ß‚Ì•Ï”
	SoundGenerator::Se se = SoundGenerator::Se::SE_INVALID;
	//”»’è—p•Ï”
	bool r = false;
	Keyboard k = Manager::instance().keyboard();
	char c = 0;
	Keyboard::Key in;
	bool flag = false;//“ü—Í2Ží—Þ‚Ì”»•Ê
	switch (key) {
	case KEY_W: c = 'w'; se = SoundGenerator::Se::SELECT_CURSOR_MOVE; break;
	case KEY_A: c = 'a'; se = SoundGenerator::Se::SELECT_CURSOR_MOVE; break;
	case KEY_S: c = 's'; se = SoundGenerator::Se::SELECT_CURSOR_MOVE; break;
	case KEY_D: c = 'd'; se = SoundGenerator::Se::SELECT_CURSOR_MOVE; break;
	case KEY_O: c = 'o'; se = SoundGenerator::Se::SELECT_FINISH; break;
	case KEY_P: c = 'p'; se = SoundGenerator::Se::SELECT_FINISH; break;
	case KEY_R: c = 'r'; se = SoundGenerator::Se::SELECT_FINISH; break;
	case KEY_SPACE: c = ' '; se = SoundGenerator::Se::SELECT_FINISH; break;
	case KEY_LEFT:	in = Keyboard::Key::KEY_LEFT; flag = true; break;
	case KEY_RIGHT: in = Keyboard::Key::KEY_RIGHT; flag = true; break;
	case KEY_SHIFT: in = Keyboard::Key::KEY_SHIFT; se = SoundGenerator::Se::SELECT_FINISH; flag = true; break;
	default: ASSERT(false); break;
	}
	if (flag)
		r = k.isTriggered(in);
	else
		r = k.isTriggered(c);

	if (c == ' ' && demoPlay)
		r = true;

	//Se‚ð–Â‚ç‚·‚©‚Ç‚¤‚©
	if (r) {
		switch (se) {
		case SoundGenerator::Se::SELECT_CURSOR_MOVE:
			SoundGenerator::instance()->playSe(SoundGenerator::Se::SELECT_CURSOR_MOVE);
			break;
		case SoundGenerator::Se::SELECT_FINISH:
			SoundGenerator::instance()->playSe(SoundGenerator::Se::SELECT_FINISH);
			break;
		case SoundGenerator::Se::SE_INVALID:
			break;
		default:
			HALT("File:Button.cpp [isTriggered()] switch's Se Error");
		}
	}

	return r;
}

void Button::thisGameIsDemo()
{
	demoPlay = true;
}

void Button::resetDemo()
{
	demoPlay = false;
}

bool Button::demo() const
{
	return demoPlay;
}