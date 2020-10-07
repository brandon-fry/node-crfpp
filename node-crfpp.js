const CrfTestNative = require('./build/Release/nodecrfpp').CrfTest;

/**
    Simple wrapper class for the CrfTest C++ addon class. This class provides
    similar functionality to the crf_test.exe app provided in the CRF++ library.
    This means that a trained model is required to begin processing input.

    See CrfTest.hpp for detailed function documentation.
*/
class CrfTest {
	constructor(argString) {
		this._crfWrapper = new CrfTestNative(argString);
	}

	decode(tokens) {
		return this._crfWrapper.decode(tokens);
	}

	decodeBest(tokens) {
		return this._crfWrapper.decodeBest(tokens);
	}

    decodeToTagsList(tokens) {
		return this._crfWrapper.decodeToTagsList(tokens);
	}

	toString(tokens) {
		return this._crfWrapper.toString(tokens);
	}
};

module.exports.CrfTest = CrfTest;
