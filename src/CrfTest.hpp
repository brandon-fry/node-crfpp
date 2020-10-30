#ifndef CRFTEST_HPP
#define CRFTEST_HPP

#include "crfpp.h"
#include <napi.h>

/**
 * Provides "tagger" functionality from the CRF++ library. This class provides
 * similar functionality to the crf_test.exe app provided in the CRF++ library.
 * This means that a trained model is required to begin processing input.
 */
class CrfTest : public Napi::ObjectWrap<CrfTest>
{
public:

    /**
     * Constructor
     * @param type:string, argument list. e.g. "-v 1 -m tmp/model_file input.txt"
     */
    CrfTest(const Napi::CallbackInfo& info);

    /**
     * Parses passed tokens with the crf++ tagger and creates structured data from results.
     *
     * Output structure:
     * {
     *   overallConfidence: 0.3269402109724775,
     *   taggedData: [
     *     { word: '1', tag: 'TAG', confidence: 0.9269402109724775 },
     *     { word: '2', tag: 'TAG', confidence: 0.8269402109724775 }
     *   ]
     * }
     *
     * @param type:[String], list of tokens to parse.
     * @return type:Object, structured result data.
     */
    Napi::Value decode(const Napi::CallbackInfo& info);

    /**
     * Similar to decode, but returns a list of objects representing the nbest
     *   results from the tagger.
     *
     * Output structure:
     * [
     *   {
     *     overallConfidence: 0.3269402109724775,
     *     taggedData: [
     *       { word: '1', tag: 'TAG', confidence: 0.9269402109724775 }
     *     ]
     *   }
     * ]
     *
     * @param type:[String], list of tokens to parse.
     * @param type:int, (Optional) nbest value to use. If not assigned, the nbest
     *   value provided in the constructor argument (i.e. '-nX') will be used.
     * @return type:[Object], list of structured result data.
     */
    Napi::Value decodeNbest(const Napi::CallbackInfo& info);

    /**
     * Similar to decode, but assigns the answer tag with the highest confidence
     *   value instead of the normal answer tag.
     *
     * @param type:[String], list of tokens to parse.
     * @return type:Object, structured result data.
     */
    Napi::Value decodeBestTag(const Napi::CallbackInfo& info);

    /**
     * Parses tokens and assigns each word to its set answer tag. Consecutive words
     *   with the same answer tag are concatenated with a space. Non-consecutive words
     *   with the same answer tag are separated by a comma. Probability is calculated
     *   as the average of all answer tags.
     *
     * Output structure:
     * {
     *   avgTagConfidence: 0.8873412702159211,
     *   overallConfidence: 0.3269402109724775,
     *   tags: {
     *     TAG1: 'word1 word2, word4',
     *     TAG2: 'word3'
     *   }
     * }
     *
     * @param type:[String], list of tokens to parse.
     * @return type:Object, structured result data.
     */
    Napi::Value decodeToTagsList(const Napi::CallbackInfo& info);

    /**
     * Similar to decodeToTagsList, but returns a list of objects representing the nbest
     *   results from the tagger.
     *
     * Output structure:
     * [
     *   {
     *     avgTagConfidence: 0.8873412702159211,
     *     overallConfidence: 0.3269402109724775,
     *     tags: {
     *       TAG1: 'word1 word2, word4',
     *       TAG2: 'word3'
     *     }
     *   }
     * ]
     *
     * @param type:[String], list of tokens to parse.
     * @param type:int, (Optional) nbest value to use. If not assigned, the nbest
     *   value provided in the constructor argument (i.e. '-nX') will be used.
     * @return type:[Object], list of structured result data.
     */
    Napi::Value decodeToTagsListNbest(const Napi::CallbackInfo& info);

    /**
     * Parses passed tokens and returns tagged data in tabulated string form.
     *   Output is determined by the args passed to the constructor of this class.
     *
     * @note This gives the same output that crf_test.exe would give.
     * @param type:[String], list of tokens to parse.
     * @return type:string, tagged output.
     */
    Napi::Value toString(const Napi::CallbackInfo& info);

private:

    // Loops over tagger object and gets stuctured "word, tag, conf" data.
    // @return List of structured data
    Napi::Array getTaggedData(const Napi::Env& env);

    // Loops over tagger object and gets structured "tag to word" data
    Napi::Object getTagsToWords(const Napi::Env& env);

    // Adds passed tokens to tagger and parses them.
    // @param env js env
    // @param tokens the list of tokens to tag.
    void parseInput(const Napi::Env& env, const Napi::Array& tokens);

    // CRF++ tagger object
    CRFPP::Tagger* mTagger;
};

#endif
