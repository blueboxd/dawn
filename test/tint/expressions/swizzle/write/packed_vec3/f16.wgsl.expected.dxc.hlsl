SKIP: FAILED

swizzle/write/packed_vec3/f16.wgsl:3:8 error: using f16 types in 'storage' address space is not implemented yet
    v: vec3<f16>,
       ^^^^^^^^^

swizzle/write/packed_vec3/f16.wgsl:2:1 note: see layout of struct:
/*           align(8) size(8) */ struct S {
/* offset(0) align(8) size(6) */   v : vec3<f16>;
/* offset(6) align(1) size(2) */   // -- implicit struct size padding --;
/*                            */ };
struct S {
^^^^^^

swizzle/write/packed_vec3/f16.wgsl:6:48 note: see declaration of variable
@group(0) @binding(0) var<storage, read_write> U : S;
                                               ^

