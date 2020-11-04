#include "CrfTest.hpp"

#include <string>
#include <iostream>
#include <unordered_map>

using namespace Napi;
using namespace std;

CrfTest::CrfTest(const Napi::CallbackInfo& info) : Napi::ObjectWrap<CrfTest>(info) {
    Napi::Env env = info.Env();

    if (info.Length() < 1 || !info[0].IsString()) {
        Napi::TypeError::New(env, "Class must be constructed with String arg")
            .ThrowAsJavaScriptException();
    } else {
        // Create the tagger object with passed argument string
        Napi::String args = info[0].As<Napi::String>();
        mTagger = CRFPP::createTagger(args.Utf8Value().c_str());
    }

    if( !mTagger ) {
        Napi::TypeError::New(env, "Failed to create tagger with provided arguments")
            .ThrowAsJavaScriptException();
    }
}

Napi::Value CrfTest::decode(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    if (info.Length() < 1 || !info[0].IsArray()) {
        Napi::TypeError::New(env, "Incorrect arguments. Array expected.")
            .ThrowAsJavaScriptException();
    }

    // Ensure vlevel is at least 1 for probability data
    const auto oldVlevel = mTagger->vlevel();
    mTagger->set_vlevel(1);

    parseInput(env, info[0].As<Napi::Array>());

    Napi::Object result = Napi::Object::New(env);
    result.Set("overallConfidence", Napi::Number::New(env, mTagger->prob()));
    result.Set("taggedData", getTaggedData(env));

    // Reset vlevel
    mTagger->set_vlevel(oldVlevel);

    return result;
}

Napi::Value CrfTest::decodeNbest(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    if (info.Length() < 1 || !info[0].IsArray()) {
        Napi::TypeError::New(env, "Incorrect arguments. Expected Array.'")
            .ThrowAsJavaScriptException();
    }

    const auto oldNbest = mTagger->nbest();
    if (info[1] && info[1].IsNumber()) {
         // Set nbest value if provided, otherwise nbest value comes from constructor
        // argument. E.g. "-m 'model' -n5"
        mTagger->set_nbest(info[1].As<Napi::Number>().Uint32Value());
    }

    parseInput(env, info[0].As<Napi::Array>());

    // Loop nbest times and append to return list
    Napi::Array returnList = Napi::Array::New(env, mTagger->nbest());
    for (size_t i = 0; i < mTagger->nbest(); ++i) {
        if (!mTagger->next()) {
            break;
        }

        Napi::Object result = Napi::Object::New(env);
        result.Set("overallConfidence", Napi::Number::New(env, mTagger->empty() ? 0 : mTagger->prob()));
        result.Set("taggedData", getTaggedData(env));

        returnList[i] = result;
    }

    // Reset nbest value
    mTagger->set_nbest(oldNbest);

    return returnList;
}

Napi::Value CrfTest::decodeBestTag(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    if (info.Length() < 1 || !info[0].IsArray()) {
        Napi::TypeError::New(env, "Incorrect arguments. Array expected.")
            .ThrowAsJavaScriptException();
    }

    // Ensure vlevel is at 2 so all tags are given per token
    const auto oldVlevel = mTagger->vlevel();
    mTagger->set_vlevel(2);

    parseInput(env, info[0].As<Napi::Array>());
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
        decodedTag.Set("confidence", Napi::Number::New(env, mTagger->prob(i,bestProbIndex)));

        returnList[i] = decodedTag;
    }

    Napi::Object result = Napi::Object::New(env);
    // Since the "best" tags were taken, the conditional probability no longer applies.
    result.Set("overallConfidence", Napi::Number::New(env, 0));
    result.Set("taggedData", returnList);

    // Reset vlevel
    mTagger->set_vlevel(oldVlevel);

    return result;
}

Napi::Value CrfTest::decodeToTagsList(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    if (info.Length() < 1 || !info[0].IsArray()) {
        Napi::TypeError::New(env, "Incorrect arguments. Array expected.")
            .ThrowAsJavaScriptException();
    }

    // Ensure vlevel is at least 1 for probability data
    const auto oldVlevel = mTagger->vlevel();
    mTagger->set_vlevel(1);

    parseInput(env, info[0].As<Napi::Array>());

    // Get the return object before resetting vlevel
    const Napi::Object& returnObj = getTagsToWords(env);

    // Reset vlevel
    mTagger->set_vlevel(oldVlevel);

    return returnObj;
}

Napi::Value CrfTest::decodeToTagsListNbest(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    if (info.Length() < 1 || !info[0].IsArray()) {
        Napi::TypeError::New(env, "Incorrect arguments. Array expected.")
            .ThrowAsJavaScriptException();
    }

    // Ensure vlevel is at least 1 for probability data
    const auto oldVlevel = mTagger->vlevel();
    mTagger->set_vlevel(1);

    const auto oldNbest = mTagger->nbest();
    if (info[1] && info[1].IsNumber()) {
        // Set nbest value if provided, otherwise nbest value comes from constructor
        // argument. E.g. "-m 'model' -n5"
        mTagger->set_nbest(info[1].As<Napi::Number>().Uint32Value());
    }

    parseInput(env, info[0].As<Napi::Array>());

    Napi::Array returnList = Napi::Array::New(env, mTagger->nbest());
    for (size_t i = 0; i < mTagger->nbest(); ++i) {
        if (!mTagger->next()) {
            break;
        }

        returnList[i] = getTagsToWords(env);
    }

    // Reset tagger values
    mTagger->set_vlevel(oldVlevel);
    mTagger->set_nbest(oldNbest);

    return returnList;
}

Napi::Value CrfTest::toString(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    if (info.Length() < 1 || !info[0].IsArray()) {
        Napi::TypeError::New(env, "Incorrect arguments. Array expected.")
            .ThrowAsJavaScriptException();
    }

    parseInput(env, info[0].As<Napi::Array>());
    return Napi::String::New(env, mTagger->toString());
}

Napi::Object CrfTest::getTagsToWords(const Napi::Env& env) {
    std::unordered_map<std::string, std::string> tagToWord;
    std::string previousTag = "";
    double probSum = 0;

    // Initialize map with all given tags
    for (size_t i = 0; i < mTagger->ysize(); ++i) {
        tagToWord.emplace(mTagger->yname(i), "");
    }

    // Map each token's word to its assigned tag
    for (uint i = 0; i < mTagger->size(); ++i) {
        if (mTagger->xsize() > 0) {
            if (tagToWord[mTagger->y2(i)] == "") {
                // First entry, add the word only, do not add spaces, comma, etc.
                tagToWord[mTagger->y2(i)] = mTagger->x(i, 0);
            }
            else if (previousTag == mTagger->y2(i)) {
                // Only add a space between consecutive words with same tag
                tagToWord[mTagger->y2(i)] += std::string(" ") + mTagger->x(i, 0);
            } else {
                // Add ' ,' between non-consecutive words with same tag
                tagToWord[mTagger->y2(i)] += std::string(", ") + mTagger->x(i, 0);
            }

            previousTag = mTagger->y2(i);
        }

        // Update overall probability
        if (mTagger->vlevel() > 0) {
            for (size_t j = 0; j < mTagger->ysize(); ++j) {
                if (mTagger->y2(i) == mTagger->yname(j)) {
                    probSum += mTagger->prob(i, j);
                    break;
                }
            }
        }
    }

    // Create return object
    Napi::Object returnObj = Napi::Object::New(env);

    // Set probability
    returnObj.Set("avgTagConfidence", Napi::Number::New(env, probSum / mTagger->size()));
    returnObj.Set("overallConfidence", Napi::Number::New(env, mTagger->empty() ? 0 : mTagger->prob()));

    // Set tags
    Napi::Object tagsObj = Napi::Object::New(env);
    for (const auto& pair: tagToWord) {
        tagsObj.Set(Napi::String::New(env, pair.first), Napi::String::New(env, pair.second));
    }
    returnObj.Set("tags", tagsObj);

    return returnObj;
}

Napi::Array CrfTest::getTaggedData(const Napi::Env& env) {
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
                    decodedTag.Set("confidence", Napi::Number::New(env, mTagger->prob(i,j)));
                    break;
                }
            }
        } else {
            // No probability data available
            decodedTag.Set("confidence", Napi::Number::New(env, 0));
        }

        returnList[i] = decodedTag;
    }

    return returnList;
}

void CrfTest::parseInput(const Napi::Env& env, const Napi::Array& tokens)
{
    mTagger->clear();

    if( tokens.Length() == 0 ) {
        return;
    }

    // Insert input
    for(uint i = 0; i < tokens.Length(); ++i) {
       Napi::Value token = tokens[i];
       if (token.IsString()) {
           Napi::String tokenString = token.As<Napi::String>();
           mTagger->add(tokenString.Utf8Value().c_str());
       }
    }

    // Parse input
    bool isParsed = mTagger->parse();
    if ( isParsed == false) {
        Napi::TypeError::New(env, "Failed to parse input")
            .ThrowAsJavaScriptException();
    }
}

Napi::Object Init(Napi::Env env, Napi::Object exports) {
    Napi::Function func = CrfTest::DefineClass(
        env,
        "CrfTest",
        {
            CrfTest::InstanceMethod("decode", &CrfTest::decode),
            CrfTest::InstanceMethod("decodeBestTag", &CrfTest::decodeBestTag),
            CrfTest::InstanceMethod("decodeNbest", &CrfTest::decodeNbest),
            CrfTest::InstanceMethod("decodeToTagsList", &CrfTest::decodeToTagsList),
            CrfTest::InstanceMethod("decodeToTagsListNbest", &CrfTest::decodeToTagsListNbest),
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
