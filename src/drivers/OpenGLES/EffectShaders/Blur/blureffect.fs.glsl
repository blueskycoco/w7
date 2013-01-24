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

varying   vec2        textureCoordinate;
varying   vec4        diffuse;

const     float       offset = 0.005;

// Convolution kernel for Gaussian Blur, up to 7x1
const     int         MAX_FILTER_SIZE   = 7;
uniform   float       filterOffsets[MAX_FILTER_SIZE];
uniform   float       filterWeights[MAX_FILTER_SIZE];


//
// Cheap, fast Neighborhood blur.  
// Does not look as smooth as a Convolution (Gaussian) blur.
//
vec4 SimpleBlur( sampler2D texture, vec2 texcoord, float offset )
{
    vec4 sampleN = vec4(0.0);
    vec4 sampleS = vec4(0.0);
    vec4 sampleE = vec4(0.0);
    vec4 sampleW = vec4(0.0);

    sampleN = texture2D( texture, vec2(texcoord.x,           texcoord.y - offset) );
    sampleS = texture2D( texture, vec2(texcoord.x,           texcoord.y + offset) );
    sampleE = texture2D( texture, vec2(texcoord.x - offset,  texcoord.y) );
    sampleW = texture2D( texture, vec2(texcoord.x + offset,  texcoord.y) );
    
    return (sampleN + sampleS + sampleE + sampleW)*0.25;
}


//
// Convolution (Gaussian) blur.
// Smoother but more expensive.
//
vec4 ConvolutionBlur( sampler2D texture, vec2 texcoord, float filterOffsets[MAX_FILTER_SIZE], float filterWeights[MAX_FILTER_SIZE], bool horizontalPass )
{
    vec4 pixel = vec4(0.0, 0.0, 0.0, 0.0);
    vec2 coord;

    for (int index = 0; index < MAX_FILTER_SIZE; ++index)
    {
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


void main(void)
{
    gl_FragColor  = diffuse * ConvolutionBlur( texture, textureCoordinate, filterOffsets, filterWeights, horizontalPass );
}
