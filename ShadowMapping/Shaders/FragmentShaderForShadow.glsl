#version 430

#define MAX_LIGHTS 32

struct LightInfo
{
   int LightSwitch;
   vec4 Position;
   vec4 AmbientColor;
   vec4 DiffuseColor;
   vec4 SpecularColor;
   vec3 SpotlightDirection;
   float SpotlightExponent;
   float SpotlightCutoffAngle;
   vec3 AttenuationFactors;
};
uniform LightInfo Lights[MAX_LIGHTS];

struct MateralInfo {
   vec4 EmissionColor;
   vec4 AmbientColor;
   vec4 DiffuseColor;
   vec4 SpecularColor;
   float SpecularExponent;
};
uniform MateralInfo Material;

uniform sampler2D BaseTexture;
uniform sampler2DShadow DepthMap;

uniform int UseLight;
uniform int LightIndex;
uniform int LightNum;
uniform vec4 GlobalAmbient;

uniform mat4 WorldMatrix;
uniform mat4 ViewMatrix;
uniform mat4 ProjectionMatrix;

in vec3 position_in_ec;
in vec3 normal_in_ec;
in vec2 tex_coord;

in vec4 depth_map_coord;

layout (location = 0) out vec4 final_color;

const float zero = 0.0f;
const float one = 1.0f;

bool IsPointLight(in vec4 light_position)
{
   return light_position.w != zero;
}

float getAttenuation(in vec3 light_vector, in int light_index)
{
   vec3 distance_scale;
   distance_scale.x = one;
   distance_scale.z = dot( light_vector, light_vector );
   distance_scale.y = sqrt( distance_scale.z );
   return one / dot( distance_scale, Lights[light_index].AttenuationFactors );
}

float getSpotlightFactor(in vec3 normalized_light_vector, in int light_index)
{
   if (Lights[light_index].SpotlightCutoffAngle >= 180.0f) return one;

   vec4 direction_in_ec = 
      transpose( inverse( ViewMatrix * WorldMatrix ) ) * 
      vec4(Lights[light_index].SpotlightDirection, 1.0f);
   vec3 normalized_direction = normalize( direction_in_ec.xyz );
   float cutoff_angle = clamp( Lights[light_index].SpotlightCutoffAngle, zero, 90.0f );
   float factor = dot( -normalized_light_vector, normalized_direction );
   return factor >= cos( radians( cutoff_angle ) ) ? pow( factor, Lights[light_index].SpotlightExponent ) : zero;
}

vec4 calculateLightingEquation()
{
   vec4 color = Material.EmissionColor + GlobalAmbient * Material.AmbientColor;

   if (Lights[LightIndex].LightSwitch == 0) return color;
      
   vec4 light_position_in_ec = ViewMatrix * Lights[LightIndex].Position;
      
   float final_effect_factor = one;
   vec3 light_vector = light_position_in_ec.xyz - position_in_ec;
   if (IsPointLight( light_position_in_ec )) {
      float attenuation = getAttenuation( light_vector, LightIndex );

      light_vector = normalize( light_vector );
      float spotlight_factor = getSpotlightFactor( light_vector, LightIndex );
      final_effect_factor = attenuation * spotlight_factor;
   }
   else light_vector = normalize( light_position_in_ec.xyz );
   
   if (final_effect_factor <= zero) return color;

   vec4 local_color = Lights[LightIndex].AmbientColor * Material.AmbientColor;

   float diffuse_intensity = max( dot( normal_in_ec, light_vector ), zero );
   local_color += diffuse_intensity * Lights[LightIndex].DiffuseColor * Material.DiffuseColor;

   vec3 halfway_vector = normalize( light_vector - normalize( position_in_ec ) );
   float specular_intensity = max( dot( normal_in_ec, halfway_vector ), zero );
   local_color += 
      pow( specular_intensity, Material.SpecularExponent ) * 
      Lights[LightIndex].SpecularColor * Material.SpecularColor;

   color += local_color;
   return color;
}

bool isShadow()
{
   if (zero <= depth_map_coord.x && depth_map_coord.x <= depth_map_coord.w &&
       zero <= depth_map_coord.y && depth_map_coord.y <= depth_map_coord.w &&
       zero < depth_map_coord.w) {
      return textureProj( DepthMap, depth_map_coord ) <= depth_map_coord.z;
   }
   return false;
}

void main()
{ 
   final_color = isShadow() ? vec4(zero, zero, zero, one) : texture( BaseTexture, tex_coord );

   if (UseLight != 0) {
      final_color *= calculateLightingEquation();
   }
   else final_color *= Material.DiffuseColor;

   float x = textureProj( DepthMap, vec4(0.5,0.5,0.5,1.0) );
   final_color = vec4(x, x, x, one);
}