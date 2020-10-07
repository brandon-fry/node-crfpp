const CrfTest = require("../node-crfpp").CrfTest;
const assert = require("assert");

assert(CrfTest, "The expected module is undefined");

function testBasic()
{
    const instance = new CrfTest("");
    assert(instance.decode, "The decode method is not defined");

    // todo: add a simple model to test input/output
}

function testInvalidParams()
{
    const instance = new CrfTest();
}

assert.doesNotThrow(testBasic, undefined, "testBasic threw an expection");
assert.throws(testInvalidParams, undefined, "testInvalidParams didn't throw");

console.log("Tests passed- everything looks OK!");
