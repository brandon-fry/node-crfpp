#ifndef CRFTEST_HPP
#define CRFTEST_HPP

#include "crfpp.h"

#include <napi.h>

class CrfTest : public Napi::ObjectWrap<CrfTest>
{
public:

    // Constructor
    //
    // @param type:string, argument list. e.g. "-v 1 -m tmp/model_file input.txt"
    CrfTest(const Napi::CallbackInfo& info);

    // Parses passed tokens and creates structured data for each.
    //
    // @note the constructor's argument list must contain "-v 1" or greater to
    //   receive probability data.
    // @param type:[], list of tokens to parse.
    // @return type:[], list of {word: "", tag: "", prob: 0} objects.
    Napi::Value decode(const Napi::CallbackInfo& info);

    // Parses passed tokens and creates structured data for each, assigning the
    //   answer tag with the best probablity instead of the normal answer tag.
    //
    // @param type:[], list of tokens to parse.
    // @return type:[], list of {word: "", tag: "", prob: 0} objects.
    // @throws exception if "-v 2" or greater is not set on construction of this class.
    Napi::Value decodeBest(const Napi::CallbackInfo& info);

    // Parses tokens and assigns each word to its set answer tag. Consecutive words
    //   with the same answer tag are concatenated with a space. Non-consecutive words
    //   with the same answer tag are separated by a comma. Probability is calculated
    //   as the average of all answer tags.
    // Output structure:
    // {
    //   overallProb: 0.999,
    //   tags: {
    //     TAG1: 'word1 word2, word4',
    //     TAG2: 'word3',
    //     ...
    //   }
    // }
    //
    // @note the constructor's argument list must contain "-v 1" or greater to
    //   receive probability data.
    // @param type:[], list of tokens.
    // @return type:Object.
    Napi::Value decodeToTagsList(const Napi::CallbackInfo& info);

    // Parses passed tokens and returns tagged data in tabulated string form.
    //   Output is determined by the args passed to the constructor of this class.
    //
    // @note This gives the same output that crf_test.exe would give.
    // @param type:[], list of tokens to parse.
    // @return type:string, tagged output.
    Napi::Value toString(const Napi::CallbackInfo& info);

private:
    // Adds passed tokens to tagger
    //
    // @param type:[], list of tokens to tag.
    void parseInput(const Napi::CallbackInfo& info);

    CRFPP::Tagger* mTagger;
};

#endif
