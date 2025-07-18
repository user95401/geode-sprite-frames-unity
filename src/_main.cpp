#include <Geode/Geode.hpp>
using namespace geode::prelude;

#define DEBUG if (getMod()->getSettingValue<bool>("debug"))

#include <Geode/modify/CCSpriteFrameCache.hpp>
class $modify(CCSpriteFrameCacheUnityExt, CCSpriteFrameCache) {
public:
    static std::string getTexturePathFromPlist(const std::string & plistPath) {
        size_t lastDot = plistPath.find_last_of('.');
        if (lastDot != std::string::npos) return plistPath.substr(0, lastDot) + ".png";
        return plistPath + ".png";
    }
    //void addSpriteFramesWithFile(char const*) = imac 0x308940, m1 0x29f998, ios 0x3b5070;
    void addSpriteFramesWithFile(const char* pszPlist) {
        DEBUG log::info("{}({})", __FUNCTION__, pszPlist ? pszPlist : "nil ch");
        CCSpriteFrameCache::addSpriteFramesWithFile(pszPlist);

        std::string name = (pszPlist ? pszPlist : "nil ch");

        // loaded from mod folder?
        auto split = string::split(name, "/");
        DEBUG log::debug("split = {}", split);
        if (split.size() > 1) {
			auto textureNameOfPlist = getTexturePathFromPlist(name);
            auto textureNameToMergeInto = getTexturePathFromPlist(split[1]);
            DEBUG log::debug("textureNameOfPlist = {}", textureNameOfPlist);
            DEBUG log::debug("textureNameToMergeInto = {}", textureNameToMergeInto);
            if (fileExistsInSearchPaths(textureNameToMergeInto.c_str())) {
                auto texCache = CCTextureCache::sharedTextureCache();
                auto textureOfPlist = texCache->addImage(textureNameOfPlist.c_str(), 0);
                auto textureToMergeInto = texCache->addImage(textureNameToMergeInto.c_str(), 0);
                DEBUG log::debug("textureOfPlist = {}", textureOfPlist);
                DEBUG log::debug("textureToMergeInto = {}", textureToMergeInto);
                
                auto canvas = CCNode::create();
                canvas->setID("canvas for " + textureNameToMergeInto);
                DEBUG SceneManager::get()->keepAcrossScenes(canvas);

                static std::unordered_map<std::string, Ref<CCSprite>> CACHED_MAIN_CONTENTS;

                CCSprite* mainContent;
                if (CACHED_MAIN_CONTENTS.contains(textureNameToMergeInto)) {
                    mainContent = CACHED_MAIN_CONTENTS[textureNameToMergeInto];
                }
                else {
                    auto img = new CCImage(); 
                    img->m_bPreMulti = true;
                    img->initWithImageFile(textureNameToMergeInto.c_str());
                    auto tex = new CCTexture2D();
                    tex->initWithImage(img);
                    tex->setAliasTexParameters();
					mainContent = CCSprite::createWithTexture(tex);
                    mainContent->setBlendFunc({ GL_ONE, GL_ONE });
                    CACHED_MAIN_CONTENTS[textureNameToMergeInto] = mainContent;
                }

                mainContent->setID("" + textureNameToMergeInto);

                canvas->setContentSize(mainContent->getContentSize());
                canvas->addChild(mainContent);

                static std::unordered_map<
                    std::string, 
                    std::unordered_map<std::string, Ref<CCSprite>>
                > ADDED_FRAMES;

                for (auto [key, frame] : CCDictionaryExt<std::string, CCSpriteFrame*>(m_pSpriteFrames)) {
					if (frame->getTexture() == textureOfPlist) {
                        auto addedFrame = CCSprite::createWithSpriteFrame(frame);
                        addedFrame->setID(key);
                        frame->getTexture()->setAliasTexParameters();
                        ADDED_FRAMES[textureNameToMergeInto].erase(key);
                        ADDED_FRAMES[textureNameToMergeInto][key] = addedFrame;
					}
                }

                for (auto [a, sprite] : ADDED_FRAMES[textureNameToMergeInto]) canvas->addChild(sprite);

                {
                    canvas->setLayout(RowLayout::create()
                        ->setGap(1.f)
                        ->setAutoScale(false)
                        ->setGrowCrossAxis(true)
                        ->setCrossAxisOverflow(true)
                        ->setAxisAlignment(AxisAlignment::Start)
                        ->setCrossAxisAlignment(AxisAlignment::End)
                        ->setCrossAxisAlignment(AxisAlignment::Start)
                    );
                }

                mainContent->setAnchorPoint({ 0.f, 1.f });
                mainContent->setPositionX(0.f);
                mainContent->setPositionY((int)canvas->getContentHeight());

                auto rend = CCRenderTexture::create(
                    canvas->getContentSize().width, 
                    canvas->getContentSize().height, 
                    kCCTexture2DPixelFormat_Default
                );
				rend->beginWithClear(0, 0, 0, 0);
				canvas->visit();
				rend->end();

                auto img = rend->newCCImage();
                img->m_bPreMulti = true;
                textureToMergeInto->initWithImage(img);
                delete img;

                DEBUG if (true) {
                    rend->getSprite()->setAnchorPoint({ 1.f, 1.f });
                    rend->getSprite()->setID("render");
                    canvas->addChild(rend->getSprite());

                    limitNodeSize(canvas, CCScene::get()->getContentSize(), 1, 0.01f);
                    canvas->setAnchorPoint({ -1.f, 0.f });
                };

                for (auto [key, sprite] : ADDED_FRAMES[textureNameToMergeInto]) {
                    auto frame = sprite->displayFrame();

                    auto rect = sprite->boundingBox();
                    rect.origin.y = canvas->getContentSize().height - rect.origin.y - rect.size.height;
                    auto rectInPixels = CC_RECT_POINTS_TO_PIXELS(rect);
                    frame->initWithTexture(textureToMergeInto, rectInPixels, false, CCPointZero, rectInPixels.size);

                    this->removeSpriteFrameByName(key.c_str());
                    this->addSpriteFrame(frame, key.c_str());

                    sprite->initWithSpriteFrameName(key.c_str());
                }
            }
        }
    }
};