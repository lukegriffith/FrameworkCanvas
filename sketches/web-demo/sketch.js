function setup() { createCanvas(800, 800); }   // size is overridden by the harness
function draw() {
	background(10);
	noStroke(); fill(255, 100, 100);
	circle(width / 2, height / 2, min(width, height) * (0.3 + 0.05 * sin(ctx.progress * TWO_PI)));
}
