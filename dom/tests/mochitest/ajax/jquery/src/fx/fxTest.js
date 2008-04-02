module("fx");

test("animate(Hash, Object, Function)", function() {
	expect(1);
	stop();
	var hash = {opacity: 'show'};
	var hashCopy = $.extend({}, hash);
	$('#foo').animate(hash, 0, function() {
		ok( hash.opacity == hashCopy.opacity, 'Check if animate changed the hash parameter' );
		start();
	});
});

test("toggle()", function() {
	expect(3);
	var x = $("#foo");
	ok( x.is(":visible") );
	x.toggle();
	ok( x.is(":hidden") );
	x.toggle();
	ok( x.is(":visible") );
});