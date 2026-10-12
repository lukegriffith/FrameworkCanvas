// B5: 14x14 data grid, radial travelling wave. Loops: wave phase = dist*K - progress*TWO_PI*2.
const N = 14;
function setup() { createCanvas(ctx.w, ctx.h); }
function draw() {
  background(244, 241, 236);
  const unit = min(width, height);
  const cell = unit * 0.78 / N;
  const x0 = (width - cell * N) / 2, y0 = (height - cell * N) / 2;
  const A = color(0, 122, 120), B = color(240, 90, 60), base = color(210, 206, 200);
  noStroke();
  for (let j = 0; j < N; j++) for (let i = 0; i < N; i++) {
    const dx = i - (N - 1) / 2, dy = j - (N - 1) / 2;
    const d = sqrt(dx * dx + dy * dy);
    const w = sin(d * 0.9 - ctx.progress * TWO_PI * 2); // -1..1
    const e = (w + 1) / 2;
    const r = cell * (0.12 + 0.36 * e);
    const c = e < 0.5 ? lerpColor(A, base, e * 2) : lerpColor(base, B, (e - 0.5) * 2);
    fill(c);
    circle(x0 + (i + 0.5) * cell, y0 + (j + 0.5) * cell, r * 2);
  }
}
