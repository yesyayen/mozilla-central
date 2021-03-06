// Iterator prototype surfaces.

load(libdir + "asserts.js");
load(libdir + "iteration.js");

function test(constructor) {
    var proto = Object.getPrototypeOf(constructor()[std_iterator]());
    var names = Object.getOwnPropertyNames(proto);
    names.sort();
    assertDeepEq(names, [std_iterator, 'next']);

    var desc = Object.getOwnPropertyDescriptor(proto, 'next');
    assertEq(desc.configurable, true);
    assertEq(desc.enumerable, false);
    assertEq(desc.writable, true);

    assertEq(proto[std_iterator](), proto);
    assertIteratorResult(proto.next(), undefined, true);
}

//test(Array);
test(Map);
test(Set);
