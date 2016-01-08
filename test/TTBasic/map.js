var three = { val: 3 };
var five = 5;
var six = { val: 6 };

var x = new Map();
var y = x;
y.baz = 5;

var z = new Map();
z.set(1, -1);
z.set(2, -2);
z.set(five, 5);

WScript.SetTimeout(testFunction, 50);

/////////////////

function testFunction()
{
    ttdTestReport("x === y", x === y, true);
    ttdTestReport("x.baz", x.baz, 5);
    ttdTestReport("z.has(five)", z.has(five), true);
    ttdTestReport("z.get(five)", z.get(five), 5);

    ////
    x.set(three, 3);
    z.delete(five);
    ////

    ttdTestReport("post update 1 -- y.has(three)", y.has(three), true);
    ttdTestReport("post update 1 -- y.get(three)", y.get(three), 3);
    ttdTestReport("post update 1 -- z.has(five)", z.has(five), false);

    ////
    z.set(six, 6);
    six = null;

    y.set(three, 4);
    y.set(five, 5);
    ////

    ttdTestReport("post update 2 -- x.has(five)", x.has(five), true);
    ttdTestReport("post update 2 -- x.get(five)", x.get(five), 5);
    ttdTestReport("post update 2 -- x.get(three)", x.get(three), 4);
    
    if(this.ttdTestsFailed)
    {
        ttdTestWrite("Failures!");
    }
    else
    {
        ttdTestWrite("All tests passed!");   
    }
}