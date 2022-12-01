#version 310 es

struct GammaTransferParams {
  float G;
  float A;
  float B;
  float C;
  float D;
  float E;
  float F;
  uint padding;
};

struct ExternalTextureParams {
  uint numPlanes;
  uint doYuvToRgbConversionOnly;
  mat3x4 yuvToRgbConversionMatrix;
  GammaTransferParams gammaDecodeParams;
  GammaTransferParams gammaEncodeParams;
  mat3 gamutConversionMatrix;
  mat2 rotationMatrix;
};

struct ExternalTextureParams_std140 {
  uint numPlanes;
  uint doYuvToRgbConversionOnly;
  mat3x4 yuvToRgbConversionMatrix;
  GammaTransferParams gammaDecodeParams;
  GammaTransferParams gammaEncodeParams;
  mat3 gamutConversionMatrix;
  vec2 rotationMatrix_0;
  vec2 rotationMatrix_1;
};

layout(binding = 2, std140) uniform ext_tex_params_block_std140_ubo {
  ExternalTextureParams_std140 inner;
} ext_tex_params;

vec3 gammaCorrection(vec3 v, GammaTransferParams params) {
  bvec3 cond = lessThan(abs(v), vec3(params.D));
  vec3 t = (sign(v) * ((params.C * abs(v)) + params.F));
  vec3 f = (sign(v) * (pow(((params.A * abs(v)) + params.B), vec3(params.G)) + params.E));
  return mix(f, t, cond);
}

vec4 textureLoadExternal(highp sampler2D plane0_1, highp sampler2D plane1_1, uvec2 coord, ExternalTextureParams params) {
  vec3 color = vec3(0.0f, 0.0f, 0.0f);
  if ((params.numPlanes == 1u)) {
    color = texelFetch(plane0_1, ivec2(coord), 0).rgb;
  } else {
    color = (vec4(texelFetch(plane0_1, ivec2(coord), 0).r, texelFetch(plane1_1, ivec2(coord), 0).rg, 1.0f) * params.yuvToRgbConversionMatrix);
  }
  if ((params.doYuvToRgbConversionOnly == 0u)) {
    color = gammaCorrection(color, params.gammaDecodeParams);
    color = (params.gamutConversionMatrix * color);
    color = gammaCorrection(color, params.gammaEncodeParams);
  }
  return vec4(color, 1.0f);
}

uniform highp sampler2D arg_0_1;
uniform highp sampler2D ext_tex_plane_1_1;
ExternalTextureParams conv_ExternalTextureParams(ExternalTextureParams_std140 val) {
  return ExternalTextureParams(val.numPlanes, val.doYuvToRgbConversionOnly, val.yuvToRgbConversionMatrix, val.gammaDecodeParams, val.gammaEncodeParams, val.gamutConversionMatrix, mat2(val.rotationMatrix_0, val.rotationMatrix_1));
}

void textureLoad_1bfdfb() {
  vec4 res = textureLoadExternal(arg_0_1, ext_tex_plane_1_1, uvec2(1u), conv_ExternalTextureParams(ext_tex_params.inner));
}

vec4 vertex_main() {
  textureLoad_1bfdfb();
  return vec4(0.0f);
}

void main() {
  gl_PointSize = 1.0;
  vec4 inner_result = vertex_main();
  gl_Position = inner_result;
  gl_Position.y = -(gl_Position.y);
  gl_Position.z = ((2.0f * gl_Position.z) - gl_Position.w);
  return;
}
#version 310 es
precision mediump float;

struct GammaTransferParams {
  float G;
  float A;
  float B;
  float C;
  float D;
  float E;
  float F;
  uint padding;
};

struct ExternalTextureParams {
  uint numPlanes;
  uint doYuvToRgbConversionOnly;
  mat3x4 yuvToRgbConversionMatrix;
  GammaTransferParams gammaDecodeParams;
  GammaTransferParams gammaEncodeParams;
  mat3 gamutConversionMatrix;
  mat2 rotationMatrix;
};

struct ExternalTextureParams_std140 {
  uint numPlanes;
  uint doYuvToRgbConversionOnly;
  mat3x4 yuvToRgbConversionMatrix;
  GammaTransferParams gammaDecodeParams;
  GammaTransferParams gammaEncodeParams;
  mat3 gamutConversionMatrix;
  vec2 rotationMatrix_0;
  vec2 rotationMatrix_1;
};

layout(binding = 2, std140) uniform ext_tex_params_block_std140_ubo {
  ExternalTextureParams_std140 inner;
} ext_tex_params;

vec3 gammaCorrection(vec3 v, GammaTransferParams params) {
  bvec3 cond = lessThan(abs(v), vec3(params.D));
  vec3 t = (sign(v) * ((params.C * abs(v)) + params.F));
  vec3 f = (sign(v) * (pow(((params.A * abs(v)) + params.B), vec3(params.G)) + params.E));
  return mix(f, t, cond);
}

vec4 textureLoadExternal(highp sampler2D plane0_1, highp sampler2D plane1_1, uvec2 coord, ExternalTextureParams params) {
  vec3 color = vec3(0.0f, 0.0f, 0.0f);
  if ((params.numPlanes == 1u)) {
    color = texelFetch(plane0_1, ivec2(coord), 0).rgb;
  } else {
    color = (vec4(texelFetch(plane0_1, ivec2(coord), 0).r, texelFetch(plane1_1, ivec2(coord), 0).rg, 1.0f) * params.yuvToRgbConversionMatrix);
  }
  if ((params.doYuvToRgbConversionOnly == 0u)) {
    color = gammaCorrection(color, params.gammaDecodeParams);
    color = (params.gamutConversionMatrix * color);
    color = gammaCorrection(color, params.gammaEncodeParams);
  }
  return vec4(color, 1.0f);
}

uniform highp sampler2D arg_0_1;
uniform highp sampler2D ext_tex_plane_1_1;
ExternalTextureParams conv_ExternalTextureParams(ExternalTextureParams_std140 val) {
  return ExternalTextureParams(val.numPlanes, val.doYuvToRgbConversionOnly, val.yuvToRgbConversionMatrix, val.gammaDecodeParams, val.gammaEncodeParams, val.gamutConversionMatrix, mat2(val.rotationMatrix_0, val.rotationMatrix_1));
}

void textureLoad_1bfdfb() {
  vec4 res = textureLoadExternal(arg_0_1, ext_tex_plane_1_1, uvec2(1u), conv_ExternalTextureParams(ext_tex_params.inner));
}

void fragment_main() {
  textureLoad_1bfdfb();
}

void main() {
  fragment_main();
  return;
}
#version 310 es

struct GammaTransferParams {
  float G;
  float A;
  float B;
  float C;
  float D;
  float E;
  float F;
  uint padding;
};

struct ExternalTextureParams {
  uint numPlanes;
  uint doYuvToRgbConversionOnly;
  mat3x4 yuvToRgbConversionMatrix;
  GammaTransferParams gammaDecodeParams;
  GammaTransferParams gammaEncodeParams;
  mat3 gamutConversionMatrix;
  mat2 rotationMatrix;
};

struct ExternalTextureParams_std140 {
  uint numPlanes;
  uint doYuvToRgbConversionOnly;
  mat3x4 yuvToRgbConversionMatrix;
  GammaTransferParams gammaDecodeParams;
  GammaTransferParams gammaEncodeParams;
  mat3 gamutConversionMatrix;
  vec2 rotationMatrix_0;
  vec2 rotationMatrix_1;
};

layout(binding = 2, std140) uniform ext_tex_params_block_std140_ubo {
  ExternalTextureParams_std140 inner;
} ext_tex_params;

vec3 gammaCorrection(vec3 v, GammaTransferParams params) {
  bvec3 cond = lessThan(abs(v), vec3(params.D));
  vec3 t = (sign(v) * ((params.C * abs(v)) + params.F));
  vec3 f = (sign(v) * (pow(((params.A * abs(v)) + params.B), vec3(params.G)) + params.E));
  return mix(f, t, cond);
}

vec4 textureLoadExternal(highp sampler2D plane0_1, highp sampler2D plane1_1, uvec2 coord, ExternalTextureParams params) {
  vec3 color = vec3(0.0f, 0.0f, 0.0f);
  if ((params.numPlanes == 1u)) {
    color = texelFetch(plane0_1, ivec2(coord), 0).rgb;
  } else {
    color = (vec4(texelFetch(plane0_1, ivec2(coord), 0).r, texelFetch(plane1_1, ivec2(coord), 0).rg, 1.0f) * params.yuvToRgbConversionMatrix);
  }
  if ((params.doYuvToRgbConversionOnly == 0u)) {
    color = gammaCorrection(color, params.gammaDecodeParams);
    color = (params.gamutConversionMatrix * color);
    color = gammaCorrection(color, params.gammaEncodeParams);
  }
  return vec4(color, 1.0f);
}

uniform highp sampler2D arg_0_1;
uniform highp sampler2D ext_tex_plane_1_1;
ExternalTextureParams conv_ExternalTextureParams(ExternalTextureParams_std140 val) {
  return ExternalTextureParams(val.numPlanes, val.doYuvToRgbConversionOnly, val.yuvToRgbConversionMatrix, val.gammaDecodeParams, val.gammaEncodeParams, val.gamutConversionMatrix, mat2(val.rotationMatrix_0, val.rotationMatrix_1));
}

void textureLoad_1bfdfb() {
  vec4 res = textureLoadExternal(arg_0_1, ext_tex_plane_1_1, uvec2(1u), conv_ExternalTextureParams(ext_tex_params.inner));
}

void compute_main() {
  textureLoad_1bfdfb();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
