diagnostic_filtering/compound_statement_attribute.wgsl:8:11 warning: 'textureSample' must only be called from uniform control flow
      _ = textureSample(t, s, vec2(0, 0));
          ^^^^^^^^^^^^^

diagnostic_filtering/compound_statement_attribute.wgsl:7:5 note: control flow depends on possibly non-uniform value
    if (x > 0) {
    ^^

diagnostic_filtering/compound_statement_attribute.wgsl:7:9 note: user-defined input 'x' of 'main' may be non-uniform
    if (x > 0) {
        ^

@group(0) @binding(1) var t : texture_2d<f32>;

@group(0) @binding(2) var s : sampler;

@fragment
fn main(@location(0) x : f32) {
  @diagnostic(warning, derivative_uniformity) {
    if ((x > 0)) {
      _ = textureSample(t, s, vec2(0, 0));
    }
  }
}
