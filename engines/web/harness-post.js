// HARNESS (browser side, loaded AFTER p5 and the sketch, before p5 starts). Not for sketch authors.
//
// Wraps the sketch's setup()/draw() so rendering is deterministic and driver-controlled:
//  - p5's own animation loop never calls the sketch's draw(); only __step() does.
//  - canvas size is forced to ctx.w x ctx.h whatever the sketch asks for.
//  - randomSeed/noiseSeed are applied before the sketch's setup().
//  - millis() is derived from ctx.t, not the wall clock.
(function () {
	if (typeof window.draw !== 'function') {
		window.__error = 'sketch.js must define function draw()';
		return;
	}
	if (window.p5) window.p5.disableFriendlyErrors = true;   // faster; real errors still throw

	const userSetup = window.setup;
	const userDraw = window.draw;
	let allow = false;      // true only while __step() is driving a frame

	window.draw = function () {
		if (!allow) return;
		return userDraw.apply(this, arguments);
	};

	window.setup = function () {
		const createCanvas = window.createCanvas;
		window.createCanvas = (w, h, renderer) => createCanvas.call(window, ctx.w, ctx.h, renderer);
		const pixelDensity = window.pixelDensity;
		pixelDensity.call(window, 1);
		window.pixelDensity = () => 1;
		window.millis = () => ctx.t * 1000;
		window.frameRate = () => ctx.fps;
		window.loop = () => {};
		window.randomSeed(ctx.seed);
		window.noiseSeed(ctx.seed);

		if (userSetup) userSetup.apply(this, arguments);

		if (window.width !== ctx.w || window.height !== ctx.h) window.createCanvas(ctx.w, ctx.h);
		window.noLoop();
		window.__ready = true;
	};

	// Opaque black backdrop so transparent canvases export the same way the OF lane
	// clears to black, and ffmpeg never has to guess about alpha.
	let flat = null;
	// Returns null on success, or the error's stack so the driver can show the sketch.js line
	// (puppeteer's evaluate() would otherwise drop it).
	window.__step = function (f) {
		window.__setFrame(f);
		allow = true;
		try { window.redraw(); return null; }
		catch (e) { return String((e && e.stack) || e); }
		finally { allow = false; }
	};
	window.__capture = function () {
		const src = window.drawingContext.canvas;
		if (!flat) flat = document.createElement('canvas');
		flat.width = src.width; flat.height = src.height;
		const g = flat.getContext('2d');
		g.fillStyle = '#000';
		g.fillRect(0, 0, flat.width, flat.height);
		g.drawImage(src, 0, 0);
		return flat.toDataURL('image/png');
	};
})();
