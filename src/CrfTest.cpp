#include "CrfTest.hpp"

#include <string>
#include <iostream>
#include <unordered_map>

using namespace Napi;
using namespace std;

CrfTest::CrfTest(const Napi::CallbackInfo& info) : Napi::ObjectWrap<CrfTest>(info) {
    Napi::Env env = info.Env();

    if (info.Length() <= 0 || !info[0].IsString()) {
        Napi::TypeError::New(env, "String expected").ThrowAsJavaScriptException();
        return;
    } else {
        // Create the tagger object with passed argument string
        Napi::String modelPath = info[0].As<Napi::String>();
        mTagger = CRFPP::createTagger(modelPath.Utf8Value().c_str());
    }
}

Napi::Value CrfTest::decode(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    // Message once about vlevel requirement for probability
    if (mTagger->vlevel() < 1) {
        std::cout << "vlevel was not set. No probability data available." << std::endl;
    }

    parseInput(info);
    Napi::Array returnList = Napi::Array::New(env, mTagger->size());

    for (uint i = 0; i < mTagger->size(); ++i) {
      Napi::Object decodedTag = Napi::Object::New(env);

      if (mTagger->xsize() > 0) {
          decodedTag.Set("word", Napi::String::New(env, mTagger->x(i, 0)));
      }

      decodedTag.Set("tag", Napi::String::New(env, mTagger->y2(i)));

      if (mTagger->vlevel() > 0) {
          for (size_t j = 0; j < mTagger->ysize(); ++j) {
              if (mTagger->y2(i) == mTagger->yname(j)) {
                  decodedTag.Set("prob", Napi::Number::New(env, mTagger->prob(i,j)));
                  break;
              }
          }
      } else {
          // No probability data available
          decodedTag.Set("prob", Napi::Number::New(env, 0));
      }

      returnList[i] = decodedTag;
    }

    return returnList;
}

Napi::Value CrfTest::decodeBest(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    if (mTagger->vlevel() < 2) {
        // Without probability values, the best values cannot be determined. Throw error.
        Napi::TypeError::New(env, "v level of 2 is required").ThrowAsJavaScriptException();
    }

    parseInput(info);
    Napi::Array returnList = Napi::Array::New(env, mTagger->size());

    // Find highest probability for each token
    for (uint i = 0; i < mTagger->size(); ++i) {
        Napi::Object decodedTag = Napi::Object::New(env);

        if (mTagger->xsize() > 0) {
            decodedTag.Set("word", Napi::String::New(env, mTagger->x(i, 0)));
        }

        // Find tag with best probability
        int bestProbIndex = 0;
        for (size_t j = 0; j < mTagger->ysize(); ++j) {
            if (mTagger->prob(i,j) > mTagger->prob(i,bestProbIndex)) {
                bestProbIndex = j;
            }
        }

        decodedTag.Set("tag", Napi::String::New(env, mTagger->yname(bestProbIndex)));
        decodedTag.Set("prob", Napi::Number::New(env, mTagger->prob(i,bestProbIndex)));

        returnList[i] = decodedTag;
    }

    return returnList;
}

Napi::Value CrfTest::decodeToTagsList(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    // Message once about vlevel requirement for probability
    if (mTagger->vlevel() < 1) {
        std::cout << "vlevel was not set. No probability data available." << std::endl;
    }

    parseInput(info);
    std::unordered_map<std::string, std::string> tagToWord;
    std::string previousTag = "";
    double probSum = 0;

    // Initialize map with all given tags
    for (size_t i = 0; i < mTagger->ysize(); ++i) {
        std::cout << "tag " << i << " " << mTagger->yname(i) << std::endl;
        tagToWord.emplace(mTagger->yname(i), "");
    }

    // Map each token's word to its assigned tag
    for (uint i = 0; i < mTagger->size(); ++i) {
        if (mTagger->xsize() > 0) {
            std::cout << "tagToWord[mTagger->y2(i)] " << tagToWord[mTagger->y2(i)] << std::endl;
            if (tagToWord[mTagger->y2(i)] == "") {
                // First entry, add the word only, do not add spaces, comma, etc.
                tagToWord[mTagger->y2(i)] = mTagger->x(i, 0);
            }
            else if (previousTag == mTagger->y2(i)) {
                // Only add a space between consecutive words with same tag
                tagToWord[mTagger->y2(i)] += std::string(" ") + mTagger->x(i, 0);
            } else {
                // Add ' ,' between non-consecutive words with same tag
                tagToWord[mTagger->y2(i)] += std::string(" ,") + mTagger->x(i, 0);
            }

            previousTag = mTagger->y2(i);
        }

        // Update overall probability
        if (mTagger->vlevel() > 0) {
            for (size_t j = 0; j < mTagger->ysize(); ++j) {
                if (mTagger->y2(i) == mTagger->yname(j)) {
                    std::cout << "prob " << mTagger->prob(i, j) << std::endl;
                    probSum += mTagger->prob(i, j);
                    std::cout << "probSum " << probSum << std::endl;

                    break;
                }
            }
        }
    }

    // Build return object
    Napi::Object returnObj = Napi::Object::New(env);

    // Set probability
    returnObj.Set("overallProb", Napi::Number::New(env, probSum / mTagger->size()));

    // Set tags
    Napi::Object tagsObj = Napi::Object::New(env);
    for (const auto& pair: tagToWord) {
        tagsObj.Set(Napi::String::New(env, pair.first), Napi::String::New(env, pair.second));
    }
    returnObj.Set("tags", tagsObj);

    return returnObj;
}

Napi::Value CrfTest::toString(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    parseInput(info);
    return Napi::String::New(env, mTagger->toString());
}

void CrfTest::parseInput(const Napi::CallbackInfo& info)
{
    Napi::Env env = info.Env();

    if (!mTagger) {
        Napi::TypeError::New(env, "Tagger object not created").ThrowAsJavaScriptException();
    }

    mTagger->clear();

    // Insert input
    Napi::Array inputTokens = info[0].As<Napi::Array>();
    for(uint i = 0; i < inputTokens.Length(); ++i) {
      Napi::Value inputToken = inputTokens[i];
      if (inputToken.IsString()) {
          Napi::String inputStr = inputToken.As<Napi::String>();
          mTagger->add(inputStr.Utf8Value().c_str());
      }
    }

    // Parse input
    bool isParsed = mTagger->parse();
    if ( isParsed == false) {
        Napi::TypeError::New(env, "Failed to parse").ThrowAsJavaScriptException();
    }
}

Napi::Object Init(Napi::Env env, Napi::Object exports) {
    Napi::Function func = CrfTest::DefineClass(
        env,
        "CrfTest",
        {
            CrfTest::InstanceMethod("decode", &CrfTest::decode),
            CrfTest::InstanceMethod("decodeBest", &CrfTest::decodeBest),
            CrfTest::InstanceMethod("decodeToTagsList", &CrfTest::decodeToTagsList),
            CrfTest::InstanceMethod("toString", &CrfTest::toString)
        }
    );

    Napi::FunctionReference* constructor = new Napi::FunctionReference();
    *constructor = Napi::Persistent(func);
    env.SetInstanceData(constructor);

    exports.Set("CrfTest", func);
    return exports;
}

NODE_API_MODULE(crfwrapper, Init);
