# node-crfpp
### Helper module providing a convenient API to utilize the CRF++ library for sequential data labeling.

This module builds the CRF++ as a native addon, providing an entry point to this library through the CrfTest class. With this class, users can access functionality similar to what is normally provided by the crf_test.exe executable. Additionally, helper functions that return tagged data in the form of lists and objects are provided as a convenience for further result processing.

Note: Currently, the functionality provided by crf_learn.exe is not implemented. This means that a pre-generated model file is required. See test/test-node-crfpp.js for example usage.

## Requirements
* node >= 14.0.0
* node-gyp

## Installation

#### Prerequisite
Requires node-gyp: https://github.com/nodejs/node-gyp#installation
``` bash
$ npm install --save node-crfpp
```

## Usage
Utilize the API provided in CrfTest. The toString function will return the same string result that crf_test.exe would based on the input parameters provided to the CrfTest constructor. The various decode* functions provide the same results, but in a more structured form.

#### Example
``` js
const CrfTest = require('node-crfpp').CrfTest
modelPath = path.resolve('./model_file');
classifier = new CrfTest(`-m ${modelPath} -v2 -n4`);
// words and TAGs separated with '\t'
classifier.toString(["word1 POS_TAG", "word2  POS_TAG"]);
```

## License
[MIT](LICENSE)
