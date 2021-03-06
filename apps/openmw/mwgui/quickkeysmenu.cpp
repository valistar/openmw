#include "quickkeysmenu.hpp"

#include <boost/lexical_cast.hpp>

#include "../mwbase/environment.hpp"
#include "../mwbase/world.hpp"
#include "../mwworld/player.hpp"
#include "../mwworld/inventorystore.hpp"
#include "../mwworld/actionequip.hpp"
#include "../mwmechanics/spells.hpp"
#include "../mwmechanics/creaturestats.hpp"
#include "../mwmechanics/spellsuccess.hpp"
#include "../mwgui/inventorywindow.hpp"
#include "../mwgui/bookwindow.hpp"
#include "../mwgui/scrollwindow.hpp"

#include "windowmanagerimp.hpp"
#include "itemselection.hpp"


namespace
{
    bool sortItems(const MWWorld::Ptr& left, const MWWorld::Ptr& right)
    {
        int cmp = MWWorld::Class::get(left).getName(left).compare(
                    MWWorld::Class::get(right).getName(right));
        return cmp < 0;
    }

    bool sortSpells(const std::string& left, const std::string& right)
    {
        const ESM::Spell* a = MWBase::Environment::get().getWorld()->getStore().spells.find(left);
        const ESM::Spell* b = MWBase::Environment::get().getWorld()->getStore().spells.find(right);

        int cmp = a->mName.compare(b->mName);
        return cmp < 0;
    }
}

namespace MWGui
{

    QuickKeysMenu::QuickKeysMenu(MWBase::WindowManager& parWindowManager)
        : WindowBase("openmw_quickkeys_menu.layout", parWindowManager)
        , mAssignDialog(0)
        , mItemSelectionDialog(0)
        , mMagicSelectionDialog(0)
    {
        getWidget(mOkButton, "OKButton");
        getWidget(mInstructionLabel, "InstructionLabel");

        mMainWidget->setSize(mMainWidget->getWidth(),
                             mMainWidget->getHeight() + (mInstructionLabel->getTextSize().height - mInstructionLabel->getHeight()));

        mOkButton->eventMouseButtonClick += MyGUI::newDelegate(this, &QuickKeysMenu::onOkButtonClicked);
        center();


        for (int i = 0; i < 10; ++i)
        {
            MyGUI::Button* button;
            getWidget(button, "QuickKey" + boost::lexical_cast<std::string>(i+1));

            button->eventMouseButtonClick += MyGUI::newDelegate(this, &QuickKeysMenu::onQuickKeyButtonClicked);

            unassign(button, i);

            mQuickKeyButtons.push_back(button);
        }
    }

    QuickKeysMenu::~QuickKeysMenu()
    {
        delete mAssignDialog;
        delete mItemSelectionDialog;
        delete mMagicSelectionDialog;
    }

    void QuickKeysMenu::unassign(MyGUI::Widget* key, int index)
    {
        while (key->getChildCount ())
            MyGUI::Gui::getInstance ().destroyWidget (key->getChildAt(0));

        key->setUserData(Type_Unassigned);

        MyGUI::TextBox* textBox = key->createWidgetReal<MyGUI::TextBox>("SandText", MyGUI::FloatCoord(0,0,1,1), MyGUI::Align::Default);
        textBox->setTextAlign (MyGUI::Align::Center);
        textBox->setCaption (boost::lexical_cast<std::string>(index+1));
        textBox->setNeedMouseFocus (false);
    }

    void QuickKeysMenu::onQuickKeyButtonClicked(MyGUI::Widget* sender)
    {
        int index = -1;
        for (int i = 0; i < 10; ++i)
        {
            if (sender == mQuickKeyButtons[i] || sender->getParent () == mQuickKeyButtons[i])
            {
                index = i;
                break;
            }
        }
        assert(index != -1);
        mSelectedIndex = index;

        {
            // open assign dialog
            if (!mAssignDialog)
                mAssignDialog = new QuickKeysMenuAssign(mWindowManager, this);
            mAssignDialog->setVisible (true);
        }
    }

    void QuickKeysMenu::onOkButtonClicked (MyGUI::Widget *sender)
    {
        mWindowManager.removeGuiMode(GM_QuickKeysMenu);
    }


    void QuickKeysMenu::onItemButtonClicked(MyGUI::Widget* sender)
    {
        if (!mItemSelectionDialog )
        {
            mItemSelectionDialog = new ItemSelectionDialog("#{sQuickMenu6}", ContainerBase::Filter_All, mWindowManager);
            mItemSelectionDialog->eventItemSelected += MyGUI::newDelegate(this, &QuickKeysMenu::onAssignItem);
            mItemSelectionDialog->eventDialogCanceled += MyGUI::newDelegate(this, &QuickKeysMenu::onAssignItemCancel);
        }
        mItemSelectionDialog->setVisible(true);
        mItemSelectionDialog->openContainer(MWBase::Environment::get().getWorld()->getPlayer().getPlayer());
        mItemSelectionDialog->drawItems ();

        mAssignDialog->setVisible (false);
    }

    void QuickKeysMenu::onMagicButtonClicked(MyGUI::Widget* sender)
    {
        if (!mMagicSelectionDialog )
        {
            mMagicSelectionDialog = new MagicSelectionDialog(mWindowManager, this);
        }
        mMagicSelectionDialog->setVisible(true);

        mAssignDialog->setVisible (false);
    }

    void QuickKeysMenu::onUnassignButtonClicked(MyGUI::Widget* sender)
    {
        unassign(mQuickKeyButtons[mSelectedIndex], mSelectedIndex);
        mAssignDialog->setVisible (false);
    }

    void QuickKeysMenu::onCancelButtonClicked(MyGUI::Widget* sender)
    {
        mAssignDialog->setVisible (false);
    }

    void QuickKeysMenu::onAssignItem(MWWorld::Ptr item)
    {
        MyGUI::Button* button = mQuickKeyButtons[mSelectedIndex];
        while (button->getChildCount ())
            MyGUI::Gui::getInstance ().destroyWidget (button->getChildAt(0));

        button->setUserData(Type_Item);

        MyGUI::ImageBox* frame = button->createWidget<MyGUI::ImageBox>("ImageBox", MyGUI::IntCoord(9, 8, 42, 42), MyGUI::Align::Default);
        std::string backgroundTex = "textures\\menu_icon_barter.dds";
        frame->setImageTexture (backgroundTex);
        frame->setImageCoord (MyGUI::IntCoord(4, 4, 40, 40));
        frame->setUserString ("ToolTipType", "ItemPtr");
        frame->setUserData(item);
        frame->eventMouseButtonClick += MyGUI::newDelegate(this, &QuickKeysMenu::onQuickKeyButtonClicked);


        MyGUI::ImageBox* image = frame->createWidget<MyGUI::ImageBox>("ImageBox", MyGUI::IntCoord(5, 5, 32, 32), MyGUI::Align::Default);
        std::string path = std::string("icons\\");
        path += MWWorld::Class::get(item).getInventoryIcon(item);
        int pos = path.rfind(".");
        path.erase(pos);
        path.append(".dds");
        image->setImageTexture (path);
        image->setNeedMouseFocus (false);

        mItemSelectionDialog->setVisible(false);
    }

    void QuickKeysMenu::onAssignItemCancel()
    {
        mItemSelectionDialog->setVisible(false);
    }

    void QuickKeysMenu::onAssignMagicItem (MWWorld::Ptr item)
    {
        MyGUI::Button* button = mQuickKeyButtons[mSelectedIndex];
        while (button->getChildCount ())
            MyGUI::Gui::getInstance ().destroyWidget (button->getChildAt(0));

        button->setUserData(Type_MagicItem);

        MyGUI::ImageBox* frame = button->createWidget<MyGUI::ImageBox>("ImageBox", MyGUI::IntCoord(9, 8, 42, 42), MyGUI::Align::Default);
        std::string backgroundTex = "textures\\menu_icon_select_magic_magic.dds";
        frame->setImageTexture (backgroundTex);
        frame->setImageCoord (MyGUI::IntCoord(2, 2, 40, 40));
        frame->setUserString ("ToolTipType", "ItemPtr");
        frame->setUserData(item);
        frame->eventMouseButtonClick += MyGUI::newDelegate(this, &QuickKeysMenu::onQuickKeyButtonClicked);

        MyGUI::ImageBox* image = frame->createWidget<MyGUI::ImageBox>("ImageBox", MyGUI::IntCoord(5, 5, 32, 32), MyGUI::Align::Default);
        std::string path = std::string("icons\\");
        path += MWWorld::Class::get(item).getInventoryIcon(item);
        int pos = path.rfind(".");
        path.erase(pos);
        path.append(".dds");
        image->setImageTexture (path);
        image->setNeedMouseFocus (false);

        mMagicSelectionDialog->setVisible(false);
    }

    void QuickKeysMenu::onAssignMagic (const std::string& spellId)
    {
        MyGUI::Button* button = mQuickKeyButtons[mSelectedIndex];
        while (button->getChildCount ())
            MyGUI::Gui::getInstance ().destroyWidget (button->getChildAt(0));

        button->setUserData(Type_Magic);

        MyGUI::ImageBox* frame = button->createWidget<MyGUI::ImageBox>("ImageBox", MyGUI::IntCoord(9, 8, 42, 42), MyGUI::Align::Default);
        std::string backgroundTex = "textures\\menu_icon_select_magic.dds";
        frame->setImageTexture (backgroundTex);
        frame->setImageCoord (MyGUI::IntCoord(2, 2, 40, 40));
        frame->setUserString ("ToolTipType", "Spell");
        frame->setUserString ("Spell", spellId);
        frame->eventMouseButtonClick += MyGUI::newDelegate(this, &QuickKeysMenu::onQuickKeyButtonClicked);

        MyGUI::ImageBox* image = frame->createWidget<MyGUI::ImageBox>("ImageBox", MyGUI::IntCoord(5, 5, 32, 32), MyGUI::Align::Default);

        // use the icon of the first effect
        const ESM::Spell* spell = MWBase::Environment::get().getWorld()->getStore().spells.find(spellId);
        const ESM::MagicEffect* effect = MWBase::Environment::get().getWorld()->getStore().magicEffects.find(spell->mEffects.mList.front().mEffectID);
        std::string path = effect->mIcon;
        int slashPos = path.find("\\");
        path.insert(slashPos+1, "b_");
        path = std::string("icons\\") + path;
        int pos = path.rfind(".");
        path.erase(pos);
        path.append(".dds");

        image->setImageTexture (path);
        image->setNeedMouseFocus (false);

        mMagicSelectionDialog->setVisible(false);
    }

    void QuickKeysMenu::onAssignMagicCancel ()
    {
        mMagicSelectionDialog->setVisible(false);
    }

    void QuickKeysMenu::activateQuickKey(int index)
    {
        MyGUI::Button* button = mQuickKeyButtons[index-1];

        QuickKeyType type = *button->getUserData<QuickKeyType>();

        MWWorld::Ptr player = MWBase::Environment::get().getWorld()->getPlayer().getPlayer();
        MWWorld::InventoryStore& store = MWWorld::Class::get(player).getInventoryStore(player);
        MWMechanics::CreatureStats& stats = MWWorld::Class::get(player).getCreatureStats(player);
        MWMechanics::Spells& spells = stats.getSpells();

        if (type == Type_Magic)
        {
            std::string spellId = button->getChildAt(0)->getUserString("Spell");
            spells.setSelectedSpell(spellId);
            store.setSelectedEnchantItem(store.end());
            mWindowManager.setSelectedSpell(spellId, int(MWMechanics::getSpellSuccessChance(spellId, player)));
        }
        else if (type == Type_Item)
        {
            MWWorld::Ptr item = *button->getChildAt (0)->getUserData<MWWorld::Ptr>();

            // make sure the item is available
            if (item.getRefData ().getCount() == 0)
            {
                MWBase::Environment::get().getWindowManager ()->messageBox (
                            "#{sQuickMenu5} " + MWWorld::Class::get(item).getName(item), std::vector<std::string>());
                return;
            }

            boost::shared_ptr<MWWorld::Action> action = MWWorld::Class::get(item).use(item);

            action->execute (MWBase::Environment::get().getWorld()->getPlayer().getPlayer());

            // this is necessary for books/scrolls: if they are already in the player's inventory,
            // the "Take" button should not be visible.
            // NOTE: the take button is "reset" when the window opens, so we can safely do the following
            // without screwing up future book windows
            mWindowManager.getBookWindow()->setTakeButtonShow(false);
            mWindowManager.getScrollWindow()->setTakeButtonShow(false);

            // since we changed equipping status, update the inventory window
            mWindowManager.getInventoryWindow()->drawItems();
        }
        else if (type == Type_MagicItem)
        {
            MWWorld::Ptr item = *button->getChildAt (0)->getUserData<MWWorld::Ptr>();

            // make sure the item is available
            if (item.getRefData ().getCount() == 0)
            {
                MWBase::Environment::get().getWindowManager ()->messageBox (
                            "#{sQuickMenu5} " + MWWorld::Class::get(item).getName(item), std::vector<std::string>());
                return;
            }

            // retrieve ContainerStoreIterator to the item
            MWWorld::ContainerStoreIterator it = store.begin();
            for (; it != store.end(); ++it)
            {
                if (*it == item)
                {
                    break;
                }
            }
            assert(it != store.end());

            // equip, if it can be equipped
            if (!MWWorld::Class::get(item).getEquipmentSlots(item).first.empty())
            {
                // Note: can't use Class::use here because enchanted scrolls for example would then open the scroll window instead of equipping

                MWWorld::ActionEquip action(item);
                action.execute (MWBase::Environment::get().getWorld ()->getPlayer ().getPlayer ());

                // since we changed equipping status, update the inventory window
                mWindowManager.getInventoryWindow()->drawItems();
            }

            store.setSelectedEnchantItem(it);
            spells.setSelectedSpell("");
            mWindowManager.setSelectedEnchantItem(item, 100); /// \todo track charge %
        }
    }

    // ---------------------------------------------------------------------------------------------------------

    QuickKeysMenuAssign::QuickKeysMenuAssign (MWBase::WindowManager &parWindowManager, QuickKeysMenu* parent)
        : WindowModal("openmw_quickkeys_menu_assign.layout", parWindowManager)
        , mParent(parent)
    {
        getWidget(mLabel, "Label");
        getWidget(mItemButton, "ItemButton");
        getWidget(mMagicButton, "MagicButton");
        getWidget(mUnassignButton, "UnassignButton");
        getWidget(mCancelButton, "CancelButton");

        mItemButton->eventMouseButtonClick += MyGUI::newDelegate(mParent, &QuickKeysMenu::onItemButtonClicked);
        mMagicButton->eventMouseButtonClick += MyGUI::newDelegate(mParent, &QuickKeysMenu::onMagicButtonClicked);
        mUnassignButton->eventMouseButtonClick += MyGUI::newDelegate(mParent, &QuickKeysMenu::onUnassignButtonClicked);
        mCancelButton->eventMouseButtonClick += MyGUI::newDelegate(mParent, &QuickKeysMenu::onCancelButtonClicked);


        int maxWidth = mItemButton->getTextSize ().width + 24;
        maxWidth = std::max(maxWidth, mMagicButton->getTextSize ().width + 24);
        maxWidth = std::max(maxWidth, mUnassignButton->getTextSize ().width + 24);
        maxWidth = std::max(maxWidth, mCancelButton->getTextSize ().width + 24);

        mMainWidget->setSize(maxWidth + 24, mMainWidget->getHeight());
        mLabel->setSize(maxWidth, mLabel->getHeight());

        mItemButton->setCoord((maxWidth - mItemButton->getTextSize().width-24)/2 + 8,
                              mItemButton->getTop(),
                              mItemButton->getTextSize().width + 24,
                              mItemButton->getHeight());
        mMagicButton->setCoord((maxWidth - mMagicButton->getTextSize().width-24)/2 + 8,
                              mMagicButton->getTop(),
                              mMagicButton->getTextSize().width + 24,
                              mMagicButton->getHeight());
        mUnassignButton->setCoord((maxWidth - mUnassignButton->getTextSize().width-24)/2 + 8,
                              mUnassignButton->getTop(),
                              mUnassignButton->getTextSize().width + 24,
                              mUnassignButton->getHeight());
        mCancelButton->setCoord((maxWidth - mCancelButton->getTextSize().width-24)/2 + 8,
                              mCancelButton->getTop(),
                              mCancelButton->getTextSize().width + 24,
                              mCancelButton->getHeight());

        center();
    }


    // ---------------------------------------------------------------------------------------------------------

    MagicSelectionDialog::MagicSelectionDialog(MWBase::WindowManager &parWindowManager, QuickKeysMenu* parent)
        : WindowModal("openmw_magicselection_dialog.layout", parWindowManager)
        , mParent(parent)
        , mWidth(0)
        , mHeight(0)
    {
        getWidget(mCancelButton, "CancelButton");
        getWidget(mMagicList, "MagicList");
        mCancelButton->eventMouseButtonClick += MyGUI::newDelegate(this, &MagicSelectionDialog::onCancelButtonClicked);

        center();
    }

    void MagicSelectionDialog::onCancelButtonClicked (MyGUI::Widget *sender)
    {
        mParent->onAssignMagicCancel ();
    }

    void MagicSelectionDialog::open ()
    {
        WindowModal::open();

        while (mMagicList->getChildCount ())
            MyGUI::Gui::getInstance ().destroyWidget (mMagicList->getChildAt (0));

        mHeight = 0;

        const int spellHeight = 18;

        MWWorld::Ptr player = MWBase::Environment::get().getWorld()->getPlayer().getPlayer();
        MWWorld::InventoryStore& store = MWWorld::Class::get(player).getInventoryStore(player);
        MWMechanics::CreatureStats& stats = MWWorld::Class::get(player).getCreatureStats(player);
        MWMechanics::Spells& spells = stats.getSpells();

        /// \todo lots of copy&pasted code from SpellWindow

        // retrieve powers & spells, sort by name
        std::vector<std::string> spellList;

        for (MWMechanics::Spells::TIterator it = spells.begin(); it != spells.end(); ++it)
        {
            spellList.push_back(*it);
        }

        std::vector<std::string> powers;
        std::vector<std::string>::iterator it = spellList.begin();
        while (it != spellList.end())
        {
            const ESM::Spell* spell = MWBase::Environment::get().getWorld()->getStore().spells.find(*it);
            if (spell->mData.mType == ESM::Spell::ST_Power)
            {
                powers.push_back(*it);
                it = spellList.erase(it);
            }
            else if (spell->mData.mType == ESM::Spell::ST_Ability
                || spell->mData.mType == ESM::Spell::ST_Blight
                || spell->mData.mType == ESM::Spell::ST_Curse
                || spell->mData.mType == ESM::Spell::ST_Disease)
            {
                it = spellList.erase(it);
            }
            else
                ++it;
        }
        std::sort(powers.begin(), powers.end(), sortSpells);
        std::sort(spellList.begin(), spellList.end(), sortSpells);

        // retrieve usable magic items & sort
        std::vector<MWWorld::Ptr> items;
        for (MWWorld::ContainerStoreIterator it(store.begin()); it != store.end(); ++it)
        {
            std::string enchantId = MWWorld::Class::get(*it).getEnchantment(*it);
            if (enchantId != "")
            {
                // only add items with "Cast once" or "Cast on use"
                const ESM::Enchantment* enchant = MWBase::Environment::get().getWorld()->getStore().enchants.find(enchantId);
                int type = enchant->mData.mType;
                if (type != ESM::Enchantment::CastOnce
                    && type != ESM::Enchantment::WhenUsed)
                    continue;

                items.push_back(*it);
            }
        }
        std::sort(items.begin(), items.end(), sortItems);


        int height = estimateHeight(items.size() + powers.size() + spellList.size());
        bool scrollVisible = height > mMagicList->getHeight();
        mWidth = mMagicList->getWidth() - scrollVisible * 18;


        // powers
        addGroup("#{sPowers}", "");

        for (std::vector<std::string>::const_iterator it = powers.begin(); it != powers.end(); ++it)
        {
            const ESM::Spell* spell = MWBase::Environment::get().getWorld()->getStore().spells.find(*it);
            MyGUI::Button* t = mMagicList->createWidget<MyGUI::Button>("SpellText",
                MyGUI::IntCoord(4, mHeight, mWidth-8, spellHeight), MyGUI::Align::Left | MyGUI::Align::Top);
            t->setCaption(spell->mName);
            t->setTextAlign(MyGUI::Align::Left);
            t->setUserString("ToolTipType", "Spell");
            t->setUserString("Spell", *it);
            t->eventMouseWheel += MyGUI::newDelegate(this, &MagicSelectionDialog::onMouseWheel);
            t->eventMouseButtonClick += MyGUI::newDelegate(this, &MagicSelectionDialog::onSpellSelected);

            mHeight += spellHeight;
        }

        // other spells
        addGroup("#{sSpells}", "");
        for (std::vector<std::string>::const_iterator it = spellList.begin(); it != spellList.end(); ++it)
        {
            const ESM::Spell* spell = MWBase::Environment::get().getWorld()->getStore().spells.find(*it);
            MyGUI::Button* t = mMagicList->createWidget<MyGUI::Button>("SpellText",
                MyGUI::IntCoord(4, mHeight, mWidth-8, spellHeight), MyGUI::Align::Left | MyGUI::Align::Top);
            t->setCaption(spell->mName);
            t->setTextAlign(MyGUI::Align::Left);
            t->setUserString("ToolTipType", "Spell");
            t->setUserString("Spell", *it);
            t->eventMouseWheel += MyGUI::newDelegate(this, &MagicSelectionDialog::onMouseWheel);
            t->eventMouseButtonClick += MyGUI::newDelegate(this, &MagicSelectionDialog::onSpellSelected);

            mHeight += spellHeight;
        }


        // enchanted items
        addGroup("#{sMagicItem}", "");

        for (std::vector<MWWorld::Ptr>::const_iterator it = items.begin(); it != items.end(); ++it)
        {
            MWWorld::Ptr item = *it;

            // check if the item is currently equipped (will display in a different color)
            bool equipped = false;
            for (int i=0; i < MWWorld::InventoryStore::Slots; ++i)
            {
                if (store.getSlot(i) != store.end() && *store.getSlot(i) == item)
                {
                    equipped = true;
                    break;
                }
            }

            MyGUI::Button* t = mMagicList->createWidget<MyGUI::Button>(equipped ? "SpellText" : "SpellTextUnequipped",
                MyGUI::IntCoord(4, mHeight, mWidth-8, spellHeight), MyGUI::Align::Left | MyGUI::Align::Top);
            t->setCaption(MWWorld::Class::get(item).getName(item));
            t->setTextAlign(MyGUI::Align::Left);
            t->setUserData(item);
            t->setUserString("ToolTipType", "ItemPtr");
            t->eventMouseButtonClick += MyGUI::newDelegate(this, &MagicSelectionDialog::onEnchantedItemSelected);
            t->eventMouseWheel += MyGUI::newDelegate(this, &MagicSelectionDialog::onMouseWheel);

            mHeight += spellHeight;
        }


        mMagicList->setCanvasSize (mWidth, std::max(mMagicList->getHeight(), mHeight));

    }

    void MagicSelectionDialog::addGroup(const std::string &label, const std::string& label2)
    {
        if (mMagicList->getChildCount() > 0)
        {
            MyGUI::ImageBox* separator = mMagicList->createWidget<MyGUI::ImageBox>("MW_HLine",
                MyGUI::IntCoord(4, mHeight, mWidth-8, 18),
                MyGUI::Align::Left | MyGUI::Align::Top);
            separator->setNeedMouseFocus(false);
            mHeight += 18;
        }

        MyGUI::TextBox* groupWidget = mMagicList->createWidget<MyGUI::TextBox>("SandBrightText",
            MyGUI::IntCoord(0, mHeight, mWidth, 24),
            MyGUI::Align::Left | MyGUI::Align::Top | MyGUI::Align::HStretch);
        groupWidget->setCaptionWithReplacing(label);
        groupWidget->setTextAlign(MyGUI::Align::Left);
        groupWidget->setNeedMouseFocus(false);

        if (label2 != "")
        {
            MyGUI::TextBox* groupWidget2 = mMagicList->createWidget<MyGUI::TextBox>("SandBrightText",
                MyGUI::IntCoord(0, mHeight, mWidth-4, 24),
                MyGUI::Align::Left | MyGUI::Align::Top);
            groupWidget2->setCaptionWithReplacing(label2);
            groupWidget2->setTextAlign(MyGUI::Align::Right);
            groupWidget2->setNeedMouseFocus(false);
        }

        mHeight += 24;
    }


    void MagicSelectionDialog::onMouseWheel(MyGUI::Widget* _sender, int _rel)
    {
        if (mMagicList->getViewOffset().top + _rel*0.3 > 0)
            mMagicList->setViewOffset(MyGUI::IntPoint(0, 0));
        else
            mMagicList->setViewOffset(MyGUI::IntPoint(0, mMagicList->getViewOffset().top + _rel*0.3));
    }

    void MagicSelectionDialog::onEnchantedItemSelected(MyGUI::Widget* _sender)
    {
        MWWorld::Ptr player = MWBase::Environment::get().getWorld()->getPlayer().getPlayer();
        MWWorld::Ptr item = *_sender->getUserData<MWWorld::Ptr>();

        mParent->onAssignMagicItem (item);
    }

    void MagicSelectionDialog::onSpellSelected(MyGUI::Widget* _sender)
    {
        mParent->onAssignMagic (_sender->getUserString("Spell"));
    }

    int MagicSelectionDialog::estimateHeight(int numSpells) const
    {
        int height = 0;
        height += 24 * 3 + 18 * 2; // group headings
        height += numSpells * 18;
        return height;
    }

}
