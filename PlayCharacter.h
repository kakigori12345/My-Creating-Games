#pragma once

#include "Character.h"

class PlayCharacter : public Character {
public:
	PlayCharacter();
	~PlayCharacter();
	void addParts(Parts parts);
	void subParts(Parts parts);
	bool judgeParts(Parts parts);
private:
};