// HARNESS (browser side, loaded BEFORE p5 and the sketch). Not for sketch authors.
//
// Provides the `ctx` global and makes randomness reproducible. Time is never read from
// a clock: the driver (render.mjs) sets the frame index and everything derives from it.
(function () {
	const cfg = window.__FC__;
	let frame = cfg.start;

	// mulberry32: tiny, fast, good enough PRNG. Replaces Math.random so sketches that
	// use it (models do, constantly) are reproducible from SEED.
	function mulberry32(a) {
		return function () {
			a |= 0; a = (a + 0x6D2B79F5) | 0;
			let t = Math.imul(a ^ (a >>> 15), 1 | a);
			t = (t + Math.imul(t ^ (t >>> 7), 61 | t)) ^ t;
			return ((t ^ (t >>> 14)) >>> 0) / 4294967296;
		};
	}
	Math.random = mulberry32(cfg.seed >>> 0);

	// Live getters on a frozen object: a sketch can keep a reference to `ctx` and always
	// sees the current frame, but cannot modify it.
	const ctx = {};
	const def = (name, get) => Object.defineProperty(ctx, name, { get, enumerable: true });
	def('w', () => cfg.w);
	def('h', () => cfg.h);
	def('fps', () => cfg.fps);
	def('duration', () => cfg.duration);
	def('seed', () => cfg.seed);
	def('recording', () => true);
	def('params', () => cfg.params);
	def('frame', () => frame);
	def('t', () => frame / cfg.fps);
	def('progress', () => { const p = frame / cfg.fps / cfg.duration; return p - Math.floor(p); });
	Object.freeze(ctx);

	window.ctx = ctx;
	window.__setFrame = (f) => { frame = f; };
})();
