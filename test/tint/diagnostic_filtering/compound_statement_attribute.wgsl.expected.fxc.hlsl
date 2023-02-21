diagnostic_filtering/compound_statement_attribute.wgsl:8:11 warning: 'textureSample' must only be called from uniform control flow
      _ = textureSample(t, s, vec2(0, 0));
          ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

diagnostic_filtering/compound_statement_attribute.wgsl:7:5 note: control flow depends on possibly non-uniform value
    if (x > 0) {
    ^^

diagnostic_filtering/compound_statement_attribute.wgsl:7:9 note: user-defined input 'x' of 'main' may be non-uniform
    if (x > 0) {
        ^

Texture2D<float4> t : register(t1, space0);
SamplerState s : register(s2, space0);

struct tint_symbol_1 {
  float x : TEXCOORD0;
};

void main_inner(float x) {
  {
    if ((x > 0.0f)) {
    }
  }
}

void main(tint_symbol_1 tint_symbol) {
  main_inner(tint_symbol.x);
  return;
}
