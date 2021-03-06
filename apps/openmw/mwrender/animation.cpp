#include "animation.hpp"

#include <OgreHardwarePixelBuffer.h>
#include <OgreSkeletonInstance.h>
#include <OgreEntity.h>
#include <OgreBone.h>
#include <OgreSubMesh.h>
#include <OgreSceneManager.h>


namespace MWRender
{

Animation::Animation()
    : mInsert(NULL)
    , mTime(0.0f)
    , mSkipFrame(false)
{
}

Animation::~Animation()
{
    Ogre::SceneManager *sceneMgr = mInsert->getCreator();
    for(size_t i = 0;i < mEntityList.mEntities.size();i++)
        sceneMgr->destroyEntity(mEntityList.mEntities[i]);
    mEntityList.mEntities.clear();
}


struct checklow {
    bool operator()(const char &a, const char &b) const
    {
        return ::tolower(a) == ::tolower(b);
    }
};

bool Animation::findGroupTimes(const std::string &groupname, Animation::GroupTimes *times)
{
    const std::string &start = groupname+": start";
    const std::string &startloop = groupname+": loop start";
    const std::string &stop = groupname+": stop";
    const std::string &stoploop = groupname+": loop stop";

    NifOgre::TextKeyMap::const_iterator iter;
    for(iter = mTextKeys.begin();iter != mTextKeys.end();iter++)
    {
        if(times->mStart >= 0.0f && times->mLoopStart >= 0.0f && times->mLoopStop >= 0.0f && times->mStop >= 0.0f)
            return true;

        std::string::const_iterator strpos = iter->second.begin();
        std::string::const_iterator strend = iter->second.end();
        size_t strlen = strend-strpos;

        if(start.size() <= strlen && std::mismatch(strpos, strend, start.begin(), checklow()).first == strend)
        {
            times->mStart = iter->first;
            times->mLoopStart = iter->first;
        }
        else if(startloop.size() <= strlen && std::mismatch(strpos, strend, startloop.begin(), checklow()).first == strend)
        {
            times->mLoopStart = iter->first;
        }
        else if(stoploop.size() <= strlen && std::mismatch(strpos, strend, stoploop.begin(), checklow()).first == strend)
        {
            times->mLoopStop = iter->first;
        }
        else if(stop.size() <= strlen && std::mismatch(strpos, strend, stop.begin(), checklow()).first == strend)
        {
            times->mStop = iter->first;
            if(times->mLoopStop < 0.0f)
                times->mLoopStop = iter->first;
            break;
        }
    }

    return (times->mStart >= 0.0f && times->mLoopStart >= 0.0f && times->mLoopStop >= 0.0f && times->mStop >= 0.0f);
}


void Animation::playGroup(std::string groupname, int mode, int loops)
{
    GroupTimes times;
    times.mLoops = loops;

    if(groupname == "all")
    {
        times.mStart = times.mLoopStart = 0.0f;
        times.mLoopStop = times.mStop = 0.0f;

        NifOgre::TextKeyMap::const_reverse_iterator iter = mTextKeys.rbegin();
        if(iter != mTextKeys.rend())
            times.mLoopStop = times.mStop = iter->first;
    }
    else if(!findGroupTimes(groupname, &times))
        throw std::runtime_error("Failed to find animation group "+groupname);

    if(mode == 0 && mCurGroup.mLoops > 0)
        mNextGroup = times;
    else
    {
        mCurGroup = times;
        mNextGroup = GroupTimes();
        mTime = ((mode==2) ? mCurGroup.mLoopStart : mCurGroup.mStart);
    }
}

void Animation::skipAnim()
{
    mSkipFrame = true;
}

void Animation::runAnimation(float timepassed)
{
    if(mCurGroup.mLoops > 0 && !mSkipFrame)
    {
        mTime += timepassed;
        if(mTime >= mCurGroup.mLoopStop)
        {
            if(mCurGroup.mLoops > 1)
            {
                mCurGroup.mLoops--;
                mTime = mTime - mCurGroup.mLoopStop + mCurGroup.mLoopStart;
            }
            else if(mTime >= mCurGroup.mStop)
            {
                if(mNextGroup.mLoops > 0)
                    mTime = mTime - mCurGroup.mStop + mNextGroup.mStart;
                else
                    mTime = mCurGroup.mStop;
                mCurGroup = mNextGroup;
                mNextGroup = GroupTimes();
            }
        }

        if(mEntityList.mSkelBase)
        {
            Ogre::AnimationStateSet *aset = mEntityList.mSkelBase->getAllAnimationStates();
            Ogre::AnimationStateIterator as = aset->getAnimationStateIterator();
            while(as.hasMoreElements())
            {
                Ogre::AnimationState *state = as.getNext();
                state->setTimePosition(mTime);
            }
        }
    }
    mSkipFrame = false;
}

}
