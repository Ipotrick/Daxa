

#extension GL_GOOGLE_include_directive : enable
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_buffer_reference : enable
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : enable
#extension GL_KHR_shader_subgroup_basic : enable
#extension GL_KHR_shader_subgroup_vote : enable
#extension GL_KHR_shader_subgroup_arithmetic : enable
#extension GL_KHR_shader_subgroup_ballot : enable
#extension GL_KHR_shader_subgroup_shuffle : enable
#extension GL_KHR_shader_subgroup_shuffle_relative : enable
#extension GL_KHR_shader_subgroup_clustered : enable
#extension GL_KHR_shader_subgroup_quad : enable
#extension GL_EXT_scalar_block_layout : require
#extension GL_EXT_shader_image_load_formatted : require
#extension GL_EXT_control_flow_attributes : require
#extension GL_EXT_shader_image_int64 : require





#line 1 "tests/2_daxa_api/6_task_list/shaders/shared.inl"


#line 1 "include/daxa/daxa.inl"












#line 1 "include/daxa/daxa.glsl"

























































struct daxa_BufferId
{
           uint value;
};
struct daxa_ImageId
{
           uint value;
};
struct daxa_SamplerId
{
           uint value;
};


       uint daxa_id_to_index(daxa_BufferId id){ return((0x00FFFFFF)& id . value);}
       uint daxa_id_to_index(daxa_ImageId id){ return((0x00FFFFFF)& id . value);}
       uint daxa_id_to_index(daxa_SamplerId id){ return((0x00FFFFFF)& id . value);}
layout(scalar, binding = 4, set = 0)readonly buffer daxa_BufferDeviceAddressBufferBlock { uint64_t addresses[];}
daxa_buffer_device_address_buffer;
       uint64_t daxa_id_to_address(daxa_BufferId buffer_id){ return daxa_buffer_device_address_buffer . addresses[daxa_id_to_index(buffer_id)];}




layout(binding = 3, set = 0)uniform sampler daxa_SamplerTable[];




































































                        layout(binding = 2, set = 0)uniform texture1D daxa_ImageTabletexture1D[];struct daxa_Image1Df32 { daxa_ImageId id;};layout(binding = 2, set = 0)uniform itexture1D daxa_ImageTableitexture1D[];struct daxa_Image1Di32 { daxa_ImageId id;};layout(binding = 2, set = 0)uniform utexture1D daxa_ImageTableutexture1D[];struct daxa_Image1Du32 { daxa_ImageId id;};layout(binding = 1, set = 0)uniform image1D daxa_RWImageTableimage1D[];struct daxa_RWImage1Df32 { daxa_ImageId id;};layout(binding = 1, set = 0)uniform iimage1D daxa_RWImageTableiimage1D[];struct daxa_RWImage1Di32 { daxa_ImageId id;};layout(binding = 1, set = 0)uniform uimage1D daxa_RWImageTableuimage1D[];struct daxa_RWImage1Du32 { daxa_ImageId id;};layout(binding = 1, set = 0)uniform i64image1D daxa_RWImageTablei64image1D[];struct daxa_RWImage1Di64 { daxa_ImageId id;};layout(binding = 1, set = 0)uniform u64image1D daxa_RWImageTableu64image1D[];struct daxa_RWImage1Du64 { daxa_ImageId id;};
                        layout(binding = 2, set = 0)uniform texture2D daxa_ImageTabletexture2D[];struct daxa_Image2Df32 { daxa_ImageId id;};layout(binding = 2, set = 0)uniform itexture2D daxa_ImageTableitexture2D[];struct daxa_Image2Di32 { daxa_ImageId id;};layout(binding = 2, set = 0)uniform utexture2D daxa_ImageTableutexture2D[];struct daxa_Image2Du32 { daxa_ImageId id;};layout(binding = 1, set = 0)uniform image2D daxa_RWImageTableimage2D[];struct daxa_RWImage2Df32 { daxa_ImageId id;};layout(binding = 1, set = 0)uniform iimage2D daxa_RWImageTableiimage2D[];struct daxa_RWImage2Di32 { daxa_ImageId id;};layout(binding = 1, set = 0)uniform uimage2D daxa_RWImageTableuimage2D[];struct daxa_RWImage2Du32 { daxa_ImageId id;};layout(binding = 1, set = 0)uniform i64image2D daxa_RWImageTablei64image2D[];struct daxa_RWImage2Di64 { daxa_ImageId id;};layout(binding = 1, set = 0)uniform u64image2D daxa_RWImageTableu64image2D[];struct daxa_RWImage2Du64 { daxa_ImageId id;};
                        layout(binding = 2, set = 0)uniform texture3D daxa_ImageTabletexture3D[];struct daxa_Image3Df32 { daxa_ImageId id;};layout(binding = 2, set = 0)uniform itexture3D daxa_ImageTableitexture3D[];struct daxa_Image3Di32 { daxa_ImageId id;};layout(binding = 2, set = 0)uniform utexture3D daxa_ImageTableutexture3D[];struct daxa_Image3Du32 { daxa_ImageId id;};layout(binding = 1, set = 0)uniform image3D daxa_RWImageTableimage3D[];struct daxa_RWImage3Df32 { daxa_ImageId id;};layout(binding = 1, set = 0)uniform iimage3D daxa_RWImageTableiimage3D[];struct daxa_RWImage3Di32 { daxa_ImageId id;};layout(binding = 1, set = 0)uniform uimage3D daxa_RWImageTableuimage3D[];struct daxa_RWImage3Du32 { daxa_ImageId id;};layout(binding = 1, set = 0)uniform i64image3D daxa_RWImageTablei64image3D[];struct daxa_RWImage3Di64 { daxa_ImageId id;};layout(binding = 1, set = 0)uniform u64image3D daxa_RWImageTableu64image3D[];struct daxa_RWImage3Du64 { daxa_ImageId id;};
                          layout(binding = 2, set = 0)uniform textureCube daxa_ImageTabletextureCube[];struct daxa_ImageCubef32 { daxa_ImageId id;};layout(binding = 2, set = 0)uniform itextureCube daxa_ImageTableitextureCube[];struct daxa_ImageCubei32 { daxa_ImageId id;};layout(binding = 2, set = 0)uniform utextureCube daxa_ImageTableutextureCube[];struct daxa_ImageCubeu32 { daxa_ImageId id;};layout(binding = 1, set = 0)uniform imageCube daxa_RWImageTableimageCube[];struct daxa_RWImageCubef32 { daxa_ImageId id;};layout(binding = 1, set = 0)uniform iimageCube daxa_RWImageTableiimageCube[];struct daxa_RWImageCubei32 { daxa_ImageId id;};layout(binding = 1, set = 0)uniform uimageCube daxa_RWImageTableuimageCube[];struct daxa_RWImageCubeu32 { daxa_ImageId id;};layout(binding = 1, set = 0)uniform i64imageCube daxa_RWImageTablei64imageCube[];struct daxa_RWImageCubei64 { daxa_ImageId id;};layout(binding = 1, set = 0)uniform u64imageCube daxa_RWImageTableu64imageCube[];struct daxa_RWImageCubeu64 { daxa_ImageId id;};
                               layout(binding = 2, set = 0)uniform textureCubeArray daxa_ImageTabletextureCubeArray[];struct daxa_ImageCubeArrayf32 { daxa_ImageId id;};layout(binding = 2, set = 0)uniform itextureCubeArray daxa_ImageTableitextureCubeArray[];struct daxa_ImageCubeArrayi32 { daxa_ImageId id;};layout(binding = 2, set = 0)uniform utextureCubeArray daxa_ImageTableutextureCubeArray[];struct daxa_ImageCubeArrayu32 { daxa_ImageId id;};layout(binding = 1, set = 0)uniform imageCubeArray daxa_RWImageTableimageCubeArray[];struct daxa_RWImageCubeArrayf32 { daxa_ImageId id;};layout(binding = 1, set = 0)uniform iimageCubeArray daxa_RWImageTableiimageCubeArray[];struct daxa_RWImageCubeArrayi32 { daxa_ImageId id;};layout(binding = 1, set = 0)uniform uimageCubeArray daxa_RWImageTableuimageCubeArray[];struct daxa_RWImageCubeArrayu32 { daxa_ImageId id;};layout(binding = 1, set = 0)uniform i64imageCubeArray daxa_RWImageTablei64imageCubeArray[];struct daxa_RWImageCubeArrayi64 { daxa_ImageId id;};layout(binding = 1, set = 0)uniform u64imageCubeArray daxa_RWImageTableu64imageCubeArray[];struct daxa_RWImageCubeArrayu64 { daxa_ImageId id;};
                             layout(binding = 2, set = 0)uniform texture1DArray daxa_ImageTabletexture1DArray[];struct daxa_Image1DArrayf32 { daxa_ImageId id;};layout(binding = 2, set = 0)uniform itexture1DArray daxa_ImageTableitexture1DArray[];struct daxa_Image1DArrayi32 { daxa_ImageId id;};layout(binding = 2, set = 0)uniform utexture1DArray daxa_ImageTableutexture1DArray[];struct daxa_Image1DArrayu32 { daxa_ImageId id;};layout(binding = 1, set = 0)uniform image1DArray daxa_RWImageTableimage1DArray[];struct daxa_RWImage1DArrayf32 { daxa_ImageId id;};layout(binding = 1, set = 0)uniform iimage1DArray daxa_RWImageTableiimage1DArray[];struct daxa_RWImage1DArrayi32 { daxa_ImageId id;};layout(binding = 1, set = 0)uniform uimage1DArray daxa_RWImageTableuimage1DArray[];struct daxa_RWImage1DArrayu32 { daxa_ImageId id;};layout(binding = 1, set = 0)uniform i64image1DArray daxa_RWImageTablei64image1DArray[];struct daxa_RWImage1DArrayi64 { daxa_ImageId id;};layout(binding = 1, set = 0)uniform u64image1DArray daxa_RWImageTableu64image1DArray[];struct daxa_RWImage1DArrayu64 { daxa_ImageId id;};
                             layout(binding = 2, set = 0)uniform texture2DArray daxa_ImageTabletexture2DArray[];struct daxa_Image2DArrayf32 { daxa_ImageId id;};layout(binding = 2, set = 0)uniform itexture2DArray daxa_ImageTableitexture2DArray[];struct daxa_Image2DArrayi32 { daxa_ImageId id;};layout(binding = 2, set = 0)uniform utexture2DArray daxa_ImageTableutexture2DArray[];struct daxa_Image2DArrayu32 { daxa_ImageId id;};layout(binding = 1, set = 0)uniform image2DArray daxa_RWImageTableimage2DArray[];struct daxa_RWImage2DArrayf32 { daxa_ImageId id;};layout(binding = 1, set = 0)uniform iimage2DArray daxa_RWImageTableiimage2DArray[];struct daxa_RWImage2DArrayi32 { daxa_ImageId id;};layout(binding = 1, set = 0)uniform uimage2DArray daxa_RWImageTableuimage2DArray[];struct daxa_RWImage2DArrayu32 { daxa_ImageId id;};layout(binding = 1, set = 0)uniform i64image2DArray daxa_RWImageTablei64image2DArray[];struct daxa_RWImage2DArrayi64 { daxa_ImageId id;};layout(binding = 1, set = 0)uniform u64image2DArray daxa_RWImageTableu64image2DArray[];struct daxa_RWImage2DArrayu64 { daxa_ImageId id;};
                          layout(binding = 2, set = 0)uniform texture2DMS daxa_ImageTabletexture2DMS[];struct daxa_Image2DMSf32 { daxa_ImageId id;};layout(binding = 2, set = 0)uniform itexture2DMS daxa_ImageTableitexture2DMS[];struct daxa_Image2DMSi32 { daxa_ImageId id;};layout(binding = 2, set = 0)uniform utexture2DMS daxa_ImageTableutexture2DMS[];struct daxa_Image2DMSu32 { daxa_ImageId id;};layout(binding = 1, set = 0)uniform image2DMS daxa_RWImageTableimage2DMS[];struct daxa_RWImage2DMSf32 { daxa_ImageId id;};layout(binding = 1, set = 0)uniform iimage2DMS daxa_RWImageTableiimage2DMS[];struct daxa_RWImage2DMSi32 { daxa_ImageId id;};layout(binding = 1, set = 0)uniform uimage2DMS daxa_RWImageTableuimage2DMS[];struct daxa_RWImage2DMSu32 { daxa_ImageId id;};layout(binding = 1, set = 0)uniform i64image2DMS daxa_RWImageTablei64image2DMS[];struct daxa_RWImage2DMSi64 { daxa_ImageId id;};layout(binding = 1, set = 0)uniform u64image2DMS daxa_RWImageTableu64image2DMS[];struct daxa_RWImage2DMSu64 { daxa_ImageId id;};
                               layout(binding = 2, set = 0)uniform texture2DMSArray daxa_ImageTabletexture2DMSArray[];struct daxa_Image2DMSArrayf32 { daxa_ImageId id;};layout(binding = 2, set = 0)uniform itexture2DMSArray daxa_ImageTableitexture2DMSArray[];struct daxa_Image2DMSArrayi32 { daxa_ImageId id;};layout(binding = 2, set = 0)uniform utexture2DMSArray daxa_ImageTableutexture2DMSArray[];struct daxa_Image2DMSArrayu32 { daxa_ImageId id;};layout(binding = 1, set = 0)uniform image2DMSArray daxa_RWImageTableimage2DMSArray[];struct daxa_RWImage2DMSArrayf32 { daxa_ImageId id;};layout(binding = 1, set = 0)uniform iimage2DMSArray daxa_RWImageTableiimage2DMSArray[];struct daxa_RWImage2DMSArrayi32 { daxa_ImageId id;};layout(binding = 1, set = 0)uniform uimage2DMSArray daxa_RWImageTableuimage2DMSArray[];struct daxa_RWImage2DMSArrayu32 { daxa_ImageId id;};layout(binding = 1, set = 0)uniform i64image2DMSArray daxa_RWImageTablei64image2DMSArray[];struct daxa_RWImage2DMSArrayi64 { daxa_ImageId id;};layout(binding = 1, set = 0)uniform u64image2DMSArray daxa_RWImageTableu64image2DMSArray[];struct daxa_RWImage2DMSArrayu64 { daxa_ImageId id;};




























































                                     vec4 imageLoad(daxa_RWImage1Df32 image, int index){ return imageLoad(daxa_RWImageTableimage1D[daxa_id_to_index(image . id)], index);} void imageStore(daxa_RWImage1Df32 image, int index, vec4 data){ imageStore(daxa_RWImageTableimage1D[daxa_id_to_index(image . id)], index, data);} int imageSize(daxa_RWImage1Df32 image){ return imageSize(daxa_RWImageTableimage1D[daxa_id_to_index(image . id)]);} ivec4 imageLoad(daxa_RWImage1Di32 image, int index){ return imageLoad(daxa_RWImageTableiimage1D[daxa_id_to_index(image . id)], index);} void imageStore(daxa_RWImage1Di32 image, int index, ivec4 data){ imageStore(daxa_RWImageTableiimage1D[daxa_id_to_index(image . id)], index, data);} int imageSize(daxa_RWImage1Di32 image){ return imageSize(daxa_RWImageTableiimage1D[daxa_id_to_index(image . id)]);} uvec4 imageLoad(daxa_RWImage1Du32 image, int index){ return imageLoad(daxa_RWImageTableuimage1D[daxa_id_to_index(image . id)], index);} void imageStore(daxa_RWImage1Du32 image, int index, uvec4 data){ imageStore(daxa_RWImageTableuimage1D[daxa_id_to_index(image . id)], index, data);} int imageSize(daxa_RWImage1Du32 image){ return imageSize(daxa_RWImageTableuimage1D[daxa_id_to_index(image . id)]);}
                                     vec4 imageLoad(daxa_RWImage2Df32 image, ivec2 index){ return imageLoad(daxa_RWImageTableimage2D[daxa_id_to_index(image . id)], index);} void imageStore(daxa_RWImage2Df32 image, ivec2 index, vec4 data){ imageStore(daxa_RWImageTableimage2D[daxa_id_to_index(image . id)], index, data);} ivec2 imageSize(daxa_RWImage2Df32 image){ return imageSize(daxa_RWImageTableimage2D[daxa_id_to_index(image . id)]);} ivec4 imageLoad(daxa_RWImage2Di32 image, ivec2 index){ return imageLoad(daxa_RWImageTableiimage2D[daxa_id_to_index(image . id)], index);} void imageStore(daxa_RWImage2Di32 image, ivec2 index, ivec4 data){ imageStore(daxa_RWImageTableiimage2D[daxa_id_to_index(image . id)], index, data);} ivec2 imageSize(daxa_RWImage2Di32 image){ return imageSize(daxa_RWImageTableiimage2D[daxa_id_to_index(image . id)]);} uvec4 imageLoad(daxa_RWImage2Du32 image, ivec2 index){ return imageLoad(daxa_RWImageTableuimage2D[daxa_id_to_index(image . id)], index);} void imageStore(daxa_RWImage2Du32 image, ivec2 index, uvec4 data){ imageStore(daxa_RWImageTableuimage2D[daxa_id_to_index(image . id)], index, data);} ivec2 imageSize(daxa_RWImage2Du32 image){ return imageSize(daxa_RWImageTableuimage2D[daxa_id_to_index(image . id)]);}
                                     vec4 imageLoad(daxa_RWImage3Df32 image, ivec3 index){ return imageLoad(daxa_RWImageTableimage3D[daxa_id_to_index(image . id)], index);} void imageStore(daxa_RWImage3Df32 image, ivec3 index, vec4 data){ imageStore(daxa_RWImageTableimage3D[daxa_id_to_index(image . id)], index, data);} ivec3 imageSize(daxa_RWImage3Df32 image){ return imageSize(daxa_RWImageTableimage3D[daxa_id_to_index(image . id)]);} ivec4 imageLoad(daxa_RWImage3Di32 image, ivec3 index){ return imageLoad(daxa_RWImageTableiimage3D[daxa_id_to_index(image . id)], index);} void imageStore(daxa_RWImage3Di32 image, ivec3 index, ivec4 data){ imageStore(daxa_RWImageTableiimage3D[daxa_id_to_index(image . id)], index, data);} ivec3 imageSize(daxa_RWImage3Di32 image){ return imageSize(daxa_RWImageTableiimage3D[daxa_id_to_index(image . id)]);} uvec4 imageLoad(daxa_RWImage3Du32 image, ivec3 index){ return imageLoad(daxa_RWImageTableuimage3D[daxa_id_to_index(image . id)], index);} void imageStore(daxa_RWImage3Du32 image, ivec3 index, uvec4 data){ imageStore(daxa_RWImageTableuimage3D[daxa_id_to_index(image . id)], index, data);} ivec3 imageSize(daxa_RWImage3Du32 image){ return imageSize(daxa_RWImageTableuimage3D[daxa_id_to_index(image . id)]);}
                                       vec4 imageLoad(daxa_RWImageCubef32 image, ivec3 index){ return imageLoad(daxa_RWImageTableimageCube[daxa_id_to_index(image . id)], index);} void imageStore(daxa_RWImageCubef32 image, ivec3 index, vec4 data){ imageStore(daxa_RWImageTableimageCube[daxa_id_to_index(image . id)], index, data);} ivec2 imageSize(daxa_RWImageCubef32 image){ return imageSize(daxa_RWImageTableimageCube[daxa_id_to_index(image . id)]);} ivec4 imageLoad(daxa_RWImageCubei32 image, ivec3 index){ return imageLoad(daxa_RWImageTableiimageCube[daxa_id_to_index(image . id)], index);} void imageStore(daxa_RWImageCubei32 image, ivec3 index, ivec4 data){ imageStore(daxa_RWImageTableiimageCube[daxa_id_to_index(image . id)], index, data);} ivec2 imageSize(daxa_RWImageCubei32 image){ return imageSize(daxa_RWImageTableiimageCube[daxa_id_to_index(image . id)]);} uvec4 imageLoad(daxa_RWImageCubeu32 image, ivec3 index){ return imageLoad(daxa_RWImageTableuimageCube[daxa_id_to_index(image . id)], index);} void imageStore(daxa_RWImageCubeu32 image, ivec3 index, uvec4 data){ imageStore(daxa_RWImageTableuimageCube[daxa_id_to_index(image . id)], index, data);} ivec2 imageSize(daxa_RWImageCubeu32 image){ return imageSize(daxa_RWImageTableuimageCube[daxa_id_to_index(image . id)]);}
                                            vec4 imageLoad(daxa_RWImageCubeArrayf32 image, ivec3 index){ return imageLoad(daxa_RWImageTableimageCubeArray[daxa_id_to_index(image . id)], index);} void imageStore(daxa_RWImageCubeArrayf32 image, ivec3 index, vec4 data){ imageStore(daxa_RWImageTableimageCubeArray[daxa_id_to_index(image . id)], index, data);} ivec3 imageSize(daxa_RWImageCubeArrayf32 image){ return imageSize(daxa_RWImageTableimageCubeArray[daxa_id_to_index(image . id)]);} ivec4 imageLoad(daxa_RWImageCubeArrayi32 image, ivec3 index){ return imageLoad(daxa_RWImageTableiimageCubeArray[daxa_id_to_index(image . id)], index);} void imageStore(daxa_RWImageCubeArrayi32 image, ivec3 index, ivec4 data){ imageStore(daxa_RWImageTableiimageCubeArray[daxa_id_to_index(image . id)], index, data);} ivec3 imageSize(daxa_RWImageCubeArrayi32 image){ return imageSize(daxa_RWImageTableiimageCubeArray[daxa_id_to_index(image . id)]);} uvec4 imageLoad(daxa_RWImageCubeArrayu32 image, ivec3 index){ return imageLoad(daxa_RWImageTableuimageCubeArray[daxa_id_to_index(image . id)], index);} void imageStore(daxa_RWImageCubeArrayu32 image, ivec3 index, uvec4 data){ imageStore(daxa_RWImageTableuimageCubeArray[daxa_id_to_index(image . id)], index, data);} ivec3 imageSize(daxa_RWImageCubeArrayu32 image){ return imageSize(daxa_RWImageTableuimageCubeArray[daxa_id_to_index(image . id)]);}
                                          vec4 imageLoad(daxa_RWImage1DArrayf32 image, ivec2 index){ return imageLoad(daxa_RWImageTableimage1DArray[daxa_id_to_index(image . id)], index);} void imageStore(daxa_RWImage1DArrayf32 image, ivec2 index, vec4 data){ imageStore(daxa_RWImageTableimage1DArray[daxa_id_to_index(image . id)], index, data);} ivec2 imageSize(daxa_RWImage1DArrayf32 image){ return imageSize(daxa_RWImageTableimage1DArray[daxa_id_to_index(image . id)]);} ivec4 imageLoad(daxa_RWImage1DArrayi32 image, ivec2 index){ return imageLoad(daxa_RWImageTableiimage1DArray[daxa_id_to_index(image . id)], index);} void imageStore(daxa_RWImage1DArrayi32 image, ivec2 index, ivec4 data){ imageStore(daxa_RWImageTableiimage1DArray[daxa_id_to_index(image . id)], index, data);} ivec2 imageSize(daxa_RWImage1DArrayi32 image){ return imageSize(daxa_RWImageTableiimage1DArray[daxa_id_to_index(image . id)]);} uvec4 imageLoad(daxa_RWImage1DArrayu32 image, ivec2 index){ return imageLoad(daxa_RWImageTableuimage1DArray[daxa_id_to_index(image . id)], index);} void imageStore(daxa_RWImage1DArrayu32 image, ivec2 index, uvec4 data){ imageStore(daxa_RWImageTableuimage1DArray[daxa_id_to_index(image . id)], index, data);} ivec2 imageSize(daxa_RWImage1DArrayu32 image){ return imageSize(daxa_RWImageTableuimage1DArray[daxa_id_to_index(image . id)]);}
                                          vec4 imageLoad(daxa_RWImage2DArrayf32 image, ivec3 index){ return imageLoad(daxa_RWImageTableimage2DArray[daxa_id_to_index(image . id)], index);} void imageStore(daxa_RWImage2DArrayf32 image, ivec3 index, vec4 data){ imageStore(daxa_RWImageTableimage2DArray[daxa_id_to_index(image . id)], index, data);} ivec3 imageSize(daxa_RWImage2DArrayf32 image){ return imageSize(daxa_RWImageTableimage2DArray[daxa_id_to_index(image . id)]);} ivec4 imageLoad(daxa_RWImage2DArrayi32 image, ivec3 index){ return imageLoad(daxa_RWImageTableiimage2DArray[daxa_id_to_index(image . id)], index);} void imageStore(daxa_RWImage2DArrayi32 image, ivec3 index, ivec4 data){ imageStore(daxa_RWImageTableiimage2DArray[daxa_id_to_index(image . id)], index, data);} ivec3 imageSize(daxa_RWImage2DArrayi32 image){ return imageSize(daxa_RWImageTableiimage2DArray[daxa_id_to_index(image . id)]);} uvec4 imageLoad(daxa_RWImage2DArrayu32 image, ivec3 index){ return imageLoad(daxa_RWImageTableuimage2DArray[daxa_id_to_index(image . id)], index);} void imageStore(daxa_RWImage2DArrayu32 image, ivec3 index, uvec4 data){ imageStore(daxa_RWImageTableuimage2DArray[daxa_id_to_index(image . id)], index, data);} ivec3 imageSize(daxa_RWImage2DArrayu32 image){ return imageSize(daxa_RWImageTableuimage2DArray[daxa_id_to_index(image . id)]);}



                                       vec4 imageLoad(daxa_RWImage2DMSf32 image, ivec2 index, int s){ return imageLoad(daxa_RWImageTableimage2DMS[daxa_id_to_index(image . id)], index, s);} void imageStore(daxa_RWImage2DMSf32 image, ivec2 index, int s, vec4 data){ imageStore(daxa_RWImageTableimage2DMS[daxa_id_to_index(image . id)], index, s, data);} ivec2 imageSize(daxa_RWImage2DMSf32 image){ return imageSize(daxa_RWImageTableimage2DMS[daxa_id_to_index(image . id)]);} ivec4 imageLoad(daxa_RWImage2DMSi32 image, ivec2 index, int s){ return imageLoad(daxa_RWImageTableiimage2DMS[daxa_id_to_index(image . id)], index, s);} void imageStore(daxa_RWImage2DMSi32 image, ivec2 index, int s, ivec4 data){ imageStore(daxa_RWImageTableiimage2DMS[daxa_id_to_index(image . id)], index, s, data);} ivec2 imageSize(daxa_RWImage2DMSi32 image){ return imageSize(daxa_RWImageTableiimage2DMS[daxa_id_to_index(image . id)]);} uvec4 imageLoad(daxa_RWImage2DMSu32 image, ivec2 index, int s){ return imageLoad(daxa_RWImageTableuimage2DMS[daxa_id_to_index(image . id)], index, s);} void imageStore(daxa_RWImage2DMSu32 image, ivec2 index, int s, uvec4 data){ imageStore(daxa_RWImageTableuimage2DMS[daxa_id_to_index(image . id)], index, s, data);} ivec2 imageSize(daxa_RWImage2DMSu32 image){ return imageSize(daxa_RWImageTableuimage2DMS[daxa_id_to_index(image . id)]);}
                                            vec4 imageLoad(daxa_RWImage2DMSArrayf32 image, ivec3 index, int s){ return imageLoad(daxa_RWImageTableimage2DMSArray[daxa_id_to_index(image . id)], index, s);} void imageStore(daxa_RWImage2DMSArrayf32 image, ivec3 index, int s, vec4 data){ imageStore(daxa_RWImageTableimage2DMSArray[daxa_id_to_index(image . id)], index, s, data);} ivec3 imageSize(daxa_RWImage2DMSArrayf32 image){ return imageSize(daxa_RWImageTableimage2DMSArray[daxa_id_to_index(image . id)]);} ivec4 imageLoad(daxa_RWImage2DMSArrayi32 image, ivec3 index, int s){ return imageLoad(daxa_RWImageTableiimage2DMSArray[daxa_id_to_index(image . id)], index, s);} void imageStore(daxa_RWImage2DMSArrayi32 image, ivec3 index, int s, ivec4 data){ imageStore(daxa_RWImageTableiimage2DMSArray[daxa_id_to_index(image . id)], index, s, data);} ivec3 imageSize(daxa_RWImage2DMSArrayi32 image){ return imageSize(daxa_RWImageTableiimage2DMSArray[daxa_id_to_index(image . id)]);} uvec4 imageLoad(daxa_RWImage2DMSArrayu32 image, ivec3 index, int s){ return imageLoad(daxa_RWImageTableuimage2DMSArray[daxa_id_to_index(image . id)], index, s);} void imageStore(daxa_RWImage2DMSArrayu32 image, ivec3 index, int s, uvec4 data){ imageStore(daxa_RWImageTableuimage2DMSArray[daxa_id_to_index(image . id)], index, s, data);} ivec3 imageSize(daxa_RWImage2DMSArrayu32 image){ return imageSize(daxa_RWImageTableuimage2DMSArray[daxa_id_to_index(image . id)]);}








                                           i64vec4 imageLoad(daxa_RWImage1Di64 image, int index){ return imageLoad(daxa_RWImageTablei64image1D[daxa_id_to_index(image . id)], index);} void imageStore(daxa_RWImage1Di64 image, int index, i64vec4 data){ imageStore(daxa_RWImageTablei64image1D[daxa_id_to_index(image . id)], index, data);} int imageSize(daxa_RWImage1Di64 image){ return imageSize(daxa_RWImageTablei64image1D[daxa_id_to_index(image . id)]);} u64vec4 imageLoad(daxa_RWImage1Du64 image, int index){ return imageLoad(daxa_RWImageTableu64image1D[daxa_id_to_index(image . id)], index);} void imageStore(daxa_RWImage1Du64 image, int index, u64vec4 data){ imageStore(daxa_RWImageTableu64image1D[daxa_id_to_index(image . id)], index, data);} int imageSize(daxa_RWImage1Du64 image){ return imageSize(daxa_RWImageTableu64image1D[daxa_id_to_index(image . id)]);}
                                           i64vec4 imageLoad(daxa_RWImage2Di64 image, ivec2 index){ return imageLoad(daxa_RWImageTablei64image2D[daxa_id_to_index(image . id)], index);} void imageStore(daxa_RWImage2Di64 image, ivec2 index, i64vec4 data){ imageStore(daxa_RWImageTablei64image2D[daxa_id_to_index(image . id)], index, data);} ivec2 imageSize(daxa_RWImage2Di64 image){ return imageSize(daxa_RWImageTablei64image2D[daxa_id_to_index(image . id)]);} u64vec4 imageLoad(daxa_RWImage2Du64 image, ivec2 index){ return imageLoad(daxa_RWImageTableu64image2D[daxa_id_to_index(image . id)], index);} void imageStore(daxa_RWImage2Du64 image, ivec2 index, u64vec4 data){ imageStore(daxa_RWImageTableu64image2D[daxa_id_to_index(image . id)], index, data);} ivec2 imageSize(daxa_RWImage2Du64 image){ return imageSize(daxa_RWImageTableu64image2D[daxa_id_to_index(image . id)]);}
                                           i64vec4 imageLoad(daxa_RWImage3Di64 image, ivec3 index){ return imageLoad(daxa_RWImageTablei64image3D[daxa_id_to_index(image . id)], index);} void imageStore(daxa_RWImage3Di64 image, ivec3 index, i64vec4 data){ imageStore(daxa_RWImageTablei64image3D[daxa_id_to_index(image . id)], index, data);} ivec3 imageSize(daxa_RWImage3Di64 image){ return imageSize(daxa_RWImageTablei64image3D[daxa_id_to_index(image . id)]);} u64vec4 imageLoad(daxa_RWImage3Du64 image, ivec3 index){ return imageLoad(daxa_RWImageTableu64image3D[daxa_id_to_index(image . id)], index);} void imageStore(daxa_RWImage3Du64 image, ivec3 index, u64vec4 data){ imageStore(daxa_RWImageTableu64image3D[daxa_id_to_index(image . id)], index, data);} ivec3 imageSize(daxa_RWImage3Du64 image){ return imageSize(daxa_RWImageTableu64image3D[daxa_id_to_index(image . id)]);}
                                             i64vec4 imageLoad(daxa_RWImageCubei64 image, ivec3 index){ return imageLoad(daxa_RWImageTablei64imageCube[daxa_id_to_index(image . id)], index);} void imageStore(daxa_RWImageCubei64 image, ivec3 index, i64vec4 data){ imageStore(daxa_RWImageTablei64imageCube[daxa_id_to_index(image . id)], index, data);} ivec2 imageSize(daxa_RWImageCubei64 image){ return imageSize(daxa_RWImageTablei64imageCube[daxa_id_to_index(image . id)]);} u64vec4 imageLoad(daxa_RWImageCubeu64 image, ivec3 index){ return imageLoad(daxa_RWImageTableu64imageCube[daxa_id_to_index(image . id)], index);} void imageStore(daxa_RWImageCubeu64 image, ivec3 index, u64vec4 data){ imageStore(daxa_RWImageTableu64imageCube[daxa_id_to_index(image . id)], index, data);} ivec2 imageSize(daxa_RWImageCubeu64 image){ return imageSize(daxa_RWImageTableu64imageCube[daxa_id_to_index(image . id)]);}
                                                  i64vec4 imageLoad(daxa_RWImageCubeArrayi64 image, ivec3 index){ return imageLoad(daxa_RWImageTablei64imageCubeArray[daxa_id_to_index(image . id)], index);} void imageStore(daxa_RWImageCubeArrayi64 image, ivec3 index, i64vec4 data){ imageStore(daxa_RWImageTablei64imageCubeArray[daxa_id_to_index(image . id)], index, data);} ivec3 imageSize(daxa_RWImageCubeArrayi64 image){ return imageSize(daxa_RWImageTablei64imageCubeArray[daxa_id_to_index(image . id)]);} u64vec4 imageLoad(daxa_RWImageCubeArrayu64 image, ivec3 index){ return imageLoad(daxa_RWImageTableu64imageCubeArray[daxa_id_to_index(image . id)], index);} void imageStore(daxa_RWImageCubeArrayu64 image, ivec3 index, u64vec4 data){ imageStore(daxa_RWImageTableu64imageCubeArray[daxa_id_to_index(image . id)], index, data);} ivec3 imageSize(daxa_RWImageCubeArrayu64 image){ return imageSize(daxa_RWImageTableu64imageCubeArray[daxa_id_to_index(image . id)]);}
                                                i64vec4 imageLoad(daxa_RWImage1DArrayi64 image, ivec2 index){ return imageLoad(daxa_RWImageTablei64image1DArray[daxa_id_to_index(image . id)], index);} void imageStore(daxa_RWImage1DArrayi64 image, ivec2 index, i64vec4 data){ imageStore(daxa_RWImageTablei64image1DArray[daxa_id_to_index(image . id)], index, data);} ivec2 imageSize(daxa_RWImage1DArrayi64 image){ return imageSize(daxa_RWImageTablei64image1DArray[daxa_id_to_index(image . id)]);} u64vec4 imageLoad(daxa_RWImage1DArrayu64 image, ivec2 index){ return imageLoad(daxa_RWImageTableu64image1DArray[daxa_id_to_index(image . id)], index);} void imageStore(daxa_RWImage1DArrayu64 image, ivec2 index, u64vec4 data){ imageStore(daxa_RWImageTableu64image1DArray[daxa_id_to_index(image . id)], index, data);} ivec2 imageSize(daxa_RWImage1DArrayu64 image){ return imageSize(daxa_RWImageTableu64image1DArray[daxa_id_to_index(image . id)]);}
                                                i64vec4 imageLoad(daxa_RWImage2DArrayi64 image, ivec3 index){ return imageLoad(daxa_RWImageTablei64image2DArray[daxa_id_to_index(image . id)], index);} void imageStore(daxa_RWImage2DArrayi64 image, ivec3 index, i64vec4 data){ imageStore(daxa_RWImageTablei64image2DArray[daxa_id_to_index(image . id)], index, data);} ivec3 imageSize(daxa_RWImage2DArrayi64 image){ return imageSize(daxa_RWImageTablei64image2DArray[daxa_id_to_index(image . id)]);} u64vec4 imageLoad(daxa_RWImage2DArrayu64 image, ivec3 index){ return imageLoad(daxa_RWImageTableu64image2DArray[daxa_id_to_index(image . id)], index);} void imageStore(daxa_RWImage2DArrayu64 image, ivec3 index, u64vec4 data){ imageStore(daxa_RWImageTableu64image2DArray[daxa_id_to_index(image . id)], index, data);} ivec3 imageSize(daxa_RWImage2DArrayu64 image){ return imageSize(daxa_RWImageTableu64image2DArray[daxa_id_to_index(image . id)]);}



                                             i64vec4 imageLoad(daxa_RWImage2DMSi64 image, ivec2 index, int s){ return imageLoad(daxa_RWImageTablei64image2DMS[daxa_id_to_index(image . id)], index, s);} void imageStore(daxa_RWImage2DMSi64 image, ivec2 index, int s, i64vec4 data){ imageStore(daxa_RWImageTablei64image2DMS[daxa_id_to_index(image . id)], index, s, data);} ivec2 imageSize(daxa_RWImage2DMSi64 image){ return imageSize(daxa_RWImageTablei64image2DMS[daxa_id_to_index(image . id)]);} u64vec4 imageLoad(daxa_RWImage2DMSu64 image, ivec2 index, int s){ return imageLoad(daxa_RWImageTableu64image2DMS[daxa_id_to_index(image . id)], index, s);} void imageStore(daxa_RWImage2DMSu64 image, ivec2 index, int s, u64vec4 data){ imageStore(daxa_RWImageTableu64image2DMS[daxa_id_to_index(image . id)], index, s, data);} ivec2 imageSize(daxa_RWImage2DMSu64 image){ return imageSize(daxa_RWImageTableu64image2DMS[daxa_id_to_index(image . id)]);}
                                                  i64vec4 imageLoad(daxa_RWImage2DMSArrayi64 image, ivec3 index, int s){ return imageLoad(daxa_RWImageTablei64image2DMSArray[daxa_id_to_index(image . id)], index, s);} void imageStore(daxa_RWImage2DMSArrayi64 image, ivec3 index, int s, i64vec4 data){ imageStore(daxa_RWImageTablei64image2DMSArray[daxa_id_to_index(image . id)], index, s, data);} ivec3 imageSize(daxa_RWImage2DMSArrayi64 image){ return imageSize(daxa_RWImageTablei64image2DMSArray[daxa_id_to_index(image . id)]);} u64vec4 imageLoad(daxa_RWImage2DMSArrayu64 image, ivec3 index, int s){ return imageLoad(daxa_RWImageTableu64image2DMSArray[daxa_id_to_index(image . id)], index, s);} void imageStore(daxa_RWImage2DMSArrayu64 image, ivec3 index, int s, u64vec4 data){ imageStore(daxa_RWImageTableu64image2DMSArray[daxa_id_to_index(image . id)], index, s, data);} ivec3 imageSize(daxa_RWImage2DMSArrayu64 image){ return imageSize(daxa_RWImageTableu64image2DMSArray[daxa_id_to_index(image . id)]);}









                                          layout(binding = 1, set = 0, r32i)uniform iimage1D daxa_AtomicImageTableiimage1D[];int imageAtomicCompSwap(daxa_RWImage1Di32 image, int index, int compare, int data){ return imageAtomicCompSwap(daxa_AtomicImageTableiimage1D[daxa_id_to_index(image . id)], index, compare, data);} int imageAtomicExchange(daxa_RWImage1Di32 image, int index, int data){ return imageAtomicExchange(daxa_AtomicImageTableiimage1D[daxa_id_to_index(image . id)], index, data);} int imageAtomicAdd(daxa_RWImage1Di32 image, int index, int data){ return imageAtomicAdd(daxa_AtomicImageTableiimage1D[daxa_id_to_index(image . id)], index, data);} int imageAtomicAnd(daxa_RWImage1Di32 image, int index, int data){ return imageAtomicAnd(daxa_AtomicImageTableiimage1D[daxa_id_to_index(image . id)], index, data);} int imageAtomicOr(daxa_RWImage1Di32 image, int index, int data){ return imageAtomicOr(daxa_AtomicImageTableiimage1D[daxa_id_to_index(image . id)], index, data);} int imageAtomicXor(daxa_RWImage1Di32 image, int index, int data){ return imageAtomicXor(daxa_AtomicImageTableiimage1D[daxa_id_to_index(image . id)], index, data);} int imageAtomicMin(daxa_RWImage1Di32 image, int index, int data){ return imageAtomicMin(daxa_AtomicImageTableiimage1D[daxa_id_to_index(image . id)], index, data);} int imageAtomicMax(daxa_RWImage1Di32 image, int index, int data){ return imageAtomicMax(daxa_AtomicImageTableiimage1D[daxa_id_to_index(image . id)], index, data);} layout(binding = 1, set = 0, r32ui)uniform uimage1D daxa_AtomicImageTableuimage1D[];uint imageAtomicCompSwap(daxa_RWImage1Du32 image, int index, uint compare, uint data){ return imageAtomicCompSwap(daxa_AtomicImageTableuimage1D[daxa_id_to_index(image . id)], index, compare, data);} uint imageAtomicExchange(daxa_RWImage1Du32 image, int index, uint data){ return imageAtomicExchange(daxa_AtomicImageTableuimage1D[daxa_id_to_index(image . id)], index, data);} uint imageAtomicAdd(daxa_RWImage1Du32 image, int index, uint data){ return imageAtomicAdd(daxa_AtomicImageTableuimage1D[daxa_id_to_index(image . id)], index, data);} uint imageAtomicAnd(daxa_RWImage1Du32 image, int index, uint data){ return imageAtomicAnd(daxa_AtomicImageTableuimage1D[daxa_id_to_index(image . id)], index, data);} uint imageAtomicOr(daxa_RWImage1Du32 image, int index, uint data){ return imageAtomicOr(daxa_AtomicImageTableuimage1D[daxa_id_to_index(image . id)], index, data);} uint imageAtomicXor(daxa_RWImage1Du32 image, int index, uint data){ return imageAtomicXor(daxa_AtomicImageTableuimage1D[daxa_id_to_index(image . id)], index, data);} uint imageAtomicMin(daxa_RWImage1Du32 image, int index, uint data){ return imageAtomicMin(daxa_AtomicImageTableuimage1D[daxa_id_to_index(image . id)], index, data);} uint imageAtomicMax(daxa_RWImage1Du32 image, int index, uint data){ return imageAtomicMax(daxa_AtomicImageTableuimage1D[daxa_id_to_index(image . id)], index, data);}
                                          layout(binding = 1, set = 0, r32i)uniform iimage2D daxa_AtomicImageTableiimage2D[];int imageAtomicCompSwap(daxa_RWImage2Di32 image, ivec2 index, int compare, int data){ return imageAtomicCompSwap(daxa_AtomicImageTableiimage2D[daxa_id_to_index(image . id)], index, compare, data);} int imageAtomicExchange(daxa_RWImage2Di32 image, ivec2 index, int data){ return imageAtomicExchange(daxa_AtomicImageTableiimage2D[daxa_id_to_index(image . id)], index, data);} int imageAtomicAdd(daxa_RWImage2Di32 image, ivec2 index, int data){ return imageAtomicAdd(daxa_AtomicImageTableiimage2D[daxa_id_to_index(image . id)], index, data);} int imageAtomicAnd(daxa_RWImage2Di32 image, ivec2 index, int data){ return imageAtomicAnd(daxa_AtomicImageTableiimage2D[daxa_id_to_index(image . id)], index, data);} int imageAtomicOr(daxa_RWImage2Di32 image, ivec2 index, int data){ return imageAtomicOr(daxa_AtomicImageTableiimage2D[daxa_id_to_index(image . id)], index, data);} int imageAtomicXor(daxa_RWImage2Di32 image, ivec2 index, int data){ return imageAtomicXor(daxa_AtomicImageTableiimage2D[daxa_id_to_index(image . id)], index, data);} int imageAtomicMin(daxa_RWImage2Di32 image, ivec2 index, int data){ return imageAtomicMin(daxa_AtomicImageTableiimage2D[daxa_id_to_index(image . id)], index, data);} int imageAtomicMax(daxa_RWImage2Di32 image, ivec2 index, int data){ return imageAtomicMax(daxa_AtomicImageTableiimage2D[daxa_id_to_index(image . id)], index, data);} layout(binding = 1, set = 0, r32ui)uniform uimage2D daxa_AtomicImageTableuimage2D[];uint imageAtomicCompSwap(daxa_RWImage2Du32 image, ivec2 index, uint compare, uint data){ return imageAtomicCompSwap(daxa_AtomicImageTableuimage2D[daxa_id_to_index(image . id)], index, compare, data);} uint imageAtomicExchange(daxa_RWImage2Du32 image, ivec2 index, uint data){ return imageAtomicExchange(daxa_AtomicImageTableuimage2D[daxa_id_to_index(image . id)], index, data);} uint imageAtomicAdd(daxa_RWImage2Du32 image, ivec2 index, uint data){ return imageAtomicAdd(daxa_AtomicImageTableuimage2D[daxa_id_to_index(image . id)], index, data);} uint imageAtomicAnd(daxa_RWImage2Du32 image, ivec2 index, uint data){ return imageAtomicAnd(daxa_AtomicImageTableuimage2D[daxa_id_to_index(image . id)], index, data);} uint imageAtomicOr(daxa_RWImage2Du32 image, ivec2 index, uint data){ return imageAtomicOr(daxa_AtomicImageTableuimage2D[daxa_id_to_index(image . id)], index, data);} uint imageAtomicXor(daxa_RWImage2Du32 image, ivec2 index, uint data){ return imageAtomicXor(daxa_AtomicImageTableuimage2D[daxa_id_to_index(image . id)], index, data);} uint imageAtomicMin(daxa_RWImage2Du32 image, ivec2 index, uint data){ return imageAtomicMin(daxa_AtomicImageTableuimage2D[daxa_id_to_index(image . id)], index, data);} uint imageAtomicMax(daxa_RWImage2Du32 image, ivec2 index, uint data){ return imageAtomicMax(daxa_AtomicImageTableuimage2D[daxa_id_to_index(image . id)], index, data);}
                                          layout(binding = 1, set = 0, r32i)uniform iimage3D daxa_AtomicImageTableiimage3D[];int imageAtomicCompSwap(daxa_RWImage3Di32 image, ivec3 index, int compare, int data){ return imageAtomicCompSwap(daxa_AtomicImageTableiimage3D[daxa_id_to_index(image . id)], index, compare, data);} int imageAtomicExchange(daxa_RWImage3Di32 image, ivec3 index, int data){ return imageAtomicExchange(daxa_AtomicImageTableiimage3D[daxa_id_to_index(image . id)], index, data);} int imageAtomicAdd(daxa_RWImage3Di32 image, ivec3 index, int data){ return imageAtomicAdd(daxa_AtomicImageTableiimage3D[daxa_id_to_index(image . id)], index, data);} int imageAtomicAnd(daxa_RWImage3Di32 image, ivec3 index, int data){ return imageAtomicAnd(daxa_AtomicImageTableiimage3D[daxa_id_to_index(image . id)], index, data);} int imageAtomicOr(daxa_RWImage3Di32 image, ivec3 index, int data){ return imageAtomicOr(daxa_AtomicImageTableiimage3D[daxa_id_to_index(image . id)], index, data);} int imageAtomicXor(daxa_RWImage3Di32 image, ivec3 index, int data){ return imageAtomicXor(daxa_AtomicImageTableiimage3D[daxa_id_to_index(image . id)], index, data);} int imageAtomicMin(daxa_RWImage3Di32 image, ivec3 index, int data){ return imageAtomicMin(daxa_AtomicImageTableiimage3D[daxa_id_to_index(image . id)], index, data);} int imageAtomicMax(daxa_RWImage3Di32 image, ivec3 index, int data){ return imageAtomicMax(daxa_AtomicImageTableiimage3D[daxa_id_to_index(image . id)], index, data);} layout(binding = 1, set = 0, r32ui)uniform uimage3D daxa_AtomicImageTableuimage3D[];uint imageAtomicCompSwap(daxa_RWImage3Du32 image, ivec3 index, uint compare, uint data){ return imageAtomicCompSwap(daxa_AtomicImageTableuimage3D[daxa_id_to_index(image . id)], index, compare, data);} uint imageAtomicExchange(daxa_RWImage3Du32 image, ivec3 index, uint data){ return imageAtomicExchange(daxa_AtomicImageTableuimage3D[daxa_id_to_index(image . id)], index, data);} uint imageAtomicAdd(daxa_RWImage3Du32 image, ivec3 index, uint data){ return imageAtomicAdd(daxa_AtomicImageTableuimage3D[daxa_id_to_index(image . id)], index, data);} uint imageAtomicAnd(daxa_RWImage3Du32 image, ivec3 index, uint data){ return imageAtomicAnd(daxa_AtomicImageTableuimage3D[daxa_id_to_index(image . id)], index, data);} uint imageAtomicOr(daxa_RWImage3Du32 image, ivec3 index, uint data){ return imageAtomicOr(daxa_AtomicImageTableuimage3D[daxa_id_to_index(image . id)], index, data);} uint imageAtomicXor(daxa_RWImage3Du32 image, ivec3 index, uint data){ return imageAtomicXor(daxa_AtomicImageTableuimage3D[daxa_id_to_index(image . id)], index, data);} uint imageAtomicMin(daxa_RWImage3Du32 image, ivec3 index, uint data){ return imageAtomicMin(daxa_AtomicImageTableuimage3D[daxa_id_to_index(image . id)], index, data);} uint imageAtomicMax(daxa_RWImage3Du32 image, ivec3 index, uint data){ return imageAtomicMax(daxa_AtomicImageTableuimage3D[daxa_id_to_index(image . id)], index, data);}
                                            layout(binding = 1, set = 0, r32i)uniform iimageCube daxa_AtomicImageTableiimageCube[];int imageAtomicCompSwap(daxa_RWImageCubei32 image, ivec3 index, int compare, int data){ return imageAtomicCompSwap(daxa_AtomicImageTableiimageCube[daxa_id_to_index(image . id)], index, compare, data);} int imageAtomicExchange(daxa_RWImageCubei32 image, ivec3 index, int data){ return imageAtomicExchange(daxa_AtomicImageTableiimageCube[daxa_id_to_index(image . id)], index, data);} int imageAtomicAdd(daxa_RWImageCubei32 image, ivec3 index, int data){ return imageAtomicAdd(daxa_AtomicImageTableiimageCube[daxa_id_to_index(image . id)], index, data);} int imageAtomicAnd(daxa_RWImageCubei32 image, ivec3 index, int data){ return imageAtomicAnd(daxa_AtomicImageTableiimageCube[daxa_id_to_index(image . id)], index, data);} int imageAtomicOr(daxa_RWImageCubei32 image, ivec3 index, int data){ return imageAtomicOr(daxa_AtomicImageTableiimageCube[daxa_id_to_index(image . id)], index, data);} int imageAtomicXor(daxa_RWImageCubei32 image, ivec3 index, int data){ return imageAtomicXor(daxa_AtomicImageTableiimageCube[daxa_id_to_index(image . id)], index, data);} int imageAtomicMin(daxa_RWImageCubei32 image, ivec3 index, int data){ return imageAtomicMin(daxa_AtomicImageTableiimageCube[daxa_id_to_index(image . id)], index, data);} int imageAtomicMax(daxa_RWImageCubei32 image, ivec3 index, int data){ return imageAtomicMax(daxa_AtomicImageTableiimageCube[daxa_id_to_index(image . id)], index, data);} layout(binding = 1, set = 0, r32ui)uniform uimageCube daxa_AtomicImageTableuimageCube[];uint imageAtomicCompSwap(daxa_RWImageCubeu32 image, ivec3 index, uint compare, uint data){ return imageAtomicCompSwap(daxa_AtomicImageTableuimageCube[daxa_id_to_index(image . id)], index, compare, data);} uint imageAtomicExchange(daxa_RWImageCubeu32 image, ivec3 index, uint data){ return imageAtomicExchange(daxa_AtomicImageTableuimageCube[daxa_id_to_index(image . id)], index, data);} uint imageAtomicAdd(daxa_RWImageCubeu32 image, ivec3 index, uint data){ return imageAtomicAdd(daxa_AtomicImageTableuimageCube[daxa_id_to_index(image . id)], index, data);} uint imageAtomicAnd(daxa_RWImageCubeu32 image, ivec3 index, uint data){ return imageAtomicAnd(daxa_AtomicImageTableuimageCube[daxa_id_to_index(image . id)], index, data);} uint imageAtomicOr(daxa_RWImageCubeu32 image, ivec3 index, uint data){ return imageAtomicOr(daxa_AtomicImageTableuimageCube[daxa_id_to_index(image . id)], index, data);} uint imageAtomicXor(daxa_RWImageCubeu32 image, ivec3 index, uint data){ return imageAtomicXor(daxa_AtomicImageTableuimageCube[daxa_id_to_index(image . id)], index, data);} uint imageAtomicMin(daxa_RWImageCubeu32 image, ivec3 index, uint data){ return imageAtomicMin(daxa_AtomicImageTableuimageCube[daxa_id_to_index(image . id)], index, data);} uint imageAtomicMax(daxa_RWImageCubeu32 image, ivec3 index, uint data){ return imageAtomicMax(daxa_AtomicImageTableuimageCube[daxa_id_to_index(image . id)], index, data);}
                                                 layout(binding = 1, set = 0, r32i)uniform iimageCubeArray daxa_AtomicImageTableiimageCubeArray[];int imageAtomicCompSwap(daxa_RWImageCubeArrayi32 image, ivec3 index, int compare, int data){ return imageAtomicCompSwap(daxa_AtomicImageTableiimageCubeArray[daxa_id_to_index(image . id)], index, compare, data);} int imageAtomicExchange(daxa_RWImageCubeArrayi32 image, ivec3 index, int data){ return imageAtomicExchange(daxa_AtomicImageTableiimageCubeArray[daxa_id_to_index(image . id)], index, data);} int imageAtomicAdd(daxa_RWImageCubeArrayi32 image, ivec3 index, int data){ return imageAtomicAdd(daxa_AtomicImageTableiimageCubeArray[daxa_id_to_index(image . id)], index, data);} int imageAtomicAnd(daxa_RWImageCubeArrayi32 image, ivec3 index, int data){ return imageAtomicAnd(daxa_AtomicImageTableiimageCubeArray[daxa_id_to_index(image . id)], index, data);} int imageAtomicOr(daxa_RWImageCubeArrayi32 image, ivec3 index, int data){ return imageAtomicOr(daxa_AtomicImageTableiimageCubeArray[daxa_id_to_index(image . id)], index, data);} int imageAtomicXor(daxa_RWImageCubeArrayi32 image, ivec3 index, int data){ return imageAtomicXor(daxa_AtomicImageTableiimageCubeArray[daxa_id_to_index(image . id)], index, data);} int imageAtomicMin(daxa_RWImageCubeArrayi32 image, ivec3 index, int data){ return imageAtomicMin(daxa_AtomicImageTableiimageCubeArray[daxa_id_to_index(image . id)], index, data);} int imageAtomicMax(daxa_RWImageCubeArrayi32 image, ivec3 index, int data){ return imageAtomicMax(daxa_AtomicImageTableiimageCubeArray[daxa_id_to_index(image . id)], index, data);} layout(binding = 1, set = 0, r32ui)uniform uimageCubeArray daxa_AtomicImageTableuimageCubeArray[];uint imageAtomicCompSwap(daxa_RWImageCubeArrayu32 image, ivec3 index, uint compare, uint data){ return imageAtomicCompSwap(daxa_AtomicImageTableuimageCubeArray[daxa_id_to_index(image . id)], index, compare, data);} uint imageAtomicExchange(daxa_RWImageCubeArrayu32 image, ivec3 index, uint data){ return imageAtomicExchange(daxa_AtomicImageTableuimageCubeArray[daxa_id_to_index(image . id)], index, data);} uint imageAtomicAdd(daxa_RWImageCubeArrayu32 image, ivec3 index, uint data){ return imageAtomicAdd(daxa_AtomicImageTableuimageCubeArray[daxa_id_to_index(image . id)], index, data);} uint imageAtomicAnd(daxa_RWImageCubeArrayu32 image, ivec3 index, uint data){ return imageAtomicAnd(daxa_AtomicImageTableuimageCubeArray[daxa_id_to_index(image . id)], index, data);} uint imageAtomicOr(daxa_RWImageCubeArrayu32 image, ivec3 index, uint data){ return imageAtomicOr(daxa_AtomicImageTableuimageCubeArray[daxa_id_to_index(image . id)], index, data);} uint imageAtomicXor(daxa_RWImageCubeArrayu32 image, ivec3 index, uint data){ return imageAtomicXor(daxa_AtomicImageTableuimageCubeArray[daxa_id_to_index(image . id)], index, data);} uint imageAtomicMin(daxa_RWImageCubeArrayu32 image, ivec3 index, uint data){ return imageAtomicMin(daxa_AtomicImageTableuimageCubeArray[daxa_id_to_index(image . id)], index, data);} uint imageAtomicMax(daxa_RWImageCubeArrayu32 image, ivec3 index, uint data){ return imageAtomicMax(daxa_AtomicImageTableuimageCubeArray[daxa_id_to_index(image . id)], index, data);}
                                               layout(binding = 1, set = 0, r32i)uniform iimage1DArray daxa_AtomicImageTableiimage1DArray[];int imageAtomicCompSwap(daxa_RWImage1DArrayi32 image, ivec2 index, int compare, int data){ return imageAtomicCompSwap(daxa_AtomicImageTableiimage1DArray[daxa_id_to_index(image . id)], index, compare, data);} int imageAtomicExchange(daxa_RWImage1DArrayi32 image, ivec2 index, int data){ return imageAtomicExchange(daxa_AtomicImageTableiimage1DArray[daxa_id_to_index(image . id)], index, data);} int imageAtomicAdd(daxa_RWImage1DArrayi32 image, ivec2 index, int data){ return imageAtomicAdd(daxa_AtomicImageTableiimage1DArray[daxa_id_to_index(image . id)], index, data);} int imageAtomicAnd(daxa_RWImage1DArrayi32 image, ivec2 index, int data){ return imageAtomicAnd(daxa_AtomicImageTableiimage1DArray[daxa_id_to_index(image . id)], index, data);} int imageAtomicOr(daxa_RWImage1DArrayi32 image, ivec2 index, int data){ return imageAtomicOr(daxa_AtomicImageTableiimage1DArray[daxa_id_to_index(image . id)], index, data);} int imageAtomicXor(daxa_RWImage1DArrayi32 image, ivec2 index, int data){ return imageAtomicXor(daxa_AtomicImageTableiimage1DArray[daxa_id_to_index(image . id)], index, data);} int imageAtomicMin(daxa_RWImage1DArrayi32 image, ivec2 index, int data){ return imageAtomicMin(daxa_AtomicImageTableiimage1DArray[daxa_id_to_index(image . id)], index, data);} int imageAtomicMax(daxa_RWImage1DArrayi32 image, ivec2 index, int data){ return imageAtomicMax(daxa_AtomicImageTableiimage1DArray[daxa_id_to_index(image . id)], index, data);} layout(binding = 1, set = 0, r32ui)uniform uimage1DArray daxa_AtomicImageTableuimage1DArray[];uint imageAtomicCompSwap(daxa_RWImage1DArrayu32 image, ivec2 index, uint compare, uint data){ return imageAtomicCompSwap(daxa_AtomicImageTableuimage1DArray[daxa_id_to_index(image . id)], index, compare, data);} uint imageAtomicExchange(daxa_RWImage1DArrayu32 image, ivec2 index, uint data){ return imageAtomicExchange(daxa_AtomicImageTableuimage1DArray[daxa_id_to_index(image . id)], index, data);} uint imageAtomicAdd(daxa_RWImage1DArrayu32 image, ivec2 index, uint data){ return imageAtomicAdd(daxa_AtomicImageTableuimage1DArray[daxa_id_to_index(image . id)], index, data);} uint imageAtomicAnd(daxa_RWImage1DArrayu32 image, ivec2 index, uint data){ return imageAtomicAnd(daxa_AtomicImageTableuimage1DArray[daxa_id_to_index(image . id)], index, data);} uint imageAtomicOr(daxa_RWImage1DArrayu32 image, ivec2 index, uint data){ return imageAtomicOr(daxa_AtomicImageTableuimage1DArray[daxa_id_to_index(image . id)], index, data);} uint imageAtomicXor(daxa_RWImage1DArrayu32 image, ivec2 index, uint data){ return imageAtomicXor(daxa_AtomicImageTableuimage1DArray[daxa_id_to_index(image . id)], index, data);} uint imageAtomicMin(daxa_RWImage1DArrayu32 image, ivec2 index, uint data){ return imageAtomicMin(daxa_AtomicImageTableuimage1DArray[daxa_id_to_index(image . id)], index, data);} uint imageAtomicMax(daxa_RWImage1DArrayu32 image, ivec2 index, uint data){ return imageAtomicMax(daxa_AtomicImageTableuimage1DArray[daxa_id_to_index(image . id)], index, data);}
                                               layout(binding = 1, set = 0, r32i)uniform iimage2DArray daxa_AtomicImageTableiimage2DArray[];int imageAtomicCompSwap(daxa_RWImage2DArrayi32 image, ivec3 index, int compare, int data){ return imageAtomicCompSwap(daxa_AtomicImageTableiimage2DArray[daxa_id_to_index(image . id)], index, compare, data);} int imageAtomicExchange(daxa_RWImage2DArrayi32 image, ivec3 index, int data){ return imageAtomicExchange(daxa_AtomicImageTableiimage2DArray[daxa_id_to_index(image . id)], index, data);} int imageAtomicAdd(daxa_RWImage2DArrayi32 image, ivec3 index, int data){ return imageAtomicAdd(daxa_AtomicImageTableiimage2DArray[daxa_id_to_index(image . id)], index, data);} int imageAtomicAnd(daxa_RWImage2DArrayi32 image, ivec3 index, int data){ return imageAtomicAnd(daxa_AtomicImageTableiimage2DArray[daxa_id_to_index(image . id)], index, data);} int imageAtomicOr(daxa_RWImage2DArrayi32 image, ivec3 index, int data){ return imageAtomicOr(daxa_AtomicImageTableiimage2DArray[daxa_id_to_index(image . id)], index, data);} int imageAtomicXor(daxa_RWImage2DArrayi32 image, ivec3 index, int data){ return imageAtomicXor(daxa_AtomicImageTableiimage2DArray[daxa_id_to_index(image . id)], index, data);} int imageAtomicMin(daxa_RWImage2DArrayi32 image, ivec3 index, int data){ return imageAtomicMin(daxa_AtomicImageTableiimage2DArray[daxa_id_to_index(image . id)], index, data);} int imageAtomicMax(daxa_RWImage2DArrayi32 image, ivec3 index, int data){ return imageAtomicMax(daxa_AtomicImageTableiimage2DArray[daxa_id_to_index(image . id)], index, data);} layout(binding = 1, set = 0, r32ui)uniform uimage2DArray daxa_AtomicImageTableuimage2DArray[];uint imageAtomicCompSwap(daxa_RWImage2DArrayu32 image, ivec3 index, uint compare, uint data){ return imageAtomicCompSwap(daxa_AtomicImageTableuimage2DArray[daxa_id_to_index(image . id)], index, compare, data);} uint imageAtomicExchange(daxa_RWImage2DArrayu32 image, ivec3 index, uint data){ return imageAtomicExchange(daxa_AtomicImageTableuimage2DArray[daxa_id_to_index(image . id)], index, data);} uint imageAtomicAdd(daxa_RWImage2DArrayu32 image, ivec3 index, uint data){ return imageAtomicAdd(daxa_AtomicImageTableuimage2DArray[daxa_id_to_index(image . id)], index, data);} uint imageAtomicAnd(daxa_RWImage2DArrayu32 image, ivec3 index, uint data){ return imageAtomicAnd(daxa_AtomicImageTableuimage2DArray[daxa_id_to_index(image . id)], index, data);} uint imageAtomicOr(daxa_RWImage2DArrayu32 image, ivec3 index, uint data){ return imageAtomicOr(daxa_AtomicImageTableuimage2DArray[daxa_id_to_index(image . id)], index, data);} uint imageAtomicXor(daxa_RWImage2DArrayu32 image, ivec3 index, uint data){ return imageAtomicXor(daxa_AtomicImageTableuimage2DArray[daxa_id_to_index(image . id)], index, data);} uint imageAtomicMin(daxa_RWImage2DArrayu32 image, ivec3 index, uint data){ return imageAtomicMin(daxa_AtomicImageTableuimage2DArray[daxa_id_to_index(image . id)], index, data);} uint imageAtomicMax(daxa_RWImage2DArrayu32 image, ivec3 index, uint data){ return imageAtomicMax(daxa_AtomicImageTableuimage2DArray[daxa_id_to_index(image . id)], index, data);}



                                            layout(binding = 1, set = 0, r32i)uniform iimage2DMS daxa_AtomicImageTableiimage2DMS[];int imageAtomicCompSwap(daxa_RWImage2DMSi32 image, ivec2 index, int s, int compare, int data){ return imageAtomicCompSwap(daxa_AtomicImageTableiimage2DMS[daxa_id_to_index(image . id)], index, s, compare, data);} int imageAtomicExchange(daxa_RWImage2DMSi32 image, ivec2 index, int s, int data){ return imageAtomicExchange(daxa_AtomicImageTableiimage2DMS[daxa_id_to_index(image . id)], index, s, data);} int imageAtomicAdd(daxa_RWImage2DMSi32 image, ivec2 index, int s, int data){ return imageAtomicAdd(daxa_AtomicImageTableiimage2DMS[daxa_id_to_index(image . id)], index, s, data);} int imageAtomicAnd(daxa_RWImage2DMSi32 image, ivec2 index, int s, int data){ return imageAtomicAnd(daxa_AtomicImageTableiimage2DMS[daxa_id_to_index(image . id)], index, s, data);} int imageAtomicOr(daxa_RWImage2DMSi32 image, ivec2 index, int s, int data){ return imageAtomicOr(daxa_AtomicImageTableiimage2DMS[daxa_id_to_index(image . id)], index, s, data);} int imageAtomicXor(daxa_RWImage2DMSi32 image, ivec2 index, int s, int data){ return imageAtomicXor(daxa_AtomicImageTableiimage2DMS[daxa_id_to_index(image . id)], index, s, data);} int imageAtomicMin(daxa_RWImage2DMSi32 image, ivec2 index, int s, int data){ return imageAtomicMin(daxa_AtomicImageTableiimage2DMS[daxa_id_to_index(image . id)], index, s, data);} int imageAtomicMax(daxa_RWImage2DMSi32 image, ivec2 index, int s, int data){ return imageAtomicMax(daxa_AtomicImageTableiimage2DMS[daxa_id_to_index(image . id)], index, s, data);} layout(binding = 1, set = 0, r32ui)uniform uimage2DMS daxa_AtomicImageTableuimage2DMS[];uint imageAtomicCompSwap(daxa_RWImage2DMSu32 image, ivec2 index, int s, uint compare, uint data){ return imageAtomicCompSwap(daxa_AtomicImageTableuimage2DMS[daxa_id_to_index(image . id)], index, s, compare, data);} uint imageAtomicExchange(daxa_RWImage2DMSu32 image, ivec2 index, int s, uint data){ return imageAtomicExchange(daxa_AtomicImageTableuimage2DMS[daxa_id_to_index(image . id)], index, s, data);} uint imageAtomicAdd(daxa_RWImage2DMSu32 image, ivec2 index, int s, uint data){ return imageAtomicAdd(daxa_AtomicImageTableuimage2DMS[daxa_id_to_index(image . id)], index, s, data);} uint imageAtomicAnd(daxa_RWImage2DMSu32 image, ivec2 index, int s, uint data){ return imageAtomicAnd(daxa_AtomicImageTableuimage2DMS[daxa_id_to_index(image . id)], index, s, data);} uint imageAtomicOr(daxa_RWImage2DMSu32 image, ivec2 index, int s, uint data){ return imageAtomicOr(daxa_AtomicImageTableuimage2DMS[daxa_id_to_index(image . id)], index, s, data);} uint imageAtomicXor(daxa_RWImage2DMSu32 image, ivec2 index, int s, uint data){ return imageAtomicXor(daxa_AtomicImageTableuimage2DMS[daxa_id_to_index(image . id)], index, s, data);} uint imageAtomicMin(daxa_RWImage2DMSu32 image, ivec2 index, int s, uint data){ return imageAtomicMin(daxa_AtomicImageTableuimage2DMS[daxa_id_to_index(image . id)], index, s, data);} uint imageAtomicMax(daxa_RWImage2DMSu32 image, ivec2 index, int s, uint data){ return imageAtomicMax(daxa_AtomicImageTableuimage2DMS[daxa_id_to_index(image . id)], index, s, data);}
                                                 layout(binding = 1, set = 0, r32i)uniform iimage2DMSArray daxa_AtomicImageTableiimage2DMSArray[];int imageAtomicCompSwap(daxa_RWImage2DMSArrayi32 image, ivec3 index, int s, int compare, int data){ return imageAtomicCompSwap(daxa_AtomicImageTableiimage2DMSArray[daxa_id_to_index(image . id)], index, s, compare, data);} int imageAtomicExchange(daxa_RWImage2DMSArrayi32 image, ivec3 index, int s, int data){ return imageAtomicExchange(daxa_AtomicImageTableiimage2DMSArray[daxa_id_to_index(image . id)], index, s, data);} int imageAtomicAdd(daxa_RWImage2DMSArrayi32 image, ivec3 index, int s, int data){ return imageAtomicAdd(daxa_AtomicImageTableiimage2DMSArray[daxa_id_to_index(image . id)], index, s, data);} int imageAtomicAnd(daxa_RWImage2DMSArrayi32 image, ivec3 index, int s, int data){ return imageAtomicAnd(daxa_AtomicImageTableiimage2DMSArray[daxa_id_to_index(image . id)], index, s, data);} int imageAtomicOr(daxa_RWImage2DMSArrayi32 image, ivec3 index, int s, int data){ return imageAtomicOr(daxa_AtomicImageTableiimage2DMSArray[daxa_id_to_index(image . id)], index, s, data);} int imageAtomicXor(daxa_RWImage2DMSArrayi32 image, ivec3 index, int s, int data){ return imageAtomicXor(daxa_AtomicImageTableiimage2DMSArray[daxa_id_to_index(image . id)], index, s, data);} int imageAtomicMin(daxa_RWImage2DMSArrayi32 image, ivec3 index, int s, int data){ return imageAtomicMin(daxa_AtomicImageTableiimage2DMSArray[daxa_id_to_index(image . id)], index, s, data);} int imageAtomicMax(daxa_RWImage2DMSArrayi32 image, ivec3 index, int s, int data){ return imageAtomicMax(daxa_AtomicImageTableiimage2DMSArray[daxa_id_to_index(image . id)], index, s, data);} layout(binding = 1, set = 0, r32ui)uniform uimage2DMSArray daxa_AtomicImageTableuimage2DMSArray[];uint imageAtomicCompSwap(daxa_RWImage2DMSArrayu32 image, ivec3 index, int s, uint compare, uint data){ return imageAtomicCompSwap(daxa_AtomicImageTableuimage2DMSArray[daxa_id_to_index(image . id)], index, s, compare, data);} uint imageAtomicExchange(daxa_RWImage2DMSArrayu32 image, ivec3 index, int s, uint data){ return imageAtomicExchange(daxa_AtomicImageTableuimage2DMSArray[daxa_id_to_index(image . id)], index, s, data);} uint imageAtomicAdd(daxa_RWImage2DMSArrayu32 image, ivec3 index, int s, uint data){ return imageAtomicAdd(daxa_AtomicImageTableuimage2DMSArray[daxa_id_to_index(image . id)], index, s, data);} uint imageAtomicAnd(daxa_RWImage2DMSArrayu32 image, ivec3 index, int s, uint data){ return imageAtomicAnd(daxa_AtomicImageTableuimage2DMSArray[daxa_id_to_index(image . id)], index, s, data);} uint imageAtomicOr(daxa_RWImage2DMSArrayu32 image, ivec3 index, int s, uint data){ return imageAtomicOr(daxa_AtomicImageTableuimage2DMSArray[daxa_id_to_index(image . id)], index, s, data);} uint imageAtomicXor(daxa_RWImage2DMSArrayu32 image, ivec3 index, int s, uint data){ return imageAtomicXor(daxa_AtomicImageTableuimage2DMSArray[daxa_id_to_index(image . id)], index, s, data);} uint imageAtomicMin(daxa_RWImage2DMSArrayu32 image, ivec3 index, int s, uint data){ return imageAtomicMin(daxa_AtomicImageTableuimage2DMSArray[daxa_id_to_index(image . id)], index, s, data);} uint imageAtomicMax(daxa_RWImage2DMSArrayu32 image, ivec3 index, int s, uint data){ return imageAtomicMax(daxa_AtomicImageTableuimage2DMSArray[daxa_id_to_index(image . id)], index, s, data);}





                                                layout(binding = 1, set = 0, r64i)uniform i64image1D daxa_AtomicImageTablei64image1D[];int64_t imageAtomicCompSwap(daxa_RWImage1Di64 image, int index, int64_t compare, int64_t data){ return imageAtomicCompSwap(daxa_AtomicImageTablei64image1D[daxa_id_to_index(image . id)], index, compare, data);} int64_t imageAtomicExchange(daxa_RWImage1Di64 image, int index, int64_t data){ return imageAtomicExchange(daxa_AtomicImageTablei64image1D[daxa_id_to_index(image . id)], index, data);} int64_t imageAtomicAdd(daxa_RWImage1Di64 image, int index, int64_t data){ return imageAtomicAdd(daxa_AtomicImageTablei64image1D[daxa_id_to_index(image . id)], index, data);} int64_t imageAtomicAnd(daxa_RWImage1Di64 image, int index, int64_t data){ return imageAtomicAnd(daxa_AtomicImageTablei64image1D[daxa_id_to_index(image . id)], index, data);} int64_t imageAtomicOr(daxa_RWImage1Di64 image, int index, int64_t data){ return imageAtomicOr(daxa_AtomicImageTablei64image1D[daxa_id_to_index(image . id)], index, data);} int64_t imageAtomicXor(daxa_RWImage1Di64 image, int index, int64_t data){ return imageAtomicXor(daxa_AtomicImageTablei64image1D[daxa_id_to_index(image . id)], index, data);} int64_t imageAtomicMin(daxa_RWImage1Di64 image, int index, int64_t data){ return imageAtomicMin(daxa_AtomicImageTablei64image1D[daxa_id_to_index(image . id)], index, data);} int64_t imageAtomicMax(daxa_RWImage1Di64 image, int index, int64_t data){ return imageAtomicMax(daxa_AtomicImageTablei64image1D[daxa_id_to_index(image . id)], index, data);} layout(binding = 1, set = 0, r64ui)uniform u64image1D daxa_AtomicImageTableu64image1D[];uint64_t imageAtomicCompSwap(daxa_RWImage1Du64 image, int index, uint64_t compare, uint64_t data){ return imageAtomicCompSwap(daxa_AtomicImageTableu64image1D[daxa_id_to_index(image . id)], index, compare, data);} uint64_t imageAtomicExchange(daxa_RWImage1Du64 image, int index, uint64_t data){ return imageAtomicExchange(daxa_AtomicImageTableu64image1D[daxa_id_to_index(image . id)], index, data);} uint64_t imageAtomicAdd(daxa_RWImage1Du64 image, int index, uint64_t data){ return imageAtomicAdd(daxa_AtomicImageTableu64image1D[daxa_id_to_index(image . id)], index, data);} uint64_t imageAtomicAnd(daxa_RWImage1Du64 image, int index, uint64_t data){ return imageAtomicAnd(daxa_AtomicImageTableu64image1D[daxa_id_to_index(image . id)], index, data);} uint64_t imageAtomicOr(daxa_RWImage1Du64 image, int index, uint64_t data){ return imageAtomicOr(daxa_AtomicImageTableu64image1D[daxa_id_to_index(image . id)], index, data);} uint64_t imageAtomicXor(daxa_RWImage1Du64 image, int index, uint64_t data){ return imageAtomicXor(daxa_AtomicImageTableu64image1D[daxa_id_to_index(image . id)], index, data);} uint64_t imageAtomicMin(daxa_RWImage1Du64 image, int index, uint64_t data){ return imageAtomicMin(daxa_AtomicImageTableu64image1D[daxa_id_to_index(image . id)], index, data);} uint64_t imageAtomicMax(daxa_RWImage1Du64 image, int index, uint64_t data){ return imageAtomicMax(daxa_AtomicImageTableu64image1D[daxa_id_to_index(image . id)], index, data);}
                                                layout(binding = 1, set = 0, r64i)uniform i64image2D daxa_AtomicImageTablei64image2D[];int64_t imageAtomicCompSwap(daxa_RWImage2Di64 image, ivec2 index, int64_t compare, int64_t data){ return imageAtomicCompSwap(daxa_AtomicImageTablei64image2D[daxa_id_to_index(image . id)], index, compare, data);} int64_t imageAtomicExchange(daxa_RWImage2Di64 image, ivec2 index, int64_t data){ return imageAtomicExchange(daxa_AtomicImageTablei64image2D[daxa_id_to_index(image . id)], index, data);} int64_t imageAtomicAdd(daxa_RWImage2Di64 image, ivec2 index, int64_t data){ return imageAtomicAdd(daxa_AtomicImageTablei64image2D[daxa_id_to_index(image . id)], index, data);} int64_t imageAtomicAnd(daxa_RWImage2Di64 image, ivec2 index, int64_t data){ return imageAtomicAnd(daxa_AtomicImageTablei64image2D[daxa_id_to_index(image . id)], index, data);} int64_t imageAtomicOr(daxa_RWImage2Di64 image, ivec2 index, int64_t data){ return imageAtomicOr(daxa_AtomicImageTablei64image2D[daxa_id_to_index(image . id)], index, data);} int64_t imageAtomicXor(daxa_RWImage2Di64 image, ivec2 index, int64_t data){ return imageAtomicXor(daxa_AtomicImageTablei64image2D[daxa_id_to_index(image . id)], index, data);} int64_t imageAtomicMin(daxa_RWImage2Di64 image, ivec2 index, int64_t data){ return imageAtomicMin(daxa_AtomicImageTablei64image2D[daxa_id_to_index(image . id)], index, data);} int64_t imageAtomicMax(daxa_RWImage2Di64 image, ivec2 index, int64_t data){ return imageAtomicMax(daxa_AtomicImageTablei64image2D[daxa_id_to_index(image . id)], index, data);} layout(binding = 1, set = 0, r64ui)uniform u64image2D daxa_AtomicImageTableu64image2D[];uint64_t imageAtomicCompSwap(daxa_RWImage2Du64 image, ivec2 index, uint64_t compare, uint64_t data){ return imageAtomicCompSwap(daxa_AtomicImageTableu64image2D[daxa_id_to_index(image . id)], index, compare, data);} uint64_t imageAtomicExchange(daxa_RWImage2Du64 image, ivec2 index, uint64_t data){ return imageAtomicExchange(daxa_AtomicImageTableu64image2D[daxa_id_to_index(image . id)], index, data);} uint64_t imageAtomicAdd(daxa_RWImage2Du64 image, ivec2 index, uint64_t data){ return imageAtomicAdd(daxa_AtomicImageTableu64image2D[daxa_id_to_index(image . id)], index, data);} uint64_t imageAtomicAnd(daxa_RWImage2Du64 image, ivec2 index, uint64_t data){ return imageAtomicAnd(daxa_AtomicImageTableu64image2D[daxa_id_to_index(image . id)], index, data);} uint64_t imageAtomicOr(daxa_RWImage2Du64 image, ivec2 index, uint64_t data){ return imageAtomicOr(daxa_AtomicImageTableu64image2D[daxa_id_to_index(image . id)], index, data);} uint64_t imageAtomicXor(daxa_RWImage2Du64 image, ivec2 index, uint64_t data){ return imageAtomicXor(daxa_AtomicImageTableu64image2D[daxa_id_to_index(image . id)], index, data);} uint64_t imageAtomicMin(daxa_RWImage2Du64 image, ivec2 index, uint64_t data){ return imageAtomicMin(daxa_AtomicImageTableu64image2D[daxa_id_to_index(image . id)], index, data);} uint64_t imageAtomicMax(daxa_RWImage2Du64 image, ivec2 index, uint64_t data){ return imageAtomicMax(daxa_AtomicImageTableu64image2D[daxa_id_to_index(image . id)], index, data);}
                                                layout(binding = 1, set = 0, r64i)uniform i64image3D daxa_AtomicImageTablei64image3D[];int64_t imageAtomicCompSwap(daxa_RWImage3Di64 image, ivec3 index, int64_t compare, int64_t data){ return imageAtomicCompSwap(daxa_AtomicImageTablei64image3D[daxa_id_to_index(image . id)], index, compare, data);} int64_t imageAtomicExchange(daxa_RWImage3Di64 image, ivec3 index, int64_t data){ return imageAtomicExchange(daxa_AtomicImageTablei64image3D[daxa_id_to_index(image . id)], index, data);} int64_t imageAtomicAdd(daxa_RWImage3Di64 image, ivec3 index, int64_t data){ return imageAtomicAdd(daxa_AtomicImageTablei64image3D[daxa_id_to_index(image . id)], index, data);} int64_t imageAtomicAnd(daxa_RWImage3Di64 image, ivec3 index, int64_t data){ return imageAtomicAnd(daxa_AtomicImageTablei64image3D[daxa_id_to_index(image . id)], index, data);} int64_t imageAtomicOr(daxa_RWImage3Di64 image, ivec3 index, int64_t data){ return imageAtomicOr(daxa_AtomicImageTablei64image3D[daxa_id_to_index(image . id)], index, data);} int64_t imageAtomicXor(daxa_RWImage3Di64 image, ivec3 index, int64_t data){ return imageAtomicXor(daxa_AtomicImageTablei64image3D[daxa_id_to_index(image . id)], index, data);} int64_t imageAtomicMin(daxa_RWImage3Di64 image, ivec3 index, int64_t data){ return imageAtomicMin(daxa_AtomicImageTablei64image3D[daxa_id_to_index(image . id)], index, data);} int64_t imageAtomicMax(daxa_RWImage3Di64 image, ivec3 index, int64_t data){ return imageAtomicMax(daxa_AtomicImageTablei64image3D[daxa_id_to_index(image . id)], index, data);} layout(binding = 1, set = 0, r64ui)uniform u64image3D daxa_AtomicImageTableu64image3D[];uint64_t imageAtomicCompSwap(daxa_RWImage3Du64 image, ivec3 index, uint64_t compare, uint64_t data){ return imageAtomicCompSwap(daxa_AtomicImageTableu64image3D[daxa_id_to_index(image . id)], index, compare, data);} uint64_t imageAtomicExchange(daxa_RWImage3Du64 image, ivec3 index, uint64_t data){ return imageAtomicExchange(daxa_AtomicImageTableu64image3D[daxa_id_to_index(image . id)], index, data);} uint64_t imageAtomicAdd(daxa_RWImage3Du64 image, ivec3 index, uint64_t data){ return imageAtomicAdd(daxa_AtomicImageTableu64image3D[daxa_id_to_index(image . id)], index, data);} uint64_t imageAtomicAnd(daxa_RWImage3Du64 image, ivec3 index, uint64_t data){ return imageAtomicAnd(daxa_AtomicImageTableu64image3D[daxa_id_to_index(image . id)], index, data);} uint64_t imageAtomicOr(daxa_RWImage3Du64 image, ivec3 index, uint64_t data){ return imageAtomicOr(daxa_AtomicImageTableu64image3D[daxa_id_to_index(image . id)], index, data);} uint64_t imageAtomicXor(daxa_RWImage3Du64 image, ivec3 index, uint64_t data){ return imageAtomicXor(daxa_AtomicImageTableu64image3D[daxa_id_to_index(image . id)], index, data);} uint64_t imageAtomicMin(daxa_RWImage3Du64 image, ivec3 index, uint64_t data){ return imageAtomicMin(daxa_AtomicImageTableu64image3D[daxa_id_to_index(image . id)], index, data);} uint64_t imageAtomicMax(daxa_RWImage3Du64 image, ivec3 index, uint64_t data){ return imageAtomicMax(daxa_AtomicImageTableu64image3D[daxa_id_to_index(image . id)], index, data);}
                                                  layout(binding = 1, set = 0, r64i)uniform i64imageCube daxa_AtomicImageTablei64imageCube[];int64_t imageAtomicCompSwap(daxa_RWImageCubei64 image, ivec3 index, int64_t compare, int64_t data){ return imageAtomicCompSwap(daxa_AtomicImageTablei64imageCube[daxa_id_to_index(image . id)], index, compare, data);} int64_t imageAtomicExchange(daxa_RWImageCubei64 image, ivec3 index, int64_t data){ return imageAtomicExchange(daxa_AtomicImageTablei64imageCube[daxa_id_to_index(image . id)], index, data);} int64_t imageAtomicAdd(daxa_RWImageCubei64 image, ivec3 index, int64_t data){ return imageAtomicAdd(daxa_AtomicImageTablei64imageCube[daxa_id_to_index(image . id)], index, data);} int64_t imageAtomicAnd(daxa_RWImageCubei64 image, ivec3 index, int64_t data){ return imageAtomicAnd(daxa_AtomicImageTablei64imageCube[daxa_id_to_index(image . id)], index, data);} int64_t imageAtomicOr(daxa_RWImageCubei64 image, ivec3 index, int64_t data){ return imageAtomicOr(daxa_AtomicImageTablei64imageCube[daxa_id_to_index(image . id)], index, data);} int64_t imageAtomicXor(daxa_RWImageCubei64 image, ivec3 index, int64_t data){ return imageAtomicXor(daxa_AtomicImageTablei64imageCube[daxa_id_to_index(image . id)], index, data);} int64_t imageAtomicMin(daxa_RWImageCubei64 image, ivec3 index, int64_t data){ return imageAtomicMin(daxa_AtomicImageTablei64imageCube[daxa_id_to_index(image . id)], index, data);} int64_t imageAtomicMax(daxa_RWImageCubei64 image, ivec3 index, int64_t data){ return imageAtomicMax(daxa_AtomicImageTablei64imageCube[daxa_id_to_index(image . id)], index, data);} layout(binding = 1, set = 0, r64ui)uniform u64imageCube daxa_AtomicImageTableu64imageCube[];uint64_t imageAtomicCompSwap(daxa_RWImageCubeu64 image, ivec3 index, uint64_t compare, uint64_t data){ return imageAtomicCompSwap(daxa_AtomicImageTableu64imageCube[daxa_id_to_index(image . id)], index, compare, data);} uint64_t imageAtomicExchange(daxa_RWImageCubeu64 image, ivec3 index, uint64_t data){ return imageAtomicExchange(daxa_AtomicImageTableu64imageCube[daxa_id_to_index(image . id)], index, data);} uint64_t imageAtomicAdd(daxa_RWImageCubeu64 image, ivec3 index, uint64_t data){ return imageAtomicAdd(daxa_AtomicImageTableu64imageCube[daxa_id_to_index(image . id)], index, data);} uint64_t imageAtomicAnd(daxa_RWImageCubeu64 image, ivec3 index, uint64_t data){ return imageAtomicAnd(daxa_AtomicImageTableu64imageCube[daxa_id_to_index(image . id)], index, data);} uint64_t imageAtomicOr(daxa_RWImageCubeu64 image, ivec3 index, uint64_t data){ return imageAtomicOr(daxa_AtomicImageTableu64imageCube[daxa_id_to_index(image . id)], index, data);} uint64_t imageAtomicXor(daxa_RWImageCubeu64 image, ivec3 index, uint64_t data){ return imageAtomicXor(daxa_AtomicImageTableu64imageCube[daxa_id_to_index(image . id)], index, data);} uint64_t imageAtomicMin(daxa_RWImageCubeu64 image, ivec3 index, uint64_t data){ return imageAtomicMin(daxa_AtomicImageTableu64imageCube[daxa_id_to_index(image . id)], index, data);} uint64_t imageAtomicMax(daxa_RWImageCubeu64 image, ivec3 index, uint64_t data){ return imageAtomicMax(daxa_AtomicImageTableu64imageCube[daxa_id_to_index(image . id)], index, data);}
                                                       layout(binding = 1, set = 0, r64i)uniform i64imageCubeArray daxa_AtomicImageTablei64imageCubeArray[];int64_t imageAtomicCompSwap(daxa_RWImageCubeArrayi64 image, ivec3 index, int64_t compare, int64_t data){ return imageAtomicCompSwap(daxa_AtomicImageTablei64imageCubeArray[daxa_id_to_index(image . id)], index, compare, data);} int64_t imageAtomicExchange(daxa_RWImageCubeArrayi64 image, ivec3 index, int64_t data){ return imageAtomicExchange(daxa_AtomicImageTablei64imageCubeArray[daxa_id_to_index(image . id)], index, data);} int64_t imageAtomicAdd(daxa_RWImageCubeArrayi64 image, ivec3 index, int64_t data){ return imageAtomicAdd(daxa_AtomicImageTablei64imageCubeArray[daxa_id_to_index(image . id)], index, data);} int64_t imageAtomicAnd(daxa_RWImageCubeArrayi64 image, ivec3 index, int64_t data){ return imageAtomicAnd(daxa_AtomicImageTablei64imageCubeArray[daxa_id_to_index(image . id)], index, data);} int64_t imageAtomicOr(daxa_RWImageCubeArrayi64 image, ivec3 index, int64_t data){ return imageAtomicOr(daxa_AtomicImageTablei64imageCubeArray[daxa_id_to_index(image . id)], index, data);} int64_t imageAtomicXor(daxa_RWImageCubeArrayi64 image, ivec3 index, int64_t data){ return imageAtomicXor(daxa_AtomicImageTablei64imageCubeArray[daxa_id_to_index(image . id)], index, data);} int64_t imageAtomicMin(daxa_RWImageCubeArrayi64 image, ivec3 index, int64_t data){ return imageAtomicMin(daxa_AtomicImageTablei64imageCubeArray[daxa_id_to_index(image . id)], index, data);} int64_t imageAtomicMax(daxa_RWImageCubeArrayi64 image, ivec3 index, int64_t data){ return imageAtomicMax(daxa_AtomicImageTablei64imageCubeArray[daxa_id_to_index(image . id)], index, data);} layout(binding = 1, set = 0, r64ui)uniform u64imageCubeArray daxa_AtomicImageTableu64imageCubeArray[];uint64_t imageAtomicCompSwap(daxa_RWImageCubeArrayu64 image, ivec3 index, uint64_t compare, uint64_t data){ return imageAtomicCompSwap(daxa_AtomicImageTableu64imageCubeArray[daxa_id_to_index(image . id)], index, compare, data);} uint64_t imageAtomicExchange(daxa_RWImageCubeArrayu64 image, ivec3 index, uint64_t data){ return imageAtomicExchange(daxa_AtomicImageTableu64imageCubeArray[daxa_id_to_index(image . id)], index, data);} uint64_t imageAtomicAdd(daxa_RWImageCubeArrayu64 image, ivec3 index, uint64_t data){ return imageAtomicAdd(daxa_AtomicImageTableu64imageCubeArray[daxa_id_to_index(image . id)], index, data);} uint64_t imageAtomicAnd(daxa_RWImageCubeArrayu64 image, ivec3 index, uint64_t data){ return imageAtomicAnd(daxa_AtomicImageTableu64imageCubeArray[daxa_id_to_index(image . id)], index, data);} uint64_t imageAtomicOr(daxa_RWImageCubeArrayu64 image, ivec3 index, uint64_t data){ return imageAtomicOr(daxa_AtomicImageTableu64imageCubeArray[daxa_id_to_index(image . id)], index, data);} uint64_t imageAtomicXor(daxa_RWImageCubeArrayu64 image, ivec3 index, uint64_t data){ return imageAtomicXor(daxa_AtomicImageTableu64imageCubeArray[daxa_id_to_index(image . id)], index, data);} uint64_t imageAtomicMin(daxa_RWImageCubeArrayu64 image, ivec3 index, uint64_t data){ return imageAtomicMin(daxa_AtomicImageTableu64imageCubeArray[daxa_id_to_index(image . id)], index, data);} uint64_t imageAtomicMax(daxa_RWImageCubeArrayu64 image, ivec3 index, uint64_t data){ return imageAtomicMax(daxa_AtomicImageTableu64imageCubeArray[daxa_id_to_index(image . id)], index, data);}
                                                     layout(binding = 1, set = 0, r64i)uniform i64image1DArray daxa_AtomicImageTablei64image1DArray[];int64_t imageAtomicCompSwap(daxa_RWImage1DArrayi64 image, ivec2 index, int64_t compare, int64_t data){ return imageAtomicCompSwap(daxa_AtomicImageTablei64image1DArray[daxa_id_to_index(image . id)], index, compare, data);} int64_t imageAtomicExchange(daxa_RWImage1DArrayi64 image, ivec2 index, int64_t data){ return imageAtomicExchange(daxa_AtomicImageTablei64image1DArray[daxa_id_to_index(image . id)], index, data);} int64_t imageAtomicAdd(daxa_RWImage1DArrayi64 image, ivec2 index, int64_t data){ return imageAtomicAdd(daxa_AtomicImageTablei64image1DArray[daxa_id_to_index(image . id)], index, data);} int64_t imageAtomicAnd(daxa_RWImage1DArrayi64 image, ivec2 index, int64_t data){ return imageAtomicAnd(daxa_AtomicImageTablei64image1DArray[daxa_id_to_index(image . id)], index, data);} int64_t imageAtomicOr(daxa_RWImage1DArrayi64 image, ivec2 index, int64_t data){ return imageAtomicOr(daxa_AtomicImageTablei64image1DArray[daxa_id_to_index(image . id)], index, data);} int64_t imageAtomicXor(daxa_RWImage1DArrayi64 image, ivec2 index, int64_t data){ return imageAtomicXor(daxa_AtomicImageTablei64image1DArray[daxa_id_to_index(image . id)], index, data);} int64_t imageAtomicMin(daxa_RWImage1DArrayi64 image, ivec2 index, int64_t data){ return imageAtomicMin(daxa_AtomicImageTablei64image1DArray[daxa_id_to_index(image . id)], index, data);} int64_t imageAtomicMax(daxa_RWImage1DArrayi64 image, ivec2 index, int64_t data){ return imageAtomicMax(daxa_AtomicImageTablei64image1DArray[daxa_id_to_index(image . id)], index, data);} layout(binding = 1, set = 0, r64ui)uniform u64image1DArray daxa_AtomicImageTableu64image1DArray[];uint64_t imageAtomicCompSwap(daxa_RWImage1DArrayu64 image, ivec2 index, uint64_t compare, uint64_t data){ return imageAtomicCompSwap(daxa_AtomicImageTableu64image1DArray[daxa_id_to_index(image . id)], index, compare, data);} uint64_t imageAtomicExchange(daxa_RWImage1DArrayu64 image, ivec2 index, uint64_t data){ return imageAtomicExchange(daxa_AtomicImageTableu64image1DArray[daxa_id_to_index(image . id)], index, data);} uint64_t imageAtomicAdd(daxa_RWImage1DArrayu64 image, ivec2 index, uint64_t data){ return imageAtomicAdd(daxa_AtomicImageTableu64image1DArray[daxa_id_to_index(image . id)], index, data);} uint64_t imageAtomicAnd(daxa_RWImage1DArrayu64 image, ivec2 index, uint64_t data){ return imageAtomicAnd(daxa_AtomicImageTableu64image1DArray[daxa_id_to_index(image . id)], index, data);} uint64_t imageAtomicOr(daxa_RWImage1DArrayu64 image, ivec2 index, uint64_t data){ return imageAtomicOr(daxa_AtomicImageTableu64image1DArray[daxa_id_to_index(image . id)], index, data);} uint64_t imageAtomicXor(daxa_RWImage1DArrayu64 image, ivec2 index, uint64_t data){ return imageAtomicXor(daxa_AtomicImageTableu64image1DArray[daxa_id_to_index(image . id)], index, data);} uint64_t imageAtomicMin(daxa_RWImage1DArrayu64 image, ivec2 index, uint64_t data){ return imageAtomicMin(daxa_AtomicImageTableu64image1DArray[daxa_id_to_index(image . id)], index, data);} uint64_t imageAtomicMax(daxa_RWImage1DArrayu64 image, ivec2 index, uint64_t data){ return imageAtomicMax(daxa_AtomicImageTableu64image1DArray[daxa_id_to_index(image . id)], index, data);}
                                                     layout(binding = 1, set = 0, r64i)uniform i64image2DArray daxa_AtomicImageTablei64image2DArray[];int64_t imageAtomicCompSwap(daxa_RWImage2DArrayi64 image, ivec3 index, int64_t compare, int64_t data){ return imageAtomicCompSwap(daxa_AtomicImageTablei64image2DArray[daxa_id_to_index(image . id)], index, compare, data);} int64_t imageAtomicExchange(daxa_RWImage2DArrayi64 image, ivec3 index, int64_t data){ return imageAtomicExchange(daxa_AtomicImageTablei64image2DArray[daxa_id_to_index(image . id)], index, data);} int64_t imageAtomicAdd(daxa_RWImage2DArrayi64 image, ivec3 index, int64_t data){ return imageAtomicAdd(daxa_AtomicImageTablei64image2DArray[daxa_id_to_index(image . id)], index, data);} int64_t imageAtomicAnd(daxa_RWImage2DArrayi64 image, ivec3 index, int64_t data){ return imageAtomicAnd(daxa_AtomicImageTablei64image2DArray[daxa_id_to_index(image . id)], index, data);} int64_t imageAtomicOr(daxa_RWImage2DArrayi64 image, ivec3 index, int64_t data){ return imageAtomicOr(daxa_AtomicImageTablei64image2DArray[daxa_id_to_index(image . id)], index, data);} int64_t imageAtomicXor(daxa_RWImage2DArrayi64 image, ivec3 index, int64_t data){ return imageAtomicXor(daxa_AtomicImageTablei64image2DArray[daxa_id_to_index(image . id)], index, data);} int64_t imageAtomicMin(daxa_RWImage2DArrayi64 image, ivec3 index, int64_t data){ return imageAtomicMin(daxa_AtomicImageTablei64image2DArray[daxa_id_to_index(image . id)], index, data);} int64_t imageAtomicMax(daxa_RWImage2DArrayi64 image, ivec3 index, int64_t data){ return imageAtomicMax(daxa_AtomicImageTablei64image2DArray[daxa_id_to_index(image . id)], index, data);} layout(binding = 1, set = 0, r64ui)uniform u64image2DArray daxa_AtomicImageTableu64image2DArray[];uint64_t imageAtomicCompSwap(daxa_RWImage2DArrayu64 image, ivec3 index, uint64_t compare, uint64_t data){ return imageAtomicCompSwap(daxa_AtomicImageTableu64image2DArray[daxa_id_to_index(image . id)], index, compare, data);} uint64_t imageAtomicExchange(daxa_RWImage2DArrayu64 image, ivec3 index, uint64_t data){ return imageAtomicExchange(daxa_AtomicImageTableu64image2DArray[daxa_id_to_index(image . id)], index, data);} uint64_t imageAtomicAdd(daxa_RWImage2DArrayu64 image, ivec3 index, uint64_t data){ return imageAtomicAdd(daxa_AtomicImageTableu64image2DArray[daxa_id_to_index(image . id)], index, data);} uint64_t imageAtomicAnd(daxa_RWImage2DArrayu64 image, ivec3 index, uint64_t data){ return imageAtomicAnd(daxa_AtomicImageTableu64image2DArray[daxa_id_to_index(image . id)], index, data);} uint64_t imageAtomicOr(daxa_RWImage2DArrayu64 image, ivec3 index, uint64_t data){ return imageAtomicOr(daxa_AtomicImageTableu64image2DArray[daxa_id_to_index(image . id)], index, data);} uint64_t imageAtomicXor(daxa_RWImage2DArrayu64 image, ivec3 index, uint64_t data){ return imageAtomicXor(daxa_AtomicImageTableu64image2DArray[daxa_id_to_index(image . id)], index, data);} uint64_t imageAtomicMin(daxa_RWImage2DArrayu64 image, ivec3 index, uint64_t data){ return imageAtomicMin(daxa_AtomicImageTableu64image2DArray[daxa_id_to_index(image . id)], index, data);} uint64_t imageAtomicMax(daxa_RWImage2DArrayu64 image, ivec3 index, uint64_t data){ return imageAtomicMax(daxa_AtomicImageTableu64image2DArray[daxa_id_to_index(image . id)], index, data);}



                                                  layout(binding = 1, set = 0, r64i)uniform i64image2DMS daxa_AtomicImageTablei64image2DMS[];int64_t imageAtomicCompSwap(daxa_RWImage2DMSi64 image, ivec2 index, int s, int64_t compare, int64_t data){ return imageAtomicCompSwap(daxa_AtomicImageTablei64image2DMS[daxa_id_to_index(image . id)], index, s, compare, data);} int64_t imageAtomicExchange(daxa_RWImage2DMSi64 image, ivec2 index, int s, int64_t data){ return imageAtomicExchange(daxa_AtomicImageTablei64image2DMS[daxa_id_to_index(image . id)], index, s, data);} int64_t imageAtomicAdd(daxa_RWImage2DMSi64 image, ivec2 index, int s, int64_t data){ return imageAtomicAdd(daxa_AtomicImageTablei64image2DMS[daxa_id_to_index(image . id)], index, s, data);} int64_t imageAtomicAnd(daxa_RWImage2DMSi64 image, ivec2 index, int s, int64_t data){ return imageAtomicAnd(daxa_AtomicImageTablei64image2DMS[daxa_id_to_index(image . id)], index, s, data);} int64_t imageAtomicOr(daxa_RWImage2DMSi64 image, ivec2 index, int s, int64_t data){ return imageAtomicOr(daxa_AtomicImageTablei64image2DMS[daxa_id_to_index(image . id)], index, s, data);} int64_t imageAtomicXor(daxa_RWImage2DMSi64 image, ivec2 index, int s, int64_t data){ return imageAtomicXor(daxa_AtomicImageTablei64image2DMS[daxa_id_to_index(image . id)], index, s, data);} int64_t imageAtomicMin(daxa_RWImage2DMSi64 image, ivec2 index, int s, int64_t data){ return imageAtomicMin(daxa_AtomicImageTablei64image2DMS[daxa_id_to_index(image . id)], index, s, data);} int64_t imageAtomicMax(daxa_RWImage2DMSi64 image, ivec2 index, int s, int64_t data){ return imageAtomicMax(daxa_AtomicImageTablei64image2DMS[daxa_id_to_index(image . id)], index, s, data);} layout(binding = 1, set = 0, r64ui)uniform u64image2DMS daxa_AtomicImageTableu64image2DMS[];uint64_t imageAtomicCompSwap(daxa_RWImage2DMSu64 image, ivec2 index, int s, uint64_t compare, uint64_t data){ return imageAtomicCompSwap(daxa_AtomicImageTableu64image2DMS[daxa_id_to_index(image . id)], index, s, compare, data);} uint64_t imageAtomicExchange(daxa_RWImage2DMSu64 image, ivec2 index, int s, uint64_t data){ return imageAtomicExchange(daxa_AtomicImageTableu64image2DMS[daxa_id_to_index(image . id)], index, s, data);} uint64_t imageAtomicAdd(daxa_RWImage2DMSu64 image, ivec2 index, int s, uint64_t data){ return imageAtomicAdd(daxa_AtomicImageTableu64image2DMS[daxa_id_to_index(image . id)], index, s, data);} uint64_t imageAtomicAnd(daxa_RWImage2DMSu64 image, ivec2 index, int s, uint64_t data){ return imageAtomicAnd(daxa_AtomicImageTableu64image2DMS[daxa_id_to_index(image . id)], index, s, data);} uint64_t imageAtomicOr(daxa_RWImage2DMSu64 image, ivec2 index, int s, uint64_t data){ return imageAtomicOr(daxa_AtomicImageTableu64image2DMS[daxa_id_to_index(image . id)], index, s, data);} uint64_t imageAtomicXor(daxa_RWImage2DMSu64 image, ivec2 index, int s, uint64_t data){ return imageAtomicXor(daxa_AtomicImageTableu64image2DMS[daxa_id_to_index(image . id)], index, s, data);} uint64_t imageAtomicMin(daxa_RWImage2DMSu64 image, ivec2 index, int s, uint64_t data){ return imageAtomicMin(daxa_AtomicImageTableu64image2DMS[daxa_id_to_index(image . id)], index, s, data);} uint64_t imageAtomicMax(daxa_RWImage2DMSu64 image, ivec2 index, int s, uint64_t data){ return imageAtomicMax(daxa_AtomicImageTableu64image2DMS[daxa_id_to_index(image . id)], index, s, data);}
                                                       layout(binding = 1, set = 0, r64i)uniform i64image2DMSArray daxa_AtomicImageTablei64image2DMSArray[];int64_t imageAtomicCompSwap(daxa_RWImage2DMSArrayi64 image, ivec3 index, int s, int64_t compare, int64_t data){ return imageAtomicCompSwap(daxa_AtomicImageTablei64image2DMSArray[daxa_id_to_index(image . id)], index, s, compare, data);} int64_t imageAtomicExchange(daxa_RWImage2DMSArrayi64 image, ivec3 index, int s, int64_t data){ return imageAtomicExchange(daxa_AtomicImageTablei64image2DMSArray[daxa_id_to_index(image . id)], index, s, data);} int64_t imageAtomicAdd(daxa_RWImage2DMSArrayi64 image, ivec3 index, int s, int64_t data){ return imageAtomicAdd(daxa_AtomicImageTablei64image2DMSArray[daxa_id_to_index(image . id)], index, s, data);} int64_t imageAtomicAnd(daxa_RWImage2DMSArrayi64 image, ivec3 index, int s, int64_t data){ return imageAtomicAnd(daxa_AtomicImageTablei64image2DMSArray[daxa_id_to_index(image . id)], index, s, data);} int64_t imageAtomicOr(daxa_RWImage2DMSArrayi64 image, ivec3 index, int s, int64_t data){ return imageAtomicOr(daxa_AtomicImageTablei64image2DMSArray[daxa_id_to_index(image . id)], index, s, data);} int64_t imageAtomicXor(daxa_RWImage2DMSArrayi64 image, ivec3 index, int s, int64_t data){ return imageAtomicXor(daxa_AtomicImageTablei64image2DMSArray[daxa_id_to_index(image . id)], index, s, data);} int64_t imageAtomicMin(daxa_RWImage2DMSArrayi64 image, ivec3 index, int s, int64_t data){ return imageAtomicMin(daxa_AtomicImageTablei64image2DMSArray[daxa_id_to_index(image . id)], index, s, data);} int64_t imageAtomicMax(daxa_RWImage2DMSArrayi64 image, ivec3 index, int s, int64_t data){ return imageAtomicMax(daxa_AtomicImageTablei64image2DMSArray[daxa_id_to_index(image . id)], index, s, data);} layout(binding = 1, set = 0, r64ui)uniform u64image2DMSArray daxa_AtomicImageTableu64image2DMSArray[];uint64_t imageAtomicCompSwap(daxa_RWImage2DMSArrayu64 image, ivec3 index, int s, uint64_t compare, uint64_t data){ return imageAtomicCompSwap(daxa_AtomicImageTableu64image2DMSArray[daxa_id_to_index(image . id)], index, s, compare, data);} uint64_t imageAtomicExchange(daxa_RWImage2DMSArrayu64 image, ivec3 index, int s, uint64_t data){ return imageAtomicExchange(daxa_AtomicImageTableu64image2DMSArray[daxa_id_to_index(image . id)], index, s, data);} uint64_t imageAtomicAdd(daxa_RWImage2DMSArrayu64 image, ivec3 index, int s, uint64_t data){ return imageAtomicAdd(daxa_AtomicImageTableu64image2DMSArray[daxa_id_to_index(image . id)], index, s, data);} uint64_t imageAtomicAnd(daxa_RWImage2DMSArrayu64 image, ivec3 index, int s, uint64_t data){ return imageAtomicAnd(daxa_AtomicImageTableu64image2DMSArray[daxa_id_to_index(image . id)], index, s, data);} uint64_t imageAtomicOr(daxa_RWImage2DMSArrayu64 image, ivec3 index, int s, uint64_t data){ return imageAtomicOr(daxa_AtomicImageTableu64image2DMSArray[daxa_id_to_index(image . id)], index, s, data);} uint64_t imageAtomicXor(daxa_RWImage2DMSArrayu64 image, ivec3 index, int s, uint64_t data){ return imageAtomicXor(daxa_AtomicImageTableu64image2DMSArray[daxa_id_to_index(image . id)], index, s, data);} uint64_t imageAtomicMin(daxa_RWImage2DMSArrayu64 image, ivec3 index, int s, uint64_t data){ return imageAtomicMin(daxa_AtomicImageTableu64image2DMSArray[daxa_id_to_index(image . id)], index, s, data);} uint64_t imageAtomicMax(daxa_RWImage2DMSArrayu64 image, ivec3 index, int s, uint64_t data){ return imageAtomicMax(daxa_AtomicImageTableu64image2DMSArray[daxa_id_to_index(image . id)], index, s, data);}


















































































































                                      vec4 texture(daxa_Image1Df32 image, daxa_SamplerId sampler_id, float uv){ return texture(sampler1D(daxa_ImageTabletexture1D[daxa_id_to_index(image . id)], daxa_SamplerTable[daxa_id_to_index(sampler_id)]), uv);} vec4 textureLod(daxa_Image1Df32 image, daxa_SamplerId sampler_id, float uv, float bias){ return textureLod(sampler1D(daxa_ImageTabletexture1D[daxa_id_to_index(image . id)], daxa_SamplerTable[daxa_id_to_index(sampler_id)]), uv, bias);} vec4 textureGrad(daxa_Image1Df32 image, daxa_SamplerId sampler_id, float uv, float dTdx, float dTdy){ return textureGrad(sampler1D(daxa_ImageTabletexture1D[daxa_id_to_index(image . id)], daxa_SamplerTable[daxa_id_to_index(sampler_id)]), uv, dTdx, dTdy);} int textureSize(daxa_Image1Df32 image, daxa_SamplerId sampler_id, int lod){ return textureSize(sampler1D(daxa_ImageTabletexture1D[daxa_id_to_index(image . id)], daxa_SamplerTable[daxa_id_to_index(sampler_id)]), lod);} int textureQueryLevels(daxa_Image1Df32 image, daxa_SamplerId sampler_id){ return textureQueryLevels(sampler1D(daxa_ImageTabletexture1D[daxa_id_to_index(image . id)], daxa_SamplerTable[daxa_id_to_index(sampler_id)]));} ivec4 texture(daxa_Image1Di32 image, daxa_SamplerId sampler_id, float uv){ return texture(isampler1D(daxa_ImageTableitexture1D[daxa_id_to_index(image . id)], daxa_SamplerTable[daxa_id_to_index(sampler_id)]), uv);} ivec4 textureLod(daxa_Image1Di32 image, daxa_SamplerId sampler_id, float uv, float bias){ return textureLod(isampler1D(daxa_ImageTableitexture1D[daxa_id_to_index(image . id)], daxa_SamplerTable[daxa_id_to_index(sampler_id)]), uv, bias);} ivec4 textureGrad(daxa_Image1Di32 image, daxa_SamplerId sampler_id, float uv, float dTdx, float dTdy){ return textureGrad(isampler1D(daxa_ImageTableitexture1D[daxa_id_to_index(image . id)], daxa_SamplerTable[daxa_id_to_index(sampler_id)]), uv, dTdx, dTdy);} int textureSize(daxa_Image1Di32 image, daxa_SamplerId sampler_id, int lod){ return textureSize(isampler1D(daxa_ImageTableitexture1D[daxa_id_to_index(image . id)], daxa_SamplerTable[daxa_id_to_index(sampler_id)]), lod);} int textureQueryLevels(daxa_Image1Di32 image, daxa_SamplerId sampler_id){ return textureQueryLevels(isampler1D(daxa_ImageTableitexture1D[daxa_id_to_index(image . id)], daxa_SamplerTable[daxa_id_to_index(sampler_id)]));} uvec4 texture(daxa_Image1Du32 image, daxa_SamplerId sampler_id, float uv){ return texture(usampler1D(daxa_ImageTableutexture1D[daxa_id_to_index(image . id)], daxa_SamplerTable[daxa_id_to_index(sampler_id)]), uv);} uvec4 textureLod(daxa_Image1Du32 image, daxa_SamplerId sampler_id, float uv, float bias){ return textureLod(usampler1D(daxa_ImageTableutexture1D[daxa_id_to_index(image . id)], daxa_SamplerTable[daxa_id_to_index(sampler_id)]), uv, bias);} uvec4 textureGrad(daxa_Image1Du32 image, daxa_SamplerId sampler_id, float uv, float dTdx, float dTdy){ return textureGrad(usampler1D(daxa_ImageTableutexture1D[daxa_id_to_index(image . id)], daxa_SamplerTable[daxa_id_to_index(sampler_id)]), uv, dTdx, dTdy);} int textureSize(daxa_Image1Du32 image, daxa_SamplerId sampler_id, int lod){ return textureSize(usampler1D(daxa_ImageTableutexture1D[daxa_id_to_index(image . id)], daxa_SamplerTable[daxa_id_to_index(sampler_id)]), lod);} int textureQueryLevels(daxa_Image1Du32 image, daxa_SamplerId sampler_id){ return textureQueryLevels(usampler1D(daxa_ImageTableutexture1D[daxa_id_to_index(image . id)], daxa_SamplerTable[daxa_id_to_index(sampler_id)]));}
                                      vec4 texture(daxa_Image2Df32 image, daxa_SamplerId sampler_id, vec2 uv){ return texture(sampler2D(daxa_ImageTabletexture2D[daxa_id_to_index(image . id)], daxa_SamplerTable[daxa_id_to_index(sampler_id)]), uv);} vec4 textureLod(daxa_Image2Df32 image, daxa_SamplerId sampler_id, vec2 uv, float bias){ return textureLod(sampler2D(daxa_ImageTabletexture2D[daxa_id_to_index(image . id)], daxa_SamplerTable[daxa_id_to_index(sampler_id)]), uv, bias);} vec4 textureGrad(daxa_Image2Df32 image, daxa_SamplerId sampler_id, vec2 uv, vec2 dTdx, vec2 dTdy){ return textureGrad(sampler2D(daxa_ImageTabletexture2D[daxa_id_to_index(image . id)], daxa_SamplerTable[daxa_id_to_index(sampler_id)]), uv, dTdx, dTdy);} ivec2 textureSize(daxa_Image2Df32 image, daxa_SamplerId sampler_id, int lod){ return textureSize(sampler2D(daxa_ImageTabletexture2D[daxa_id_to_index(image . id)], daxa_SamplerTable[daxa_id_to_index(sampler_id)]), lod);} int textureQueryLevels(daxa_Image2Df32 image, daxa_SamplerId sampler_id){ return textureQueryLevels(sampler2D(daxa_ImageTabletexture2D[daxa_id_to_index(image . id)], daxa_SamplerTable[daxa_id_to_index(sampler_id)]));} ivec4 texture(daxa_Image2Di32 image, daxa_SamplerId sampler_id, vec2 uv){ return texture(isampler2D(daxa_ImageTableitexture2D[daxa_id_to_index(image . id)], daxa_SamplerTable[daxa_id_to_index(sampler_id)]), uv);} ivec4 textureLod(daxa_Image2Di32 image, daxa_SamplerId sampler_id, vec2 uv, float bias){ return textureLod(isampler2D(daxa_ImageTableitexture2D[daxa_id_to_index(image . id)], daxa_SamplerTable[daxa_id_to_index(sampler_id)]), uv, bias);} ivec4 textureGrad(daxa_Image2Di32 image, daxa_SamplerId sampler_id, vec2 uv, vec2 dTdx, vec2 dTdy){ return textureGrad(isampler2D(daxa_ImageTableitexture2D[daxa_id_to_index(image . id)], daxa_SamplerTable[daxa_id_to_index(sampler_id)]), uv, dTdx, dTdy);} ivec2 textureSize(daxa_Image2Di32 image, daxa_SamplerId sampler_id, int lod){ return textureSize(isampler2D(daxa_ImageTableitexture2D[daxa_id_to_index(image . id)], daxa_SamplerTable[daxa_id_to_index(sampler_id)]), lod);} int textureQueryLevels(daxa_Image2Di32 image, daxa_SamplerId sampler_id){ return textureQueryLevels(isampler2D(daxa_ImageTableitexture2D[daxa_id_to_index(image . id)], daxa_SamplerTable[daxa_id_to_index(sampler_id)]));} uvec4 texture(daxa_Image2Du32 image, daxa_SamplerId sampler_id, vec2 uv){ return texture(usampler2D(daxa_ImageTableutexture2D[daxa_id_to_index(image . id)], daxa_SamplerTable[daxa_id_to_index(sampler_id)]), uv);} uvec4 textureLod(daxa_Image2Du32 image, daxa_SamplerId sampler_id, vec2 uv, float bias){ return textureLod(usampler2D(daxa_ImageTableutexture2D[daxa_id_to_index(image . id)], daxa_SamplerTable[daxa_id_to_index(sampler_id)]), uv, bias);} uvec4 textureGrad(daxa_Image2Du32 image, daxa_SamplerId sampler_id, vec2 uv, vec2 dTdx, vec2 dTdy){ return textureGrad(usampler2D(daxa_ImageTableutexture2D[daxa_id_to_index(image . id)], daxa_SamplerTable[daxa_id_to_index(sampler_id)]), uv, dTdx, dTdy);} ivec2 textureSize(daxa_Image2Du32 image, daxa_SamplerId sampler_id, int lod){ return textureSize(usampler2D(daxa_ImageTableutexture2D[daxa_id_to_index(image . id)], daxa_SamplerTable[daxa_id_to_index(sampler_id)]), lod);} int textureQueryLevels(daxa_Image2Du32 image, daxa_SamplerId sampler_id){ return textureQueryLevels(usampler2D(daxa_ImageTableutexture2D[daxa_id_to_index(image . id)], daxa_SamplerTable[daxa_id_to_index(sampler_id)]));}
                                      vec4 texture(daxa_Image3Df32 image, daxa_SamplerId sampler_id, vec3 uv){ return texture(sampler3D(daxa_ImageTabletexture3D[daxa_id_to_index(image . id)], daxa_SamplerTable[daxa_id_to_index(sampler_id)]), uv);} vec4 textureLod(daxa_Image3Df32 image, daxa_SamplerId sampler_id, vec3 uv, float bias){ return textureLod(sampler3D(daxa_ImageTabletexture3D[daxa_id_to_index(image . id)], daxa_SamplerTable[daxa_id_to_index(sampler_id)]), uv, bias);} vec4 textureGrad(daxa_Image3Df32 image, daxa_SamplerId sampler_id, vec3 uv, vec3 dTdx, vec3 dTdy){ return textureGrad(sampler3D(daxa_ImageTabletexture3D[daxa_id_to_index(image . id)], daxa_SamplerTable[daxa_id_to_index(sampler_id)]), uv, dTdx, dTdy);} ivec3 textureSize(daxa_Image3Df32 image, daxa_SamplerId sampler_id, int lod){ return textureSize(sampler3D(daxa_ImageTabletexture3D[daxa_id_to_index(image . id)], daxa_SamplerTable[daxa_id_to_index(sampler_id)]), lod);} int textureQueryLevels(daxa_Image3Df32 image, daxa_SamplerId sampler_id){ return textureQueryLevels(sampler3D(daxa_ImageTabletexture3D[daxa_id_to_index(image . id)], daxa_SamplerTable[daxa_id_to_index(sampler_id)]));} ivec4 texture(daxa_Image3Di32 image, daxa_SamplerId sampler_id, vec3 uv){ return texture(isampler3D(daxa_ImageTableitexture3D[daxa_id_to_index(image . id)], daxa_SamplerTable[daxa_id_to_index(sampler_id)]), uv);} ivec4 textureLod(daxa_Image3Di32 image, daxa_SamplerId sampler_id, vec3 uv, float bias){ return textureLod(isampler3D(daxa_ImageTableitexture3D[daxa_id_to_index(image . id)], daxa_SamplerTable[daxa_id_to_index(sampler_id)]), uv, bias);} ivec4 textureGrad(daxa_Image3Di32 image, daxa_SamplerId sampler_id, vec3 uv, vec3 dTdx, vec3 dTdy){ return textureGrad(isampler3D(daxa_ImageTableitexture3D[daxa_id_to_index(image . id)], daxa_SamplerTable[daxa_id_to_index(sampler_id)]), uv, dTdx, dTdy);} ivec3 textureSize(daxa_Image3Di32 image, daxa_SamplerId sampler_id, int lod){ return textureSize(isampler3D(daxa_ImageTableitexture3D[daxa_id_to_index(image . id)], daxa_SamplerTable[daxa_id_to_index(sampler_id)]), lod);} int textureQueryLevels(daxa_Image3Di32 image, daxa_SamplerId sampler_id){ return textureQueryLevels(isampler3D(daxa_ImageTableitexture3D[daxa_id_to_index(image . id)], daxa_SamplerTable[daxa_id_to_index(sampler_id)]));} uvec4 texture(daxa_Image3Du32 image, daxa_SamplerId sampler_id, vec3 uv){ return texture(usampler3D(daxa_ImageTableutexture3D[daxa_id_to_index(image . id)], daxa_SamplerTable[daxa_id_to_index(sampler_id)]), uv);} uvec4 textureLod(daxa_Image3Du32 image, daxa_SamplerId sampler_id, vec3 uv, float bias){ return textureLod(usampler3D(daxa_ImageTableutexture3D[daxa_id_to_index(image . id)], daxa_SamplerTable[daxa_id_to_index(sampler_id)]), uv, bias);} uvec4 textureGrad(daxa_Image3Du32 image, daxa_SamplerId sampler_id, vec3 uv, vec3 dTdx, vec3 dTdy){ return textureGrad(usampler3D(daxa_ImageTableutexture3D[daxa_id_to_index(image . id)], daxa_SamplerTable[daxa_id_to_index(sampler_id)]), uv, dTdx, dTdy);} ivec3 textureSize(daxa_Image3Du32 image, daxa_SamplerId sampler_id, int lod){ return textureSize(usampler3D(daxa_ImageTableutexture3D[daxa_id_to_index(image . id)], daxa_SamplerTable[daxa_id_to_index(sampler_id)]), lod);} int textureQueryLevels(daxa_Image3Du32 image, daxa_SamplerId sampler_id){ return textureQueryLevels(usampler3D(daxa_ImageTableutexture3D[daxa_id_to_index(image . id)], daxa_SamplerTable[daxa_id_to_index(sampler_id)]));}
                                        vec4 texture(daxa_ImageCubef32 image, daxa_SamplerId sampler_id, vec3 uv){ return texture(samplerCube(daxa_ImageTabletextureCube[daxa_id_to_index(image . id)], daxa_SamplerTable[daxa_id_to_index(sampler_id)]), uv);} vec4 textureLod(daxa_ImageCubef32 image, daxa_SamplerId sampler_id, vec3 uv, float bias){ return textureLod(samplerCube(daxa_ImageTabletextureCube[daxa_id_to_index(image . id)], daxa_SamplerTable[daxa_id_to_index(sampler_id)]), uv, bias);} vec4 textureGrad(daxa_ImageCubef32 image, daxa_SamplerId sampler_id, vec3 uv, vec3 dTdx, vec3 dTdy){ return textureGrad(samplerCube(daxa_ImageTabletextureCube[daxa_id_to_index(image . id)], daxa_SamplerTable[daxa_id_to_index(sampler_id)]), uv, dTdx, dTdy);} ivec2 textureSize(daxa_ImageCubef32 image, daxa_SamplerId sampler_id, int lod){ return textureSize(samplerCube(daxa_ImageTabletextureCube[daxa_id_to_index(image . id)], daxa_SamplerTable[daxa_id_to_index(sampler_id)]), lod);} int textureQueryLevels(daxa_ImageCubef32 image, daxa_SamplerId sampler_id){ return textureQueryLevels(samplerCube(daxa_ImageTabletextureCube[daxa_id_to_index(image . id)], daxa_SamplerTable[daxa_id_to_index(sampler_id)]));} ivec4 texture(daxa_ImageCubei32 image, daxa_SamplerId sampler_id, vec3 uv){ return texture(isamplerCube(daxa_ImageTableitextureCube[daxa_id_to_index(image . id)], daxa_SamplerTable[daxa_id_to_index(sampler_id)]), uv);} ivec4 textureLod(daxa_ImageCubei32 image, daxa_SamplerId sampler_id, vec3 uv, float bias){ return textureLod(isamplerCube(daxa_ImageTableitextureCube[daxa_id_to_index(image . id)], daxa_SamplerTable[daxa_id_to_index(sampler_id)]), uv, bias);} ivec4 textureGrad(daxa_ImageCubei32 image, daxa_SamplerId sampler_id, vec3 uv, vec3 dTdx, vec3 dTdy){ return textureGrad(isamplerCube(daxa_ImageTableitextureCube[daxa_id_to_index(image . id)], daxa_SamplerTable[daxa_id_to_index(sampler_id)]), uv, dTdx, dTdy);} ivec2 textureSize(daxa_ImageCubei32 image, daxa_SamplerId sampler_id, int lod){ return textureSize(isamplerCube(daxa_ImageTableitextureCube[daxa_id_to_index(image . id)], daxa_SamplerTable[daxa_id_to_index(sampler_id)]), lod);} int textureQueryLevels(daxa_ImageCubei32 image, daxa_SamplerId sampler_id){ return textureQueryLevels(isamplerCube(daxa_ImageTableitextureCube[daxa_id_to_index(image . id)], daxa_SamplerTable[daxa_id_to_index(sampler_id)]));} uvec4 texture(daxa_ImageCubeu32 image, daxa_SamplerId sampler_id, vec3 uv){ return texture(usamplerCube(daxa_ImageTableutextureCube[daxa_id_to_index(image . id)], daxa_SamplerTable[daxa_id_to_index(sampler_id)]), uv);} uvec4 textureLod(daxa_ImageCubeu32 image, daxa_SamplerId sampler_id, vec3 uv, float bias){ return textureLod(usamplerCube(daxa_ImageTableutextureCube[daxa_id_to_index(image . id)], daxa_SamplerTable[daxa_id_to_index(sampler_id)]), uv, bias);} uvec4 textureGrad(daxa_ImageCubeu32 image, daxa_SamplerId sampler_id, vec3 uv, vec3 dTdx, vec3 dTdy){ return textureGrad(usamplerCube(daxa_ImageTableutextureCube[daxa_id_to_index(image . id)], daxa_SamplerTable[daxa_id_to_index(sampler_id)]), uv, dTdx, dTdy);} ivec2 textureSize(daxa_ImageCubeu32 image, daxa_SamplerId sampler_id, int lod){ return textureSize(usamplerCube(daxa_ImageTableutextureCube[daxa_id_to_index(image . id)], daxa_SamplerTable[daxa_id_to_index(sampler_id)]), lod);} int textureQueryLevels(daxa_ImageCubeu32 image, daxa_SamplerId sampler_id){ return textureQueryLevels(usamplerCube(daxa_ImageTableutextureCube[daxa_id_to_index(image . id)], daxa_SamplerTable[daxa_id_to_index(sampler_id)]));}
                                             vec4 texture(daxa_ImageCubeArrayf32 image, daxa_SamplerId sampler_id, vec4 uv){ return texture(samplerCubeArray(daxa_ImageTabletextureCubeArray[daxa_id_to_index(image . id)], daxa_SamplerTable[daxa_id_to_index(sampler_id)]), uv);} vec4 textureLod(daxa_ImageCubeArrayf32 image, daxa_SamplerId sampler_id, vec4 uv, float bias){ return textureLod(samplerCubeArray(daxa_ImageTabletextureCubeArray[daxa_id_to_index(image . id)], daxa_SamplerTable[daxa_id_to_index(sampler_id)]), uv, bias);} vec4 textureGrad(daxa_ImageCubeArrayf32 image, daxa_SamplerId sampler_id, vec4 uv, vec3 dTdx, vec3 dTdy){ return textureGrad(samplerCubeArray(daxa_ImageTabletextureCubeArray[daxa_id_to_index(image . id)], daxa_SamplerTable[daxa_id_to_index(sampler_id)]), uv, dTdx, dTdy);} ivec3 textureSize(daxa_ImageCubeArrayf32 image, daxa_SamplerId sampler_id, int lod){ return textureSize(samplerCubeArray(daxa_ImageTabletextureCubeArray[daxa_id_to_index(image . id)], daxa_SamplerTable[daxa_id_to_index(sampler_id)]), lod);} int textureQueryLevels(daxa_ImageCubeArrayf32 image, daxa_SamplerId sampler_id){ return textureQueryLevels(samplerCubeArray(daxa_ImageTabletextureCubeArray[daxa_id_to_index(image . id)], daxa_SamplerTable[daxa_id_to_index(sampler_id)]));} ivec4 texture(daxa_ImageCubeArrayi32 image, daxa_SamplerId sampler_id, vec4 uv){ return texture(isamplerCubeArray(daxa_ImageTableitextureCubeArray[daxa_id_to_index(image . id)], daxa_SamplerTable[daxa_id_to_index(sampler_id)]), uv);} ivec4 textureLod(daxa_ImageCubeArrayi32 image, daxa_SamplerId sampler_id, vec4 uv, float bias){ return textureLod(isamplerCubeArray(daxa_ImageTableitextureCubeArray[daxa_id_to_index(image . id)], daxa_SamplerTable[daxa_id_to_index(sampler_id)]), uv, bias);} ivec4 textureGrad(daxa_ImageCubeArrayi32 image, daxa_SamplerId sampler_id, vec4 uv, vec3 dTdx, vec3 dTdy){ return textureGrad(isamplerCubeArray(daxa_ImageTableitextureCubeArray[daxa_id_to_index(image . id)], daxa_SamplerTable[daxa_id_to_index(sampler_id)]), uv, dTdx, dTdy);} ivec3 textureSize(daxa_ImageCubeArrayi32 image, daxa_SamplerId sampler_id, int lod){ return textureSize(isamplerCubeArray(daxa_ImageTableitextureCubeArray[daxa_id_to_index(image . id)], daxa_SamplerTable[daxa_id_to_index(sampler_id)]), lod);} int textureQueryLevels(daxa_ImageCubeArrayi32 image, daxa_SamplerId sampler_id){ return textureQueryLevels(isamplerCubeArray(daxa_ImageTableitextureCubeArray[daxa_id_to_index(image . id)], daxa_SamplerTable[daxa_id_to_index(sampler_id)]));} uvec4 texture(daxa_ImageCubeArrayu32 image, daxa_SamplerId sampler_id, vec4 uv){ return texture(usamplerCubeArray(daxa_ImageTableutextureCubeArray[daxa_id_to_index(image . id)], daxa_SamplerTable[daxa_id_to_index(sampler_id)]), uv);} uvec4 textureLod(daxa_ImageCubeArrayu32 image, daxa_SamplerId sampler_id, vec4 uv, float bias){ return textureLod(usamplerCubeArray(daxa_ImageTableutextureCubeArray[daxa_id_to_index(image . id)], daxa_SamplerTable[daxa_id_to_index(sampler_id)]), uv, bias);} uvec4 textureGrad(daxa_ImageCubeArrayu32 image, daxa_SamplerId sampler_id, vec4 uv, vec3 dTdx, vec3 dTdy){ return textureGrad(usamplerCubeArray(daxa_ImageTableutextureCubeArray[daxa_id_to_index(image . id)], daxa_SamplerTable[daxa_id_to_index(sampler_id)]), uv, dTdx, dTdy);} ivec3 textureSize(daxa_ImageCubeArrayu32 image, daxa_SamplerId sampler_id, int lod){ return textureSize(usamplerCubeArray(daxa_ImageTableutextureCubeArray[daxa_id_to_index(image . id)], daxa_SamplerTable[daxa_id_to_index(sampler_id)]), lod);} int textureQueryLevels(daxa_ImageCubeArrayu32 image, daxa_SamplerId sampler_id){ return textureQueryLevels(usamplerCubeArray(daxa_ImageTableutextureCubeArray[daxa_id_to_index(image . id)], daxa_SamplerTable[daxa_id_to_index(sampler_id)]));}
                                           vec4 texture(daxa_Image1DArrayf32 image, daxa_SamplerId sampler_id, vec2 uv){ return texture(sampler1DArray(daxa_ImageTabletexture1DArray[daxa_id_to_index(image . id)], daxa_SamplerTable[daxa_id_to_index(sampler_id)]), uv);} vec4 textureLod(daxa_Image1DArrayf32 image, daxa_SamplerId sampler_id, vec2 uv, float bias){ return textureLod(sampler1DArray(daxa_ImageTabletexture1DArray[daxa_id_to_index(image . id)], daxa_SamplerTable[daxa_id_to_index(sampler_id)]), uv, bias);} vec4 textureGrad(daxa_Image1DArrayf32 image, daxa_SamplerId sampler_id, vec2 uv, float dTdx, float dTdy){ return textureGrad(sampler1DArray(daxa_ImageTabletexture1DArray[daxa_id_to_index(image . id)], daxa_SamplerTable[daxa_id_to_index(sampler_id)]), uv, dTdx, dTdy);} ivec2 textureSize(daxa_Image1DArrayf32 image, daxa_SamplerId sampler_id, int lod){ return textureSize(sampler1DArray(daxa_ImageTabletexture1DArray[daxa_id_to_index(image . id)], daxa_SamplerTable[daxa_id_to_index(sampler_id)]), lod);} int textureQueryLevels(daxa_Image1DArrayf32 image, daxa_SamplerId sampler_id){ return textureQueryLevels(sampler1DArray(daxa_ImageTabletexture1DArray[daxa_id_to_index(image . id)], daxa_SamplerTable[daxa_id_to_index(sampler_id)]));} ivec4 texture(daxa_Image1DArrayi32 image, daxa_SamplerId sampler_id, vec2 uv){ return texture(isampler1DArray(daxa_ImageTableitexture1DArray[daxa_id_to_index(image . id)], daxa_SamplerTable[daxa_id_to_index(sampler_id)]), uv);} ivec4 textureLod(daxa_Image1DArrayi32 image, daxa_SamplerId sampler_id, vec2 uv, float bias){ return textureLod(isampler1DArray(daxa_ImageTableitexture1DArray[daxa_id_to_index(image . id)], daxa_SamplerTable[daxa_id_to_index(sampler_id)]), uv, bias);} ivec4 textureGrad(daxa_Image1DArrayi32 image, daxa_SamplerId sampler_id, vec2 uv, float dTdx, float dTdy){ return textureGrad(isampler1DArray(daxa_ImageTableitexture1DArray[daxa_id_to_index(image . id)], daxa_SamplerTable[daxa_id_to_index(sampler_id)]), uv, dTdx, dTdy);} ivec2 textureSize(daxa_Image1DArrayi32 image, daxa_SamplerId sampler_id, int lod){ return textureSize(isampler1DArray(daxa_ImageTableitexture1DArray[daxa_id_to_index(image . id)], daxa_SamplerTable[daxa_id_to_index(sampler_id)]), lod);} int textureQueryLevels(daxa_Image1DArrayi32 image, daxa_SamplerId sampler_id){ return textureQueryLevels(isampler1DArray(daxa_ImageTableitexture1DArray[daxa_id_to_index(image . id)], daxa_SamplerTable[daxa_id_to_index(sampler_id)]));} uvec4 texture(daxa_Image1DArrayu32 image, daxa_SamplerId sampler_id, vec2 uv){ return texture(usampler1DArray(daxa_ImageTableutexture1DArray[daxa_id_to_index(image . id)], daxa_SamplerTable[daxa_id_to_index(sampler_id)]), uv);} uvec4 textureLod(daxa_Image1DArrayu32 image, daxa_SamplerId sampler_id, vec2 uv, float bias){ return textureLod(usampler1DArray(daxa_ImageTableutexture1DArray[daxa_id_to_index(image . id)], daxa_SamplerTable[daxa_id_to_index(sampler_id)]), uv, bias);} uvec4 textureGrad(daxa_Image1DArrayu32 image, daxa_SamplerId sampler_id, vec2 uv, float dTdx, float dTdy){ return textureGrad(usampler1DArray(daxa_ImageTableutexture1DArray[daxa_id_to_index(image . id)], daxa_SamplerTable[daxa_id_to_index(sampler_id)]), uv, dTdx, dTdy);} ivec2 textureSize(daxa_Image1DArrayu32 image, daxa_SamplerId sampler_id, int lod){ return textureSize(usampler1DArray(daxa_ImageTableutexture1DArray[daxa_id_to_index(image . id)], daxa_SamplerTable[daxa_id_to_index(sampler_id)]), lod);} int textureQueryLevels(daxa_Image1DArrayu32 image, daxa_SamplerId sampler_id){ return textureQueryLevels(usampler1DArray(daxa_ImageTableutexture1DArray[daxa_id_to_index(image . id)], daxa_SamplerTable[daxa_id_to_index(sampler_id)]));}
                                           vec4 texture(daxa_Image2DArrayf32 image, daxa_SamplerId sampler_id, vec3 uv){ return texture(sampler2DArray(daxa_ImageTabletexture2DArray[daxa_id_to_index(image . id)], daxa_SamplerTable[daxa_id_to_index(sampler_id)]), uv);} vec4 textureLod(daxa_Image2DArrayf32 image, daxa_SamplerId sampler_id, vec3 uv, float bias){ return textureLod(sampler2DArray(daxa_ImageTabletexture2DArray[daxa_id_to_index(image . id)], daxa_SamplerTable[daxa_id_to_index(sampler_id)]), uv, bias);} vec4 textureGrad(daxa_Image2DArrayf32 image, daxa_SamplerId sampler_id, vec3 uv, vec2 dTdx, vec2 dTdy){ return textureGrad(sampler2DArray(daxa_ImageTabletexture2DArray[daxa_id_to_index(image . id)], daxa_SamplerTable[daxa_id_to_index(sampler_id)]), uv, dTdx, dTdy);} ivec3 textureSize(daxa_Image2DArrayf32 image, daxa_SamplerId sampler_id, int lod){ return textureSize(sampler2DArray(daxa_ImageTabletexture2DArray[daxa_id_to_index(image . id)], daxa_SamplerTable[daxa_id_to_index(sampler_id)]), lod);} int textureQueryLevels(daxa_Image2DArrayf32 image, daxa_SamplerId sampler_id){ return textureQueryLevels(sampler2DArray(daxa_ImageTabletexture2DArray[daxa_id_to_index(image . id)], daxa_SamplerTable[daxa_id_to_index(sampler_id)]));} ivec4 texture(daxa_Image2DArrayi32 image, daxa_SamplerId sampler_id, vec3 uv){ return texture(isampler2DArray(daxa_ImageTableitexture2DArray[daxa_id_to_index(image . id)], daxa_SamplerTable[daxa_id_to_index(sampler_id)]), uv);} ivec4 textureLod(daxa_Image2DArrayi32 image, daxa_SamplerId sampler_id, vec3 uv, float bias){ return textureLod(isampler2DArray(daxa_ImageTableitexture2DArray[daxa_id_to_index(image . id)], daxa_SamplerTable[daxa_id_to_index(sampler_id)]), uv, bias);} ivec4 textureGrad(daxa_Image2DArrayi32 image, daxa_SamplerId sampler_id, vec3 uv, vec2 dTdx, vec2 dTdy){ return textureGrad(isampler2DArray(daxa_ImageTableitexture2DArray[daxa_id_to_index(image . id)], daxa_SamplerTable[daxa_id_to_index(sampler_id)]), uv, dTdx, dTdy);} ivec3 textureSize(daxa_Image2DArrayi32 image, daxa_SamplerId sampler_id, int lod){ return textureSize(isampler2DArray(daxa_ImageTableitexture2DArray[daxa_id_to_index(image . id)], daxa_SamplerTable[daxa_id_to_index(sampler_id)]), lod);} int textureQueryLevels(daxa_Image2DArrayi32 image, daxa_SamplerId sampler_id){ return textureQueryLevels(isampler2DArray(daxa_ImageTableitexture2DArray[daxa_id_to_index(image . id)], daxa_SamplerTable[daxa_id_to_index(sampler_id)]));} uvec4 texture(daxa_Image2DArrayu32 image, daxa_SamplerId sampler_id, vec3 uv){ return texture(usampler2DArray(daxa_ImageTableutexture2DArray[daxa_id_to_index(image . id)], daxa_SamplerTable[daxa_id_to_index(sampler_id)]), uv);} uvec4 textureLod(daxa_Image2DArrayu32 image, daxa_SamplerId sampler_id, vec3 uv, float bias){ return textureLod(usampler2DArray(daxa_ImageTableutexture2DArray[daxa_id_to_index(image . id)], daxa_SamplerTable[daxa_id_to_index(sampler_id)]), uv, bias);} uvec4 textureGrad(daxa_Image2DArrayu32 image, daxa_SamplerId sampler_id, vec3 uv, vec2 dTdx, vec2 dTdy){ return textureGrad(usampler2DArray(daxa_ImageTableutexture2DArray[daxa_id_to_index(image . id)], daxa_SamplerTable[daxa_id_to_index(sampler_id)]), uv, dTdx, dTdy);} ivec3 textureSize(daxa_Image2DArrayu32 image, daxa_SamplerId sampler_id, int lod){ return textureSize(usampler2DArray(daxa_ImageTableutexture2DArray[daxa_id_to_index(image . id)], daxa_SamplerTable[daxa_id_to_index(sampler_id)]), lod);} int textureQueryLevels(daxa_Image2DArrayu32 image, daxa_SamplerId sampler_id){ return textureQueryLevels(usampler2DArray(daxa_ImageTableutexture2DArray[daxa_id_to_index(image . id)], daxa_SamplerTable[daxa_id_to_index(sampler_id)]));}

                                                  vec4 texelFetch(daxa_Image1Df32 image, daxa_SamplerId sampler_id, int index, int sample_or_lod){ return texelFetch(sampler1D(daxa_ImageTabletexture1D[daxa_id_to_index(image . id)], daxa_SamplerTable[daxa_id_to_index(sampler_id)]), index, sample_or_lod);} ivec4 texelFetch(daxa_Image1Di32 image, daxa_SamplerId sampler_id, int index, int sample_or_lod){ return texelFetch(isampler1D(daxa_ImageTableitexture1D[daxa_id_to_index(image . id)], daxa_SamplerTable[daxa_id_to_index(sampler_id)]), index, sample_or_lod);} uvec4 texelFetch(daxa_Image1Du32 image, daxa_SamplerId sampler_id, int index, int sample_or_lod){ return texelFetch(usampler1D(daxa_ImageTableutexture1D[daxa_id_to_index(image . id)], daxa_SamplerTable[daxa_id_to_index(sampler_id)]), index, sample_or_lod);}
                                                  vec4 texelFetch(daxa_Image2Df32 image, daxa_SamplerId sampler_id, ivec2 index, int sample_or_lod){ return texelFetch(sampler2D(daxa_ImageTabletexture2D[daxa_id_to_index(image . id)], daxa_SamplerTable[daxa_id_to_index(sampler_id)]), index, sample_or_lod);} ivec4 texelFetch(daxa_Image2Di32 image, daxa_SamplerId sampler_id, ivec2 index, int sample_or_lod){ return texelFetch(isampler2D(daxa_ImageTableitexture2D[daxa_id_to_index(image . id)], daxa_SamplerTable[daxa_id_to_index(sampler_id)]), index, sample_or_lod);} uvec4 texelFetch(daxa_Image2Du32 image, daxa_SamplerId sampler_id, ivec2 index, int sample_or_lod){ return texelFetch(usampler2D(daxa_ImageTableutexture2D[daxa_id_to_index(image . id)], daxa_SamplerTable[daxa_id_to_index(sampler_id)]), index, sample_or_lod);}
                                                  vec4 texelFetch(daxa_Image3Df32 image, daxa_SamplerId sampler_id, ivec3 index, int sample_or_lod){ return texelFetch(sampler3D(daxa_ImageTabletexture3D[daxa_id_to_index(image . id)], daxa_SamplerTable[daxa_id_to_index(sampler_id)]), index, sample_or_lod);} ivec4 texelFetch(daxa_Image3Di32 image, daxa_SamplerId sampler_id, ivec3 index, int sample_or_lod){ return texelFetch(isampler3D(daxa_ImageTableitexture3D[daxa_id_to_index(image . id)], daxa_SamplerTable[daxa_id_to_index(sampler_id)]), index, sample_or_lod);} uvec4 texelFetch(daxa_Image3Du32 image, daxa_SamplerId sampler_id, ivec3 index, int sample_or_lod){ return texelFetch(usampler3D(daxa_ImageTableutexture3D[daxa_id_to_index(image . id)], daxa_SamplerTable[daxa_id_to_index(sampler_id)]), index, sample_or_lod);}
                                                       vec4 texelFetch(daxa_Image1DArrayf32 image, daxa_SamplerId sampler_id, ivec2 index, int sample_or_lod){ return texelFetch(sampler1DArray(daxa_ImageTabletexture1DArray[daxa_id_to_index(image . id)], daxa_SamplerTable[daxa_id_to_index(sampler_id)]), index, sample_or_lod);} ivec4 texelFetch(daxa_Image1DArrayi32 image, daxa_SamplerId sampler_id, ivec2 index, int sample_or_lod){ return texelFetch(isampler1DArray(daxa_ImageTableitexture1DArray[daxa_id_to_index(image . id)], daxa_SamplerTable[daxa_id_to_index(sampler_id)]), index, sample_or_lod);} uvec4 texelFetch(daxa_Image1DArrayu32 image, daxa_SamplerId sampler_id, ivec2 index, int sample_or_lod){ return texelFetch(usampler1DArray(daxa_ImageTableutexture1DArray[daxa_id_to_index(image . id)], daxa_SamplerTable[daxa_id_to_index(sampler_id)]), index, sample_or_lod);}
                                                       vec4 texelFetch(daxa_Image2DArrayf32 image, daxa_SamplerId sampler_id, ivec3 index, int sample_or_lod){ return texelFetch(sampler2DArray(daxa_ImageTabletexture2DArray[daxa_id_to_index(image . id)], daxa_SamplerTable[daxa_id_to_index(sampler_id)]), index, sample_or_lod);} ivec4 texelFetch(daxa_Image2DArrayi32 image, daxa_SamplerId sampler_id, ivec3 index, int sample_or_lod){ return texelFetch(isampler2DArray(daxa_ImageTableitexture2DArray[daxa_id_to_index(image . id)], daxa_SamplerTable[daxa_id_to_index(sampler_id)]), index, sample_or_lod);} uvec4 texelFetch(daxa_Image2DArrayu32 image, daxa_SamplerId sampler_id, ivec3 index, int sample_or_lod){ return texelFetch(usampler2DArray(daxa_ImageTableutexture2DArray[daxa_id_to_index(image . id)], daxa_SamplerTable[daxa_id_to_index(sampler_id)]), index, sample_or_lod);}

                                             vec4 textureGatherX(daxa_Image2Df32 image, daxa_SamplerId sampler_id, vec2 uv){ return textureGather(sampler2D(daxa_ImageTabletexture2D[daxa_id_to_index(image . id)], daxa_SamplerTable[daxa_id_to_index(sampler_id)]), uv, 0);} vec4 textureGatherY(daxa_Image2Df32 image, daxa_SamplerId sampler_id, vec2 uv){ return textureGather(sampler2D(daxa_ImageTabletexture2D[daxa_id_to_index(image . id)], daxa_SamplerTable[daxa_id_to_index(sampler_id)]), uv, 1);} vec4 textureGatherZ(daxa_Image2Df32 image, daxa_SamplerId sampler_id, vec2 uv){ return textureGather(sampler2D(daxa_ImageTabletexture2D[daxa_id_to_index(image . id)], daxa_SamplerTable[daxa_id_to_index(sampler_id)]), uv, 2);} vec4 textureGatherW(daxa_Image2Df32 image, daxa_SamplerId sampler_id, vec2 uv){ return textureGather(sampler2D(daxa_ImageTabletexture2D[daxa_id_to_index(image . id)], daxa_SamplerTable[daxa_id_to_index(sampler_id)]), uv, 3);} ivec4 textureGatherX(daxa_Image2Di32 image, daxa_SamplerId sampler_id, vec2 uv){ return textureGather(isampler2D(daxa_ImageTableitexture2D[daxa_id_to_index(image . id)], daxa_SamplerTable[daxa_id_to_index(sampler_id)]), uv, 0);} ivec4 textureGatherY(daxa_Image2Di32 image, daxa_SamplerId sampler_id, vec2 uv){ return textureGather(isampler2D(daxa_ImageTableitexture2D[daxa_id_to_index(image . id)], daxa_SamplerTable[daxa_id_to_index(sampler_id)]), uv, 1);} ivec4 textureGatherZ(daxa_Image2Di32 image, daxa_SamplerId sampler_id, vec2 uv){ return textureGather(isampler2D(daxa_ImageTableitexture2D[daxa_id_to_index(image . id)], daxa_SamplerTable[daxa_id_to_index(sampler_id)]), uv, 2);} ivec4 textureGatherW(daxa_Image2Di32 image, daxa_SamplerId sampler_id, vec2 uv){ return textureGather(isampler2D(daxa_ImageTableitexture2D[daxa_id_to_index(image . id)], daxa_SamplerTable[daxa_id_to_index(sampler_id)]), uv, 3);} uvec4 textureGatherX(daxa_Image2Du32 image, daxa_SamplerId sampler_id, vec2 uv){ return textureGather(usampler2D(daxa_ImageTableutexture2D[daxa_id_to_index(image . id)], daxa_SamplerTable[daxa_id_to_index(sampler_id)]), uv, 0);} uvec4 textureGatherY(daxa_Image2Du32 image, daxa_SamplerId sampler_id, vec2 uv){ return textureGather(usampler2D(daxa_ImageTableutexture2D[daxa_id_to_index(image . id)], daxa_SamplerTable[daxa_id_to_index(sampler_id)]), uv, 1);} uvec4 textureGatherZ(daxa_Image2Du32 image, daxa_SamplerId sampler_id, vec2 uv){ return textureGather(usampler2D(daxa_ImageTableutexture2D[daxa_id_to_index(image . id)], daxa_SamplerTable[daxa_id_to_index(sampler_id)]), uv, 2);} uvec4 textureGatherW(daxa_Image2Du32 image, daxa_SamplerId sampler_id, vec2 uv){ return textureGather(usampler2D(daxa_ImageTableutexture2D[daxa_id_to_index(image . id)], daxa_SamplerTable[daxa_id_to_index(sampler_id)]), uv, 3);}
                                               vec4 textureGatherX(daxa_ImageCubef32 image, daxa_SamplerId sampler_id, vec3 uv){ return textureGather(samplerCube(daxa_ImageTabletextureCube[daxa_id_to_index(image . id)], daxa_SamplerTable[daxa_id_to_index(sampler_id)]), uv, 0);} vec4 textureGatherY(daxa_ImageCubef32 image, daxa_SamplerId sampler_id, vec3 uv){ return textureGather(samplerCube(daxa_ImageTabletextureCube[daxa_id_to_index(image . id)], daxa_SamplerTable[daxa_id_to_index(sampler_id)]), uv, 1);} vec4 textureGatherZ(daxa_ImageCubef32 image, daxa_SamplerId sampler_id, vec3 uv){ return textureGather(samplerCube(daxa_ImageTabletextureCube[daxa_id_to_index(image . id)], daxa_SamplerTable[daxa_id_to_index(sampler_id)]), uv, 2);} vec4 textureGatherW(daxa_ImageCubef32 image, daxa_SamplerId sampler_id, vec3 uv){ return textureGather(samplerCube(daxa_ImageTabletextureCube[daxa_id_to_index(image . id)], daxa_SamplerTable[daxa_id_to_index(sampler_id)]), uv, 3);} ivec4 textureGatherX(daxa_ImageCubei32 image, daxa_SamplerId sampler_id, vec3 uv){ return textureGather(isamplerCube(daxa_ImageTableitextureCube[daxa_id_to_index(image . id)], daxa_SamplerTable[daxa_id_to_index(sampler_id)]), uv, 0);} ivec4 textureGatherY(daxa_ImageCubei32 image, daxa_SamplerId sampler_id, vec3 uv){ return textureGather(isamplerCube(daxa_ImageTableitextureCube[daxa_id_to_index(image . id)], daxa_SamplerTable[daxa_id_to_index(sampler_id)]), uv, 1);} ivec4 textureGatherZ(daxa_ImageCubei32 image, daxa_SamplerId sampler_id, vec3 uv){ return textureGather(isamplerCube(daxa_ImageTableitextureCube[daxa_id_to_index(image . id)], daxa_SamplerTable[daxa_id_to_index(sampler_id)]), uv, 2);} ivec4 textureGatherW(daxa_ImageCubei32 image, daxa_SamplerId sampler_id, vec3 uv){ return textureGather(isamplerCube(daxa_ImageTableitextureCube[daxa_id_to_index(image . id)], daxa_SamplerTable[daxa_id_to_index(sampler_id)]), uv, 3);} uvec4 textureGatherX(daxa_ImageCubeu32 image, daxa_SamplerId sampler_id, vec3 uv){ return textureGather(usamplerCube(daxa_ImageTableutextureCube[daxa_id_to_index(image . id)], daxa_SamplerTable[daxa_id_to_index(sampler_id)]), uv, 0);} uvec4 textureGatherY(daxa_ImageCubeu32 image, daxa_SamplerId sampler_id, vec3 uv){ return textureGather(usamplerCube(daxa_ImageTableutextureCube[daxa_id_to_index(image . id)], daxa_SamplerTable[daxa_id_to_index(sampler_id)]), uv, 1);} uvec4 textureGatherZ(daxa_ImageCubeu32 image, daxa_SamplerId sampler_id, vec3 uv){ return textureGather(usamplerCube(daxa_ImageTableutextureCube[daxa_id_to_index(image . id)], daxa_SamplerTable[daxa_id_to_index(sampler_id)]), uv, 2);} uvec4 textureGatherW(daxa_ImageCubeu32 image, daxa_SamplerId sampler_id, vec3 uv){ return textureGather(usamplerCube(daxa_ImageTableutextureCube[daxa_id_to_index(image . id)], daxa_SamplerTable[daxa_id_to_index(sampler_id)]), uv, 3);}
                                                    vec4 textureGatherX(daxa_ImageCubeArrayf32 image, daxa_SamplerId sampler_id, vec4 uv){ return textureGather(samplerCubeArray(daxa_ImageTabletextureCubeArray[daxa_id_to_index(image . id)], daxa_SamplerTable[daxa_id_to_index(sampler_id)]), uv, 0);} vec4 textureGatherY(daxa_ImageCubeArrayf32 image, daxa_SamplerId sampler_id, vec4 uv){ return textureGather(samplerCubeArray(daxa_ImageTabletextureCubeArray[daxa_id_to_index(image . id)], daxa_SamplerTable[daxa_id_to_index(sampler_id)]), uv, 1);} vec4 textureGatherZ(daxa_ImageCubeArrayf32 image, daxa_SamplerId sampler_id, vec4 uv){ return textureGather(samplerCubeArray(daxa_ImageTabletextureCubeArray[daxa_id_to_index(image . id)], daxa_SamplerTable[daxa_id_to_index(sampler_id)]), uv, 2);} vec4 textureGatherW(daxa_ImageCubeArrayf32 image, daxa_SamplerId sampler_id, vec4 uv){ return textureGather(samplerCubeArray(daxa_ImageTabletextureCubeArray[daxa_id_to_index(image . id)], daxa_SamplerTable[daxa_id_to_index(sampler_id)]), uv, 3);} ivec4 textureGatherX(daxa_ImageCubeArrayi32 image, daxa_SamplerId sampler_id, vec4 uv){ return textureGather(isamplerCubeArray(daxa_ImageTableitextureCubeArray[daxa_id_to_index(image . id)], daxa_SamplerTable[daxa_id_to_index(sampler_id)]), uv, 0);} ivec4 textureGatherY(daxa_ImageCubeArrayi32 image, daxa_SamplerId sampler_id, vec4 uv){ return textureGather(isamplerCubeArray(daxa_ImageTableitextureCubeArray[daxa_id_to_index(image . id)], daxa_SamplerTable[daxa_id_to_index(sampler_id)]), uv, 1);} ivec4 textureGatherZ(daxa_ImageCubeArrayi32 image, daxa_SamplerId sampler_id, vec4 uv){ return textureGather(isamplerCubeArray(daxa_ImageTableitextureCubeArray[daxa_id_to_index(image . id)], daxa_SamplerTable[daxa_id_to_index(sampler_id)]), uv, 2);} ivec4 textureGatherW(daxa_ImageCubeArrayi32 image, daxa_SamplerId sampler_id, vec4 uv){ return textureGather(isamplerCubeArray(daxa_ImageTableitextureCubeArray[daxa_id_to_index(image . id)], daxa_SamplerTable[daxa_id_to_index(sampler_id)]), uv, 3);} uvec4 textureGatherX(daxa_ImageCubeArrayu32 image, daxa_SamplerId sampler_id, vec4 uv){ return textureGather(usamplerCubeArray(daxa_ImageTableutextureCubeArray[daxa_id_to_index(image . id)], daxa_SamplerTable[daxa_id_to_index(sampler_id)]), uv, 0);} uvec4 textureGatherY(daxa_ImageCubeArrayu32 image, daxa_SamplerId sampler_id, vec4 uv){ return textureGather(usamplerCubeArray(daxa_ImageTableutextureCubeArray[daxa_id_to_index(image . id)], daxa_SamplerTable[daxa_id_to_index(sampler_id)]), uv, 1);} uvec4 textureGatherZ(daxa_ImageCubeArrayu32 image, daxa_SamplerId sampler_id, vec4 uv){ return textureGather(usamplerCubeArray(daxa_ImageTableutextureCubeArray[daxa_id_to_index(image . id)], daxa_SamplerTable[daxa_id_to_index(sampler_id)]), uv, 2);} uvec4 textureGatherW(daxa_ImageCubeArrayu32 image, daxa_SamplerId sampler_id, vec4 uv){ return textureGather(usamplerCubeArray(daxa_ImageTableutextureCubeArray[daxa_id_to_index(image . id)], daxa_SamplerTable[daxa_id_to_index(sampler_id)]), uv, 3);}
                                                  vec4 textureGatherX(daxa_Image2DArrayf32 image, daxa_SamplerId sampler_id, vec3 uv){ return textureGather(sampler2DArray(daxa_ImageTabletexture2DArray[daxa_id_to_index(image . id)], daxa_SamplerTable[daxa_id_to_index(sampler_id)]), uv, 0);} vec4 textureGatherY(daxa_Image2DArrayf32 image, daxa_SamplerId sampler_id, vec3 uv){ return textureGather(sampler2DArray(daxa_ImageTabletexture2DArray[daxa_id_to_index(image . id)], daxa_SamplerTable[daxa_id_to_index(sampler_id)]), uv, 1);} vec4 textureGatherZ(daxa_Image2DArrayf32 image, daxa_SamplerId sampler_id, vec3 uv){ return textureGather(sampler2DArray(daxa_ImageTabletexture2DArray[daxa_id_to_index(image . id)], daxa_SamplerTable[daxa_id_to_index(sampler_id)]), uv, 2);} vec4 textureGatherW(daxa_Image2DArrayf32 image, daxa_SamplerId sampler_id, vec3 uv){ return textureGather(sampler2DArray(daxa_ImageTabletexture2DArray[daxa_id_to_index(image . id)], daxa_SamplerTable[daxa_id_to_index(sampler_id)]), uv, 3);} ivec4 textureGatherX(daxa_Image2DArrayi32 image, daxa_SamplerId sampler_id, vec3 uv){ return textureGather(isampler2DArray(daxa_ImageTableitexture2DArray[daxa_id_to_index(image . id)], daxa_SamplerTable[daxa_id_to_index(sampler_id)]), uv, 0);} ivec4 textureGatherY(daxa_Image2DArrayi32 image, daxa_SamplerId sampler_id, vec3 uv){ return textureGather(isampler2DArray(daxa_ImageTableitexture2DArray[daxa_id_to_index(image . id)], daxa_SamplerTable[daxa_id_to_index(sampler_id)]), uv, 1);} ivec4 textureGatherZ(daxa_Image2DArrayi32 image, daxa_SamplerId sampler_id, vec3 uv){ return textureGather(isampler2DArray(daxa_ImageTableitexture2DArray[daxa_id_to_index(image . id)], daxa_SamplerTable[daxa_id_to_index(sampler_id)]), uv, 2);} ivec4 textureGatherW(daxa_Image2DArrayi32 image, daxa_SamplerId sampler_id, vec3 uv){ return textureGather(isampler2DArray(daxa_ImageTableitexture2DArray[daxa_id_to_index(image . id)], daxa_SamplerTable[daxa_id_to_index(sampler_id)]), uv, 3);} uvec4 textureGatherX(daxa_Image2DArrayu32 image, daxa_SamplerId sampler_id, vec3 uv){ return textureGather(usampler2DArray(daxa_ImageTableutexture2DArray[daxa_id_to_index(image . id)], daxa_SamplerTable[daxa_id_to_index(sampler_id)]), uv, 0);} uvec4 textureGatherY(daxa_Image2DArrayu32 image, daxa_SamplerId sampler_id, vec3 uv){ return textureGather(usampler2DArray(daxa_ImageTableutexture2DArray[daxa_id_to_index(image . id)], daxa_SamplerTable[daxa_id_to_index(sampler_id)]), uv, 1);} uvec4 textureGatherZ(daxa_Image2DArrayu32 image, daxa_SamplerId sampler_id, vec3 uv){ return textureGather(usampler2DArray(daxa_ImageTableutexture2DArray[daxa_id_to_index(image . id)], daxa_SamplerTable[daxa_id_to_index(sampler_id)]), uv, 2);} uvec4 textureGatherW(daxa_Image2DArrayu32 image, daxa_SamplerId sampler_id, vec3 uv){ return textureGather(usampler2DArray(daxa_ImageTableutexture2DArray[daxa_id_to_index(image . id)], daxa_SamplerTable[daxa_id_to_index(sampler_id)]), uv, 3);}











































































#line 14 "include/daxa/daxa.inl"













































































#line 4 "tests/2_daxa_api/6_task_list/shaders/shared.inl"











struct MipmappingGpuInput { vec3 paint_col;float mouse_x;float mouse_y;float p_mouse_x;float p_mouse_y;float paint_radius;};layout(buffer_reference, scalar, buffer_reference_align = 4)buffer daxa_RWBufferMipmappingGpuInput { vec3 paint_col;float mouse_x;float mouse_y;float p_mouse_x;float p_mouse_y;float paint_radius;};layout(buffer_reference, scalar, buffer_reference_align = 4)readonly buffer daxa_BufferMipmappingGpuInput { vec3 paint_col;float mouse_x;float mouse_y;float p_mouse_x;float p_mouse_y;float paint_radius;};daxa_RWBufferMipmappingGpuInput daxa_id_to_rwbuffer(daxa_BufferId buffer_id){ return daxa_RWBufferMipmappingGpuInput(daxa_buffer_device_address_buffer . addresses[daxa_id_to_index(buffer_id)]);} daxa_BufferMipmappingGpuInput daxa_id_to_buffer(daxa_BufferId buffer_id){ return daxa_BufferMipmappingGpuInput(daxa_buffer_device_address_buffer . addresses[daxa_id_to_index(buffer_id)]);}

struct MipmappingComputePushConstant
{
                       daxa_RWImage2Df32 image;
                                    daxa_RWBufferMipmappingGpuInput gpu_input;
               uvec2 frame_dim;
};



#line 7 "mipmapping.glsl"

                                                    layout(push_constant, scalar)uniform _DAXA_PUSH_CONSTANT { MipmappingComputePushConstant daxa_push_constant;};

  float segment_distance(vec2 p, vec2 a, vec2 b)
{
          vec2 ba = b - a;
          vec2 pa = p - a;
      float h = clamp(dot(pa, ba)/ dot(ba, ba), 0, 1);
    return length(pa - h * ba);
}



layout(local_size_x = 8, local_size_y = 8, local_size_z = 1)in;
void main()
{
          uvec3 pixel_i = gl_GlobalInvocationID . xyz;
    if(pixel_i . x >= daxa_push_constant . frame_dim . x || pixel_i . y >= daxa_push_constant . frame_dim . y)
        return;

          vec2 render_size = daxa_push_constant . frame_dim;
          vec2 inv_render_size = vec2(1.0)/ render_size;
          vec2 pixel_pos = pixel_i . xy;
          vec2 mouse_pos = vec2(daxa_push_constant . gpu_input . mouse_x, daxa_push_constant . gpu_input . mouse_y);
          vec2 prev_mouse_pos = vec2(daxa_push_constant . gpu_input . p_mouse_x, daxa_push_constant . gpu_input . p_mouse_y);

          vec2 uv = pixel_pos * inv_render_size;

      float dist = segment_distance(pixel_pos, prev_mouse_pos, mouse_pos);

    if(dist < daxa_push_constant . gpu_input . paint_radius)
    {
              vec3 col = daxa_push_constant . gpu_input . paint_col;


        imageStore(daxa_push_constant . image, ivec2(pixel_i . xy), vec4(col, 1));



    }
}
