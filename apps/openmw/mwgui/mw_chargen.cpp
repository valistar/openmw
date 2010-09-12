#include "mw_chargen.hpp"

#include <assert.h>
#include <iostream>
#include <iterator>

using namespace MWGui;

RaceDialog::RaceDialog()
  : Layout("openmw_chargen_race_layout.xml")
  , sexIndex(0)
  , faceIndex(0)
  , hairIndex(0)
  , faceCount(10)
  , hairCount(14)
{
	mMainWidget->setCoord(mMainWidget->getCoord() + MyGUI::IntPoint(0, 100));

	// These are just demo values, you should replace these with
	// real calls from outside the class later.

	setText("AppearanceT", "Appearance");
	getWidget(appearanceBox, "AppearanceBox");

	getWidget(headRotate, "HeadRotate");
	headRotate->setScrollRange(50);
	headRotate->setScrollPosition(20);
	headRotate->setScrollViewPage(10);
	headRotate->eventScrollChangePosition = MyGUI::newDelegate(this, &RaceDialog::onHeadRotate);

	// Set up next/previous buttons
	MyGUI::ButtonPtr prevButton, nextButton;

	getWidget(prevButton, "PrevSexButton");
	getWidget(nextButton, "NextSexButton");
	prevButton->eventMouseButtonClick = MyGUI::newDelegate(this, &RaceDialog::onSelectPreviousSex);
	nextButton->eventMouseButtonClick = MyGUI::newDelegate(this, &RaceDialog::onSelectNextSex);

	getWidget(prevButton, "PrevFaceButton");
	getWidget(nextButton, "NextFaceButton");
	prevButton->eventMouseButtonClick = MyGUI::newDelegate(this, &RaceDialog::onSelectPreviousFace);
	nextButton->eventMouseButtonClick = MyGUI::newDelegate(this, &RaceDialog::onSelectNextFace);

	getWidget(prevButton, "PrevHairButton");
	getWidget(nextButton, "NextHairButton");
	prevButton->eventMouseButtonClick = MyGUI::newDelegate(this, &RaceDialog::onSelectPreviousHair);
	nextButton->eventMouseButtonClick = MyGUI::newDelegate(this, &RaceDialog::onSelectNextHair);

	setText("RaceT", "Race");
	getWidget(raceList, "RaceList");
	raceList->setScrollVisible(true);
	raceList->eventListSelectAccept = MyGUI::newDelegate(this, &RaceDialog::onSelectRace);
	raceList->eventListMouseItemActivate = MyGUI::newDelegate(this, &RaceDialog::onSelectRace);

	getWidget(skillList, "SkillList");
	getWidget(specialsList, "SpecialsList");

	updateRaces();
	updateSkills();
	updateSpecials();
}

int wrap(int index, int max)
{
	if (index < 0)
		return max - 1;
	else if (index >= max)
		return 0;
	else
		return index;
}

void RaceDialog::selectPreviousSex()
{
	sexIndex = wrap(sexIndex - 1, 2);
}

void RaceDialog::selectNextSex()
{
	sexIndex = wrap(sexIndex + 1, 2);
}

void RaceDialog::selectPreviousFace()
{
	faceIndex = wrap(faceIndex - 1, faceCount);
}

void RaceDialog::selectNextFace()
{
	faceIndex = wrap(faceIndex + 1, faceCount);
}

void RaceDialog::selectPreviousHair()
{
	hairIndex = wrap(hairIndex - 1, hairCount);
}

void RaceDialog::selectNextHair()
{
	hairIndex = wrap(hairIndex - 1, hairCount);
}

// widget controls

void RaceDialog::onHeadRotate(MyGUI::VScroll*, size_t _position)
{
	// TODO: Rotate head
}

void RaceDialog::onSelectPreviousSex(MyGUI::Widget*)
{
	selectPreviousSex();
}

void RaceDialog::onSelectNextSex(MyGUI::Widget*)
{
	selectNextSex();
}

void RaceDialog::onSelectPreviousFace(MyGUI::Widget*)
{
	selectPreviousFace();
}

void RaceDialog::onSelectNextFace(MyGUI::Widget*)
{
	selectNextFace();
}

void RaceDialog::onSelectPreviousHair(MyGUI::Widget*)
{
	selectPreviousHair();
}

void RaceDialog::onSelectNextHair(MyGUI::Widget*)
{
	selectNextHair();
}

void RaceDialog::onSelectRace(MyGUI::List* _sender, size_t _index)
{
	// TODO: Select actual race
	updateSkills();
	updateSpecials();
}

// update widget content

void RaceDialog::updateRaces()
{
	raceList->removeAllItems();
	raceList->addItem("Argonian");
	raceList->addItem("Breton");
	raceList->addItem("Dark Elf");
	raceList->addItem("High Elf");
	raceList->addItem("Imperial");
	raceList->addItem("Khajiit");
	raceList->addItem("Nord");
	raceList->addItem("Orc");
}

void RaceDialog::updateSkills()
{
	for (std::vector<MyGUI::WidgetPtr>::iterator it = skillItems.begin(); it != skillItems.end(); ++it)
	{
		MyGUI::Gui::getInstance().destroyWidget(*it);
	}
	skillItems.clear();

	MyGUI::StaticTextPtr skillName, skillLevel;
	const int lineHeight = 18;
	MyGUI::IntCoord coord1(0, 0, skillList->getWidth() - (40 + 4), 18);
	MyGUI::IntCoord coord2(coord1.left + coord1.width, 0, 40, 18);

	const char *inputList[][2] = {
		{"Athletics", "5"},
		{"Destruction", "10"},
		{"Light Armor", "5"},
		{"Long Blade", "5"},
		{"Marksman", "5"},
		{"Mysticism", "5"},
		{"Short Blade", "10"},
		{0,0}
	};

	for (int i = 0; inputList[i][0]; ++i)
	{
		skillName = skillList->createWidget<MyGUI::StaticText>("SandText", coord1, MyGUI::Align::Default, "SkillName0");
		skillName->setCaption(inputList[i][0]);
		skillLevel = skillList->createWidget<MyGUI::StaticText>("SandTextRight", coord2, MyGUI::Align::Default, "SkillLevel0");
		skillLevel->setCaption(inputList[i][1]);

		skillItems.push_back(skillName);
		skillItems.push_back(skillLevel);

		coord1.top += lineHeight;
		coord2.top += lineHeight;
	}
}

void RaceDialog::updateSpecials()
{
	for (std::vector<MyGUI::WidgetPtr>::iterator it = specialsItems.begin(); it != specialsItems.end(); ++it)
	{
		MyGUI::Gui::getInstance().destroyWidget(*it);
	}
	specialsItems.clear();

	MyGUI::StaticTextPtr specialsName;
	const int lineHeight = 18;
	MyGUI::IntCoord coord(0, 0, specialsList->getWidth(), 18);

	const char *inputList[] = {
		"Depth Perception",
		"Resist Fire",
		"Ancestor Guardian",
		0
	};

	for (int i = 0; inputList[i]; ++i)
	{
		std::string name = "specialsName";
		name += i;
		specialsName = specialsList->createWidget<MyGUI::StaticText>("SandText", coord, MyGUI::Align::Default, name);
		specialsName->setCaption(inputList[i]);

		specialsItems.push_back(specialsName);

		coord.top += lineHeight;
	}
}