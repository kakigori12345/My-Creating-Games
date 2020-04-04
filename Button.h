#pragma once

//�L�[�{�[�h����̓��͂��󂯕t����N���X
//�Q�[������������݂��Ȃ��̂ŃV���O���g��
class Button {
public:
	enum Key {
		KEY_W,		//[w]
		KEY_A,		//[a]
		KEY_S,		//[s]
		KEY_D,		//[d]
		KEY_O,		//[o]
		KEY_P,		//[p]
		KEY_R,		//[r]
		KEY_LEFT,	//�������L�[
		KEY_RIGHT,	//�E�����L�[
		KEY_SPACE,	//SPACE�L�[
		KEY_SHIFT	//SHIFT�L�[
	};
	static Button* instance();//�g���Ƃ��ɒ��p�Ƃ��ĉ
	static void create();//�g���O�ɂǂ�����1��Ăяo��
	static void destroy();
	bool isOn(Key) const;//�����Ă����true��Ԃ�������
	bool isTriggered(Key) const;//1��̓��͂���true��Ԃ��B���������Ă�true��Ԃ��͉̂�����1�t���[���̂�
	void thisGameIsDemo();	//�f���v���C�ł��邱�Ƃ�`����
	void resetDemo();		//�f���v���C������
	bool demo() const;		//�f���v���C���H
private:
	Button();
	~Button();
	static Button* mInstance;
	mutable bool demoPlay;
};