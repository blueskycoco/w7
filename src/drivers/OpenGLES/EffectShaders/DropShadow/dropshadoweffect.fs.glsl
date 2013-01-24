//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this sample source code is subject to the terms of the Microsoft
// license agreement under which you licensed this sample source code. If
// you did not accept the terms of the license agreement, you are not
// authorized to use this sample source code. For the terms of the license,
// please see the license agreement between you and Microsoft or, if applicable,
// see the LICENSE.RTF on your install media or the root of your tools installation.
// THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES OR INDEMNITIES.
//


precision highp float;

uniform   sampler2D   texture;
uniform   bool        horizontalPass;
uniform   bool        verticalPass;

uniform   bool        downSample;
uniform   float       numSamples;
uniform   float       sampleSize;

varying   vec2        textureCoordinate;
varying   vec4        diffuse;

// Convolution kernel for Gaussian Blur, up to 7x1
const     int         MAX_FILTER_SIZE   = 7;
uniform   float       filterOffsets[MAX_FILTER_SIZE];
uniform   float       filterWeights[MAX_FILTER_SIZE];


//
// Convolution (Gaussian) blur.
// Smoother but more expensive.
//
vec4 ConvolutionBlur( sampler2D texture, vec2 texcoord, float filterOffsets[MAX_FILTER_SIZE], float filterWeights[MAX_FILTER_SIZE], bool horizontalPass, bool verticalPass )
{
    vec4 pixel = vec4(0.0, 0.0, 0.0, 0.0);

    for (int index = 0; index < MAX_FILTER_SIZE; ++index)
    {
        vec2 coord;
        if (horizontalPass)
        {
            coord  = vec2(texcoord.x + filterOffsets[index], texcoord.y);
        }
        else
        {
            coord  = vec2(texcoord.x, texcoord.y + filterOffsets[index]);
        }
        
        vec4 temp   = texture2D( texture, coord );
        pixel      += (temp * filterWeights[index]);
    }
    
    return pixel;
}

/*
vec2 downsampleCoordinate( vec2 coordinate, float numSamples, float sampleSize )
{
    vec2 downsampledCoordinate;

    // Which sample bucket does this pixel fall in?
    float xbucket = floor(textureCoordinate.x  / sampleSize);
    float ybucket = floor(textureCoordinate.y  / sampleSize);
       
    downsampledCoordinate.x = xbucket / numSamples;
    downsampledCoordinate.y = ybucket / numSamples;
    
    return downsampledCoordinate;
}
*/

void main(void)
{
    gl_FragColor.rgb = diffuse.rgb;
    gl_FragColor.a   = diffuse.a * ConvolutionBlur( texture, textureCoordinate, filterOffsets, filterWeights, horizontalPass, verticalPass  ).a;
}
